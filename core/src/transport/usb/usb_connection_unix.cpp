#include <libusb-1.0/libusb.h>
#include <cstring>
#include <algorithm>
#include <spdlog/spdlog.h>

#include "tomtom/core/defines.hpp"
#include "tomtom/core/transport/usb/usb_connection.hpp"
#include "tomtom/core/transport/usb/usb_connection_impl.hpp"

namespace tomtom::core::transport
{
    // libusb context - shared across all USB connections
    static libusb_context *g_usb_context = nullptr;
    static int g_context_ref_count = 0;

    // Helper to manage libusb context lifecycle
    static bool ensure_libusb_context()
    {
        if (g_context_ref_count == 0)
        {
            int result = libusb_init(&g_usb_context);
            if (result != LIBUSB_SUCCESS)
            {
                spdlog::error("Failed to initialize libusb: {}", libusb_error_name(result));
                return false;
            }
        }
        g_context_ref_count++;
        return true;
    }

    static void release_libusb_context()
    {
        if (g_context_ref_count > 0)
        {
            g_context_ref_count--;
            if (g_context_ref_count == 0 && g_usb_context != nullptr)
            {
                libusb_exit(g_usb_context);
                g_usb_context = nullptr;
            }
        }
    }

    USBDeviceConnection::Impl::Impl(const DeviceInfo &info)
        : device_info_(info),
          is_open_(false)
    {
        ensure_libusb_context();
    }

    USBDeviceConnection::Impl::~Impl()
    {
        close();
        release_libusb_context();
    }

    bool USBDeviceConnection::Impl::open()
    {
        if (is_open_)
        {
            return true;
        }

        if (!g_usb_context)
        {
            spdlog::error("libusb context not initialized");
            return false;
        }

        // Get USB device details from variant
        const auto &usb_details = std::get<USBDeviceDetails>(device_info_.details);

        // Enumerate devices to find our target
        libusb_device **device_list = nullptr;
        ssize_t device_count = libusb_get_device_list(g_usb_context, &device_list);

        if (device_count < 0)
        {
            spdlog::error("Failed to get device list: {}", libusb_error_name(device_count));
            return false;
        }

        libusb_device *target_device = nullptr;
        libusb_device_handle *handle = nullptr;

        // Find device matching our path (bus:address format)
        for (ssize_t i = 0; i < device_count; i++)
        {
            libusb_device *dev = device_list[i];

            uint8_t bus = libusb_get_bus_number(dev);
            uint8_t addr = libusb_get_device_address(dev);

            char path[32];
            snprintf(path, sizeof(path), "%03d:%03d", bus, addr);

            if (usb_details.device_path == path)
            {
                target_device = dev;
                break;
            }
        }

        if (!target_device)
        {
            spdlog::error("Device not found: {}", usb_details.device_path);
            libusb_free_device_list(device_list, 1);
            return false;
        }

        // Open the device
        int result = libusb_open(target_device, &handle);
        if (result != LIBUSB_SUCCESS)
        {
            spdlog::error("Failed to open device: {}", libusb_error_name(result));
            libusb_free_device_list(device_list, 1);
            return false;
        }

        // Detach kernel driver if active
        bool kernel_driver_detached = false;
        if (libusb_kernel_driver_active(handle, 0) == 1)
        {
            result = libusb_detach_kernel_driver(handle, 0);
            if (result == LIBUSB_SUCCESS)
            {
                kernel_driver_detached = true;
                spdlog::debug("Kernel driver detached");
            }
            else
            {
                spdlog::warn("Failed to detach kernel driver: {}", libusb_error_name(result));
            }
        }

        // Claim interface 0
        int attempts = 0;
        while (attempts < 60)
        {
            result = libusb_claim_interface(handle, 0);
            if (result == LIBUSB_SUCCESS)
            {
                break;
            }
            else if (result == LIBUSB_ERROR_BUSY)
            {
                attempts++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else
            {
                spdlog::error("Failed to claim interface: {}", libusb_error_name(result));
                if (kernel_driver_detached)
                {
                    libusb_attach_kernel_driver(handle, 0);
                }
                libusb_close(handle);
                libusb_free_device_list(device_list, 1);
                return false;
            }
        }

        if (result != LIBUSB_SUCCESS)
        {
            spdlog::error("Failed to claim interface after {} attempts", attempts);
            if (kernel_driver_detached)
            {
                libusb_attach_kernel_driver(handle, 0);
            }
            libusb_close(handle);
            libusb_free_device_list(device_list, 1);
            return false;
        }

        // Store handle
        device_handle_ = handle;
        attach_kernel_driver_ = kernel_driver_detached;

        // Get device descriptor for endpoint information
        libusb_device_descriptor desc;
        libusb_get_device_descriptor(target_device, &desc);

        // Get configuration descriptor
        libusb_config_descriptor *config = nullptr;
        result = libusb_get_active_config_descriptor(target_device, &config);
        if (result == LIBUSB_SUCCESS && config != nullptr)
        {
            // Find HID interface and endpoints
            for (int i = 0; i < config->bNumInterfaces; i++)
            {
                const libusb_interface *iface = &config->interface[i];
                if (iface->num_altsetting > 0)
                {
                    const libusb_interface_descriptor *iface_desc = &iface->altsetting[0];

                    // Look for HID interface (class 3)
                    if (iface_desc->bInterfaceClass == LIBUSB_CLASS_HID)
                    {
                        for (int j = 0; j < iface_desc->bNumEndpoints; j++)
                        {
                            const libusb_endpoint_descriptor *ep = &iface_desc->endpoint[j];

                            if ((ep->bEndpointAddress & LIBUSB_ENDPOINT_IN) != 0)
                            {
                                input_endpoint_ = ep->bEndpointAddress;
                                input_report_len_ = ep->wMaxPacketSize;
                                spdlog::debug("Input endpoint: 0x{:02X}, max packet size: {}",
                                              input_endpoint_, input_report_len_);
                            }
                            else
                            {
                                output_endpoint_ = ep->bEndpointAddress;
                                output_report_len_ = ep->wMaxPacketSize;
                                spdlog::debug("Output endpoint: 0x{:02X}, max packet size: {}",
                                              output_endpoint_, output_report_len_);
                            }
                        }
                    }
                }
            }
            libusb_free_config_descriptor(config);
        }

        libusb_free_device_list(device_list, 1);
        is_open_ = true;
        return true;
    }

    void USBDeviceConnection::Impl::close()
    {
        if (!is_open_)
            return;

        if (device_handle_ != nullptr)
        {
            libusb_device_handle *handle = static_cast<libusb_device_handle *>(device_handle_);

            libusb_release_interface(handle, 0);

            if (attach_kernel_driver_)
            {
                libusb_attach_kernel_driver(handle, 0);
            }

            libusb_close(handle);
            device_handle_ = nullptr;
        }

        cached_report_.clear();
        cached_offset_ = 0;
        is_open_ = false;
    }

    bool USBDeviceConnection::Impl::isOpen() const
    {
        return is_open_;
    }

    int USBDeviceConnection::Impl::read(uint8_t *buffer, size_t size, int timeout_ms)
    {
        if (!is_open_ || device_handle_ == nullptr || buffer == nullptr || size == 0)
        {
            cached_report_.clear();
            return -1;
        }

        libusb_device_handle *handle = static_cast<libusb_device_handle *>(device_handle_);
        size_t bytesCopied = 0;

        while (bytesCopied < size)
        {
            // If cache is empty or fully consumed, read new data
            if (cached_offset_ >= cached_report_.size())
            {
                cached_report_.assign(input_report_len_, 0);
                cached_offset_ = 0;

                int transferred = 0;
                int result = libusb_interrupt_transfer(
                    handle,
                    input_endpoint_,
                    cached_report_.data(),
                    input_report_len_,
                    &transferred,
                    timeout_ms);

                if (result != LIBUSB_SUCCESS && result != LIBUSB_ERROR_TIMEOUT)
                {
                    spdlog::error("USB read failed: {}", libusb_error_name(result));
                    return bytesCopied > 0 ? static_cast<int>(bytesCopied) : -1;
                }

                if (transferred == 0)
                {
                    break;
                }

                cached_report_.resize(transferred);
            }

            // Copy from cached report into caller buffer
            size_t remainingInCache = cached_report_.size() - cached_offset_;
            size_t remainingRequested = size - bytesCopied;
            size_t toCopy = std::min(remainingInCache, remainingRequested);

            std::memcpy(
                buffer + bytesCopied,
                cached_report_.data() + cached_offset_,
                toCopy);

            cached_offset_ += toCopy;
            bytesCopied += toCopy;
        }

        return static_cast<int>(bytesCopied);
    }

    int USBDeviceConnection::Impl::write(const uint8_t *buffer, size_t size, int timeout_ms)
    {
        if (!is_open_ || device_handle_ == nullptr || buffer == nullptr || size == 0)
        {
            return -1;
        }

        libusb_device_handle *handle = static_cast<libusb_device_handle *>(device_handle_);

        // Prepare output buffer
        std::vector<uint8_t> outputBuffer(output_report_len_, 0);
        size_t dataToCopy = std::min(size, static_cast<size_t>(output_report_len_));
        std::memcpy(outputBuffer.data(), buffer, dataToCopy);

        int transferred = 0;
        int result = libusb_interrupt_transfer(
            handle,
            output_endpoint_,
            outputBuffer.data(),
            output_report_len_,
            &transferred,
            timeout_ms);

        if (result != LIBUSB_SUCCESS)
        {
            spdlog::error("USB write failed: {}", libusb_error_name(result));
            return -1;
        }

        return static_cast<int>(dataToCopy);
    }

    const DeviceInfo &USBDeviceConnection::Impl::deviceInfo() const
    {
        return device_info_;
    }

    std::vector<DeviceInfo> USBDeviceConnection::Impl::enumerate()
    {
        std::vector<DeviceInfo> devices;

        if (!ensure_libusb_context())
        {
            return devices;
        }

        libusb_device **device_list = nullptr;
        ssize_t device_count = libusb_get_device_list(g_usb_context, &device_list);

        if (device_count < 0)
        {
            spdlog::error("Failed to get device list: {}", libusb_error_name(device_count));
            release_libusb_context();
            return devices;
        }

        for (ssize_t i = 0; i < device_count; i++)
        {
            libusb_device *dev = device_list[i];
            libusb_device_descriptor desc;

            if (libusb_get_device_descriptor(dev, &desc) != LIBUSB_SUCCESS)
            {
                continue;
            }

            // Check if this is a TomTom device
            if (desc.idVendor != static_cast<uint16_t>(VendorID::TOMTOM))
            {
                continue;
            }

            // Open device to read strings
            libusb_device_handle *handle = nullptr;
            if (libusb_open(dev, &handle) != LIBUSB_SUCCESS)
            {
                continue;
            }

            DeviceInfo info;
            info.vendor_id = static_cast<VendorID>(desc.idVendor);
            info.product_id = static_cast<ProductID>(desc.idProduct);
            info.transport = TransportType::USB;

            // Get device path (bus:address)
            uint8_t bus = libusb_get_bus_number(dev);
            uint8_t addr = libusb_get_device_address(dev);

            USBDeviceDetails usb_details;
            char path[32];
            snprintf(path, sizeof(path), "%03d:%03d", bus, addr);
            usb_details.device_path = path;
            info.details = usb_details;

            // Get string descriptors
            unsigned char buffer[256];

            if (desc.iManufacturer > 0)
            {
                int len = libusb_get_string_descriptor_ascii(
                    handle, desc.iManufacturer, buffer, sizeof(buffer));
                if (len > 0)
                {
                    info.manufacturer = std::string(reinterpret_cast<char *>(buffer), len);
                }
            }

            if (desc.iProduct > 0)
            {
                int len = libusb_get_string_descriptor_ascii(
                    handle, desc.iProduct, buffer, sizeof(buffer));
                if (len > 0)
                {
                    info.product_name = std::string(reinterpret_cast<char *>(buffer), len);
                }
            }

            if (desc.iSerialNumber > 0)
            {
                int len = libusb_get_string_descriptor_ascii(
                    handle, desc.iSerialNumber, buffer, sizeof(buffer));
                if (len > 0)
                {
                    info.serial_number = std::string(reinterpret_cast<char *>(buffer), len);
                }
            }

            libusb_close(handle);

            devices.push_back(info);
            spdlog::debug("Found TomTom device: VID=0x{:04X}, PID=0x{:04X}, Path={}",
                          static_cast<uint16_t>(info.vendor_id),
                          static_cast<uint16_t>(info.product_id),
                          usb_details.device_path);
        }

        libusb_free_device_list(device_list, 1);
        release_libusb_context();

        return devices;
    }
}