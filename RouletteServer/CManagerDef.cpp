#include "EventDef.h"
#include "CManagerDef.h"
#include "CEventManager.h"
#include "CGameManager.h"


CEventManager& CManagerDef::getEventInstance() {
    static CEventManager instance;
    return instance;
}

CGameManager& CManagerDef::getGameManagerInstance() {
    static CGameManager instance;
    return instance;
}