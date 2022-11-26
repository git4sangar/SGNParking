//sgn

#include <iostream>
#include <memory>
#include <sstream>
#include <SQLiteCpp/SQLiteCpp.h>

#include "DBInterface.h"
#include "DateTime.h"
#include "Constants.h"

int32_t DBInterface::vehicleType2No(VInfo::VehicleType pVType) {
    switch(pVType) {
        case VInfo::TwoWheeler:
            return TWO_WHEELER_TYPE_NO;
        case VInfo::FourWheeler:
            return FOUR_WHEELER_TYPE_NO;
    }
    return -1;
}

VInfo::VehicleType DBInterface::vehileNo2Type(int32_t pVTypeNo) {
    switch(pVTypeNo) {
        case TWO_WHEELER_TYPE_NO:
            return VInfo::TwoWheeler;
        case FOUR_WHEELER_TYPE_NO:
            return VInfo::FourWheeler;
    }
    return VInfo::None;
}

void DBInterface::addNewVehicle(VInfo::VehicleType pVType, uint32_t pVNo, uint64_t pChatId) {
    std::stringstream ss;
    DateTime::Ptr pNow      = std::make_shared<DateTime>();

    SQLite::Transaction transaction(*mpDB);
    ss  << "INSERT INTO vinfo (chat_id, vehicle_no_digits, in_time, vehicle_type) VALUES ("
        << pChatId << ", " << pVNo << ", " << pNow->getEpoch() << ", " << vehicleType2No(pVType) << ");";
    mpDB->exec(ss.str());
    transaction.commit();    
}

VInfo::VehicleType DBInterface::getVType(int32_t pVNo) {
    if(pVNo <= 0) VInfo::None;
    std::stringstream ss;

    ss.str(""); ss << "SELECT * FROM vinfo WHERE vehicle_no_digits = " << pVNo << ";";
    SQLite::Statement query(*mpDB, ss.str());
    if(query.executeStep()) {
        VInfo::Ptr pVInfo = parseVInfo(&query);
        return pVInfo->mVehicleType;
    }
    return VInfo::None;
} 

VInfo::VehicleType DBInterface::getVType(const std::string& pStrVNo) {
    if(pStrVNo.empty()) VInfo::None;
    std::stringstream ss;

    ss.str(""); ss << "SELECT * FROM vinfo WHERE vehicle_no_text = \"" << pStrVNo << "\";";
    SQLite::Statement query(*mpDB, ss.str());
    if(query.executeStep()) {
        VInfo::Ptr pVInfo = parseVInfo(&query);
        return pVInfo->mVehicleType;
    }
    return VInfo::None;
}

VInfo::Ptr DBInterface::parseVInfo(SQLite::Statement *pQuery) {
    VInfo::Ptr pVInfo           = std::make_shared<VInfo>();
    pVInfo->mId                 = pQuery->getColumn("id").getInt64();
    pVInfo->mChatId             = pQuery->getColumn("chat_id").getInt64();
    pVInfo->mVehicleNoDigits    = pQuery->getColumn("vehicle_no_digits").getInt();
    pVInfo->mVehicleNoText      = pQuery->getColumn("vehicle_no_text").getString();
    pVInfo->mActualCharge       = pQuery->getColumn("actual_charge").getInt();
    pVInfo->mAmountPaid         = pQuery->getColumn("amount_paid").getInt();
    pVInfo->mInTime             = pQuery->getColumn("in_time").getInt64();
    pVInfo->mOutTime            = pQuery->getColumn("out_time").getInt64();
    pVInfo->mPaymentTime        = pQuery->getColumn("payment_time").getInt64();
    pVInfo->mPaidTill           = pQuery->getColumn("paid_till").getInt64();
    int vehicleTypeNo           = pQuery->getColumn("vehicle_type").getInt();
    pVInfo->mVehicleType        = vehileNo2Type(vehicleTypeNo);
    return pVInfo;
}















