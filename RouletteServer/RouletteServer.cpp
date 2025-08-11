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



int main()
{
    FLAMEServer fserver;

    CManagerDef::getGameManagerInstance().init();

    fserver.setConnectCB([](ClientSession* cs) {
            CManagerDef::getGameManagerInstance().connectHandle(cs);
        });
    fserver.setRecvCB([](ClientSession* pSesssion, uint8_t* pBuff, int recvCount) {
        CManagerDef::getGameManagerInstance().recvHandle(pSesssion, pBuff, recvCount);
        });
    fserver.setDisconnectCB([](ClientSession* pSesssion) {
        CManagerDef::getGameManagerInstance().disconnectHandle(pSesssion);
        });

    fserver.start(8888);

    std::thread gameUpdate(CManagerDef::getGameManagerInstance().gameLoop);


    gameUpdate.join();

    fserver.waitShutDown();

    return 0;
}
