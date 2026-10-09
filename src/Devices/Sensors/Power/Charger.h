// #ifdef CHARGER_H
// #define CHARGER_H // once I'm ready to define this, move this below ifndef


/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2023 Randy Ubillos
 *  
 *  https://github.com/rubillos/RV-Bridge
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
////////////////////////////////////////////////////////////////
//                                                            //
//    RV-Bridge: A HomeKit to RV-C interface for the ESP32    //
//                                                            //
////////////////////////////////////////////////////////////////

#pragma once

#include "RVConstants.h"

#include "PowerSensor.h"

enum class ChargerOperatingState : uint8_t {
    Disabled           = 0,
    NotCharging        = 1,
    Bulk               = 2,
    Absorption         = 3,
    Overcharge         = 4,
    Equalize           = 5,
    Float              = 6,
    ConstantVoltageCurrent = 7
};

class Charger : public PowerSensor {
    friend class PowerSensorView; // AC tiles via base view
    friend class ChargerView;

private:
    static constexpr uint8_t STATUS_INSTANCE_INDEX        = 0;
    static constexpr uint8_t STATUS_CHARGE_VOLT_MSB       = 1;
    static constexpr uint8_t STATUS_CHARGE_VOLT_LSB       = 2;
    static constexpr uint8_t STATUS_CHARGE_CURR_MSB       = 3;
    static constexpr uint8_t STATUS_CHARGE_CURR_LSB       = 4;
    static constexpr uint8_t STATUS_CURRENT_PERCENT_INDEX = 5;
    static constexpr uint8_t STATUS_OPERATING_STATE_INDEX = 6;
    static constexpr uint8_t STATUS_FLAGS_INDEX           = 7;

    static constexpr uint8_t STATUS2_INSTANCE_INDEX       = 0;
    static constexpr uint8_t STATUS2_DC_SOURCE_INDEX      = 1;
    static constexpr uint8_t STATUS2_PRIORITY_INDEX       = 2;
    static constexpr uint8_t STATUS2_MEAS_VOLT_MSB        = 3;
    static constexpr uint8_t STATUS2_MEAS_VOLT_LSB        = 4;
    static constexpr uint8_t STATUS2_MEAS_CURR_MSB        = 5;
    static constexpr uint8_t STATUS2_MEAS_CURR_LSB        = 6;
    static constexpr uint8_t STATUS2_TEMPERATURE_INDEX    = 7;

    static constexpr uint8_t CONFIG_MAX_CURRENT_MSB        = 6;
    static constexpr uint8_t CONFIG_MAX_CURRENT_LSB        = 7;

    static constexpr uint8_t CONFIG2_MAX_CURRENT_PERCENT   = 1;
    static constexpr uint8_t CONFIG2_RECHARGE_VOLTAGE_MSB  = 5;
    static constexpr uint8_t CONFIG2_RECHARGE_VOLTAGE_LSB  = 6;

    static constexpr uint8_t CONFIG3_BULK_VOLTAGE_MSB      = 1;
    static constexpr uint8_t CONFIG3_BULK_VOLTAGE_LSB      = 2;
    static constexpr uint8_t CONFIG3_ABSORPTION_VOLTAGE_MSB = 3;
    static constexpr uint8_t CONFIG3_ABSORPTION_VOLTAGE_LSB = 4;
    static constexpr uint8_t CONFIG3_FLOAT_VOLTAGE_MSB     = 5;
    static constexpr uint8_t CONFIG3_FLOAT_VOLTAGE_LSB     = 6;
    static constexpr uint8_t CONFIG3_TEMP_COMPENSATION     = 7;

    static constexpr uint8_t CONFIG4_BULK_TIME_MSB        = 1;
    static constexpr uint8_t CONFIG4_BULK_TIME_LSB        = 2;
    static constexpr uint8_t CONFIG4_ABSORPTION_TIME_MSB  = 3;
    static constexpr uint8_t CONFIG4_ABSORPTION_TIME_LSB  = 4;
    static constexpr uint8_t CONFIG4_FLOAT_TIME_MSB       = 5;
    static constexpr uint8_t CONFIG4_FLOAT_TIME_LSB       = 6;

    static constexpr float   VDC_PRECISION = 0.05f;
    static constexpr float   ADC_PRECISION = 0.05f;
    static constexpr uint16_t ADC_ZERO_U16 = 0x7D00;

    uint8_t statusData_[DATA_SIZE];
    uint8_t status2Data_[DATA_SIZE];
    uint8_t configurationData_[DATA_SIZE];
    uint8_t configuration2Data_[DATA_SIZE];
    uint8_t configuration3Data_[DATA_SIZE];
    uint8_t configuration4Data_[DATA_SIZE];

    uint8_t*       statusData()       { return statusData_; }
    const uint8_t* statusData() const { return statusData_; }
    uint8_t*       status2Data()       { return status2Data_; }
    const uint8_t* status2Data() const { return status2Data_; }
    const uint8_t* configurationData() const { return configurationData_; }
    const uint8_t* configuration2Data() const { return configuration2Data_; }
    const uint8_t* configuration3Data() const { return configuration3Data_; }
    const uint8_t* configuration4Data() const { return configuration4Data_; }

    void copyBuffer(const uint8_t* src, uint8_t* dst);
    static const char* operatingStateName(uint8_t state);

    uint16_t rawDesiredChargeVoltage() const;
    float    rawDesiredChargeCurrent() const;
    uint8_t  rawCurrentPercent() const;
    uint8_t  rawOperatingStateByte() const;

    uint16_t rawMeasuredChargeVoltage() const;
    float    rawMeasuredChargeCurrent() const;
    uint8_t  rawChargerTemperatureC() const;
    uint8_t  rawDcSourceInstance() const;
    uint8_t  rawChargerPriority() const;
    bool hasMaximumChargeCurrent() const;
    float maximumChargeCurrentA() const;
    bool hasMaximumChargePercent() const;
    float maximumChargePercent() const;
    bool hasRechargeVoltage() const;
    float rechargeVoltage() const;
    bool hasBulkVoltage() const;
    float bulkVoltage() const;
    bool hasAbsorptionVoltage() const;
    float absorptionVoltage() const;
    bool hasFloatVoltage() const;
    float floatVoltage() const;
    bool hasTemperatureCompensation() const;
    uint8_t temperatureCompensation() const;
    bool hasBulkTime() const;
    uint16_t bulkTimeMinutes() const;
    bool hasAbsorptionTime() const;
    uint16_t absorptionTimeMinutes() const;
    bool hasFloatTime() const;
    uint16_t floatTimeMinutes() const;

protected:
    void setData(RVC_DGN dgn, uint8_t* data) override;
    uint8_t lineOf(RVC_DGN dgn, const uint8_t* raw) const override;
    uint8_t ioOf(RVC_DGN dgn, const uint8_t* raw) const override;

public:
    Charger() = delete;
    Charger(const Charger&) = delete;
    Charger& operator=(const Charger&) = delete;
    Charger(Charger&&) = delete;
    Charger& operator=(Charger&&) = delete;
    ~Charger() = default;

    Charger(uint8_t address, uint8_t instance);

    void attachView(const char* name, bool showCurrent = true, bool showFault = true);

    // domain behaviour — AC via PowerSensor::rmsVoltage / rmsCurrent
    ChargerOperatingState operatingState() const;
    bool isCharging() const;

    uint16_t desiredChargeVoltageDecivolts() const;  // 0.1 V units for UI convenience, or use float API below
    float    desiredChargeVoltage() const;
    float    desiredChargeCurrent() const;
    uint8_t  chargeCurrentPercentOfMax() const;

    float    measuredChargeVoltage() const;
    float    measuredChargeCurrent() const;
    int16_t  chargerTemperatureC() const;
    uint8_t  associatedDcSourceInstance() const;
    uint8_t  chargerPriority() const;

    boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer,
                           uint8_t val = SOURCE_ADDRESS) override;
};


// #endif