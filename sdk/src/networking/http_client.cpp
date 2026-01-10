#include "tomtom/sdk/networking/http_client.hpp"
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#else
#include <curl/curl.h>
#endif

namespace tomtom::sdk::networking
{

#ifdef _WIN32
    // Windows implementation using WinHTTP
    HttpDownloadResult downloadFile(const std::string &url, HttpProgressCallback progress)
    {
        HttpDownloadResult result{};
        result.status_code = 0;

        spdlog::debug("Downloading file from: {}", url);

        // Parse URL
        std::wstring wide_url(url.begin(), url.end());
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        wchar_t hostName[256] = {0};
        wchar_t urlPath[2048] = {0};

        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

        if (!WinHttpCrackUrl(wide_url.c_str(), static_cast<DWORD>(wide_url.length()), 0, &urlComp))
        {
            result.error_message = "Failed to parse URL";
            spdlog::error("Failed to parse URL: {}", url);
            return result;
        }

        // Initialize WinHTTP
        HINTERNET hSession = WinHttpOpen(
            L"TomTom Watch Manager/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (!hSession)
        {
            result.error_message = "Failed to initialize WinHTTP";
            spdlog::error("Failed to initialize WinHTTP");
            return result;
        }

        // Connect to server
        HINTERNET hConnect = WinHttpConnect(
            hSession,
            hostName,
            urlComp.nPort,
            0);

        if (!hConnect)
        {
            result.error_message = "Failed to connect to server";
            spdlog::error("Failed to connect to server");
            WinHttpCloseHandle(hSession);
            return result;
        }

        // Create request
        DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(
            hConnect,
            L"GET",
            urlPath,
            NULL,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            flags);

        if (!hRequest)
        {
            result.error_message = "Failed to create request";
            spdlog::error("Failed to create request");
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        // Send request
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        {
            result.error_message = "Failed to send request";
            spdlog::error("Failed to send request");
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        // Receive response
        if (!WinHttpReceiveResponse(hRequest, NULL))
        {
            result.error_message = "Failed to receive response";
            spdlog::error("Failed to receive response");
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        // Get status code
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        WinHttpQueryHeaders(
            hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusCodeSize,
            WINHTTP_NO_HEADER_INDEX);

        result.status_code = static_cast<int>(statusCode);

        if (statusCode != 200)
        {
            result.error_message = "HTTP error: " + std::to_string(statusCode);
            spdlog::error("HTTP error: {}", statusCode);
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return result;
        }

        // Get content length
        DWORD contentLength = 0;
        DWORD contentLengthSize = sizeof(contentLength);
        WinHttpQueryHeaders(
            hRequest,
            WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &contentLength,
            &contentLengthSize,
            WINHTTP_NO_HEADER_INDEX);

        spdlog::debug("Downloading {} bytes", contentLength);

        // Read data
        std::vector<uint8_t> buffer;
        DWORD bytesAvailable = 0;
        DWORD bytesRead = 0;
        size_t totalRead = 0;

        while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0)
        {
            std::vector<uint8_t> chunk(bytesAvailable);
            if (WinHttpReadData(hRequest, chunk.data(), bytesAvailable, &bytesRead) && bytesRead > 0)
            {
                buffer.insert(buffer.end(), chunk.begin(), chunk.begin() + bytesRead);
                totalRead += bytesRead;

                if (progress)
                {
                    progress(totalRead, contentLength);
                }
            }
            else
            {
                break;
            }
        }

        result.data = std::move(buffer);
        spdlog::info("Downloaded {} bytes successfully", result.data.size());

        // Cleanup
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return result;
    }

#else
    // Unix/Linux/macOS implementation using libcurl
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t totalSize = size * nmemb;
        std::vector<uint8_t> *buffer = static_cast<std::vector<uint8_t> *>(userp);
        buffer->insert(buffer->end(), static_cast<uint8_t *>(contents), static_cast<uint8_t *>(contents) + totalSize);
        return totalSize;
    }

    static int progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        (void)ultotal;
        (void)ulnow;

        HttpProgressCallback *callback = static_cast<HttpProgressCallback *>(clientp);
        if (*callback)
        {
            (*callback)(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
        }
        return 0;
    }

    HttpDownloadResult downloadFile(const std::string &url, HttpProgressCallback progress)
    {
        HttpDownloadResult result{};
        result.status_code = 0;

        spdlog::debug("Downloading file from: {}", url);

        CURL *curl = curl_easy_init();
        if (!curl)
        {
            result.error_message = "Failed to initialize CURL";
            spdlog::error("Failed to initialize CURL");
            return result;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "TomTom Watch Manager/1.0");

        if (progress)
        {
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            result.error_message = curl_easy_strerror(res);
            spdlog::error("CURL error: {}", result.error_message);
            curl_easy_cleanup(curl);
            return result;
        }

        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        result.status_code = static_cast<int>(response_code);

        if (response_code != 200)
        {
            result.error_message = "HTTP error: " + std::to_string(response_code);
            spdlog::error("HTTP error: {}", response_code);
        }
        else
        {
            spdlog::info("Downloaded {} bytes successfully", result.data.size());
        }

        curl_easy_cleanup(curl);
        return result;
    }
#endif

} // namespace tomtom::utils
