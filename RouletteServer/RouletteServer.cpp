// RouletteServer.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include "FLAMEServer/FLAMEServer.h"
#include "CPlayer.h"
#include <unordered_map>
#include <memory>

std::unordered_map<SOCKET, std::shared_ptr<CPlayer>> Players;

void connectHandle(ClientSession* cs)
{
    if (Players.count(cs->getSocket()) != 0)
    {
        return;
    }

    std::shared_ptr<CPlayer> newPlayer = std::make_shared<CPlayer>();

    Players.emplace(cs->getSocket(), newPlayer);
}

void recvHandle(ClientSession* cs, char* pBuff, int count)
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

    fserver.setConnectCB(connectHandle);
    fserver.setRecvCB(recvHandle);
    fserver.setDisconnectCB(disconnectHandle);

    fserver.start(8888);

    int exitcode = -1;
    std::cout << "Enter 0 to exit." << std::endl;
    std::cin >> exitcode;
    while (exitcode != 0)
    {
        std::cin >> exitcode;
    }

    fserver.waitShutDown();

    return 0;
}
