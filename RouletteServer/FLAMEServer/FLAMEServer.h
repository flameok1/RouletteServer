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
/// FLAMEServer 火焰服務器
/// </summary>
class FLAMEServer
{
public:
	FLAMEServer();
	~FLAMEServer();
private:
	/// <summary>
	/// window socket 需要初始化wsa
	/// </summary>
	void initWSA();

	/// <summary>
	/// window socket 需要釋放 wsa
	/// </summary>
	void releaseWSA();

	/// <summary>
	/// 關閉socket
	/// </summary>
	void sockClose();

	/// <summary>
	/// 設定server socket non block
	/// </summary>
	bool setSocketBlockingEnabled(SOCKET fd, bool blocking);

	/// <summary>
	/// 偵聽用 nonBlock thread
	/// </summary>
	void nonBlockListenThread();

	/// <summary>
	/// 偵聽用 block thread
	/// </summary>
	void blockListenThread();

	/// <summary>
	/// 停止thread用 flag
	/// </summary>
	std::atomic<bool> shutDownFlag;

	/// <summary>
	/// 處理 session update thread ，可以send較大資料不影響 主listen thread
	/// </summary>
	void sessionHandleThread(std::shared_ptr<ClientSession> clientSession);

	/// <summary>
	/// socket和參數
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
	/// 使用block 模式
	/// </summary>
	bool _useBlockSocketServer;

	/// <summary>
	/// 已經連線的client
	/// </summary>
	std::vector<std::shared_ptr<ClientSession>> _clientSockts;

	/// <summary>
	/// 註冊的recv call back
	/// </summary>
	recvCallbackFunction _recvCB = nullptr; //void *_recvCB(char *pBuff)

	disconnectCallBackFunction _disconnectCB = nullptr;

protected:
	/// <summary>
	/// 接收資料後 處理function
	/// </summary>
	virtual void recvHandle(ClientSession *pSession, char* pBuff, int recvCount);

	/// <summary>
	/// 斷線處理function
	/// </summary>
	virtual void disconnectHandle(ClientSession* pSession);

	/// <summary>
	/// generate client session
	/// </summary>
	virtual std::shared_ptr<ClientSession> makeClientSession(SOCKET skt);
public:
	/// <summary>
	/// 啟動 server
	/// </summary>
	virtual bool start(int port);

	//設定接收CB
	void setRecvCB(recvCallBack);

	//設定斷線CB
	void setDisconnectCB(disconnectCallBack disconnectCB);

	/// <summary>
	/// 等待server 關閉
	/// </summary>
	void waitShutDown();

	/// <summary>
	/// Set Family
	/// </summary>
	void setFamily(int family);

	/// <summary>
	/// 設定socket type
	/// </summary>
	void setSocketType(int socktype);

	/// <summary>
	/// 設定socket protocal
	/// </summary>
	void setProtocol(int protocol);

	/// <summary>
	/// 更改block設定
	/// </summary>
	void switchBolckMode(bool);
};