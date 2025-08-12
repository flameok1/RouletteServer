#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

class ClientSession;
class CPlayer;

typedef std::function<void(CPlayer*,const uint8_t*, int)> protocalHandleFunction;

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

	/// <summary>
	/// http升級成websocket flag
	/// </summary>
	bool isWebSocketUpgrade = false;



	/// <summary>
	/// 暫時用socket當ID，目前沒用資料庫
	/// </summary>
	int _playerID = 0;
	int _playerMoney = 10000;

	int _batAmounts[30] = { 0 };

	static std::unordered_map<int, protocalHandleFunction> protocolCalls;

	static void loginReqHandle(CPlayer* pPlayer,  const uint8_t* protobufData, int protobufLen);
	static void StartGameReqHandle(CPlayer* pPlayer, const uint8_t* protobufData, int protobufLen);
	static void BetRequestReqHandle(CPlayer* pPlayer, const uint8_t* protobufData, int protobufLen);
public:

	void reciveHandle(ClientSession* cs, uint8_t* pBuff, int conut);

	void sendData(std::vector<uint8_t>& sendBuff);
};