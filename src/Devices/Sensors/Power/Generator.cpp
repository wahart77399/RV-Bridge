#include "RVConstants.h"

#include "Generator.h"
#include "GeneratorView.h"
#include "Packet.h"
#include "DGN.h"
#include "BridgeDiagnostics.h"

namespace {
    const char* generatorStatusName(uint8_t status)
    {
        switch (status) {
            case 0: return "Stopped";
            case 1: return "Preheat";
            case 2: return "Cranking";
            case 3: return "Running";
            case 4: return "Priming";
            case 5: return "Fault";
            case 6: return "Engine run only";
            case 7: return "Test mode";
            case 8: return "Voltage adjust mode";
            case 9: return "Fault bypass mode";
            case 10: return "Configuration mode";
            default: return "Reserved";
        }
    }
}

Generator::Generator(uint8_t address, uint8_t instance)
    : PowerSensor(address, instance)
    , status2Data_{}
    , status2Received_(false)
    , demandStatusData_{}
    , demandStatusReceived_(false)
{
    for (uint8_t byteIndex = 0; byteIndex < DATA_SIZE; ++byteIndex) {
        status2Data_[byteIndex] = 0xFF;
        demandStatusData_[byteIndex] = 0xFF;
    }
}

void Generator::setDemandStatus(const uint8_t* data)
{
    if (data != nullptr) {
        for (uint8_t byteIndex = 0; byteIndex < DATA_SIZE; ++byteIndex) {
            demandStatusData_[byteIndex] = data[byteIndex];
        }
        demandStatusReceived_ = true;
    }
}

void Generator::setStatus2(const uint8_t* data)
{
    if (data != nullptr) {
        for (uint8_t byteIndex = 0; byteIndex < DATA_SIZE; ++byteIndex) {
            status2Data_[byteIndex] = data[byteIndex];
        }
        status2Received_ = true;
    }
}

void Generator::setStatus1(const uint8_t* data)
{
    if (data != nullptr) {
        status1State_ = data[0];
        status1Received_ = true;
        BridgeDiagnostics::observeDeviceDetail(this, generatorStatusName(status1State_));
    }
}

bool Generator::hasStatus1() const
{
    return status1Received_ && status1State_ <= 10;
}

bool Generator::isRunning() const
{
    return status1State_ == 3 || status1State_ == 6;
}

bool Generator::status1Fault() const
{
    return status1State_ == 5;
}

bool Generator::hasEngineFault() const
{
    bool fault = false;
    if (status2Received_) {
        uint8_t temperatureShutdown = status2Data_[0] & 0x03;
        uint8_t oilPressureShutdown = (status2Data_[0] >> 2) & 0x03;
        uint8_t lowOilLevel = (status2Data_[0] >> 4) & 0x03;
        uint8_t cautionLight = (status2Data_[0] >> 6) & 0x03;
        fault = temperatureShutdown == 1 || oilPressureShutdown == 1 ||
                lowOilLevel == 1 || cautionLight == 1;
    }
    return fault;
}

bool Generator::hasExternalActivityStatus() const
{
    bool available = false;
    if (demandStatusReceived_) {
        uint8_t externalActivity = (demandStatusData_[0] >> 6) & 0x03;
        available = externalActivity <= 1;
    }
    return available;
}

bool Generator::externalActivityDetected() const
{
    return ((demandStatusData_[0] >> 6) & 0x03) == 1;
}

uint8_t Generator::lineOf(RVC_DGN /*dgn*/, const uint8_t* raw) const
{
    uint8_t result = NO_LINE;
    if (raw != nullptr) {
        uint8_t masked = raw[GENERATOR_BYTE_0] & GENERATOR_LINE_MASK;
        if (masked == static_cast<uint8_t>(GeneratorInstance::GENERATOR_LINE_1)) {
            result = 0;
        } else if (masked == static_cast<uint8_t>(GeneratorInstance::GENERATOR_LINE_2)) {
            result = 1;
        }
    }
    return result;
}

void Generator::attachView(const char* name, bool showCurrent, bool showFault)
{
    new GeneratorView(this, name);
}

boolean Generator::executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t /*val*/)
{
    boolean handled = false;

    if (buffer != nullptr) {
        switch (dgn) {
            case GENERATOR_STATUS_1:
                setStatus1(buffer);
                updateViews();
                handled = true;
                break;
            case GENERATOR_AC_STATUS_1:
                setData(dgn, const_cast<uint8_t*>(buffer));
                updateViews();
                handled = true;
                break;
            case GENERATOR_STATUS_2:
                setStatus2(buffer);
                updateViews();
                handled = true;
                break;
            case GENERATOR_DEMAND_STATUS:
                setDemandStatus(buffer);
                updateViews();
                handled = true;
                break;
            case GENERATOR_COMMAND:
            case GENERATOR_AC_STATUS_2:
            case GENERATOR_AC_STATUS_3:
            case GENERATOR_AC_STATUS_4:
            default:
                // intentionally not managed – same as original
                break;
        }
    }
    return handled;
}
