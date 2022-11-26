//sgn
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "Constants.h"
#include "ExitPage.h"
#include "BotKeyboards.h"
#include "DBInterface.h"

void ExitPage::init() {
    mRespMsg    = "Enter Vehicle number (only digits) and send to verify";
    mIconId     = std::string();

    if(mIconMap.size() == 0) {
        mIconMap.insert( {"Bike Paid", ICON_BIKE_PAID} );
        mIconMap.insert( {"Car Paid",  ICON_CAR_PAID} );

        mIconMap.insert( {"Bike Not Paid", ICON_BIKE_NOTPAID} );
        mIconMap.insert( {"Car Not Paid", ICON_CAR_NOTPAID} );
    }
}

std::string ExitPage::getMsg() { return mRespMsg; }

GenericReply::Ptr ExitPage::prepareMenu(BotMessage::Ptr pMsg) {
    ReplyKeyboardMarkup::Ptr pMainMenu;
    std::vector<KeyboardButton::Ptr> row1;

    KeyboardButton::Ptr pKBBtn;
    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = HOME_PAGE;
    row1.push_back(pKBBtn);

    pMainMenu       = std::make_shared<ReplyKeyboardMarkup>();
    pMainMenu->mKeyboard.push_back(row1);
    pMainMenu->mResizeKeyboard = true;
    return pMainMenu;
}

void ExitPage::onClick(BotMessage::Ptr pMsg) {
    std::stringstream ss;

    ss.str("");
    if(pMsg->text == EXIT_PAGE) {
        addPage(std::to_string(pMsg->chat_id), getSharedPtr());
        mRespMsg = "Pls type all digits of a vehicle no and send";
    } else if(pMsg->vehicle_no > 0) {
        VInfo::VehicleType pVType = mpDB->getVType(pMsg->vehicle_no);
             if(pVType == VInfo::VehicleType::TwoWheeler) mRespMsg = "Bike ";
        else if(pVType == VInfo::VehicleType::FourWheeler) mRespMsg = "Car ";
        else mRespMsg = "Unknown";

        if(mRespMsg != "Unknown") {
            if(pMsg->text.back() == '0')  mRespMsg += "Not Paid";
            else mRespMsg += "Paid";
        }
    }
}

std::string ExitPage::getMediaId() {
    std::string strFileId;
    if(mIconMap.count(mRespMsg) > 0) 
        strFileId = mIconMap.at(mRespMsg);
    return strFileId;
}
