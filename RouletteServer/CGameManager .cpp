#include "CGameManager.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include "CManagerDef.h"
#include "CEventManager.h"
#include "EventDef.h"

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

void CGameManager::update()
{
    while (true) {
        // 開始時間
        auto frameStartTime = std::chrono::steady_clock::now();

        //固定更新時間

        // 經過時間
        auto frameEndTime = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(frameEndTime - frameStartTime);

        // wait time
        if (elapsed_time < FRAME_DURATION) {
            std::this_thread::sleep_for(FRAME_DURATION - elapsed_time);
        }
    }
}

void CGameManager::handlePlayerStartGame(const std::shared_ptr<StartGameEvent>& event) {
    if (playerIds.count(event->playerId) == 0)
    {
        playerIds.insert(event->playerId);
    }

    std::cout << "Game Start. " << event->playerId << std::endl;
}