#include "RVConstants.h"

#include "Charger.h"
// #ifdef CHARGER_H
#include "Packet.h"
#include "DGN.h"

Charger::Charger(uint8_t address, uint8_t instance)
    : PowerSensor(address, instance)
{
    for (uint8_t i = 0; i < DATA_SIZE; ++i) {
        statusData_[i]  = 0;
        status2Data_[i] = 0;
    }
    statusData_[STATUS_INSTANCE_INDEX]  = instance;
    status2Data_[STATUS2_INSTANCE_INDEX] = instance;
}

void Charger::copyBuffer(const uint8_t* src, uint8_t* dst) {
    if (src != nullptr && dst != nullptr) {
        for (uint8_t i = 0; i < DATA_SIZE; ++i) {
            dst[i] = src[i];
        }
    }
}

void Charger::setData(RVC_DGN dgn, uint8_t* data) {
if (data != nullptr) {
        if (dgn == CHARGER_AC_STATUS_1) {
            PowerSensor::setData(dgn, data);
        } else if (dgn == CHARGER_STATUS) {
            copyBuffer(data, statusData());
        } else if (dgn == CHARGER_STATUS_2) {
            copyBuffer(data, status2Data());
        }
    }
}

uint16_t Charger::rawDesiredChargeVoltage() const {
    uint16_t result = 0;
    const uint8_t* d = statusData();
    if (d != nullptr) {
        result = getLilEndian(d[STATUS_CHARGE_VOLT_MSB], d[STATUS_CHARGE_VOLT_LSB]);
    }
    return result;
}

float Charger::rawDesiredChargeCurrent() const {
    float result = 0.0f;
    const uint8_t* d = statusData();
    if (d != nullptr) {
        uint16_t raw = getLilEndian(d[STATUS_CHARGE_CURR_MSB], d[STATUS_CHARGE_CURR_LSB]);
        result = static_cast<float>(static_cast<int16_t>(raw - ADC_ZERO_U16)) * ADC_PRECISION;
    }
    return result;
}

uint8_t Charger::rawCurrentPercent() const {
    uint8_t result = 0;
    const uint8_t* d = statusData();
    if (d != nullptr) {
        result = d[STATUS_CURRENT_PERCENT_INDEX];
    }
    return result;
}

uint8_t Charger::rawOperatingStateByte() const {
    uint8_t result = 0;
    const uint8_t* d = statusData();
    if (d != nullptr) {
        result = d[STATUS_OPERATING_STATE_INDEX];
    }
    return result;
}

uint16_t Charger::rawMeasuredChargeVoltage() const {
    uint16_t result = 0;
    const uint8_t* d = status2Data();
    if (d != nullptr) {
        result = getLilEndian(d[STATUS2_MEAS_VOLT_MSB], d[STATUS2_MEAS_VOLT_LSB]);
    }
    return result;
}

float Charger::rawMeasuredChargeCurrent() const {
    float result = 0.0f;
    const uint8_t* d = status2Data();
    if (d != nullptr) {
        uint16_t raw = getLilEndian(d[STATUS2_MEAS_CURR_MSB], d[STATUS2_MEAS_CURR_LSB]);
        result = static_cast<float>(static_cast<int16_t>(raw - ADC_ZERO_U16)) * ADC_PRECISION;
    }
    return result;
}

uint8_t Charger::rawChargerTemperatureC() const {
    uint8_t result = 0;
    const uint8_t* d = status2Data();
    if (d != nullptr) {
        result = d[STATUS2_TEMPERATURE_INDEX];
    }
    return result;
}

uint8_t Charger::rawDcSourceInstance() const {
    uint8_t result = 0;
    const uint8_t* d = status2Data();
    if (d != nullptr) {
        result = d[STATUS2_DC_SOURCE_INDEX];
    }
    return result;
}

uint8_t Charger::rawChargerPriority() const {
    uint8_t result = 0;
    const uint8_t* d = status2Data();
    if (d != nullptr) {
        result = d[STATUS2_PRIORITY_INDEX];
    }
    return result;
}

#include "ChargerView.h"
void Charger::attachView(const char* name, bool showCurrent, bool showFault) {
    ChargerView* view = new ChargerView(this, name);
    addView(view);
}

ChargerOperatingState Charger::operatingState() const {
    return static_cast<ChargerOperatingState>(rawOperatingStateByte());
}

bool Charger::isCharging() const {
    bool result = false;
    ChargerOperatingState st = operatingState();
    if (st == ChargerOperatingState::Bulk ||
        st == ChargerOperatingState::Absorption ||
        st == ChargerOperatingState::Overcharge ||
        st == ChargerOperatingState::Equalize ||
        st == ChargerOperatingState::Float ||
        st == ChargerOperatingState::ConstantVoltageCurrent) {
        result = true;
    }
    return result;
}

float Charger::desiredChargeVoltage() const {
    return static_cast<float>(rawDesiredChargeVoltage()) * VDC_PRECISION;
}

uint16_t Charger::desiredChargeVoltageDecivolts() const {
    return static_cast<uint16_t>(desiredChargeVoltage() * 10.0f);
}

float Charger::desiredChargeCurrent() const {
    return rawDesiredChargeCurrent();
}

uint8_t Charger::chargeCurrentPercentOfMax() const {
    // Table 5.3: % as uint8, precision 0.5% → raw 0..200 style often; return raw byte for now
    return rawCurrentPercent();
}

float Charger::measuredChargeVoltage() const {
    return static_cast<float>(rawMeasuredChargeVoltage()) * VDC_PRECISION;
}

float Charger::measuredChargeCurrent() const {
    return rawMeasuredChargeCurrent();
}

int16_t Charger::chargerTemperatureC() const {
    // Table 5.3 uint8 °C: -40..210 with 1°C; offset encoding varies by vendor.
    // Spec: min -40 → many implementations store temp_c + 40. Adjust when you see live data.
    return static_cast<int16_t>(rawChargerTemperatureC()) - 40;
}

uint8_t Charger::associatedDcSourceInstance() const {
    return rawDcSourceInstance();
}

uint8_t Charger::chargerPriority() const {
    return rawChargerPriority();
}

boolean Charger::executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t /*val*/) {
    boolean handled = false;

    if (buffer != nullptr) {
        switch (dgn) {
            case CHARGER_AC_STATUS_1:
            case CHARGER_STATUS:
            case CHARGER_STATUS_2:
                setData(dgn, const_cast<uint8_t*>(buffer));
                updateViews();
                handled = true;
                break;
            case CHARGER_AC_STATUS_2:
            case CHARGER_AC_STATUS_3:
            case CHARGER_AC_STATUS_4:
            case CHARGER_COMMAND:
            case CHARGER_STATUS_3:
            default:
                break;
        }
    }
    return handled;
}
// #endif
