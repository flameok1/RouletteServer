#include "CPlayer.h"
#include "CHttpParser.h"
#include <iostream>
#include <memory>
#include "ws_frame_tool.h"
#include"protocol.h"
#include "proto/login.pb.h"
#include "proto/game.pb.h"
#include "FLAMEServer/ClientSession.h"
#include "CManagerDef.h"
#include "EventDef.h"
#include "CEventManager.h"


#define RECV_BUFF_SIZE 1024

std::unordered_map<int, protocalHandleFunction> CPlayer::protocolCalls = {
    {(int)Protocol::LoginRequest, CPlayer::loginReqHandle},
    {(int)Protocol::StartGameRequest, CPlayer::StartGameReqHandle },
    {(int)Protocol::BetRequest, CPlayer::BetRequestReqHandle }
};

CPlayer::CPlayer(int id, ClientSession* pSesssion)
    :_playerID(id), _pSesssion(pSesssion)
{
    
}

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

            if (frame.opcode == 0x8)
            {
                uint16_t closeCode = 1000; // 預設正常關閉
                if (payloadsize >= 2) {
                    closeCode = (payload[0] << 8) | payload[1];
                }

                // 回送 Close Frame 以完成關閉握手
                uint8_t closeFrame[4] = { 0x88, 0x02, static_cast<uint8_t>(closeCode >> 8), static_cast<uint8_t>(closeCode & 0xFF) };
                cs->addBuff(closeFrame, 4);
                cs->setNeedColse();
                return;
            }

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

            auto protocolCall = protocolCalls.find(protocolId);
            if (protocolCall != protocolCalls.end())
            {
                protocolCall->second(this, protobufData, protobufLen);
            }
            else
            {
                std::cerr << "Unknown protocol: " << protocolId << std::endl;
            }
        }
    }
}

void CPlayer::loginReqHandle(CPlayer*pPlayer, const uint8_t* protobufData, int protobufLen)
{
    loginpackage::LoginRequest req;
    if (req.ParseFromArray(protobufData, protobufLen)) {
        std::cout << std::endl << "[Login]" << std::endl << "user=" << req.username() << std::endl;
        std::cout << "pass=" << req.password() << std::endl;

        loginpackage::LoginResponse res;

        res.set_playerid(pPlayer->_playerID);
        res.set_playermoney(pPlayer->_playerMoney);

        // 計算序列化長度
        size_t ressize = res.ByteSizeLong();

        std::cout << std::endl << std::endl << "playerid=" << res.playerid() << std::endl;
        std::cout << "playermoney=" << res.playermoney() << std::endl;

        std::shared_ptr<uint8_t> resbuffer(new uint8_t[ressize], std::default_delete<uint8_t[]>());

        if (res.SerializeToArray(resbuffer.get(), ressize)) {
            std::vector<uint8_t> sendbuff = makeWebSocketBinaryFrame(Protocol::LoginResponse, resbuffer.get(), ressize);

            pPlayer->_pSesssion->addBuff(sendbuff.data(), sendbuff.size());
        }
    }
}

void CPlayer::StartGameReqHandle(CPlayer* pPlayer, const uint8_t* protobufData, int protobufLen)
{
    gamepackage::StartRequest req;
    if (req.ParseFromArray(protobufData, protobufLen))
    {
        int money = req.playermoney();

        if (money != pPlayer->_playerMoney)
        {
            std::cout << std::endl << "Money不正確" << std::endl;
        }

        auto startEvent = std::make_shared<StartGameEvent>(pPlayer->_playerID);

        CManagerDef::getEventInstance().notifyEvent(EVENT::StartGame, startEvent);
    }
}

void CPlayer::BetRequestReqHandle(CPlayer* pPlayer, const uint8_t* protobufData, int protobufLen)
{
    gamepackage::BetRequest req;
    if (req.ParseFromArray(protobufData, protobufLen))
    {
        int betnum = req.betnum();
        int amount = req.amount();

        if (betnum < 0 || betnum >= 30)
        {
            return;
        }

        if (pPlayer->_playerMoney < amount)
        {
            return;
        }

        pPlayer->_batAmounts[betnum] += amount;
        pPlayer->_playerMoney -= amount;


        gamepackage::BetResponse res;

        res.set_betnum(betnum);
        res.set_amount(pPlayer->_batAmounts[betnum]);
        res.set_playermoney(pPlayer->_playerMoney);


        // 計算序列化長度
        size_t ressize = res.ByteSizeLong();

        std::shared_ptr<uint8_t> resbuffer(new uint8_t[ressize], std::default_delete<uint8_t[]>());

        if (res.SerializeToArray(resbuffer.get(), ressize)) {
            std::vector<uint8_t> sendbuff = makeWebSocketBinaryFrame(Protocol::BetResponse, resbuffer.get(), ressize);

            pPlayer->_pSesssion->addBuff(sendbuff.data(), sendbuff.size());
        }
    }
}

void CPlayer::sendData(std::vector<uint8_t> & sendbuff)
{
    _pSesssion->addBuff(sendbuff.data(), sendbuff.size());
}

void CPlayer::checkWin(int roundWin)
{
    _playerMoney += _batAmounts[roundWin];

    for (int i = 0; i < 30; i++)
    {
        _batAmounts[i] = 0;
    }
}

int CPlayer::getMoney()
{
    return _playerMoney;
}