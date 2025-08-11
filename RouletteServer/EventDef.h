#pragma once

#include "EventDef.h"

enum EVENT
{
    StartGame = 1
};

class IEvent
{
public:
    virtual ~IEvent() = default;
};

class StartGameEvent : public IEvent {
public:
    int playerId;

    StartGameEvent(int Id)
        : playerId(Id) {
    }
};