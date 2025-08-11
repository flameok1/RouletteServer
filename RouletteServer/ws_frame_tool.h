#pragma once

#include <vector>


// 解析結果結構
struct WSFrame {
    bool fin;
    uint8_t opcode; // 1=text, 2=binary, 8=close, 9=ping, 10=pong
    std::vector<uint8_t> payload; // unmask 後資料
};

// 回傳值：true=解析成功，false=資料不足
extern inline bool ws_parse_frame(
    const uint8_t* data, size_t len,
    size_t& frame_size, // 此 frame 的總長度 (header + payload)
    WSFrame& frame_out  // 解析後的 frame
);



extern std::vector<uint8_t> makeWebSocketBinaryFrame(uint32_t id, const uint8_t* protoData, int size);