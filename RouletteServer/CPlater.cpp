#include "CPlayer.h"
#include "CHttpParser.h"
#include <iostream>



#define RECV_BUFF_SIZE 1024

void CPlayer::reciveHandle(ClientSession* cs, char* pBuff, int count)
{
    if (_recvBuffSize < count)
    {
        std::shared_ptr<char> newBuff(new char[_recvBuffSize + RECV_BUFF_SIZE], std::default_delete<char[]>());
        _recvBuffSize += RECV_BUFF_SIZE;
        _recvBuff = newBuff;
    }

    memcpy((_recvBuff.get()), pBuff, count);


    if (!isWebSocketUpgrade)
    {
        //check http
        CHttpParser httpParser;
        httpParser.httpParser(pBuff, count);

        bool connection = httpParser.checkKeyBalue("Connection", "Upgrade");
        bool upgrade = httpParser.checkKeyBalue("Upgrade", "websocket");
        std::string webSocketKey = httpParser.getValue("Sec-WebSocket-Key");

        std::string accept = httpParser.generateWebSocketAccept(webSocketKey);

        std::string sendstr = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

        sendstr = sendstr + accept + "\r\n\r\n";

        cs->addBuff(sendstr.c_str());
    }
    else
    {

    }
}