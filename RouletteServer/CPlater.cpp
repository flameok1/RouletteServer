#include "CPlayer.h"
#include "CHttpParser.h"
#include <iostream>
#include <memory>
#include "proto/login.pb.h"
#include "ws_frame_tool.h"
#include"protocol.h"

#define RECV_BUFF_SIZE 1024


void CPlayer::reciveHandle(ClientSession* cs, uint8_t* pBuff, int count)
{
    if (_recvBuffSize < count + _nowDataCount)
    {
        std::shared_ptr<uint8_t> newBuff(new uint8_t[_recvBuffSize + RECV_BUFF_SIZE], std::default_delete<uint8_t[]>());

        if (_nowDataCount > 0)
        {
            memcpy(newBuff.get(), _recvBuff.get(), _nowDataCount);
        }

        _recvBuffSize += RECV_BUFF_SIZE;
        _recvBuff = newBuff;
    }

    memcpy((_recvBuff.get() + _nowDataCount), pBuff, count);
    _nowDataCount += count;


    if (!isWebSocketUpgrade)
    {
        //check http
        CHttpParser httpParser;
        httpParser.httpParser((char*)pBuff, count);

        bool connection = httpParser.checkKeyBalue("Connection", "Upgrade");
        bool upgrade = httpParser.checkKeyBalue("Upgrade", "websocket");
        std::string webSocketKey = httpParser.getValue("Sec-WebSocket-Key");

        if (webSocketKey == "")
        {
            return;
        }

        std::string accept = httpParser.generateWebSocketAccept(webSocketKey);

        std::string sendstr = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

        sendstr = sendstr + accept + "\r\n\r\n";

        cs->addBuff((uint8_t*)sendstr.c_str(), sendstr.length());

        isWebSocketUpgrade = true;
        _nowDataCount = 0;
    }
    else
    {
        pBuff = _recvBuff.get();
        size_t frame_size;
        WSFrame frame;

        while (_nowDataCount > 0)
        {
            if (ws_parse_frame((const uint8_t*)pBuff, count, frame_size, frame))
            {
                _nowDataCount -= frame_size;
                if (_nowDataCount > 0)
                {
                    memcpy(pBuff, pBuff + frame_size, _nowDataCount);
                }
            }
            else {
                std::cout << "資料不足，無法解析完整 frame\n";
                return;
            }

            int payloadsize = frame.payload.size();
            uint8_t* payload = frame.payload.data();

            // 讀長度（小端）
            uint32_t packetLen;
            memcpy(&packetLen, payload, 4);
            // 驗證長度
            if (packetLen + 4 != payloadsize) {
                std::cerr << "Packet length mismatch" << std::endl;
                return;
            }

            // 讀協議 ID
            uint32_t protocolId;
            memcpy(&protocolId, payload + 4, 4);

            const uint8_t* protobufData = payload + 8;
            size_t protobufLen = packetLen - 4;
            // 計算序列化長度
            int ressize = 0;

            std::vector<uint8_t> resbuffer;

            switch (protocolId)
            {
            case Protocol::LoginRequest: {
                    loginpackage::LoginRequest req;
                    if (req.ParseFromArray(protobufData, protobufLen)) {
                        std::cout << std::endl << "[Login]" << std::endl << "user=" << req.username() << std::endl;
                        std::cout << "pass=" << req.password() << std::endl;

                        loginpackage::LoginResponse res;

                        res.set_username(req.username());
                        res.set_password(req.password());

                        // 計算序列化長度
                        ressize = res.ByteSizeLong();

                        std::cout << std::endl << std::endl << "user=" << res.username() << std::endl;
                        std::cout << "pass=" << res.password() << std::endl;

                        std::shared_ptr<uint8_t> resbuffer(new uint8_t[ressize], std::default_delete<uint8_t[]>());

                        if (res.SerializeToArray(resbuffer.get(), ressize)) {
                            std::vector<uint8_t> sendbuff = makeWebSocketBinaryFrame(Protocol::LoginResponse, resbuffer.get(), ressize);

                            cs->addBuff(sendbuff.data(), sendbuff.size());
                        }
                    }
                    break;
                }
                default:
                    std::cerr << "Unknown protocol: " << protocolId << std::endl;
                    break;
            }
        }
    }
}