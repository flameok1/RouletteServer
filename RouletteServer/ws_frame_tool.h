#pragma once
#include <cstdint>
#include <vector>
#include <cstddef>
#include <cstring>
#include <string>
#include <memory>

// 解析結果結構
struct WSFrame {
    bool fin;
    uint8_t opcode; // 1=text, 2=binary, 8=close, 9=ping, 10=pong
    std::vector<uint8_t> payload; // unmask 後資料
};

// 回傳值：true=解析成功，false=資料不足
inline bool ws_parse_frame(
    const uint8_t* data, size_t len,
    size_t& frame_size, // 此 frame 的總長度 (header + payload)
    WSFrame& frame_out  // 解析後的 frame
) {
    if (len < 2) return false; // 至少要 2 bytes

    size_t pos = 0;
    uint8_t b0 = data[pos++];
    uint8_t b1 = data[pos++];

    frame_out.fin = (b0 & 0x80) != 0;
    frame_out.opcode = b0 & 0x0F;
    bool mask = (b1 & 0x80) != 0;
    uint64_t payload_len = b1 & 0x7F;

    // Extended payload length
    if (payload_len == 126) {
        if (len < pos + 2) return false;
        payload_len = ((uint64_t)data[pos] << 8) | data[pos + 1];
        pos += 2;
    }
    else if (payload_len == 127) {
        if (len < pos + 8) return false;
        payload_len = 0;
        for (int i = 0; i < 8; ++i) {
            payload_len = (payload_len << 8) | data[pos + i];
        }
        pos += 8;
    }

    // Mask key
    uint8_t mask_key[4] = { 0 };
    if (mask) {
        if (len < pos + 4) return false;
        memcpy(mask_key, data + pos, 4);
        pos += 4;
    }

    // Check payload availability
    if (len < pos + payload_len) return false;

    // Extract payload
    frame_out.payload.resize((size_t)payload_len);
    if (payload_len > 0) {
        if (mask) {
            for (uint64_t i = 0; i < payload_len; ++i) {
                frame_out.payload[(size_t)i] = data[pos + (size_t)i] ^ mask_key[i % 4];
            }
        }
        else {
            memcpy(frame_out.payload.data(), data + pos, (size_t)payload_len);
        }
    }

    pos += (size_t)payload_len;
    frame_size = pos;
    return true;
}



std::vector<uint8_t> makeWebSocketBinaryFrame(uint32_t id, const uint8_t* protoData, int size)
{
    // 先組出你的自定義 payload（小端序）
    uint32_t packetLen = static_cast<uint32_t>(size + 4); // ID (4 bytes) + protobuf

    std::shared_ptr<uint8_t> payload(new uint8_t[packetLen + 4], std::default_delete<uint8_t[]>());
    uint8_t* pPayload = payload.get();

    // 寫入 len（小端序）websocket會自己記錄len，這邊相對沒用到，但是一般封包會用到，就保留不忘記
    std::memcpy(pPayload, &packetLen, 4);

    // 寫入 ID（小端序）
    std::memcpy(pPayload + 4, &id, 4);

    // 寫入 protobuf bytes
    std::memcpy(pPayload + 8, protoData, size);

    // 現在開始組 WebSocket frame
    std::vector<uint8_t> frame;
    frame.reserve(2 + packetLen + 4);

    uint8_t fin_opcode = 0x82; // FIN=1 + binary opcode(0x2)
    frame.push_back(fin_opcode);

    size_t len = packetLen + 4;

    // 長度部分（伺服器 → 客戶端，不需要 mask）
    if (len <= 125) {
        frame.push_back(static_cast<uint8_t>(len));
    }
    else if (len <= 65535) {
        frame.push_back(126);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    }
    else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back((len >> (8 * i)) & 0xFF);
        }
    }

    // 寫入 payload
    frame.insert(frame.end(), pPayload, pPayload + packetLen + 4);

    return frame;
}