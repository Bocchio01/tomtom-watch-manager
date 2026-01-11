#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <CoreFoundation/CoreFoundation.h>
#include <cstring>
#include <algorithm>
#include <spdlog/spdlog.h>

#include "tomtom/core/defines.hpp"
#include "tomtom/core/transport/usb/usb_connection.hpp"
#include "tomtom/core/transport/usb/usb_connection_impl.hpp"

namespace tomtom::core::transport
{
    // Helper functions for CoreFoundation string conversions
    static std::string cf_string_to_std_string(CFStringRef cf_str)
    {
        if (!cf_str)
            return "";

        CFIndex length = CFStringGetLength(cf_str);
        CFIndex max_size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
        std::vector<char> buffer(max_size);

        if (CFStringGetCString(cf_str, buffer.data(), max_size, kCFStringEncodingUTF8))
        {
            return std::string(buffer.data());
        }

        return "";
    }

    static int32_t get_int_property(IOHIDDeviceRef device, CFStringRef key)
    {
        CFTypeRef ref = IOHIDDeviceGetProperty(device, key);
        if (!ref || CFGetTypeID(ref) != CFNumberGetTypeID())
            return 0;

        int32_t value = 0;
        CFNumberGetValue(static_cast<CFNumberRef>(ref), kCFNumberSInt32Type, &value);
        return value;
    }

    static std::string get_string_property(IOHIDDeviceRef device, CFStringRef key)
    {
        CFTypeRef ref = IOHIDDeviceGetProperty(device, key);
        if (!ref || CFGetTypeID(ref) != CFStringGetTypeID())
            return "";

        return cf_string_to_std_string(static_cast<CFStringRef>(ref));
    }

    // Forward declare Impl struct for callback
    struct USBDeviceConnectionImpl
    {
        std::vector<uint8_t> cached_report_;
        size_t cached_offset_;
        bool has_pending_data_;
    };

    // Callback for input reports
    static void input_report_callback(void *context, IOReturn result, void *sender,
                                      IOHIDReportType type, uint32_t report_id,
                                      uint8_t *report, CFIndex report_length)
    {
        (void)sender;
        (void)type;
        (void)report_id;

        if (result != kIOReturnSuccess || !context)
            return;

        auto *impl = static_cast<USBDeviceConnectionImpl *>(context);

        // Store the report in the cache
        impl->cached_report_.assign(report, report + report_length);
        impl->cached_offset_ = 0;
        impl->has_pending_data_ = true;
    }

    USBDeviceConnection::Impl::Impl(const DeviceInfo &info)
        : cached_report_(),
          cached_offset_(0),
          has_pending_data_(false),
          device_info_(info),
          is_open_(false),
          device_handle_(nullptr),
          hid_manager_(nullptr),
          input_report_len_(0),
          output_report_len_(0)
    {
    }

    USBDeviceConnection::Impl::~Impl()
    {
        close();
    }

    bool USBDeviceConnection::Impl::open()
    {
        if (is_open_)
        {
            return true;
        }

        // Get USB device details from variant
        const auto &usb_details = std::get<USBDeviceDetails>(device_info_.details);

        // Create HID manager
        IOHIDManagerRef hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        if (!hid_manager)
        {
            spdlog::error("Failed to create HID manager");
            return false;
        }

        // Set matching criteria for TomTom devices
        CFMutableDictionaryRef matching_dict = CFDictionaryCreateMutable(
            kCFAllocatorDefault, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);

        int32_t vendor_id = static_cast<int32_t>(device_info_.vendor_id);
        CFNumberRef vendor_id_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor_id);
        CFDictionarySetValue(matching_dict, CFSTR(kIOHIDVendorIDKey), vendor_id_ref);
        CFRelease(vendor_id_ref);

        IOHIDManagerSetDeviceMatching(hid_manager, matching_dict);
        CFRelease(matching_dict);

        // Open the HID manager
        IOReturn ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
        if (ret != kIOReturnSuccess)
        {
            spdlog::error("Failed to open HID manager: 0x{:08X}", ret);
            CFRelease(hid_manager);
            return false;
        }

        // Get matching devices
        CFSetRef device_set = IOHIDManagerCopyDevices(hid_manager);
        if (!device_set)
        {
            spdlog::error("No devices found");
            IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
            CFRelease(hid_manager);
            return false;
        }

        CFIndex device_count = CFSetGetCount(device_set);
        std::vector<IOHIDDeviceRef> devices(device_count);
        std::vector<const void *> temp_values(device_count);
        CFSetGetValues(device_set, temp_values.data());
        for (CFIndex i = 0; i < device_count; i++)
        {
            devices[i] = static_cast<IOHIDDeviceRef>(const_cast<void *>(temp_values[i]));
        }

        IOHIDDeviceRef target_device = nullptr;

        // Find our specific device by matching path or serial number
        for (CFIndex i = 0; i < device_count; i++)
        {
            IOHIDDeviceRef dev = devices[i];

            // Check product ID
            int32_t pid = get_int_property(dev, CFSTR(kIOHIDProductIDKey));
            if (pid != static_cast<int32_t>(device_info_.product_id))
                continue;

            // Check serial number if available
            std::string serial = get_string_property(dev, CFSTR(kIOHIDSerialNumberKey));
            if (!device_info_.serial_number.empty() && serial == device_info_.serial_number)
            {
                target_device = dev;
                break;
            }

            // Fallback: use the first matching device
            if (!target_device)
            {
                target_device = dev;
            }
        }

        if (!target_device)
        {
            spdlog::error("Device not found: {}", usb_details.device_path);
            CFRelease(device_set);
            IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
            CFRelease(hid_manager);
            return false;
        }

        // Open the device
        ret = IOHIDDeviceOpen(target_device, kIOHIDOptionsTypeSeizeDevice);
        if (ret != kIOReturnSuccess)
        {
            spdlog::error("Failed to open device: 0x{:08X}", ret);
            CFRelease(device_set);
            IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
            CFRelease(hid_manager);
            return false;
        }

        // Get report lengths
        input_report_len_ = get_int_property(target_device, CFSTR(kIOHIDMaxInputReportSizeKey));
        output_report_len_ = get_int_property(target_device, CFSTR(kIOHIDMaxOutputReportSizeKey));

        spdlog::debug("HID Capabilities: InputLen={}, OutputLen={}",
                      input_report_len_, output_report_len_);

        // Register input report callback
        if (input_report_len_ > 0)
        {
            cached_report_.resize(input_report_len_);

            IOHIDDeviceRegisterInputReportCallback(
                target_device,
                cached_report_.data(),
                input_report_len_,
                input_report_callback,
                this);

            // Schedule with run loop
            IOHIDDeviceScheduleWithRunLoop(
                target_device,
                CFRunLoopGetCurrent(),
                kCFRunLoopDefaultMode);
        }

        // Store device handle and manager
        device_handle_ = target_device;
        hid_manager_ = hid_manager;

        CFRelease(device_set);
        is_open_ = true;
        return true;
    }

    void USBDeviceConnection::Impl::close()
    {
        if (!is_open_)
            return;

        if (device_handle_ != nullptr)
        {
            IOHIDDeviceRef device = static_cast<IOHIDDeviceRef>(device_handle_);

            // Unschedule from run loop
            IOHIDDeviceUnscheduleFromRunLoop(
                device,
                CFRunLoopGetCurrent(),
                kCFRunLoopDefaultMode);

            // Close device
            IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);
            device_handle_ = nullptr;
        }

        if (hid_manager_ != nullptr)
        {
            IOHIDManagerRef manager = static_cast<IOHIDManagerRef>(hid_manager_);
            IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
            CFRelease(manager);
            hid_manager_ = nullptr;
        }

        cached_report_.clear();
        cached_offset_ = 0;
        has_pending_data_ = false;
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
        size_t bytesCopied = 0;

        auto start_time = std::chrono::steady_clock::now();

        while (bytesCopied < size)
        {
            // If cache is empty or fully consumed, wait for new data
            if (cached_offset_ >= cached_report_.size() || !has_pending_data_)
            {
                // Run the run loop to process callbacks
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);

                // Check timeout
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - start_time)
                                   .count();

                if (elapsed >= timeout_ms)
                {
                    // Timeout - return what we have
                    break;
                }

                // If still no data, continue waiting
                if (!has_pending_data_)
                {
                    continue;
                }

                cached_offset_ = 0;
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

            // Mark as consumed if we've read all cached data
            if (cached_offset_ >= cached_report_.size())
            {
                has_pending_data_ = false;
            }
        }

        return static_cast<int>(bytesCopied);
    }

    int USBDeviceConnection::Impl::write(const uint8_t *buffer, size_t size, int timeout_ms)
    {
        (void)timeout_ms; // Currently unused on macOS HID

        if (!is_open_ || device_handle_ == nullptr || buffer == nullptr || size == 0)
        {
            return -1;
        }

        IOHIDDeviceRef device = static_cast<IOHIDDeviceRef>(device_handle_);

        // Prepare output buffer
        std::vector<uint8_t> outputBuffer(output_report_len_, 0);
        size_t dataToCopy = std::min(size, static_cast<size_t>(output_report_len_));
        std::memcpy(outputBuffer.data(), buffer, dataToCopy);

        // Send output report (report ID 0 for most devices)
        IOReturn ret = IOHIDDeviceSetReport(
            device,
            kIOHIDReportTypeOutput,
            0, // report ID
            outputBuffer.data(),
            output_report_len_);

        if (ret != kIOReturnSuccess)
        {
            spdlog::error("HID Write failed: 0x{:08X}", ret);
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

        // Create HID manager
        IOHIDManagerRef hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        if (!hid_manager)
        {
            spdlog::error("Failed to create HID manager");
            return devices;
        }

        // Set matching criteria for TomTom devices
        CFMutableDictionaryRef matching_dict = CFDictionaryCreateMutable(
            kCFAllocatorDefault, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);

        int32_t vendor_id = static_cast<int32_t>(VendorID::TOMTOM);
        CFNumberRef vendor_id_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendor_id);
        CFDictionarySetValue(matching_dict, CFSTR(kIOHIDVendorIDKey), vendor_id_ref);
        CFRelease(vendor_id_ref);

        IOHIDManagerSetDeviceMatching(hid_manager, matching_dict);
        CFRelease(matching_dict);

        // Open the HID manager
        IOReturn ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
        if (ret != kIOReturnSuccess)
        {
            spdlog::error("Failed to open HID manager: 0x{:08X}", ret);
            CFRelease(hid_manager);
            return devices;
        }

        // Get matching devices
        CFSetRef device_set = IOHIDManagerCopyDevices(hid_manager);
        if (device_set)
        {
            CFIndex device_count = CFSetGetCount(device_set);
            std::vector<IOHIDDeviceRef> hid_devices(device_count);
            std::vector<const void *> temp_values(device_count);
            CFSetGetValues(device_set, temp_values.data());
            for (CFIndex i = 0; i < device_count; i++)
            {
                hid_devices[i] = static_cast<IOHIDDeviceRef>(const_cast<void *>(temp_values[i]));
            }

            for (CFIndex i = 0; i < device_count; i++)
            {
                IOHIDDeviceRef dev = hid_devices[i];

                DeviceInfo info;

                int32_t vid = get_int_property(dev, CFSTR(kIOHIDVendorIDKey));
                int32_t pid = get_int_property(dev, CFSTR(kIOHIDProductIDKey));

                info.vendor_id = static_cast<VendorID>(vid);
                info.product_id = static_cast<ProductID>(pid);
                info.transport = TransportType::USB;

                // Get string properties
                info.manufacturer = get_string_property(dev, CFSTR(kIOHIDManufacturerKey));
                info.product_name = get_string_property(dev, CFSTR(kIOHIDProductKey));
                info.serial_number = get_string_property(dev, CFSTR(kIOHIDSerialNumberKey));

                // Set USB-specific details
                USBDeviceDetails usb_details;

                // On macOS, we use serial number as the device path identifier
                usb_details.device_path = info.serial_number.empty()
                                              ? std::to_string(i)
                                              : info.serial_number;

                info.details = usb_details;

                devices.push_back(info);

                spdlog::debug("Found TomTom device: VID=0x{:04X}, PID=0x{:04X}, Serial={}",
                              static_cast<uint16_t>(info.vendor_id),
                              static_cast<uint16_t>(info.product_id),
                              info.serial_number);
            }

            CFRelease(device_set);
        }

        IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
        CFRelease(hid_manager);

        return devices;
    }
}