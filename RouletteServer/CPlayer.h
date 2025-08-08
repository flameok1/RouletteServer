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
	int _recvBuffSize;

	/// <summary>
	/// recv buff shared ptr
	/// </summary>
	std::shared_ptr<char> _recvBuff;

	bool isWebSocketUpgrade = false;

public:

	void reciveHandle(ClientSession* cs, char* pBuff, int conut);
};