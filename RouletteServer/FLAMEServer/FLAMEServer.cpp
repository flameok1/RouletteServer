#include "FLAMEServer.h"
#include "stdio.h"
#include <thread>
#include <iostream>
#include "ClientSession.h"

FLAMEServer::FLAMEServer()
	:_socket(INVALID_SOCKET),_family(AF_INET), _socktype(SOCK_STREAM), _protocol(0), _port(0), _recvCB(nullptr), _useBlockSocketServer(false), shutDownFlag(false)
{
    initWSA();
}

FLAMEServer::~FLAMEServer()
{
    releaseWSA();
}


void FLAMEServer::initWSA()
{
#ifdef _WIN32
	// Initialize Winsock for Window
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //會累加ref
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
	}
#endif


}


void FLAMEServer::releaseWSA()
{
#ifdef _WIN32
	WSACleanup(); //會累減ref
#endif
}

void FLAMEServer::sockClose()
{
    int status = 0;

#ifdef _WIN32
    status = shutdown(_socket, SD_BOTH);
    if (status == 0) { status = closesocket(_socket); }
#else
    status = shutdown(_socket, SHUT_RDWR);
    if (status == 0) { status = close(_socket); }
#endif
}


bool FLAMEServer::start(int port)
{
    
    /*
    int iResult;
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    */

    _port = port;

    // Create a SOCKET for the server to listen for client connections.
    _socket = socket(_family, _socktype, _protocol);
    if (_socket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        //freeaddrinfo(result);
        return false;
    }

    if (!_useBlockSocketServer)
    {
        setSocketBlockingEnabled(_socket, false);

        std::thread listenThread(&FLAMEServer::nonBlockListenThread, this);
        listenThread.detach();
    }
    else
    {
        std::thread listenThread(&FLAMEServer::blockListenThread, this);
        listenThread.detach();
    }


    return true;
}

void FLAMEServer::nonBlockListenThread()
{
    int iResult;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    // Setup the TCP listening socket
    iResult = bind(_socket, (struct sockaddr*)&address, (int)sizeof(sockaddr_in));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        //freeaddrinfo(result);
        closesocket(_socket);
        return;
    }

    iResult = listen(_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(_socket);
        return;
    }
    
    fd_set rfd, wfd;
    struct timeval timeout;
    int ret;

    while (true)
    {
        if (shutDownFlag)
        {
            shutDownFlag = false;
            return;
        }

        FD_ZERO(&rfd); //清除read buff
        FD_ZERO(&wfd); //清除write buff
        FD_SET(_socket, &rfd); //每個socket對應ID是不重複的，不會互相覆蓋

        for (auto cs : _clientSockts)
        {
            FD_SET(cs->getSocket(), &rfd);
            FD_SET(cs->getSocket(), &wfd);
        }

        timeout.tv_sec = SELECT_TIMEOUT / 1000;
        timeout.tv_usec = SELECT_TIMEOUT % 1000 * 1000;

#ifdef _WIN32
        //時間參數：如果為NULL會永遠阻塞，直到出現可讀/可寫/異常才會凡回，如果不為NULL代表多久後返回
        //select返回值大於0，表示響應的socket數量，如果等於0超時，如果小於0則代表發生錯誤
        ret = select(0, &rfd, &wfd, NULL, &timeout);
#else
        ret = select(fd + 1, &rfd, &wfd, NULL, &timeout);
#endif
        if (ret <= 0)
            continue;

        if (FD_ISSET(_socket, &rfd))
        {
            SOCKET clientSocket;
            struct sockaddr_in client_address;
            int len = sizeof(client_address);
            clientSocket = accept(_socket, (struct sockaddr*)&client_address, &len);
            if (clientSocket == INVALID_SOCKET) {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(_socket);
                return;
            }
            setSocketBlockingEnabled(clientSocket, false);
            std::shared_ptr<ClientSession> newClient = makeClientSession(clientSocket);
            _clientSockts.push_back(newClient);
            printf("accept a connect\n");

            if (_connectCB != nullptr)
            {
                _connectCB(newClient.get());
            }
        }
        else
        {
            for (int i = 0; i < _clientSockts.size(); i++)
            {
                bool removeFlag = false;
                if (FD_ISSET(_clientSockts[i]->getSocket(), &rfd))
                {
                    int totalRecv = _clientSockts[i]->recvData();
                    
                    //當接收到的數據小於等於0時，代表客戶端斷開連接或發生錯誤
                    if (totalRecv <= 0)
                    {
                        if (totalRecv < 0)
                        {
                            std::cout << "Socket recv data error." << std::endl;
                        }
                        
                        removeFlag = true;
                    }
                    else if (totalRecv > 0 && _clientSockts[i]->getRecvBuff() != nullptr)
                    {
                        recvHandle(_clientSockts[i].get(), _clientSockts[i]->getRecvBuff(), totalRecv);
                    }
                }

                if (FD_ISSET(_clientSockts[i]->getSocket(), &wfd))
                {
                    _clientSockts[i]->sendData();
                }

                if (_clientSockts[i]->isNeedClose())
                {
                    _clientSockts[i]->closeSession();
                    disconnectHandle(_clientSockts[i].get());
                    removeFlag = true;
                }

                if (removeFlag)
                {
                    _clientSockts.erase(_clientSockts.begin() + i);
                    i--;
                }
            }
        }
    }
}

void FLAMEServer::blockListenThread()
{
    int iResult;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    // Setup the TCP listening socket
    iResult = bind(_socket, (struct sockaddr*)&address, (int)sizeof(sockaddr_in));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        //freeaddrinfo(result);
        closesocket(_socket);
        return;
    }

    iResult = listen(_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(_socket);
        return;
    }

    while (true)
    {
        SOCKET clientSocket;
        struct sockaddr_in client_address;
        int len = sizeof(client_address);
        clientSocket = accept(_socket, (struct sockaddr*)&client_address, &len);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(_socket);
            return;
        }

        printf("accept a connect\n");

        setSocketBlockingEnabled(clientSocket, false);

        //create
        std::shared_ptr<ClientSession> newSession = makeClientSession(clientSocket);

        std::thread listenThread(&FLAMEServer::sessionHandleThread, this , newSession);
        listenThread.detach();
    }
}

void FLAMEServer::sessionHandleThread(std::shared_ptr<ClientSession> clientSession)
{
    while (true)
    {
        int totalRecv = clientSession->recvData();

        //當接收到的數據小於等於0時，代表客戶端斷開連接或發生錯誤
        if (totalRecv < 0)
        {
            std::cout << "Socket recv data error." << std::endl;
        }

        if (totalRecv > 0 && clientSession->getRecvBuff() != nullptr)
        {
            recvHandle(clientSession.get(), clientSession->getRecvBuff(), totalRecv);
        }

        clientSession->sendData();

        if (clientSession->isNeedClose())
        {
            clientSession->closeSession();
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(0));
    }
}

std::shared_ptr<ClientSession> FLAMEServer::makeClientSession(SOCKET skt)
{
    return std::make_shared<ClientSession>(skt);
}

bool FLAMEServer::setSocketBlockingEnabled(SOCKET fd, bool blocking)
{
    if (fd < 0) return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0);
#endif
}

void FLAMEServer::setFamily(int family)
{
    _family = family;
}

void FLAMEServer::setSocketType(int socktype)
{
    _socktype = socktype;
}

void FLAMEServer::setProtocol(int protocol)
{
    _protocol = protocol;
}

void FLAMEServer::setConnectCB(connectCallBack connectCB)
{
    _connectCB = connectCB;
}

void FLAMEServer::setRecvCB(recvCallBack recvCB)
{
    _recvCB = recvCB;
}

void FLAMEServer::recvHandle(ClientSession* pSession, uint8_t* pBuff, int recvCount)
{
    if (_recvCB != nullptr)
        _recvCB(pSession, pBuff, recvCount);
}

void FLAMEServer::setDisconnectCB(disconnectCallBack disconnectCB)
{
    _disconnectCB = disconnectCB;
}

void FLAMEServer::disconnectHandle(ClientSession* pSession)
{
    if (_disconnectCB != nullptr)
    {
        _disconnectCB(pSession);
    }
}

void FLAMEServer::waitShutDown()
{
    shutDownFlag = true;

    while (true)
    {
        if (!shutDownFlag || _useBlockSocketServer)
            break;

        std::this_thread::sleep_for(std::chrono::seconds(0));
    }

    sockClose();
}

void FLAMEServer::switchBolckMode(bool set)
{
    _useBlockSocketServer = set;
}