#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace tomtom::sdk::utils
{
    /**
     * @brief Trim whitespace from both ends
     */
    inline std::string trim(const std::string &str)
    {
        auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch)
                                      { return std::isspace(ch); });
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch)
                                    { return std::isspace(ch); })
                       .base();

        return (start < end) ? std::string(start, end) : std::string();
    }

    /**
     * @brief Split string by delimiter
     */
    inline std::vector<std::string> split(const std::string &str, char delimiter)
    {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos)
        {
            tokens.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }
        tokens.push_back(str.substr(start));

        return tokens;
    }

    /**
     * @brief Convert to lowercase
     */
    inline std::string toLower(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return result;
    }

    /**
     * @brief Convert to uppercase
     */
    inline std::string toUpper(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c)
                       { return std::toupper(c); });
        return result;
    }

    /**
     * @brief Replace all occurrences of a substring
     */
    inline std::string replaceAll(std::string str, const std::string &from, const std::string &to)
    {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::string::npos)
        {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
        return str;
    }

}
