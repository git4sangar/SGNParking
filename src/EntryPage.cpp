//sgn
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "Constants.h"
#include "EntryPage.h"
#include "DBInterface.h"
#include "BotKeyboards.h"

void EntryPage::init() {
    mRespMsg    = "Enter Vehicle number (only digits) and send";
}

std::string EntryPage::getMsg() { return mRespMsg; }

GenericReply::Ptr EntryPage::prepareMenu(BotMessage::Ptr pMsg) {
    ReplyKeyboardMarkup::Ptr pMainMenu;
    std::vector<KeyboardButton::Ptr> row1, row2;

    KeyboardButton::Ptr pKBBtn;
    pMainMenu       = std::make_shared<ReplyKeyboardMarkup>();

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = VEHICLE_CAR;
    row1.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = VEHICLE_BIKE;
    row1.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = HOME_PAGE;
    row2.push_back(pKBBtn);

    pMainMenu->mKeyboard.push_back(row1);
    pMainMenu->mKeyboard.push_back(row2);
    pMainMenu->mResizeKeyboard = true;
    return pMainMenu;
}

void EntryPage::onClick(BotMessage::Ptr pMsg) {
    std::stringstream ss;
    VInfo::VehicleType vType = getVehicle(pMsg->text);

    ss.str("");
    if(pMsg->text == ENTRY_PAGE) {
        addPage(std::to_string(pMsg->chat_id), getSharedPtr());
        mRespMsg = "Pls choose Car or Bike";
    } else if(vType != VInfo::VehicleType::None) {
        BaseBot::setVType(pMsg->chat_id, vType); 
             if(vType == VInfo::VehicleType::FourWheeler) ss << "Type all digits in Car no and send";
        else if(vType == VInfo::VehicleType::TwoWheeler)  ss << "Type all digits in Bike no and send";
        mRespMsg = ss.str();
    } else if(pMsg->vehicle_no > 0) {
        vType = BaseBot::getLastSetVType(pMsg->chat_id); 
        if(vType == VInfo::VehicleType::None)
            ss << "Pls choose " << VEHICLE_CAR << " or " << VEHICLE_BIKE;
        else {
            mpDB->addNewVehicle(vType, pMsg->vehicle_no, pMsg->chat_id);
            ss << "Vehicle number updated";
        }
        mRespMsg = ss.str();
    }
}

VInfo::VehicleType EntryPage::getVehicle(const std::string& pVehicle) {
         if(pVehicle == VEHICLE_CAR)    return VInfo::VehicleType::FourWheeler;
    else if(pVehicle == VEHICLE_BIKE)   return VInfo::VehicleType::TwoWheeler;
    return VInfo::VehicleType::None;
}

