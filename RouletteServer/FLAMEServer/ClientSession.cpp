#include "ClientSession.h"



ClientSession::ClientSession(SOCKET skt)
    : _clientSocket(skt), _sendBuff(""), _recvBuffSize(0), _recvBuff(nullptr), _isNeedClose(false)
{

}

SOCKET ClientSession::getSocket()
{
    return _clientSocket;
}

void ClientSession::addBuff(const char* pBuff)
{
    _sendBuff += std::string(pBuff);
}

std::string& ClientSession::getSendBuff()
{
    return _sendBuff;
}

char* ClientSession::getRecvBuff()
{
    if (_recvBuff != nullptr)
    {
        return _recvBuff.get();
    }

    return nullptr;
}

void ClientSession::clearSendBuff()
{
    _sendBuff = "";
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
        nRecv = recv(_clientSocket, recvBuffer, 1024, 0);

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
            std::shared_ptr<char> newBuff(new char[_recvBuffSize + RECV_BUFF_SIZE], std::default_delete<char[]>());
            if (_recvBuffSize > 0)
            {
                memcpy(newBuff.get(), _recvBuff.get(), _recvBuffSize);
            }
            _recvBuffSize += RECV_BUFF_SIZE;
            _recvBuff = newBuff;

            memcpy((_recvBuff.get() + totalRecv), recvBuffer, nRecv);
        }

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
    if (_sendBuff.length() > 0)
    {
        int nsend = send(_clientSocket, _sendBuff.c_str(), (int)_sendBuff.length(), 0);
        printf("send %d byte.\n", nsend);
        clearSendBuff();

        return nsend;
    }

    return 0;
}

bool ClientSession::isNeedClose()
{
    return _isNeedClose;
}