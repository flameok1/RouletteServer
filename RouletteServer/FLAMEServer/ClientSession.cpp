#include "ClientSession.h"



ClientSession::ClientSession(SOCKET skt)
    : _clientSocket(skt)
{

}

SOCKET ClientSession::getSocket()
{
    return _clientSocket;
}

void ClientSession::addBuff(const uint8_t* buff, int size)
{
    if (_nowSendCount + size > _sendBuffSize)
    {
        //擴大buff
        std::shared_ptr<uint8_t> newBuff(new uint8_t[_sendBuffSize + RECV_BUFF_SIZE], std::default_delete<uint8_t[]>());
        if (_nowSendCount > 0)
        {
            memcpy(newBuff.get(), _sendBuff.get(), _nowSendCount);
        }
        _sendBuffSize += RECV_BUFF_SIZE;
        _sendBuff = newBuff;
    }
    
    memcpy((_sendBuff.get() + _nowSendCount), buff, size);
    _nowSendCount += size;
}

uint8_t* ClientSession::getSendBuff()
{
    return _sendBuff.get();
}

uint8_t* ClientSession::getRecvBuff()
{
    if (_recvBuff != nullptr)
    {
        return _recvBuff.get();
    }

    return nullptr;
}

void ClientSession::clearSendBuff()
{
    _nowSendCount = 0;
}

void ClientSession::closeSession()
{
    closesocket(_clientSocket);
}

int ClientSession::recvData()
{
    char recvBuffer[RECV_BUFF_SIZE]{ 0 };
    int totalRecv = 0;
    int nRecv = 0;

    while (true)
    {
        nRecv = recv(_clientSocket, recvBuffer, RECV_BUFF_SIZE, 0);

        //當接收到的數據小於等於0時，代表客戶端斷開連接或發生錯誤
        if (nRecv < 0)
        {
            return nRecv;
        }

        if (0 == nRecv)
        {
            return nRecv;
        }

        if (_recvBuffSize < totalRecv + nRecv)
        {
            //擴大buff
            std::shared_ptr<uint8_t> newBuff(new uint8_t[_recvBuffSize + RECV_BUFF_SIZE], std::default_delete<uint8_t[]>());
            if (totalRecv > 0)
            {
                memcpy(newBuff.get(), _recvBuff.get(), totalRecv);
            }
            _recvBuffSize += RECV_BUFF_SIZE;
            _recvBuff = newBuff;
        }
        
        memcpy((_recvBuff.get() + totalRecv), recvBuffer, nRecv);

        totalRecv += nRecv;

        if (nRecv < RECV_BUFF_SIZE)
        {
            *(_recvBuff.get() + totalRecv) = '\0';
            break;
        }
    }

    return totalRecv;
}

int ClientSession::sendData()
{
    if (_nowSendCount > 0)
    {
        int nsend = send(_clientSocket, (char *)_sendBuff.get(), (int)_nowSendCount, 0);
        printf("\nsend %d byte.\n", nsend);
        clearSendBuff();

        return nsend;
    }

    return 0;
}

bool ClientSession::isNeedClose()
{
    return _isNeedClose;
}

void ClientSession::setNeedColse()
{
    _isNeedClose = true;
}