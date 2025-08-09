#pragma once

#include <memory>
#include "FLAMEServer/ClientSession.h"

/// <summary>
/// 玩家Class，會在連線後建立
/// </summary>
class CPlayer
{
public:
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

	bool isWebSocketUpgrade = false;

public:

	void reciveHandle(ClientSession* cs, uint8_t* pBuff, int conut);
};