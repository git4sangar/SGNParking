//sgn


#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "Constants.h"
#include "AdminPage.h"
#include "DBInterface.h"
#include "BotKeyboards.h"


void AdminPage::init() {
    mRespMsg    = "Reports";
}

std::string AdminPage::getMsg() { return mRespMsg; }

GenericReply::Ptr AdminPage::prepareMenu(BotMessage::Ptr pMsg) {
    ReplyKeyboardMarkup::Ptr pMainMenu;
    std::vector<KeyboardButton::Ptr> row1, row2, row3;

    KeyboardButton::Ptr pKBBtn;
    pMainMenu       = std::make_shared<ReplyKeyboardMarkup>();

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = REPORTS_DAILY;
    row1.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = REPORTS_WEEKLY;
    row1.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = REPORTS_MONTHLY;
    row2.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = REPORTS_ANNUAL;
    row2.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = HOME_PAGE;
    row3.push_back(pKBBtn);

    pMainMenu->mKeyboard.push_back(row1);
    pMainMenu->mKeyboard.push_back(row2);
    pMainMenu->mKeyboard.push_back(row3);
    pMainMenu->mResizeKeyboard = true;
    return pMainMenu;
}

void AdminPage::onClick(BotMessage::Ptr pMsg) {
    if(pMsg->text == REPORTS_DAILY) {
        mRespMsg = "Total Vehicles till now: 100\nCars : 36\nBikes : 64\nCollection : ₹4000";
    } else if(pMsg->text == REPORTS_WEEKLY) {
        mRespMsg = "Total Vehicles this week: 3100\nCars : 1782\nBikes : 1318\nCollection : ₹10500";
    } else if(pMsg->text == REPORTS_MONTHLY) {
        mRespMsg = "Total Vehicles this month: 5845\nCars : 2014\nBikes : 3831\nCollection : ₹45500";
    } else if(pMsg->text == REPORTS_ANNUAL) {
        mRespMsg = "Total Vehicles this year: 55345\nCars : 22473\nBikes : 32872\nCollection : ₹587032";
    }
}
