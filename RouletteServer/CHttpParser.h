#pragma once

#include <sstream>
#include <unordered_map>
#include <string>

#include "Base64.h"
#include "SHA1.h"


class CHttpParser
{
public:
private:
    std::unordered_map<std::string, std::string> httpHeaders;
public:

    std::unordered_map<std::string, std::string>& httpParser(char* pBuff, int conut)
    {
        httpHeaders.clear();
        std::istringstream iss(pBuff);
        std::string line;

        // 1. 解析請求行
        if (std::getline(iss, line) && !line.empty()) {
            std::istringstream request_line_stream(line);
            std::string method, path, version;
            request_line_stream >> method >> path >> version;

            // 存入 map
            httpHeaders["Method"] = method;
            httpHeaders["Path"] = path;
            httpHeaders["Version"] = version;
        }

        // 2. 解析標頭
        while (std::getline(iss, line) && !line.empty() && line != "\r") {
            // 移除行尾的 \r
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                // 移除鍵前後的空格
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);

                std::string value = line.substr(colon_pos + 1);
                // 移除值前後的空格
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                httpHeaders[key] = value;
            }
        }

        return httpHeaders;
    }

    bool checkKeyBalue(std::string key, std::string value)
    {
        auto find = httpHeaders.find(key);

        if (find != httpHeaders.end())
        {
            if (find->second != value)
            {
                return false;
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    std::string& getValue(std::string key)
    {
        return httpHeaders[key];
    }

    std::string generateWebSocketAccept(const std::string& clientKey) {
        static const std::string magicGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        SHA1 sha1;
        sha1.update(clientKey + magicGUID);
        auto hash = sha1.final();

        return base64Encode(hash);
    }
};



