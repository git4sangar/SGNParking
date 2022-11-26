//sgn

#pragma once

#include <memory>

#include "nlohmann_json.hpp"
using json = nlohmann::json;

class GenericReply {
public:
    typedef std::shared_ptr<GenericReply> Ptr;
    virtual ~GenericReply() {}
    virtual json parseKeyboard() = 0;
};

class KeyboardButton {
public:
    typedef std::shared_ptr<KeyboardButton> Ptr;
    virtual ~KeyboardButton() {}
    std::string mText;
};

class ReplyKeyboardMarkup : public GenericReply {
public:
    typedef std::shared_ptr<ReplyKeyboardMarkup> Ptr;

    json parseKeyboard();

    bool mResizeKeyboard = false;
    std::vector<std::vector<KeyboardButton::Ptr>> mKeyboard;
};

class ReplyKeyboardRemove : public GenericReply {
public:
    typedef std::shared_ptr<ReplyKeyboardRemove> Ptr;
    json parseKeyboard();
};
