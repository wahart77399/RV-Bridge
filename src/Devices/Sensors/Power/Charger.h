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

    static constexpr float   VDC_PRECISION = 0.05f;
    static constexpr float   ADC_PRECISION = 0.05f;
    static constexpr uint16_t ADC_ZERO_U16 = 0x7D00;

    uint8_t statusData_[DATA_SIZE];
    uint8_t status2Data_[DATA_SIZE];

    uint8_t*       statusData()       { return statusData_; }
    const uint8_t* statusData() const { return statusData_; }
    uint8_t*       status2Data()       { return status2Data_; }
    const uint8_t* status2Data() const { return status2Data_; }

    void copyBuffer(const uint8_t* src, uint8_t* dst);

    uint16_t rawDesiredChargeVoltage() const;
    float    rawDesiredChargeCurrent() const;
    uint8_t  rawCurrentPercent() const;
    uint8_t  rawOperatingStateByte() const;

    uint16_t rawMeasuredChargeVoltage() const;
    float    rawMeasuredChargeCurrent() const;
    uint8_t  rawChargerTemperatureC() const;
    uint8_t  rawDcSourceInstance() const;
    uint8_t  rawChargerPriority() const;

protected:
    void setData(RVC_DGN dgn, uint8_t* data) override;

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