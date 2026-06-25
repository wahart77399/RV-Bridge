#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Arduino.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"
#include "AutomaticTransferSwitch.h"
#include "PacketQueue.h"

boolean AutomaticTransferSwitch::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("AutomaticTransferSwitch::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("AutomaticTransferSwitch::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case (ATS_AC_STATUS_1):
                // printf("AutomaticTransferSwitch::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                updateViews();
                cmdExecuted = true;
                break;
            case (ATS_COMMAND):
                //printf("AutomaticTransferSwitch::executeCommand: ATS_COMMAND not managed \n");
                break;
            case (ATS_AC_STATUS_2):
                //printf("AutomaticTransferSwitch::executeCommand: ATS_AC_STATUS_2 not managed \n");
                break;
            case (ATS_AC_STATUS_3):
                //printf("AutomaticTransferSwitch::executeCommand: ATS_AC_STATUS_3 not managed \n");
                break;
            case (ATS_AC_STATUS_4):
                //printf("AutomaticTransferSwitch::executeCommand: ATS_AC_STATUS_4 not managed \n");
                break;
            default:
                printf("AutomaticTransferSwitch::executeCommand: Status and Commands not managed \n");
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
#endif // ifdef HOME_KIT_2