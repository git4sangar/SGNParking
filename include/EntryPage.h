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

class EntryPage : public BaseBot, public std::enable_shared_from_this<EntryPage> {

private:
    const Logger&       mLogger;
    bool                mIsAdmin;
    std::string         mRespMsg;
    DBInterface::Ptr    mpDB;

    BaseBot::Ptr        getSharedPtr() {return shared_from_this();}
    VInfo::VehicleType  getVehicle(const std::string& pVehicle);
public:
    EntryPage(DBInterface::Ptr pDB)
        : mLogger(Logger::getInstance())
        , mIsAdmin(false)
        , mpDB(pDB)
        {}
    virtual ~EntryPage() {}

    void                init();
    std::string         getMsg();
    std::string         getPageName() { return "Entry Page"; }
    void                onClick(BotMessage::Ptr pMsg);
    GenericReply::Ptr   prepareMenu(BotMessage::Ptr pMsg);
};

