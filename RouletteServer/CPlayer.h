#pragma once
#include <memory>
#include <vector>

class ClientSession;

/// <summary>
/// 玩家Class，會在連線後建立
/// </summary>
class CPlayer
{
public:
	CPlayer(int id, ClientSession *pSesssion);
private:
	/// <summary>
	/// recv buff size
	/// </summary>
	int _recvBuffSize = 0;

	/// <summary>
	/// recv buff shared ptr
	/// </summary>
	std::shared_ptr<uint8_t> _recvBuff;

	/// <summary>
	/// 現在存放資料數量
	/// </summary>
	size_t _nowDataCount = 0;

	/// <summary>
	/// ClientSession
	/// </summary>
	ClientSession* _pSesssion = nullptr;

	bool isWebSocketUpgrade = false;

	/// <summary>
	/// 暫時用socket當ID，目前沒用資料庫
	/// </summary>
	int _playerID = 0;
	int _playerMoney = 10000;
public:

	void reciveHandle(ClientSession* cs, uint8_t* pBuff, int conut);

	void sendData(std::vector<uint8_t>& sendBuff);
};