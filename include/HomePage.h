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

class HomePage : public BaseBot, public std::enable_shared_from_this<HomePage> {
    const Logger&       mLogger;
    bool                mIsAdmin;
    DBInterface::Ptr    mpDB;

    BaseBot::Ptr    getSharedPtr() {return shared_from_this();}

public:
    HomePage(DBInterface::Ptr pDB) : mLogger(Logger::getInstance()), mpDB(pDB) {}
    virtual ~HomePage() {}

    void                init();
    std::string         getMsg();
    std::string         getPageName() { return "Home Page"; }
    void                onClick(BotMessage::Ptr pMsg);
    GenericReply::Ptr   prepareMenu(BotMessage::Ptr pMsg);    
};
