#pragma once
#include <unordered_set>
#include <memory>

class StartGameEvent;

class CGameManager
{
public:
    friend class CManagerDef;
    

private:
    CGameManager();
    ~CGameManager() = default;


    int nowGameID = 1;
    std::unordered_set<int> playerIds;

public:
	static void update();

    void init();

    void handlePlayerStartGame(const std::shared_ptr<StartGameEvent>& event);
};