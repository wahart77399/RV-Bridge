#include "RVConstants.h"
#include "Arduino.h"
// #include "ESP32CAN.h"
// #include "CAN_config.h"
#include "CanFrameTypes.h"
#include "Packet.h"
#include "DGN.h"
#include "AutomaticTransferSwitch.h"
#include "AutomaticTransferSwitchView.h"
#include "PacketQueue.h"
#include "debug.h"
#include "BridgeDiagnostics.h"

#include <cstdio>

void AutomaticTransferSwitch::attachView(const char* name, bool showCurrent , bool showFault) {
    new AutomaticTransferSwitchView(this, name);
}

void AutomaticTransferSwitch::setData(RVC_DGN dgn, uint8_t* data) {
    if (data != nullptr) {
        if (dgn == ATS_STATUS) {
            const uint8_t source = data[1] < SOURCE_COUNT ? data[1] : 0xFF;
            if (source != selectedSource_) {
                selectedSource_ = source;
                clearReadings();
                if (source < SOURCE_COUNT) {
                    for (uint8_t leg = 0; leg < LEG_COUNT; ++leg) {
                        for (uint8_t io = 0; io < NUMIO; ++io) {
                            if (sourceReadingReceived_[source][leg][io]) {
                                PowerSensor::setData(ATS_AC_STATUS_1, sourceReadings_[source][leg][io]);
                            }
                        }
                    }
                }
            }
        } else if (dgn == ATS_AC_STATUS_1) {
            const uint8_t source = (data[0] & ATS_SOURCE_MASK) >> ATS_SOURCE_SHIFT;
            const uint8_t leg = lineOf(dgn, data);
            const uint8_t io = ioOf(dgn, data);
            if (source < SOURCE_COUNT && leg < LEG_COUNT && io < NUMIO) {
                std::memcpy(sourceReadings_[source][leg][io], data, DATA_SIZE);
                sourceReadingReceived_[source][leg][io] = true;
                if (source == selectedSource_) {
                    PowerSensor::setData(dgn, data);
                }
            }
        }
    }
}

boolean AutomaticTransferSwitch::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // RV_PRINTF("AutomaticTransferSwitch::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted && data != nullptr) {
        // RV_PRINTF("AutomaticTransferSwitch::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case ATS_STATUS: {
                setData(dgn, rawData);
                uint8_t source = data[1];
                uint8_t mode = data[2] & 0x03;
                const char* sourceName = source < 7 ? sourceNames_[source] :
                                         (source == 253 ? "No source active" : "Unknown source");
                const char* modeName = mode == 0 ? "Automatic" : mode == 1 ? "Manual" : "Unknown mode";
                std::snprintf(sourceStatusDetail_, sizeof(sourceStatusDetail_), "ATS source: %s (%s)", sourceName, modeName);
                BridgeDiagnostics::observeDeviceDetail(this, sourceStatusDetail_);
                updateViews();
                cmdExecuted = true;
                break;
            }
            case (ATS_AC_STATUS_1):
                // RV_PRINTF("AutomaticTransferSwitch::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                updateViews();
                cmdExecuted = true;
                break;
            case (ATS_COMMAND):
                //RV_PRINTF("AutomaticTransferSwitch::executeCommand: ATS_COMMAND not managed \n");
                break;
            case (ATS_AC_STATUS_2):
                //RV_PRINTF("AutomaticTransferSwitch::executeCommand: ATS_AC_STATUS_2 not managed \n");
                break;
            case (ATS_AC_STATUS_3):
                //RV_PRINTF("AutomaticTransferSwitch::executeCommand: ATS_AC_STATUS_3 not managed \n");
                break;
            case (ATS_AC_STATUS_4):
                //RV_PRINTF("AutomaticTransferSwitch::executeCommand: ATS_AC_STATUS_4 not managed \n");
                break;
            default:
                RV_PRINTF("AutomaticTransferSwitch::executeCommand: Status and Commands not managed \n");
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
