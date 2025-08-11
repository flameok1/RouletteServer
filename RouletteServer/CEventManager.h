#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

class IEvent;

class IEventManager
{
public:
    using EventHandler = std::function<void(const std::shared_ptr<IEvent>&)>;

    virtual void registerNotify(int eventType, EventHandler handler) = 0;

    virtual void notifyEvent(int eventType, const std::shared_ptr<IEvent>& event) = 0;
};

class CEventManager : public IEventManager
{
public:
    friend class CManagerDef;

private:
    CEventManager();

    ~CEventManager() = default;

    std::unordered_map<int, std::vector<EventHandler>> handlers;

public:

    void registerNotify(int eventType, EventHandler handler);


    void notifyEvent(int eventType, const std::shared_ptr<IEvent>& event);
};