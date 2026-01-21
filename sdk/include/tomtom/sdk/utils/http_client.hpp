#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace tomtom::sdk::utils
{

    /**
     * @brief Progress callback for HTTP downloads
     * @param downloaded Bytes downloaded so far
     * @param total Total file size (0 if unknown)
     */
    using HttpProgressCallback = std::function<void(size_t downloaded, size_t total)>;

    /**
     * @brief HTTP download result
     */
    struct HttpDownloadResult
    {
        std::vector<uint8_t> data; ///< Downloaded data
        int status_code;           ///< HTTP status code
        std::string error_message; ///< Error message if failed

        /**
         * @brief Check if download was successful
         */
        bool success() const { return status_code == 200; }
    };

    /**
     * @brief Download a file from a URL
     * @param url URL to download from
     * @param progress Optional progress callback
     * @return Download result with data or error
     */
    HttpDownloadResult downloadFile(const std::string &url, HttpProgressCallback progress = nullptr);

}
