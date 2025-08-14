#include "CGameManager.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include "ws_frame_tool.h"
#include"protocol.h"
#include "FLAMEServer/ClientSession.h"
#include "proto/game.pb.h"
#include "CManagerDef.h"
#include "CEventManager.h"
#include "EventDef.h"
#include "CPlayer.h"
#include "CRandomSys.h"

// fps
const int TARGET_FPS = 60;
// max update time
const std::chrono::milliseconds FRAME_DURATION(1000 / TARGET_FPS);

CGameManager::CGameManager()
{
    CManagerDef::getEventInstance().registerNotify(EVENT::StartGame, [this](const std::shared_ptr<IEvent>& event) {
        this->handlePlayerStartGame(std::static_pointer_cast<StartGameEvent>(event));
        });
}

void CGameManager::init()
{

}

void CGameManager::addPlayer(ClientSession* pSession)
{
    if (_players.count(pSession->getSocket()) != 0)
    {
        return;
    }

    std::shared_ptr<CPlayer> newPlayer = std::make_shared<CPlayer>(pSession->getSocket(), pSession);

    _players.emplace(pSession->getSocket(), newPlayer);
}

void CGameManager::connectHandle(ClientSession* pSession)
{
    CManagerDef::getGameManagerInstance().addPlayer(pSession);
}

void CGameManager::recvHandle(ClientSession* pSession, uint8_t* pBuff, int count)
{
    auto PairPlayer = _players.find(pSession->getSocket());
    if (PairPlayer == _players.end())
    {
        return;
    }

    std::cout << pBuff;

    PairPlayer->second->reciveHandle(pSession, pBuff, count);

}

void CGameManager::disconnectHandle(ClientSession* pSession)
{
    if (_players.count(pSession->getSocket()) == 0)
    {
        return;
    }

    _players.erase(pSession->getSocket());
}

void CGameManager::gameLoop()
{
    while (true) {
        // 開始時間
        auto frameStartTime = std::chrono::steady_clock::now();

        //固定更新時間
        CManagerDef::getGameManagerInstance().update(FRAME_DURATION);

        // 經過時間
        auto frameEndTime = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(frameEndTime - frameStartTime);

        // wait time
        if (elapsed_time < FRAME_DURATION) {
            std::this_thread::sleep_for(FRAME_DURATION - elapsed_time);
        }
    }
}

void CGameManager::update(std::chrono::milliseconds milliseconds)
{
    _elapsedTime += milliseconds.count();

    if (_elapsedTime >= 1000)
    {
        _elapsedTime -= 1000;
        _countdownTime--;
        if (_countdownTime < 0)
        {
            _countdownTime = 0;
        }


        switch (_gameStep)
        {
        case GAME_STEP::STOP:
            break;
        case GAME_STEP::COUNTDOWN:
            sendCoolDownSync();
            break;
        case GAME_STEP::WIN:
            calcuWin();
            break;
        case GAME_STEP::NEXT_ROUND:
            _nowRound++;
            _countdownTime = ROUNT_TIME;
            _gameStep = GAME_STEP::COUNTDOWN;
            break;
        }
        
    }
}

void CGameManager::calcuWin()
{
    CRandomSys rand(0, 29);

    int winNum = rand.GetRandom();

    for (auto &keyplayer : _players)
    {
        keyplayer.second->checkWin(winNum);
    }

    sendBetResult(winNum);
}

void CGameManager::sendCoolDownSync()
{
    gamepackage::CountDownSync CDsync;

    CDsync.set_round(_nowRound);
    CDsync.set_countdown(_countdownTime);
    

    // 計算序列化長度
    auto ressize = CDsync.ByteSizeLong();

    std::shared_ptr<uint8_t> resbuffer(new uint8_t[ressize], std::default_delete<uint8_t[]>());

    if (CDsync.SerializeToArray(resbuffer.get(), ressize)) {
        std::vector<uint8_t> sendbuff = makeWebSocketBinaryFrame(Protocol::CountDownSync, resbuffer.get(), ressize);

        for (const auto& [key, value] : _players) {
            value->sendData(sendbuff);
        }
    }

    if (_countdownTime == 0)
    {
        _gameStep = GAME_STEP::WIN;
    }
}

void CGameManager::sendBetResult(int winNum)
{
    gamepackage::BetResult betResult;

    betResult.set_winnum(winNum);

    // 計算序列化長度
    size_t nowRessize = 10;
    size_t playerRessize = 0;

    std::shared_ptr<uint8_t> resbuffer(new uint8_t[nowRessize], std::default_delete<uint8_t[]>());

    for (const auto& [key, value] : _players) {
        betResult.set_playermoney(value->getMoney());

        playerRessize = betResult.ByteSizeLong();

        if (playerRessize > nowRessize)
        {
            nowRessize = playerRessize;

            resbuffer.reset(new uint8_t[nowRessize], std::default_delete<uint8_t[]>());
        }

        if (betResult.SerializeToArray(resbuffer.get(), playerRessize)) {
            std::vector<uint8_t> sendbuff = makeWebSocketBinaryFrame(Protocol::GameResult, resbuffer.get(), playerRessize);

            value->sendData(sendbuff);
        }
    }

    _gameStep = GAME_STEP::NEXT_ROUND;

    _countdownTime = NEXT_ROUNT_WAIT;
}

void CGameManager::handlePlayerStartGame(const std::shared_ptr<StartGameEvent>& event) {

    if (_gameStep == GAME_STEP::STOP)
    {
        std::cout << std::endl << "Game Start. " << std::endl;
        _gameStep = GAME_STEP::COUNTDOWN;
        _countdownTime = ROUNT_TIME;
    }
}