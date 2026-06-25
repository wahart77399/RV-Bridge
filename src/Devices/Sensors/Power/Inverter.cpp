#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Arduino.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"
#include "Inverter.h"
#include "PacketQueue.h"

boolean Inverter::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("Inverter::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("Inverter::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case (INVERTER_AC_STATUS_1):
            case (INVERTER_STATUS):
                // printf("Inverter::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                updateViews();
                cmdExecuted = true;
                break;
            case (INVERTER_COMMAND):
            case (INVERTER_AC_STATUS_2):
            case (INVERTER_AC_STATUS_3):
            case (INVERTER_AC_STATUS_4):
            default:
                printf("Inverter::executeCommand: Status and Commands not managed \n");
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}

#endif // ifdef HOME_KIT_2