//sgn 
#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include <SQLiteCpp/SQLiteCpp.h>
#include "DateTime.h"
#include "Logger.h"

class VInfo {
    public:
        enum VehicleType{ None, TwoWheeler, FourWheeler };

        typedef std::shared_ptr<VInfo> Ptr;
        VInfo()
        : mId(0)
        , mChatId(0)
        , mActualCharge(-1)
        , mAmountPaid(-1)
        , mVehicleNoDigits(0)
        , mInTime(DateTime::INVALID)
        , mOutTime(DateTime::INVALID)
        , mPaymentTime(DateTime::INVALID)
        , mPaidTill(DateTime::INVALID)
        , mVehicleType(VehicleType::None) {}

        virtual ~VInfo(){}

        uint64_t    mId, mChatId;
        int32_t     mActualCharge, mAmountPaid, mVehicleNoDigits;
        time_t      mInTime, mOutTime, mPaymentTime, mPaidTill;
        std::string mVehicleNoText;
        VehicleType mVehicleType;
};

class DBInterface : public std::enable_shared_from_this<DBInterface> {
    std::shared_ptr<SQLite::Database> mpDB;
    const Logger& mLogger;

    int32_t             vehicleType2No(VInfo::VehicleType pVType);
    VInfo::VehicleType  vehileNo2Type(int32_t pVTypeNo);
public:
    typedef std::shared_ptr<DBInterface> Ptr;
 
    DBInterface(const std::string& pDBName)
    : mpDB (std::make_shared<SQLite::Database>(pDBName, SQLite::OPEN_READWRITE))
    , mLogger(Logger::getInstance())
    {}

    virtual ~DBInterface() {}

    void                addNewVehicle(VInfo::VehicleType pVType, uint32_t pVNo, uint64_t pChatId);
    VInfo::VehicleType  getVType(int32_t pVNo);
    VInfo::VehicleType  getVType(const std::string& pVNo);
    VInfo::Ptr          parseVInfo(SQLite::Statement *pQuery);
    int32_t             getElapsedTime(int32_t pVNo);
};
