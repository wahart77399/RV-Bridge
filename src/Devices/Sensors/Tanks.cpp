#include "RVConstants.h"
#include "Arduino.h"
#include "CanFrameTypes.h"
#include "Packet.h"
#include "DGN.h"
#include "Tanks.h"
#include "PacketQueue.h"
#include "debug.h"


const uint8_t Tanks::TANKS_INDEX                  = 0; 
const uint8_t Tanks::TANKS_RELATIVE_LEVEL         = 1;  
const uint8_t Tanks::TANKS_RESOLUTION             = 2;  
const uint8_t Tanks::TANKS_ABSOLUTE_LEVEL_MSB     = 3;
const uint8_t Tanks::TANKS_ABSOLUTE_LEVEL_LSB     = 4;
const uint8_t Tanks::TANKS_SIZE_MSB               = 5;
const uint8_t Tanks::TANKS_SIZE_LSB               = 6; 
const uint8_t Tanks::FRESH_WATER_INSTANCE         = 0;
const uint8_t Tanks::BLACK_WATER_INSTANCE         = 1;
const uint8_t Tanks::GRAY_WATER_INSTANCE          = 2;
const uint8_t Tanks::LPG_INSTANCE                 = 3;
const uint8_t Tanks::FRESH_WATER_SECOND_INSTANCE  = 16;
const uint8_t Tanks::BLACK_WATER_SECOND_INSTANCE  = 17;
const uint8_t Tanks::GRAY_WATER_SECOND_INSTANCE   = 18;
const uint8_t Tanks::LPG_SECOND_INSTANCE          = 19;
const std::map<uint8_t, std::string> Tanks::tankNames = { {FRESH_WATER_INSTANCE, "Fresh Tank"}, {BLACK_WATER_INSTANCE, "Black Tank"}, {GRAY_WATER_INSTANCE, "Gray Tank"}, {LPG_INSTANCE, "LPG Tank"} };    ;

uint8_t Tanks::levelPercent() const {
    uint8_t result = OUT_OF_RANGE_DATA;
    const uint8_t* data = getCurrentData();
    if (data != nullptr) {
        const uint8_t relativeLevel = data[TANKS_RELATIVE_LEVEL];
        const uint8_t resolution = data[TANKS_RESOLUTION];
        uint32_t percent = 0;
        bool available = false;
        if (relativeLevel <= 250 && resolution > 0 && resolution <= 250) {
            percent = 100UL * relativeLevel / resolution;
            available = true;
        } else {
            const uint16_t capacity = size();
            const uint16_t absoluteLevel = level();
            if (capacity > 0 && capacity <= MAX_RVC_TANK_VALUE && absoluteLevel != INVALID_TANK_SIZE) {
                percent = 100UL * absoluteLevel / capacity;
                available = true;
            }
        }
        if (available) {
            result = static_cast<uint8_t>(percent > MAX_PERCENT ? MAX_PERCENT : percent);
        }
    }
    return result;
}

void Tanks::setAutoFillStatus(const uint8_t* data)
{
    if (data != nullptr) {
        autoFillStatusData_ = data[0];
        autoFillStatusReceived_ = true;
    }
}

bool Tanks::hasAutoFillOperatingStatus() const
{
    return autoFillStatusReceived_ && (autoFillStatusData_ & 0x03) <= 1;
}

bool Tanks::autoFillOperating() const
{
    return (autoFillStatusData_ & 0x03) == 1;
}

bool Tanks::hasAutoFillValveStatus() const
{
    return autoFillStatusReceived_ && ((autoFillStatusData_ >> 2) & 0x03) <= 1;
}

bool Tanks::autoFillValveOpen() const
{
    return ((autoFillStatusData_ >> 2) & 0x03) == 1;
}

bool Tanks::hasAutoFillResult() const
{
    return autoFillStatusReceived_ && ((autoFillStatusData_ >> 4) & 0x0F) <= 4;
}

bool Tanks::autoFillResultFailed() const
{
    uint8_t lastOperation = (autoFillStatusData_ >> 4) & 0x0F;
    return lastOperation == 2 || lastOperation == 4;
}




CAN_frame_t* Tanks::buildCommand(RVC_DGN dgn) { // do nothing - no commands will be sent to the ATS - we listen only
    return nullptr; // do nothing
}


boolean Tanks::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // RV_PRINTF("Tanks::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // RV_PRINTF("Tanks::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case (TANK_STATUS):
                // RV_PRINTF("Tanks::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                updateViews();
                cmdExecuted = true;
                break;
            case (AUTOFILL_STATUS):
                setAutoFillStatus(data);
                updateViews();
                cmdExecuted = true;
                break;
            default:
                RV_PRINTF("Tanks::executeCommand: Status and Commands not managed \n");
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
