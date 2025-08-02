#include "Arduino.h"
#include "ChassisMobility.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"

#include "PacketQueue.h"

ChassisMobility* ChassisMobility::instance = nullptr;

boolean ChassisMobility::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("ChassisMobility::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("ChassisMobility::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case CHASSIS_MOBILITY_COMMAND:
                ; // do nothing
                cmdExecuted = true;
                break;
            case (CHASSIS_MOBILITY_STATUS_2):
                printf("****************** Chassis Mobility Status 2 not supported.  ************************** \n");
                break;
            case (CHASSIS_MOBILITY_STATUS):
                // printf("ChassisMobility::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                cmdExecuted = true;
                break;
            default:
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
