#pragma once

#include <string>
#include <vector>

// ----------------- Base64 -----------------
std::string base64Encode(const std::vector<uint8_t>& data) {
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    size_t i = 0;

    while (i < data.size()) {
        uint32_t octet_a = i < data.size() ? data[i++] : 0;
        uint32_t octet_b = i < data.size() ? data[i++] : 0;
        uint32_t octet_c = i < data.size() ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        encoded.push_back(table[(triple >> 18) & 0x3F]);
        encoded.push_back(table[(triple >> 12) & 0x3F]);
        encoded.push_back((i - 1 > data.size()) ? '=' : table[(triple >> 6) & 0x3F]);
        encoded.push_back((i > data.size()) ? '=' : table[triple & 0x3F]);
    }

    // 修正 padding 判斷
    size_t mod = data.size() % 3;
    if (mod > 0) {
        encoded[encoded.size() - 1] = '=';
        if (mod == 1) {
            encoded[encoded.size() - 2] = '=';
        }
    }

    return encoded;
}
