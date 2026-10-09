#include "ChassisMobility.h"
#include "CanFrameTypes.h"
#include "Packet.h"
#include "DGN.h"
#include "debug.h"
#include "BridgeDiagnostics.h"
#include <cstdio>

ChassisMobility* ChassisMobility::instance_ = nullptr;

ChassisMobility::ChassisMobility()
    : GenericDevice(DEFAULT_CHASSIS_SOURCE_ADDRESS, DEFAULT_CHASSIS_INDEX)
    , statusReceived_(false)
    , vehicleSpeedRaw_(SPEED_UNKNOWN) {
}

ChassisMobility::ChassisMobility(uint8_t sourceAddress)
    : GenericDevice(sourceAddress, DEFAULT_CHASSIS_INDEX)
    , statusReceived_(false)
    , vehicleSpeedRaw_(SPEED_UNKNOWN) {
}

ChassisMobility::~ChassisMobility() {
}

ChassisMobility* ChassisMobility::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new ChassisMobility(DEFAULT_CHASSIS_SOURCE_ADDRESS);
    }
    return instance_;
}

bool ChassisMobility::statusReceived() const {
    return statusReceived_;
}

void ChassisMobility::statusReceived(bool value) {
    statusReceived_ = value;
}

bool ChassisMobility::hasValidStatus() const {
    return statusReceived() && vehicleSpeedRaw_ != SPEED_ERROR &&
           vehicleSpeedRaw_ != SPEED_UNKNOWN;
}

bool ChassisMobility::isBrakeEngaged() const {
    bool engaged = false;
    do {
        if (!statusReceived()) {
            break;
        }
        const uint8_t* rawData = getCurrentData();
        if (rawData == nullptr) {
            break;
        }
        uint8_t raw = rawData[BRAKE_BYTE_INDEX];
        if (raw == INVALID_BYTE) {
            break;
        }
        uint8_t brakeStatus = raw & 0x03;
        if (brakeStatus > BRAKE_ENGAGED) {
            break;
        }
        engaged = brakeStatus == BRAKE_ENGAGED;
    } while (false);
    return engaged;
}

bool ChassisMobility::isMoving() const {
    return statusReceived_ && vehicleSpeedRaw_ != SPEED_ERROR &&
           vehicleSpeedRaw_ != SPEED_UNKNOWN && vehicleSpeedRaw_ > 0;
}

bool ChassisMobility::isParked() {
    return instance_ != nullptr && instance_->isBrakeEngaged();
}

void ChassisMobility::applyStatus(const uint8_t* data) {
    do {
        if (data == nullptr) {
            break;
        }
        uint8_t* rawData = getCurrentData();
        if (rawData == nullptr) {
            break;
        }
        vehicleSpeedRaw_ = static_cast<uint16_t>(data[SPEED_LSB_INDEX]) |
                           (static_cast<uint16_t>(data[SPEED_MSB_INDEX]) << 8);
        rawData[BRAKE_BYTE_INDEX] = data[BRAKE_BYTE_INDEX];
        statusReceived(true);
        const uint8_t brakeStatus = data[BRAKE_BYTE_INDEX] & 0x03;
        const char* brakeDetail = brakeStatus == BRAKE_ENGAGED ? "Park brake engaged" :
                      brakeStatus == BRAKE_RELEASED ? "Park brake released" :
                      "Park brake unavailable";
        std::snprintf(statusDetail_, sizeof(statusDetail_), "%s; brake=%u; payload=%02X %02X %02X %02X %02X %02X %02X %02X",
                  brakeDetail, brakeStatus, data[0], data[1], data[2], data[3],
                  data[4], data[5], data[6], data[7]);
        BridgeDiagnostics::observeDeviceDetail(this, statusDetail_);
    } while (false);
}

void ChassisMobility::setData(RVC_DGN dgn, uint8_t* data) {
    do {
        if (data == nullptr) {
            break;
        }
        switch (dgn) {
            case CHASSIS_MOBILITY_STATUS:
                applyStatus(data);
                break;
            case CHASSIS_MOBILITY_STATUS_2:
            case CHASSIS_MOBILITY_COMMAND:
            default:
                break;
        }
    } while (false);
}

CAN_frame_t* ChassisMobility::buildCommand(RVC_DGN dgn) {
    (void)dgn;
    return nullptr;
}

boolean ChassisMobility::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sourceAddress) {
    (void)sourceAddress;
    boolean cmdExecuted = false;
    do {
        boolean baseHandled = GenericDevice::executeCommand(dgn, data);
        if (baseHandled) {
            cmdExecuted = true;
            break;
        }
        switch (dgn) {
            case CHASSIS_MOBILITY_STATUS:
                setData(dgn, const_cast<uint8_t*>(data));
                updateViews();
                cmdExecuted = true;
                break;
            default:
                break;
        }
    } while (false);
    return cmdExecuted;
}