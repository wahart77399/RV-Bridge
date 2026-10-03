#pragma once

#include "RVConstants.h"

#include "Arduino.h"
#include "GenericDevice.h"
#include "Packet.h"
#include "elapsedMillis.h"
#include "debug.h"

typedef enum {
    AC_POINT_INSTANCE_INDEX        = 0,
    AC_POINT_RMS_VOLTAGE_MSB_INDEX = 1,
    AC_POINT_RMS_VOLTAGE_LSB_INDEX = 2,
    AC_POINT_RMS_CURRENT_MSB_INDEX = 3,
    AC_POINT_RMS_CURRENT_LSB_INDEX = 4,
    AC_POINT_FREQUENCY_MSB_INDEX   = 5,
    AC_POINT_FREQUENCY_LSB_INDEX   = 6,
    AC_POINT_FAULTS_INDEX          = 7
} AC_POINT_DATA_INDECES;

typedef enum {
    OPEN_GROUND_NO_FAULT      = 0x00,
    OPEN_GROUND_FAULT         = 0x01,
    OPEN_NEUTRAL_NO_FAULT     = 0x00,
    OPEN_NEUTRAL_FAULT        = 0x04,
    REVERSE_POLARITY_NO_FAULT = 0x00,
    REVERSE_POLARITY_FAULT    = 0x10,
    GROUND_CURRENT_NO_FAULT   = 0x00,
    GROUND_CURRENT_FAULT      = 0x40
} AC_POINT_FAULTS;

typedef enum {
    OPEN_GROUND_FAULT_MASK      = 0x03,
    OPEN_NEUTRAL_FAULT_MASK     = 0x0c,
    REVERSE_POLARITY_FAULT_MASK = 0x30,
    GROUND_CURRENT_FAULT_MASK   = 0xc0
} AC_POINT_FAULT_MASK;

constexpr uint16_t BAD_DATA                = 0xffff;
constexpr float_t  VAC_PRECISION           = 0.05f;
constexpr float_t  AAC_PRECISION           = 0.05f;
constexpr uint16_t VAC_MAX                 = 3213;
constexpr uint16_t AAC_MAX                 = 3213;
constexpr uint16_t VAC_OFFSET              = 0;
constexpr uint16_t AAC_OFFSET              = 1600;
constexpr uint16_t AAC_ZERO                = 0x7d00;
constexpr uint32_t MAX_READING_INTERVAL_MS = 300000;

class PowerSensorView;
constexpr uint8_t MAX_LINES = 3;
constexpr uint8_t NUMIO = 2;
constexpr uint8_t OUTPUT_LINE = 0;
constexpr uint8_t INPUT_LINE = 1;
constexpr uint8_t NO_LINE = 0xff;

class PowerSensor : public GenericDevice {
    friend class PowerSensorView;

private:

    uint8_t       readings_[MAX_LINES][NUMIO][DATA_SIZE];
    uint16_t      lastValidVolts_[MAX_LINES][NUMIO];
    int16_t       lastValidAmps_[MAX_LINES][NUMIO];
    elapsedMillis voltReadingElapsedTime_[MAX_LINES][NUMIO];
    elapsedMillis ampReadingElapsedTime_[MAX_LINES][NUMIO];

    uint16_t lastValidVolts(uint8_t line, uint8_t io) const     { return lastValidVolts_[line][io]; }
    void     lastValidVolts(uint8_t line, uint8_t io, uint16_t v) { lastValidVolts_[line][io] = v; }
    int16_t  lastValidAmps(uint8_t line, uint8_t io) const      { return lastValidAmps_[line][io]; }
    void     lastValidAmps(uint8_t line, uint8_t io, int16_t a)   { lastValidAmps_[line][io] = a; }
    static bool isValidSlot(uint8_t line, uint8_t io)           { return (line < MAX_LINES) && (io < NUMIO); }

    uint16_t getACPointValue(const uint8_t* raw,
                             AC_POINT_DATA_INDECES msb,
                             AC_POINT_DATA_INDECES lsb) const;
    uint16_t validateVolts(uint8_t line, uint8_t io, float volts);
    int16_t  validateAmps(uint8_t line, uint8_t io, float amps);

protected:
    uint8_t*       dataBuffer()       { return getCurrentData(); }
    const uint8_t* dataBuffer() const { return getCurrentData(); }
    void setData(RVC_DGN dgn, uint8_t* sourceData) override;
    CAN_frame_t* buildCommand(RVC_DGN dgn) override;

    // which line / input-output an AC point message describes; NO_LINE if it is not an AC point
    virtual uint8_t lineOf(RVC_DGN /*dgn*/, const uint8_t* /*raw*/) const { return 0; }
    virtual uint8_t ioOf(RVC_DGN /*dgn*/, const uint8_t* /*raw*/) const   { return INPUT_LINE; }

public:
    PowerSensor() = delete;
    PowerSensor(const PowerSensor&) = delete;
    PowerSensor& operator=(const PowerSensor&) = delete;
    PowerSensor(PowerSensor&&) = delete;
    PowerSensor& operator=(PowerSensor&&) = delete;
    ~PowerSensor() = default;

    PowerSensor(uint8_t address, uint8_t instance);

    virtual void attachView(const char* name, bool showCurrent = true, bool showFault = true) = 0;

    virtual uint16_t rmsVoltage(uint8_t line = 0, uint8_t io = INPUT_LINE);
    virtual int16_t  rmsCurrent(uint8_t line = 0, uint8_t io = INPUT_LINE);

    boolean isOpenGroundFault(uint8_t line = 0) const;
    boolean isOpenNeutralFault(uint8_t line = 0) const;
    boolean isReversePolarityFault(uint8_t line = 0) const;
    boolean isGroundCurrentFault(uint8_t line = 0) const;

    virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer,
                                   uint8_t val = SOURCE_ADDRESS)=0;
};
