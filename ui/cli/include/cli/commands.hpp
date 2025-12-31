#pragma once

#include <string>
#include <vector>
#include "tomtom/manager.hpp"
#include "tomtom/watch.hpp"

namespace tomtom::cli
{

    class Commands
    {
    public:
        explicit Commands(Manager &manager);

        int listDevices();
        int getDeviceInfo();
        int listFiles();
        int downloadFile(const std::string &fileId, const std::string &outputPath);
        int uploadFile(const std::string &inputPath, const std::string &fileId);
        int deleteFile(const std::string &fileId);
        int syncWatch();
        int updateFirmware(const std::string &firmwarePath);
        int getCurrentTime();

    private:
        void printWatchInfo(const Watch &watch);
        void printWatchInfo(const WatchInfo &info);
        void printError(const std::string &operation, WatchError error);

        Manager &manager_;
    };

}
::cli