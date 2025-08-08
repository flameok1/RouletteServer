// RouletteServer.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include "FLAMEServer/FLAMEServer.h"


void recvHandle(ClientSession* cs, char* pBuff, int conut)
{

}

void disconnectHandle(ClientSession* cs)
{

}

int main()
{
    FLAMEServer fserver;

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
