// Inverter.cpp
#include "RVConstants.h"

#include "Inverter.h"
#include "InverterView.h"
#include "Packet.h"
#include "DGN.h"

Inverter::Inverter(uint8_t address, uint8_t instance)
    : PowerSensor(address, instance) {}

InverterStatus Inverter::status() const
{
    InverterStatus result = InverterStatus::Disabled;
    const uint8_t* d = dataBuffer();
    if (d != nullptr) {
        result = static_cast<InverterStatus>(d[INVERTER_STATUS_INDEX]);
    }
    return result;
}

// INVERTER_STATUS is not an AC point, so it must not land in a line's readings
uint8_t Inverter::lineOf(RVC_DGN dgn, const uint8_t* raw) const
{
    uint8_t result = NO_LINE;
    if ((dgn == INVERTER_AC_STATUS_1) && (raw != nullptr)) {
        result = ((raw[INVERTER_LINE_INDEX] & INVERTER_LINE_MASK) == INVERTER_LINE_2_VALUE) ? 1 : 0;
    }
    return result;
}

void Inverter::attachView(const char* name, bool showCurrent, bool showFault)
{
    InverterView* view = new InverterView(this, name);
    addView(view);
}

boolean Inverter::executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t /*val*/)
{
     // RV_PRINTF("Inverter::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, buffer);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // RV_PRINTF("Inverter::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = const_cast<uint8_t* >(buffer);
        switch (dgn) {
            case (INVERTER_AC_STATUS_1):
            case (INVERTER_STATUS):
                //  RV_PRINTF("Inverter::executeCommand case INVERTER_STATUS\n");
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
                RV_PRINTF("Inverter::executeCommand: Status and Commands not managed \n");
                break;
        }
    }
    return cmdExecuted; // Command execution failed   
}
