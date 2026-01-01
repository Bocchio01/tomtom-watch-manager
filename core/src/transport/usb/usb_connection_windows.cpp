#define NOMINMAX
#include <windows.h>
#include <winusb.h>
extern "C"
{
#include <hidsdi.h>
#include <hidpi.h>
}
#include <setupapi.h>
#include <initguid.h>
#include <iostream>
#include <vector>
#include <variant>
#include <algorithm>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/transport/usb/usb_connection.hpp"
#include "tomtom/transport/usb/usb_connection_impl.hpp"

namespace tomtom::transport
{
    USBDeviceConnection::Impl::Impl(const DeviceInfo &info)
        : device_info_(info),
          is_open_(false),
          device_handle_(INVALID_HANDLE_VALUE),
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

        // Open device handle for HID (Synchronous I/O)
        device_handle_ = CreateFileA(
            usb_details.device_path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (device_handle_ == INVALID_HANDLE_VALUE)
        {
            spdlog::error("Failed to open device. Error: {}", GetLastError());
            return false;
        }

        // Verify it's a HID device
        HIDD_ATTRIBUTES attributes;
        attributes.Size = sizeof(HIDD_ATTRIBUTES);

        if (!HidD_GetAttributes(device_handle_, &attributes))
        {
            spdlog::error("Failed to get HID attributes. Error: {}", GetLastError());
            CloseHandle(device_handle_);
            device_handle_ = INVALID_HANDLE_VALUE;
            return false;
        }

        // Get preparsed data for capabilities
        PHIDP_PREPARSED_DATA preparsedData;
        if (HidD_GetPreparsedData(device_handle_, &preparsedData))
        {
            HIDP_CAPS caps;
            if (HidP_GetCaps(preparsedData, &caps) == HIDP_STATUS_SUCCESS)
            {
                input_report_len_ = caps.InputReportByteLength;
                output_report_len_ = caps.OutputReportByteLength;
                spdlog::debug("HID Capabilities: InputLen={}, OutputLen={}",
                              input_report_len_, output_report_len_);
            }
            HidD_FreePreparsedData(preparsedData);
        }

        is_open_ = true;
        return true;
    }

    void USBDeviceConnection::Impl::close()
    {
        if (!is_open_)
            return;

        if (device_handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(device_handle_);
            device_handle_ = INVALID_HANDLE_VALUE;
        }

        is_open_ = false;
    }

    bool USBDeviceConnection::Impl::isOpen() const
    {
        return is_open_;
    }

    int USBDeviceConnection::Impl::read(uint8_t *buffer, size_t size, int timeout_ms)
    {
        (void)timeout_ms; // unused for synchronous HID

        if (!is_open_ || device_handle_ == INVALID_HANDLE_VALUE || buffer == nullptr || size == 0)
        {
            return -1;
        }

        size_t bytesCopied = 0;

        while (bytesCopied < size)
        {
            // If cache is empty or fully consumed, read a new HID report
            if (cached_offset_ >= cached_report_.size())
            {
                cached_report_.assign(input_report_len_, 0);
                cached_offset_ = 0;

                DWORD bytesRead = 0;
                BOOL result = ReadFile(
                    device_handle_,
                    cached_report_.data(),
                    static_cast<DWORD>(cached_report_.size()),
                    &bytesRead,
                    nullptr);

                if (!result)
                {
                    spdlog::error("HID Read failed. Error: {}", GetLastError());
                    return bytesCopied > 0 ? static_cast<int>(bytesCopied) : -1;
                }

                cached_report_.resize(bytesRead);

                // No data read — nothing more to provide
                if (bytesRead == 0)
                {
                    break;
                }
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
        (void)timeout_ms; // Currently unused on Windows HID synchronous I/O

        if (!is_open_ || device_handle_ == INVALID_HANDLE_VALUE)
        {
            return -1;
        }

        // timeout_ms is currently unused on Windows HID synchronous I/O
        (void)timeout_ms;

        // Write EXACT packet, NO report ID prepended
        std::vector<uint8_t> outputBuffer(output_report_len_, 0);

        size_t dataToCopy = std::min(size, static_cast<size_t>(output_report_len_));
        std::memcpy(outputBuffer.data(), buffer, dataToCopy);

        DWORD bytesWritten = 0;

        BOOL result = WriteFile(
            device_handle_,
            outputBuffer.data(),
            static_cast<DWORD>(outputBuffer.size()),
            &bytesWritten,
            nullptr);

        if (!result)
        {
            spdlog::error("HID Write failed. Error: {}", GetLastError());
            return -1;
        }

        return static_cast<int>(dataToCopy);
    }

    const DeviceInfo &USBDeviceConnection::Impl::deviceInfo() const
    {
        return device_info_;
    }

    // Implementation of the static enumerate method
    std::vector<DeviceInfo> USBDeviceConnection::Impl::enumerate()
    {
        std::vector<DeviceInfo> devices;

        // Get HID GUID
        GUID hidGuid;
        HidD_GetHidGuid(&hidGuid);

        // Get device information set for HID devices
        HDEVINFO deviceInfoSet = SetupDiGetClassDevsA(
            &hidGuid,
            nullptr,
            nullptr,
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (deviceInfoSet == INVALID_HANDLE_VALUE)
        {
            spdlog::error("Failed to get device information set. Error: {}", GetLastError());
            return devices;
        }

        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        // Enumerate all HID devices
        for (DWORD i = 0; SetupDiEnumDeviceInterfaces(
                 deviceInfoSet,
                 nullptr,
                 &hidGuid,
                 i,
                 &deviceInterfaceData);
             i++)
        {
            DWORD requiredSize = 0;
            SetupDiGetDeviceInterfaceDetailA(
                deviceInfoSet,
                &deviceInterfaceData,
                nullptr,
                0,
                &requiredSize,
                nullptr);

            auto deviceInterfaceDetailDataBuffer = std::make_unique<uint8_t[]>(requiredSize);
            auto *deviceInterfaceDetailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_A>(deviceInterfaceDetailDataBuffer.get());

            deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

            SP_DEVINFO_DATA deviceInfoData;
            deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            if (SetupDiGetDeviceInterfaceDetailA(
                    deviceInfoSet,
                    &deviceInterfaceData,
                    deviceInterfaceDetailData,
                    requiredSize,
                    nullptr,
                    &deviceInfoData))
            {
                HANDLE hDevice = CreateFileA(
                    deviceInterfaceDetailData->DevicePath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr,
                    OPEN_EXISTING,
                    0,
                    nullptr);

                if (hDevice != INVALID_HANDLE_VALUE)
                {
                    HIDD_ATTRIBUTES attributes;
                    attributes.Size = sizeof(HIDD_ATTRIBUTES);

                    if (HidD_GetAttributes(hDevice, &attributes))
                    {
                        // Check if this is a TomTom device
                        if (attributes.VendorID == static_cast<uint16_t>(VendorID::TOMTOM))
                        {
                            DeviceInfo info;
                            info.vendor_id = static_cast<VendorID>(attributes.VendorID);
                            info.product_id = static_cast<ProductID>(attributes.ProductID);
                            info.transport = TransportType::USB;

                            // Set USB-specific details
                            USBDeviceDetails usb_details;
                            usb_details.device_path = std::string(deviceInterfaceDetailData->DevicePath);
                            info.details = usb_details;

                            wchar_t buffer[256] = {0};
                            if (HidD_GetManufacturerString(hDevice, buffer, sizeof(buffer)))
                            {
                                std::wstring wStr(buffer);
                                info.manufacturer = std::string(wStr.begin(), wStr.end());
                            }

                            if (HidD_GetProductString(hDevice, buffer, sizeof(buffer)))
                            {
                                std::wstring wStr(buffer);
                                info.product_name = std::string(wStr.begin(), wStr.end());
                            }

                            if (HidD_GetSerialNumberString(hDevice, buffer, sizeof(buffer)))
                            {
                                std::wstring wStr(buffer);
                                info.serial_number = std::string(wStr.begin(), wStr.end());
                            }

                            devices.push_back(info);
                            spdlog::debug("Found TomTom device: VID=0x{:04X}, PID=0x{:04X}, Path={}",
                                          static_cast<uint16_t>(info.vendor_id),
                                          static_cast<uint16_t>(info.product_id),
                                          usb_details.device_path);
                        }
                    }
                    CloseHandle(hDevice);
                }
            }
        }

        SetupDiDestroyDeviceInfoList(deviceInfoSet);
        return devices;
    }

}