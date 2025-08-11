#pragma once


/// <summary>
/// ManagerDef不用實體，但需要class，要不然不能用friend，建構式開方就不能唯一
/// </summary>
class CEventManager;
class CGameManager;

class CManagerDef
{
public:
    static CEventManager& getEventInstance();
    static CGameManager& getGameManagerInstance();
};




