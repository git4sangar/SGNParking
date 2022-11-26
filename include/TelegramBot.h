//sgn

#pragma once

#include <iostream>
#include <memory>

#include "Logger.h"
#include "HttpClient.h"
#include "BotKeyboards.h"

struct BotMessage {
    typedef std::shared_ptr<BotMessage> Ptr;

    BotMessage() : chat_id(0) {}
    virtual ~BotMessage() {}

    std::string name;
    std::string text;
    int64_t chat_id;
    int32_t vehicle_no;
};

class TelegramBot : public std::enable_shared_from_this<TelegramBot>, public HttpResponse {
    public:
        enum Style      { HTML = 0, Markdown, MarkdownV2 };
        typedef std::shared_ptr<TelegramBot> Ptr;
        TelegramBot()
            : mLogger(Logger::getInstance())
            , mpHttpClient(nullptr) {}

        void                init(std::string pCAFile);
        int                 sendMessage(int64_t pChatId, std::string pMsg, GenericReply::Ptr pKeyboard = nullptr, Style pStyle = Style::HTML);
        void                sendPhoto(int64_t pChatId, std::string pFileId);
        void                startGettingUpdates(int64_t pOffset, int32_t pLimit);
        BotMessage::Ptr     parseMessage(const json& jsonMsg);
        void                onMessage(BotMessage::Ptr pMsg);
        int32_t             getVehicleNo(const std::string& pText);

        void onHttpSuccess(const std::string& pResp);
        void onHttpFailure(const std::string& pError) {}

    private:
        std::string     mToken;
        Logger&         mLogger;
        HttpClient::Ptr mpHttpClient;

        uint64_t            getUpdateIdManually(const std::string& pResp);
        std::string         getStyleString(Style pStyle);
        TelegramBot::Ptr    getSharedPtr() {return shared_from_this();}
};
