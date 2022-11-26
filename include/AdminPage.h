//sgn
#pragma once
#include <iostream>
#include <memory>
#include <map>

#include "TelegramBot.h"
#include "BaseBot.h"
#include "BotKeyboards.h"
#include "DBInterface.h"
#include "Logger.h"

class AdminPage : public BaseBot, public std::enable_shared_from_this<AdminPage> {

private:
    const Logger&       mLogger;
    bool                mIsAdmin;
    std::string         mRespMsg;
    VInfo::VehicleType  mVehicle;
    DBInterface::Ptr    mpDB;

    BaseBot::Ptr        getSharedPtr() {return shared_from_this();}
public:
    AdminPage(DBInterface::Ptr pDB)
        : mLogger(Logger::getInstance())
        , mIsAdmin(false)
        , mVehicle(VInfo::VehicleType::None)
        , mpDB(pDB)
        {}
    virtual ~AdminPage() {}

    void                init();
    std::string         getMsg();
    std::string         getPageName() { return "Admin Page"; }
    void                onClick(BotMessage::Ptr pMsg);
    GenericReply::Ptr   prepareMenu(BotMessage::Ptr pMsg); };
