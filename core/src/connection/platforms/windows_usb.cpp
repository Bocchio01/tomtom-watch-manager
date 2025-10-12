#define NOMINMAX
#include <windows.h>
extern "C"
{
#include <hidsdi.h>
}
#include <setupapi.h>
#include <initguid.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <spdlog/spdlog.h>

#include "tomtom/defines.hpp"
#include "tomtom/connection/usb_connection.hpp"
#include "tomtom/connection/platforms/windows_usb.hpp"

namespace tomtom
{
    namespace platforms
    {

        WindowsUSBImpl::WindowsUSBImpl(const DeviceInfo &info)
            : device_info(info),
              device_handle(INVALID_HANDLE_VALUE),
              usb_handle(nullptr),
              is_open(false)
        {
        }

        WindowsUSBImpl::~WindowsUSBImpl()
        {
            close();
        }

        std::vector<DeviceInfo> WindowsUSBImpl::enumerateDevices()
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

                // Get required buffer size
                DWORD requiredSize = 0;
                SetupDiGetDeviceInterfaceDetailA(
                    deviceInfoSet,
                    &deviceInterfaceData,
                    nullptr,
                    0,
                    &requiredSize,
                    nullptr);

                // Allocate buffer for device interface detail
                PSP_DEVICE_INTERFACE_DETAIL_DATA_A deviceInterfaceDetailData =
                    (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)malloc(requiredSize);

                if (!deviceInterfaceDetailData)
                {
                    continue;
                }

                deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

                SP_DEVINFO_DATA deviceInfoData;
                deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

                // Get device interface detail
                if (SetupDiGetDeviceInterfaceDetailA(
                        deviceInfoSet,
                        &deviceInterfaceData,
                        deviceInterfaceDetailData,
                        requiredSize,
                        nullptr,
                        &deviceInfoData))
                {

                    // Open device handle for HID
                    HANDLE hDevice = CreateFileA(
                        deviceInterfaceDetailData->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        nullptr,
                        OPEN_EXISTING,
                        0, // No overlapped for HID
                        nullptr);

                    if (hDevice != INVALID_HANDLE_VALUE)
                    {
                        // Get HID attributes (VID/PID)
                        HIDD_ATTRIBUTES attributes;
                        attributes.Size = sizeof(HIDD_ATTRIBUTES);

                        if (HidD_GetAttributes(hDevice, &attributes))
                        {
                            // Check if this is a TomTom device
                            if (attributes.VendorID == TOMTOM_VENDOR_ID)
                            {
                                DeviceInfo info;
                                info.vendor_id = attributes.VendorID;
                                info.product_id = attributes.ProductID;
                                info.device_path = std::string(deviceInterfaceDetailData->DevicePath);

                                // Get manufacturer string
                                wchar_t manufacturerBuffer[256] = {0};
                                if (HidD_GetManufacturerString(hDevice, manufacturerBuffer,
                                                               sizeof(manufacturerBuffer)))
                                {
                                    std::wstring wMfg(manufacturerBuffer);
                                    info.manufacturer = std::string(wMfg.begin(), wMfg.end());
                                }

                                // Get product string
                                wchar_t productBuffer[256] = {0};
                                if (HidD_GetProductString(hDevice, productBuffer,
                                                          sizeof(productBuffer)))
                                {
                                    std::wstring wProd(productBuffer);
                                    info.product_name = std::string(wProd.begin(), wProd.end());
                                }

                                // Get serial number
                                wchar_t serialBuffer[256] = {0};
                                if (HidD_GetSerialNumberString(hDevice, serialBuffer,
                                                               sizeof(serialBuffer)))
                                {
                                    std::wstring wSerial(serialBuffer);
                                    info.serial_number = std::string(wSerial.begin(), wSerial.end());
                                }

                                devices.push_back(info);
                                spdlog::debug("Found TomTom device: VID=0x{:04X}, PID=0x{:04X}, Serial={}, Path={}",
                                              info.vendor_id, info.product_id, info.serial_number, info.device_path);
                            }
                        }
                        else
                        {
                            spdlog::error("Failed to get HID attributes. Error: {}", GetLastError());
                        }

                        CloseHandle(hDevice);
                    }
                }

                free(deviceInterfaceDetailData);
            }

            SetupDiDestroyDeviceInfoList(deviceInfoSet);

            spdlog::debug("Found {} TomTom device(s)", devices.size());

            return devices;
        }

        bool WindowsUSBImpl::open()
        {
            if (is_open)
            {
                return true;
            }

            // Open device handle for HID (no FILE_FLAG_OVERLAPPED needed for sync operations)
            device_handle = CreateFileA(
                device_info.device_path.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr,
                OPEN_EXISTING,
                0, // Synchronous I/O
                nullptr);

            if (device_handle == INVALID_HANDLE_VALUE)
            {
                spdlog::error("Failed to open device. Error: {}", GetLastError());
                return false;
            }

            // Verify it's a HID device
            HIDD_ATTRIBUTES attributes;
            attributes.Size = sizeof(HIDD_ATTRIBUTES);

            if (!HidD_GetAttributes(device_handle, &attributes))
            {
                spdlog::error("Failed to get HID attributes. Error: {}", GetLastError());
                CloseHandle(device_handle);
                device_handle = INVALID_HANDLE_VALUE;
                return false;
            }

            // Get preparsed data for capabilities
            PHIDP_PREPARSED_DATA preparsedData;
            if (HidD_GetPreparsedData(device_handle, &preparsedData))
            {
                HIDP_CAPS caps_raw;
                if (HidP_GetCaps(preparsedData, &caps_raw) == HIDP_STATUS_SUCCESS)
                {
                    capabilities.input_len = caps_raw.InputReportByteLength;
                    capabilities.output_len = caps_raw.OutputReportByteLength;
                    capabilities.feature_len = caps_raw.FeatureReportByteLength;
                    spdlog::debug("HID Capabilities: InputLen={}, OutputLen={}, FeatureLen={}",
                                  capabilities.input_len, capabilities.output_len, capabilities.feature_len);
                }
                HidD_FreePreparsedData(preparsedData);
            }

            is_open = true;
            return true;
        }

        void WindowsUSBImpl::close()
        {
            if (!is_open)
            {
                return;
            }

            if (device_handle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(device_handle);
                device_handle = INVALID_HANDLE_VALUE;
            }

            is_open = false;
        }

        bool WindowsUSBImpl::isOpen() const
        {
            return is_open;
        }

        // Fixed read function - DO NOT skip first byte!
        int WindowsUSBImpl::read(uint8_t *buffer, size_t size, int timeout_ms)
        {
            if (!is_open || device_handle == INVALID_HANDLE_VALUE)
            {
                return -1;
            }

            std::vector<UCHAR> inputBuffer(capabilities.input_len);
            DWORD bytesRead = 0;

            // HID read - synchronous
            BOOL result = ReadFile(
                device_handle,
                inputBuffer.data(),
                capabilities.input_len,
                &bytesRead,
                nullptr);

            if (!result)
            {
                DWORD error = GetLastError();
                spdlog::error("HID Read failed. Error: {}", error);
                return -1;
            }

            spdlog::debug("HID Read: {} bytes", bytesRead);

            // Copy data directly - NO offset for report ID!
            size_t dataToCopy = std::min(size, static_cast<size_t>(bytesRead));

            if (dataToCopy > 0)
            {
                std::memcpy(buffer, inputBuffer.data(), dataToCopy);
            }

            return static_cast<int>(dataToCopy);
        }

        // Fixed write function - DO NOT prepend report ID!
        int WindowsUSBImpl::write(const uint8_t *buffer, size_t size, int timeout_ms)
        {
            if (!is_open || device_handle == INVALID_HANDLE_VALUE)
            {
                return -1;
            }

            // For TomTom watches, we write the EXACT packet directly
            // NO report ID byte needed!
            std::vector<UCHAR> outputBuffer(capabilities.output_len, 0);

            // Copy user data directly (no offset for report ID)
            size_t dataToCopy = std::min(size, static_cast<size_t>(capabilities.output_len));
            std::memcpy(outputBuffer.data(), buffer, dataToCopy);

            // Pad the rest with zeros (already done by vector initialization)

            DWORD bytesWritten = 0;

            // Write the full output report
            BOOL result = WriteFile(
                device_handle,
                outputBuffer.data(),
                capabilities.output_len,
                &bytesWritten,
                nullptr);

            if (!result)
            {
                DWORD error = GetLastError();
                spdlog::error("HID Write failed. Error: {}", error);
                return -1;
            }

            spdlog::debug("HID Write successful: {} bytes written", bytesWritten);
            return static_cast<int>(dataToCopy);
        }

    } // namespace platforms
} // namespace tomtom