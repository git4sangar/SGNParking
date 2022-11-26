//sgn

#include <iostream>
#include <memory>
#include <unistd.h>
#include <map>
#include "nlohmann_json.hpp"

//  socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Constants.h"
#include "BotKeyboards.h"
#include "Logger.h"
#include "HomePage.h"
#include "HttpClient.h"
#include "TelegramBot.h"

using json = nlohmann::json;

void petWatchDog() {
   int sockfd = 0;
   struct hostent *he;
   sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   struct sockaddr_in their_addr;

   he                       = gethostbyname("localhost");
   their_addr.sin_family    = AF_INET;
   their_addr.sin_port      = htons(WATCHDOG_PORT);
   their_addr.sin_addr      = *((struct in_addr *)he->h_addr);
   bzero(&(their_addr.sin_zero), 8);
   sendto(sockfd, BIN_FILE_PATH, strlen(BIN_FILE_PATH)+1, 0,
             (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
   close(sockfd);
}

void TelegramBot::init(std::string strCAFile) {
    petWatchDog();
    mpHttpClient    = std::make_shared<HttpClient>(strCAFile);
    mpHttpClient->subscribeListener(getSharedPtr());
}

int32_t TelegramBot::getVehicleNo(const std::string& pText) {
    int32_t vehicleNo   = -1;
    if(pText.length() <= MAX_VEHICLE_NO_LENGTH) {
        try {vehicleNo = std::stoi(pText); }
        catch(std::exception &e) { vehicleNo = -1; }
    }
    return vehicleNo;
}

void TelegramBot::onMessage(BotMessage::Ptr pMsg) {
    pMsg->vehicle_no    = getVehicleNo(pMsg->text);
    const auto& allPages= BaseBot::getPages();
    BaseBot::Ptr pBot   = allPages.at(HOME_PAGE);

    //  Find the appropriate page
    std::map<std::string, BaseBot::Ptr>::const_iterator itrPage, itrTextPage, itrChatIdPage;
    itrPage     = itrTextPage  = itrChatIdPage = allPages.end();
    itrTextPage = allPages.find(pMsg->text);
    if(pMsg->vehicle_no > 0)
        itrChatIdPage   = allPages.find(std::to_string(pMsg->chat_id));

         if(itrTextPage     != allPages.end()) itrPage = itrTextPage;
    else if(itrChatIdPage   != allPages.end()) itrPage = itrChatIdPage;
    
    if(itrPage != allPages.end()) {
        pBot = itrPage->second;
        mLogger << "Found page " << pBot->getPageName() << std::endl;
        pBot->init();
        pBot->onClick(pMsg);
        auto pKeyboard  = pBot->prepareMenu(pMsg);

        auto respMsg    = pBot->getMsg();
        sendMessage(pMsg->chat_id, respMsg, pKeyboard);

        auto mediaId    = pBot->getMediaId();
        sendPhoto(pMsg->chat_id, mediaId);
    } else {
        sendMessage(pMsg->chat_id, "Unknown Cmd", pBot->prepareMenu(pMsg));
        mLogger << "No page for : " << pMsg->text << std::endl;
    }
}

int TelegramBot::sendMessage(int64_t pChatId, std::string pMsg, GenericReply::Ptr pKeyboard, Style pStyle) {
	json jsKeyboard, jsMsg;
    std::vector<std::string> headers;
	std::string strUrl	= std::string(TELEGRAM_BOT_BASE_URL) + std::string(BOT_TOKEN) + "/sendMessage";
	jsMsg["chat_id"]	= pChatId;
	jsMsg["text"]		= pMsg;
	jsMsg["parse_mode"]	= getStyleString(pStyle);
	jsMsg["resize_keyboard"] = true;
	if(pKeyboard) {
		jsMsg["reply_markup"] = pKeyboard->parseKeyboard();
	}

	mpHttpClient->postReq(strUrl, jsMsg.dump(), headers, true);
	return 0;
}

void TelegramBot::sendPhoto(int64_t pChatId, std::string pFileId) {
    if(pFileId.empty()) return;
    std::vector<std::string> headers;
    std::string strUrl  = std::string(TELEGRAM_BOT_BASE_URL) + std::string(BOT_TOKEN) + "/sendPhoto";

    json jsMsg;
    jsMsg["chat_id"]    = pChatId;
    jsMsg["photo"]      = pFileId;
    mpHttpClient->postReq(strUrl, jsMsg.dump(), headers, true);
}

void TelegramBot::startGettingUpdates(int64_t pOffset, int32_t pLimit) {
    std::vector<std::string> headers;
    std::string strUrl = std::string(TELEGRAM_BOT_BASE_URL) + std::string(BOT_TOKEN) + "/getUpdates";

    json jsMsg;
    jsMsg["offset"]     = pOffset;
    jsMsg["limit"]      = pLimit;
    jsMsg["timeout"]    = 5;

    mpHttpClient->postReq(strUrl, jsMsg.dump(), headers);
}

void TelegramBot::onHttpSuccess(const std::string& pResp) {
    static uint64_t updateId    = 0;
    petWatchDog();

    uint64_t thisId = getUpdateIdManually(pResp);

    updateId    = (thisId > updateId) ? thisId : updateId;
    if(pResp != "{\"ok\":true,\"result\":[]}")
        mLogger << "HttpResp : " << pResp << std::endl;

	auto root = json::parse(pResp, nullptr, false);
	if (!root.is_discarded() && root["ok"]) {
    	for (const auto& result : root["result"]) {
            if (result.contains("update_id")) {
                updateId = result.value<uint64_t>("update_id", 0);
                mLogger << "Got update id from json : " << updateId << std::endl;
            }

    		if (result.contains("message")) {
	    		const auto& msg	= result["message"];
		    	auto pMsg		= parseMessage(msg);
			    onMessage(pMsg);
    		}
	    }
    }
    //sleep(1);
    startGettingUpdates(updateId + 1, 10);
}

BotMessage::Ptr TelegramBot::parseMessage(const json& jsonMsg) {
	BotMessage::Ptr pMsg= std::make_shared<BotMessage>();
	pMsg->text          = jsonMsg.value<std::string>("text", "");
	if (jsonMsg.contains("from")) {
		const auto& jsonFrom = jsonMsg["from"];
		pMsg->chat_id	= jsonFrom.value<int64_t>("id", 0);
		pMsg->name		= jsonFrom.value<std::string>("first_name", "");
	}
	return pMsg;
}

std::string TelegramBot::getStyleString(Style pStyle) {
	switch (pStyle) {
	case HTML:
		return "HTML";
	case Markdown:
		return "Markdown";
	case MarkdownV2:
		return "MarkdownV2";
	}
	return "";
}

json ReplyKeyboardMarkup::parseKeyboard() {
	json jsKeyboard;
	json jsRow = json::array();
	for (const auto& row : mKeyboard) {
		json jsColumn = json::array();
		for (const auto& column : row) {
			json keyBtn;
			keyBtn["text"] = column->mText;
			jsColumn.push_back(keyBtn);
		}
		jsRow.push_back(jsColumn);
	}
	jsKeyboard["keyboard"] = jsRow;
    jsKeyboard["resize_keyboard"] = mResizeKeyboard;  
	return jsKeyboard;
}

json ReplyKeyboardRemove::parseKeyboard() {
	json jsKeyboard;
	jsKeyboard["remove_keyboard"] = true;
	return jsKeyboard;
}



uint64_t TelegramBot::getUpdateIdManually(const std::string& pResp) {
    uint64_t updateId = 0;
    std::string strUpdateId = "\"update_id\":",
                strId       = "0";

    std::size_t found = pResp.find(strUpdateId);
    if(found != std::string::npos) {
        found   += strUpdateId.length();
        for(int32_t iLoop = found; iLoop < pResp.length(); iLoop++)
            if(pResp.at(iLoop) != ',') strId += pResp.at(iLoop);
            else break;
    }
    try { updateId = std::stoull(strId); }
    catch(std::exception &e) { mLogger << "Exception parsing update id " << e.what() << std::endl; }

    return updateId;
}
