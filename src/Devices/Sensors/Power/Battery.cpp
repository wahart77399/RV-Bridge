#include "RVConstants.h"
#include "Arduino.h"
#include "CanFrameTypes.h"
#include "Battery.h"
#include "debug.h"


CAN_frame_t* Battery::buildCommand(RVC_DGN dgn) { // do nothing - no commands will be sent to the ATS - we listen only
    return nullptr; // do nothing
}

#include "Packet.h"
#include "DGN.h"
#include "PacketQueue.h"


boolean Battery::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // RV_PRINTF("Battery::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // RV_PRINTF("Battery::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        setData(dgn, rawData);
        updateViews();
        cmdExecuted = true;
    }
    return cmdExecuted; // Command execution failed
}
