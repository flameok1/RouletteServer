#pragma once

#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */
#include <fcntl.h>
#endif

#include <atomic>
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include "ClientSession.h"

#define SELECT_TIMEOUT 10

typedef void (*recvCallBack)(ClientSession* pSesssion, char* pBuff, int recvCount);
typedef std::function<void(ClientSession*, char*, int)> recvCallbackFunction;

typedef void (*disconnectCallBack)(ClientSession* pSesssion);
typedef std::function<void(ClientSession*)> disconnectCallBackFunction;

/// <summary>
/// FLAMEServer ���K�A�Ⱦ�
/// </summary>
class FLAMEServer
{
public:
	FLAMEServer();
	~FLAMEServer();
private:
	/// <summary>
	/// window socket �ݭn��l��wsa
	/// </summary>
	void initWSA();

	/// <summary>
	/// window socket �ݭn���� wsa
	/// </summary>
	void releaseWSA();

	/// <summary>
	/// ����socket
	/// </summary>
	void sockClose();

	/// <summary>
	/// �]�wserver socket non block
	/// </summary>
	bool setSocketBlockingEnabled(SOCKET fd, bool blocking);

	/// <summary>
	/// ��ť�� nonBlock thread
	/// </summary>
	void nonBlockListenThread();

	/// <summary>
	/// ��ť�� block thread
	/// </summary>
	void blockListenThread();

	/// <summary>
	/// ����thread�� flag
	/// </summary>
	std::atomic<bool> shutDownFlag;

	/// <summary>
	/// �B�z session update thread �A�i�Hsend���j��Ƥ��v�T �Dlisten thread
	/// </summary>
	void sessionHandleThread(std::shared_ptr<ClientSession> clientSession);

	/// <summary>
	/// socket�M�Ѽ�
	/// </summary>
	SOCKET _socket;
	int _family;
	int _socktype;
	int _protocol;

	/// <summary>
	/// port
	/// </summary>
	int _port;

	/// <summary>
	/// �ϥ�block �Ҧ�
	/// </summary>
	bool _useBlockSocketServer;

	/// <summary>
	/// �w�g�s�u��client
	/// </summary>
	std::vector<std::shared_ptr<ClientSession>> _clientSockts;

	/// <summary>
	/// ���U��recv call back
	/// </summary>
	recvCallbackFunction _recvCB = nullptr; //void *_recvCB(char *pBuff)

	disconnectCallBackFunction _disconnectCB = nullptr;

protected:
	/// <summary>
	/// ������ƫ� �B�zfunction
	/// </summary>
	virtual void recvHandle(ClientSession *pSession, char* pBuff, int recvCount);

	/// <summary>
	/// �_�u�B�zfunction
	/// </summary>
	virtual void disconnectHandle(ClientSession* pSession);

	/// <summary>
	/// generate client session
	/// </summary>
	virtual std::shared_ptr<ClientSession> makeClientSession(SOCKET skt);
public:
	/// <summary>
	/// �Ұ� server
	/// </summary>
	virtual bool start(int port);

	//�]�w����CB
	void setRecvCB(recvCallBack);

	//�]�w�_�uCB
	void setDisconnectCB(disconnectCallBack disconnectCB);

	/// <summary>
	/// ����server ����
	/// </summary>
	void waitShutDown();

	/// <summary>
	/// Set Family
	/// </summary>
	void setFamily(int family);

	/// <summary>
	/// �]�wsocket type
	/// </summary>
	void setSocketType(int socktype);

	/// <summary>
	/// �]�wsocket protocal
	/// </summary>
	void setProtocol(int protocol);

	/// <summary>
	/// ���block�]�w
	/// </summary>
	void switchBolckMode(bool);
};