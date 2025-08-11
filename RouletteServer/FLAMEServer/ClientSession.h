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

#include <vector>
#include <functional>
#include <memory>
#include <string>

#define RECV_BUFF_SIZE 1024

class ClientSession
{
public:
	ClientSession(SOCKET skt);
private:
	/// <summary>
	/// client socket
	/// </summary>
	SOCKET _clientSocket;

	int _nowSendCount = 0;
	int _sendBuffSize = 0;

	/// <summary>
	/// send buff
	/// </summary>
	std::shared_ptr<uint8_t> _sendBuff = nullptr;

	/// <summary>
	/// recv buff size
	/// </summary>
	int _recvBuffSize = 0;

	/// <summary>
	/// recv buff shared ptr
	/// </summary>
	std::shared_ptr<uint8_t> _recvBuff = nullptr;

protected:
	/// <summary>
	/// 關閉flag
	/// </summary>
	bool _isNeedClose = false;
public:
	/// <summary>
	/// 取得socket
	/// </summary>
	SOCKET getSocket();

	/// <summary>
	/// 取得recv buff
	/// </summary>
	uint8_t* getRecvBuff();

	/// <summary>
	/// 新增資料到send buff
	/// </summary>
	void addBuff(const uint8_t*, int);

	/// <summary>
	/// 取得send buff
	/// </summary>
	uint8_t* getSendBuff();

	/// <summary>
	/// 清除send buff
	/// </summary>
	void clearSendBuff();

	/// <summary>
	/// 關閉socket
	/// </summary>
	void closeSession();

	/// <summary>
	/// 接收網路資料
	/// </summary>
	virtual int recvData();
	
	/// <summary>
	/// 傳送網路資料
	/// </summary>
	virtual int sendData();

	/// <summary>
	/// 是不是需要關閉
	/// </summary>
	bool isNeedClose();

	/// <summary>
	/// 設定關閉flag
	/// </summary>
	void setNeedColse();
};
