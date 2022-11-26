//sgn 
#include <iostream>
#include <memory>
#include <vector>

#include "Constants.h"
#include "HomePage.h"
#include "BotKeyboards.h"

void HomePage::init() {}

std::string HomePage::getMsg() { return "Choose Entry or Exit"; }

GenericReply::Ptr HomePage::prepareMenu(BotMessage::Ptr pMsg) {
    ReplyKeyboardMarkup::Ptr pMainMenu;
    std::vector<KeyboardButton::Ptr> row1, row2;

    KeyboardButton::Ptr pKBBtn;

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = ENTRY_PAGE;
    row1.push_back(pKBBtn);

    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = EXIT_PAGE;
    row1.push_back(pKBBtn);
    
    pKBBtn          = std::make_shared<KeyboardButton>();
    pKBBtn->mText   = ADMIN_PAGE;
    row2.push_back(pKBBtn);

    pMainMenu       = std::make_shared<ReplyKeyboardMarkup>();
    pMainMenu->mKeyboard.push_back(row1);
    pMainMenu->mKeyboard.push_back(row2);

    pMainMenu->mResizeKeyboard = true;
    return pMainMenu;
}

void HomePage::onClick(BotMessage::Ptr pMsg) {}

