#include "RVConstants.h"

#include "Charger.h"
// #ifdef CHARGER_H
#include "Packet.h"
#include "DGN.h"
#include "BridgeDiagnostics.h"

Charger::Charger(uint8_t address, uint8_t instance)
    : PowerSensor(address, instance)
{
    for (uint8_t i = 0; i < DATA_SIZE; ++i) {
        statusData_[i]  = 0xFF;
        status2Data_[i] = 0xFF;
        configurationData_[i] = 0xFF;
        configuration2Data_[i] = 0xFF;
        configuration3Data_[i] = 0xFF;
        configuration4Data_[i] = 0xFF;
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

const char* Charger::operatingStateName(uint8_t state) {
    const char* result = "Reserved";
    switch (static_cast<ChargerOperatingState>(state)) {
        case ChargerOperatingState::Disabled: result = "Disabled"; break;
        case ChargerOperatingState::NotCharging: result = "Not Charging"; break;
        case ChargerOperatingState::Bulk: result = "Bulk"; break;
        case ChargerOperatingState::Absorption: result = "Absorption"; break;
        case ChargerOperatingState::Overcharge: result = "Overcharge"; break;
        case ChargerOperatingState::Equalize: result = "Equalize"; break;
        case ChargerOperatingState::Float: result = "Float"; break;
        case ChargerOperatingState::ConstantVoltageCurrent: result = "Constant Voltage/Current"; break;
        default:
            if (state == 0xFF) {
                result = "Unknown";
            }
            break;
    }
    return result;
}

uint8_t Charger::lineOf(RVC_DGN dgn, const uint8_t* raw) const {
    uint8_t result = NO_LINE;
    if (dgn == CHARGER_AC_STATUS_1 && raw != nullptr) {
        uint8_t line = (raw[0] >> 4) & 0x03;
        if (line <= 1) {
            result = line;
        }
    }
    return result;
}

uint8_t Charger::ioOf(RVC_DGN dgn, const uint8_t* raw) const {
    uint8_t result = NUMIO;
    if (dgn == CHARGER_AC_STATUS_1 && raw != nullptr) {
        uint8_t io = (raw[0] >> 6) & 0x03;
        if (io == 0) {
            result = INPUT_LINE;
        } else if (io == 1) {
            result = OUTPUT_LINE;
        }
    }
    return result;
}

void Charger::setData(RVC_DGN dgn, uint8_t* data) {
if (data != nullptr) {
        if (dgn == CHARGER_AC_STATUS_1) {
            PowerSensor::setData(dgn, data);
        } else if (dgn == CHARGER_STATUS) {
            copyBuffer(data, statusData());
            BridgeDiagnostics::observeDeviceDetail(this, operatingStateName(rawOperatingStateByte()));
        } else if (dgn == CHARGER_STATUS_2) {
            copyBuffer(data, status2Data());
        } else if (dgn == CHARGER_CONFIGURATION_STATUS) {
            copyBuffer(data, configurationData_);
        } else if (dgn == CHARGER_CONFIGURATION_STATUS_2) {
            copyBuffer(data, configuration2Data_);
        } else if (dgn == CHARGER_CONFIGURATION_STATUS_3) {
            copyBuffer(data, configuration3Data_);
        } else if (dgn == CHARGER_CONFIGURATION_STATUS_4) {
            copyBuffer(data, configuration4Data_);
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
    new ChargerView(this, name);
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

bool Charger::hasMaximumChargeCurrent() const {
    return getLilEndian(configurationData_[CONFIG_MAX_CURRENT_MSB],
                        configurationData_[CONFIG_MAX_CURRENT_LSB]) != 0xFFFF;
}

float Charger::maximumChargeCurrentA() const {
    uint16_t raw = getLilEndian(configurationData_[CONFIG_MAX_CURRENT_MSB],
                                configurationData_[CONFIG_MAX_CURRENT_LSB]);
    return static_cast<float>(static_cast<int32_t>(raw) - ADC_ZERO_U16) * ADC_PRECISION;
}

bool Charger::hasMaximumChargePercent() const {
    return configuration2Data_[CONFIG2_MAX_CURRENT_PERCENT] != 0xFF;
}

float Charger::maximumChargePercent() const {
    return configuration2Data_[CONFIG2_MAX_CURRENT_PERCENT] * 0.5f;
}

bool Charger::hasRechargeVoltage() const {
    return getLilEndian(configuration2Data_[CONFIG2_RECHARGE_VOLTAGE_MSB],
                        configuration2Data_[CONFIG2_RECHARGE_VOLTAGE_LSB]) != 0xFFFF;
}

float Charger::rechargeVoltage() const {
    uint16_t raw = getLilEndian(configuration2Data_[CONFIG2_RECHARGE_VOLTAGE_MSB],
                                configuration2Data_[CONFIG2_RECHARGE_VOLTAGE_LSB]);
    return static_cast<float>(raw) * VDC_PRECISION;
}

bool Charger::hasBulkVoltage() const {
    return getLilEndian(configuration3Data_[CONFIG3_BULK_VOLTAGE_MSB],
                        configuration3Data_[CONFIG3_BULK_VOLTAGE_LSB]) != 0xFFFF;
}

float Charger::bulkVoltage() const {
    return getLilEndian(configuration3Data_[CONFIG3_BULK_VOLTAGE_MSB],
                        configuration3Data_[CONFIG3_BULK_VOLTAGE_LSB]) * VDC_PRECISION;
}

bool Charger::hasAbsorptionVoltage() const {
    return getLilEndian(configuration3Data_[CONFIG3_ABSORPTION_VOLTAGE_MSB],
                        configuration3Data_[CONFIG3_ABSORPTION_VOLTAGE_LSB]) != 0xFFFF;
}

float Charger::absorptionVoltage() const {
    return getLilEndian(configuration3Data_[CONFIG3_ABSORPTION_VOLTAGE_MSB],
                        configuration3Data_[CONFIG3_ABSORPTION_VOLTAGE_LSB]) * VDC_PRECISION;
}

bool Charger::hasFloatVoltage() const {
    return getLilEndian(configuration3Data_[CONFIG3_FLOAT_VOLTAGE_MSB],
                        configuration3Data_[CONFIG3_FLOAT_VOLTAGE_LSB]) != 0xFFFF;
}

float Charger::floatVoltage() const {
    return getLilEndian(configuration3Data_[CONFIG3_FLOAT_VOLTAGE_MSB],
                        configuration3Data_[CONFIG3_FLOAT_VOLTAGE_LSB]) * VDC_PRECISION;
}

bool Charger::hasTemperatureCompensation() const {
    return configuration3Data_[CONFIG3_TEMP_COMPENSATION] <= 250;
}

uint8_t Charger::temperatureCompensation() const {
    return configuration3Data_[CONFIG3_TEMP_COMPENSATION];
}

bool Charger::hasBulkTime() const {
    return getLilEndian(configuration4Data_[CONFIG4_BULK_TIME_MSB],
                        configuration4Data_[CONFIG4_BULK_TIME_LSB]) != 0xFFFF;
}

uint16_t Charger::bulkTimeMinutes() const {
    return getLilEndian(configuration4Data_[CONFIG4_BULK_TIME_MSB],
                        configuration4Data_[CONFIG4_BULK_TIME_LSB]);
}

bool Charger::hasAbsorptionTime() const {
    return getLilEndian(configuration4Data_[CONFIG4_ABSORPTION_TIME_MSB],
                        configuration4Data_[CONFIG4_ABSORPTION_TIME_LSB]) != 0xFFFF;
}

uint16_t Charger::absorptionTimeMinutes() const {
    return getLilEndian(configuration4Data_[CONFIG4_ABSORPTION_TIME_MSB],
                        configuration4Data_[CONFIG4_ABSORPTION_TIME_LSB]);
}

bool Charger::hasFloatTime() const {
    return getLilEndian(configuration4Data_[CONFIG4_FLOAT_TIME_MSB],
                        configuration4Data_[CONFIG4_FLOAT_TIME_LSB]) != 0xFFFF;
}

uint16_t Charger::floatTimeMinutes() const {
    return getLilEndian(configuration4Data_[CONFIG4_FLOAT_TIME_MSB],
                        configuration4Data_[CONFIG4_FLOAT_TIME_LSB]);
}

boolean Charger::executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t /*val*/) {
    boolean handled = false;

    if (buffer != nullptr) {
        switch (dgn) {
            case CHARGER_AC_STATUS_1:
            case CHARGER_STATUS:
            case CHARGER_STATUS_2:
            case CHARGER_CONFIGURATION_STATUS:
            case CHARGER_CONFIGURATION_STATUS_2:
            case CHARGER_CONFIGURATION_STATUS_3:
            case CHARGER_CONFIGURATION_STATUS_4:
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
