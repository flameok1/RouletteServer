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

	/// <summary>
	/// send buff
	/// </summary>
	std::string _sendBuff;

	/// <summary>
	/// recv buff size
	/// </summary>
	int _recvBuffSize;

	/// <summary>
	/// recv buff shared ptr
	/// </summary>
	std::shared_ptr<char> _recvBuff;

protected:
	/// <summary>
	/// ����flag
	/// </summary>
	bool _isNeedClose;
public:
	/// <summary>
	/// ���osocket
	/// </summary>
	SOCKET getSocket();

	/// <summary>
	/// ���orecv buff
	/// </summary>
	char* getRecvBuff();

	/// <summary>
	/// �s�W��ƨ�send buff
	/// </summary>
	void addBuff(const char* pBuff);

	/// <summary>
	/// ���osend buff
	/// </summary>
	std::string& getSendBuff();

	/// <summary>
	/// �M��send buff
	/// </summary>
	void clearSendBuff();

	/// <summary>
	/// ����socket
	/// </summary>
	void closeSession();

	/// <summary>
	/// �����������
	/// </summary>
	virtual int recvData();
	
	/// <summary>
	/// �ǰe�������
	/// </summary>
	virtual int sendData();

	/// <summary>
	/// �O���O�ݭn����
	/// </summary>
	bool isNeedClose();
};
