// src/Devices/Sensors/Power/PowerSensor.cpp
#include "RVConstants.h"
#ifdef HOME_KIT_2

#include "PowerSensor.h"
#include "PowerSensorView.h"
#include "Packet.h"
#include "DGN.h"

// ------------------------------------------------------------------
PowerSensor::PowerSensor(uint8_t address, uint8_t instance)
    : GenericDevice(address, instance)
{
    memset(readings_, INVALID_DATA, sizeof(readings_));
    for (uint8_t line = 0; line < MAX_LINES; ++line) {
        for (uint8_t io = 0; io < NUMIO; ++io) {
            lastValidVolts_[line][io] = 0;
            lastValidAmps_[line][io]  = 0;
            voltReadingElapsedTime_[line][io] = 0;
            ampReadingElapsedTime_[line][io]  = 0;
        }
    }
}

// ------------------------------------------------------------------
// Private helpers – working logic preserved
// ------------------------------------------------------------------
uint16_t PowerSensor::getACPointValue(const uint8_t* raw,
                                      AC_POINT_DATA_INDECES msb,
                                      AC_POINT_DATA_INDECES lsb) const
{
    uint16_t result = BAD_DATA;

    if (raw != nullptr &&
        msb > AC_POINT_INSTANCE_INDEX &&
        lsb < AC_POINT_FAULTS_INDEX &&
        (lsb - msb) == 1) {
        result = getLilEndian(raw[msb], raw[lsb]);
    }
    return result;
}

uint16_t PowerSensor::validateVolts(uint8_t line, uint8_t io, float volts)
{
    uint16_t result = 0;

    if (!(volts >= 0.0f && volts <= (VAC_MAX * VAC_PRECISION)) || volts == 0.0f) {
        if (voltReadingElapsedTime_[line][io] < MAX_READING_INTERVAL_MS) {
            result = lastValidVolts(line, io);
        }
        else {
            result = 0;
            lastValidVolts(line, io, 0);
        }
    }
    else {
        result = static_cast<uint16_t>(volts);
        lastValidVolts(line, io, result);
        voltReadingElapsedTime_[line][io] = 0;
    }
    return result;
}

int16_t PowerSensor::validateAmps(uint8_t line, uint8_t io, float amps)
{
    int16_t result = 0;

    if (!(amps >= AAC_LOWER_LIMIT && amps <= AAC_UPPER_LIMIT) || amps == 0.0f) {
        if (ampReadingElapsedTime_[line][io] < MAX_READING_INTERVAL_MS) {
            result = lastValidAmps(line, io);
        }
        else {
            result = 0;
            lastValidAmps(line, io, 0);
        }
    }
    else {
        result = static_cast<int16_t>(amps);
        lastValidAmps(line, io, result);
        ampReadingElapsedTime_[line][io] = 0;
    }
    return result;
}

// ------------------------------------------------------------------
// Protected
// ------------------------------------------------------------------
void PowerSensor::setData(RVC_DGN dgn, uint8_t* sourceData)
{
    uint8_t* destination = dataBuffer();

    if (sourceData != nullptr) {
        if (destination != nullptr) {
            for (int i = AC_POINT_INSTANCE_INDEX; i < 8; ++i) {
                destination[i] = sourceData[i];
            }
        }
        const uint8_t line = lineOf(dgn, sourceData);
        const uint8_t io   = ioOf(dgn, sourceData);
        if (isValidSlot(line, io)) {
            memcpy(readings_[line][io], sourceData, DATA_SIZE);
        }
    }
}

CAN_frame_t* PowerSensor::buildCommand(RVC_DGN /*dgn*/)
{
    return nullptr;   // listen-only for power sensors
}

// ------------------------------------------------------------------
// Public behaviour
// ------------------------------------------------------------------
/**
void PowerSensor::attachView(const char* name, bool showCurrent, bool showFault)
{
    PowerSensorView* view = new PowerSensorView(this, name, showCurrent, showFault);
    addView(view);
}
    */

uint16_t PowerSensor::rmsVoltage(uint8_t line, uint8_t io)
{
    uint16_t result = 0;
    if (isValidSlot(line, io)) {
        uint16_t value = getACPointValue(readings_[line][io],
                                         AC_POINT_RMS_VOLTAGE_MSB_INDEX,
                                         AC_POINT_RMS_VOLTAGE_LSB_INDEX);
        if (value <= VAC_MAX) {
            result = validateVolts(line, io, (value - VAC_OFFSET) * VAC_PRECISION);
        }
    }
    return result;
}

int16_t PowerSensor::rmsCurrent(uint8_t line, uint8_t io)
{
    int16_t result = 0;
    if (isValidSlot(line, io)) {
        uint16_t value = getACPointValue(readings_[line][io],
                                         AC_POINT_RMS_CURRENT_MSB_INDEX,
                                         AC_POINT_RMS_CURRENT_LSB_INDEX);
        float amps = static_cast<float>(static_cast<int32_t>(value) - static_cast<int32_t>(AAC_ZERO)) * AAC_PRECISION;
        result = validateAmps(line, io, amps);
    }
    return result;
}

boolean PowerSensor::isOpenGroundFault(uint8_t /*line*/) const
{
    boolean result = false;
    const uint8_t* raw = dataBuffer();

    if (raw != nullptr) {
        uint8_t fault = raw[AC_POINT_FAULTS_INDEX] & OPEN_GROUND_FAULT_MASK;
        result = (fault == OPEN_GROUND_FAULT);
    }
    return result;
}

boolean PowerSensor::isOpenNeutralFault(uint8_t /*line*/) const
{
    boolean result = false;
    const uint8_t* raw = dataBuffer();

    if (raw != nullptr) {
        uint8_t fault = raw[AC_POINT_FAULTS_INDEX] & OPEN_NEUTRAL_FAULT_MASK;
        result = (fault == OPEN_NEUTRAL_FAULT);
    }
    return result;
}

boolean PowerSensor::isReversePolarityFault(uint8_t /*line*/) const
{
    boolean result = false;
    const uint8_t* raw = dataBuffer();

    if (raw != nullptr) {
        uint8_t fault = raw[AC_POINT_FAULTS_INDEX] & REVERSE_POLARITY_FAULT_MASK;
        result = (fault == REVERSE_POLARITY_FAULT);
    }
    return result;
}

boolean PowerSensor::isGroundCurrentFault(uint8_t /*line*/) const
{
    boolean result = false;
    const uint8_t* raw = dataBuffer();

    if (raw != nullptr) {
        uint8_t fault = raw[AC_POINT_FAULTS_INDEX] & GROUND_CURRENT_FAULT_MASK;
        result = (fault == GROUND_CURRENT_FAULT);
    }
    return result;
}

#endif // HOME_KIT_2
