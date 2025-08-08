#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>

class SHA1 {
public:
    SHA1() { reset(); }

    void update(const std::string& data) {
        update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
    }

    // 將 64-bit 整數轉成 big-endian
    uint64_t toBigEndian64(uint64_t x) {
        uint64_t result = 0;
        for (int i = 0; i < 8; i++) {
            result |= ((x >> (56 - i * 8)) & 0xFF) << (i * 8);
        }
        return result;
    }

    void update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            buffer[bufferOffset++] = data[i];
            transforms++;
            if (bufferOffset == 64) {
                transform();
                bufferOffset = 0;
            }
        }
        totalBits += len * 8;
    }

    std::vector<uint8_t> final() {
        uint64_t totalBitsBE = toBigEndian64(totalBits);

        buffer[bufferOffset++] = 0x80;
        if (bufferOffset > 56) {
            while (bufferOffset < 64) buffer[bufferOffset++] = 0x00;
            transform();
            bufferOffset = 0;
        }
        while (bufferOffset < 56) buffer[bufferOffset++] = 0x00;

        memcpy(buffer + 56, &totalBitsBE, 8);
        transform();

        std::vector<uint8_t> hash(20);
        for (int i = 0; i < 5; i++) {
            hash[i * 4] = (state[i] >> 24) & 0xFF;
            hash[i * 4 + 1] = (state[i] >> 16) & 0xFF;
            hash[i * 4 + 2] = (state[i] >> 8) & 0xFF;
            hash[i * 4 + 3] = state[i] & 0xFF;
        }
        return hash;
    }

private:
    uint32_t state[5];
    uint8_t buffer[64];
    size_t bufferOffset;
    uint64_t totalBits;
    size_t transforms;

    static uint32_t rol(uint32_t value, size_t bits) {
        return (value << bits) | (value >> (32 - bits));
    }

    void reset() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        state[4] = 0xC3D2E1F0;
        bufferOffset = 0;
        totalBits = 0;
        transforms = 0;
    }

    void transform() {
        uint32_t w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = (buffer[i * 4] << 24) |
                (buffer[i * 4 + 1] << 16) |
                (buffer[i * 4 + 2] << 8) |
                (buffer[i * 4 + 3]);
        }
        for (int i = 16; i < 80; i++) {
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];
        uint32_t e = state[4];

        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
    }
};