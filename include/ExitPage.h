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

class ExitPage : public BaseBot, public std::enable_shared_from_this<ExitPage> {
    const Logger&       mLogger;
    bool                mIsAdmin;
    std::string         mRespMsg, mIconId;
    DBInterface::Ptr    mpDB;

    std::map<std::string, std::string> mIconMap;
    BaseBot::Ptr    getSharedPtr() {return shared_from_this();}
public:
    ExitPage(DBInterface::Ptr pDB)
        : mLogger(Logger::getInstance())
        , mIsAdmin(false)
        , mpDB(pDB)
        {}
    virtual ~ExitPage() {}

    void                init();
    std::string         getMsg();
    std::string         getMediaId();
    std::string         getPageName() { return "Exit Page"; }
    void                onClick(BotMessage::Ptr pMsg);
    GenericReply::Ptr   prepareMenu(BotMessage::Ptr pMsg);
};

