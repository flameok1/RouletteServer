// RouletteServer.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <unordered_map>
#include <memory>
#include <thread>

#include "FLAMEServer/FLAMEServer.h"
#include "FLAMEServer/ClientSession.h"
#include "CPlayer.h"
#include "CGameManager.h"
#include "CEventManager.h"
#include "CManagerDef.h"



std::unordered_map<SOCKET, std::shared_ptr<CPlayer>> Players;

void connectHandle(ClientSession* cs)
{
    if (Players.count(cs->getSocket()) != 0)
    {
        return;
    }

    std::shared_ptr<CPlayer> newPlayer = std::make_shared<CPlayer>(cs->getSocket());

    Players.emplace(cs->getSocket(), newPlayer);
}

void recvHandle(ClientSession* cs, uint8_t* pBuff, int count)
{
    auto PairPlayer = Players.find(cs->getSocket());
    if (PairPlayer == Players.end())
    {
        return;
    }

    std::cout << pBuff;

    PairPlayer->second->reciveHandle(cs, pBuff, count);
    
}

void disconnectHandle(ClientSession* cs)
{

}

int main()
{
    FLAMEServer fserver;

    CManagerDef::getGameManagerInstance().init();

    fserver.setConnectCB(connectHandle);
    fserver.setRecvCB(recvHandle);
    fserver.setDisconnectCB(disconnectHandle);

    fserver.start(8888);

    std::thread gameUpdate(CManagerDef::getGameManagerInstance().update);


    gameUpdate.join();

    fserver.waitShutDown();

    return 0;
}
