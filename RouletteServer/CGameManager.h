#pragma once
#include <unordered_map>
#include <memory>
#include <chrono>

class StartGameEvent;
class CPlayer;
class ClientSession;

class CGameManager
{
public:
    friend class CManagerDef;
    

private:
    CGameManager();
    ~CGameManager() = default;


    int _nowGameID = 1;

    std::unordered_map<int, std::shared_ptr<CPlayer>> _players;

    bool _isStartGame = false;

    int _elapsedTime = 0;

    int _cowndownTime = 30;


    void update(std::chrono::milliseconds milliseconds);

    void sendCoolDownSync();
public:
	/// <summary>
	/// 遊戲主要loop
	/// </summary>
	static void gameLoop();

    void init();

    void addPlayer(ClientSession* cs);

    void connectHandle(ClientSession* cs);

    void recvHandle(ClientSession* cs, uint8_t* pBuff, int count);

    void disconnectHandle(ClientSession* pSession);

    void handlePlayerStartGame(const std::shared_ptr<StartGameEvent>& event);
};