#include "EventDef.h"
#include "CManagerDef.h"
#include "CEventManager.h"
#include "CGameManager.h"

CEventManager::CEventManager() {
}

void CEventManager::registerNotify(int eventType, EventHandler handler) {
    handlers[eventType].push_back(handler);
}


void CEventManager::notifyEvent(int eventType, const std::shared_ptr<IEvent>& event) {
    auto it = handlers.find(eventType);
    if (it != handlers.end()) {
        for (const auto& handler : it->second) {
            handler(event);
        }
    }
}