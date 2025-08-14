#pragma once
#include <unordered_map>
#include <memory>
#include <chrono>

class StartGameEvent;
class CPlayer;
class ClientSession;

enum GAME_STEP {
    STOP,
    COUNTDOWN,
    WIN,
    NEXT_ROUND,
};

const int ROUNT_TIME = 30;

const int NEXT_ROUNT_WAIT = 5;

class CGameManager
{
public:
    friend class CManagerDef;
    

private:
    CGameManager();
    ~CGameManager() = default;


    int _nowGameID = 1;

    std::unordered_map<int, std::shared_ptr<CPlayer>> _players;

    int _elapsedTime = 0;

    int _nowRound = 1;

    int _countdownTime = ROUNT_TIME;

    int _gameStep = GAME_STEP::STOP;

    void update(std::chrono::milliseconds milliseconds);

    void calcuWin();

    void sendCoolDownSync();

    void sendBetResult(int);
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