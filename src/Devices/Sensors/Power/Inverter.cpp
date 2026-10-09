// Inverter.cpp
#include "RVConstants.h"

#include "Inverter.h"
#include "InverterView.h"
#include "Packet.h"
#include "DGN.h"
#include "BridgeDiagnostics.h"

Inverter::Inverter(uint8_t address, uint8_t instance)
    : PowerSensor(address, instance)
    , componentTemperatureC_{}
    , componentTemperatureAvailable_{}
    , dcVoltageV_(0.0f)
    , dcCurrentA_(0.0f)
    , dcVoltageAvailable_(false)
    , dcCurrentAvailable_(false) {}

void Inverter::setTemperatureStatus(const uint8_t* data, uint8_t sensorOffset)
{
    if (data != nullptr) {
        for (uint8_t statusIndex = 0; statusIndex < INVERTER_TEMPERATURES_PER_STATUS; ++statusIndex) {
            uint8_t sensorIndex = sensorOffset + statusIndex;
            uint8_t msbIndex = 1 + statusIndex * 2;
            uint16_t rawTemperature = getLilEndian(data[msbIndex], data[msbIndex + 1]);
            componentTemperatureAvailable_[sensorIndex] = rawTemperature != 0xFFFF;
            if (componentTemperatureAvailable_[sensorIndex]) {
                componentTemperatureC_[sensorIndex] = convToTempC(rawTemperature);
            }
        }
    }
}

void Inverter::setDcStatus(const uint8_t* data)
{
    if (data != nullptr) {
        uint16_t rawVoltage = getLilEndian(data[1], data[2]);
        dcVoltageAvailable_ = rawVoltage != 0xFFFF;
        if (dcVoltageAvailable_) {
            dcVoltageV_ = rawVoltage * 0.05f;
        }

        uint16_t rawCurrent = getLilEndian(data[3], data[4]);
        dcCurrentAvailable_ = rawCurrent != 0xFFFF;
        if (dcCurrentAvailable_) {
            dcCurrentA_ = (static_cast<int32_t>(rawCurrent) - 0x7D00) * 0.05f;
        }
    }
}

InverterStatus Inverter::status() const
{
    InverterStatus result = InverterStatus::Disabled;
    const uint8_t* d = dataBuffer();
    if (d != nullptr) {
        result = static_cast<InverterStatus>(d[INVERTER_STATUS_INDEX]);
    }
    return result;
}

const char* Inverter::statusName(uint8_t state)
{
    const char* result = "Reserved";
    switch (static_cast<InverterStatus>(state)) {
        case InverterStatus::Disabled: result = "Disabled"; break;
        case InverterStatus::Invert: result = "Inverting"; break;
        case InverterStatus::PassThru: result = "AC Pass-through"; break;
        case InverterStatus::ApsOnly: result = "Auxiliary Power Only"; break;
        case InverterStatus::LoadSense: result = "Load Sense"; break;
        case InverterStatus::WaitingInvert: result = "Waiting to Invert"; break;
        case InverterStatus::GenSupport: result = "Generator Support"; break;
        default:
            if (state == 0xFF) {
                result = "Unknown";
            }
            break;
    }
    return result;
}

// INVERTER_STATUS is not an AC point, so it must not land in a line's readings
uint8_t Inverter::lineOf(RVC_DGN dgn, const uint8_t* raw) const
{
    uint8_t result = NO_LINE;
    if ((dgn == INVERTER_AC_STATUS_1) && (raw != nullptr)) {
        const uint8_t line = raw[INVERTER_LINE_INDEX] & INVERTER_LINE_MASK;
        if (line == INVERTER_LINE_1_VALUE) {
            result = 0;
        } else if (line == INVERTER_LINE_2_VALUE) {
            result = 1;
        }
    }
    return result;
}

uint8_t Inverter::ioOf(RVC_DGN dgn, const uint8_t* raw) const
{
    uint8_t result = INPUT_LINE;
    if (dgn == INVERTER_AC_STATUS_1) {
        result = NUMIO;
        if (raw != nullptr) {
            const uint8_t io = raw[INVERTER_IO_INDEX] & INVERTER_IO_MASK;
            if (io == INVERTER_INPUT_VALUE) {
                result = INPUT_LINE;
            } else if (io == INVERTER_OUTPUT_VALUE) {
                result = OUTPUT_LINE;
            }
        }
    }
    return result;
}

void Inverter::attachView(const char* name, bool showCurrent, bool showFault)
{
    new InverterView(this, name);
}

boolean Inverter::executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t /*val*/)
{
     // RV_PRINTF("Inverter::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, buffer);
    if (!cmdExecuted && buffer != nullptr) {
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
                if (dgn == INVERTER_STATUS) {
                    BridgeDiagnostics::observeDeviceDetail(this, statusName(buffer[INVERTER_STATUS_INDEX]));
                }
                updateViews();
                cmdExecuted = true;
                break;
            case (INVERTER_TEMPERATURE_STATUS):
                setTemperatureStatus(buffer, 0);
                updateViews();
                cmdExecuted = true;
                break;
            case (INVERTER_TEMPERATURE_STATUS_2):
                setTemperatureStatus(buffer, INVERTER_TEMPERATURES_PER_STATUS);
                updateViews();
                cmdExecuted = true;
                break;
            case (INVERTER_DC_STATUS):
                setDcStatus(buffer);
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
