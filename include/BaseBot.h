//sgn

#pragma once

#include <iostream>
#include <memory>
#include <map>

#include "BotKeyboards.h"
#include "TelegramBot.h"
#include "DBInterface.h"

class BaseBot {
public:
    typedef std::shared_ptr<BaseBot> Ptr;
    BaseBot() {}
    virtual ~BaseBot() {}

    static const std::map<std::string, BaseBot::Ptr>& getPages() { return mPages; }
    static void addPage(std::string pName, BaseBot::Ptr pPage) {
        if(pName.empty() || !pPage) return;
        mPages[pName]   = pPage;
    }
    static VInfo::VehicleType getLastSetVType(int64_t pChatId) {
        VInfo::VehicleType vType = VInfo::VehicleType::None;
        if(mVType.count(pChatId) > 0) vType = mVType[pChatId];
        return vType;
    }
    static void removePage(std::string pName) { mPages.erase(pName); }
    static void setVType(int64_t pChatId, VInfo::VehicleType pType) { mVType[pChatId] = pType; }

    virtual void                init() {}
    virtual std::string         getMediaId() { return std::string(); }
    virtual std::string         getMsg() = 0;
    virtual std::string         getPageName() = 0;
    virtual GenericReply::Ptr   prepareMenu(BotMessage::Ptr pMsg) = 0;
    virtual void                onClick(BotMessage::Ptr pMsg) = 0;

private:
    static std::map<std::string, BaseBot::Ptr>      mPages;
    static std::map<int64_t, VInfo::VehicleType>    mVType;
};
