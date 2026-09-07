#include "RVConstants.h"
#ifdef HOME_KIT_2

#ifndef AC_POINT_DEFINITION_H
#define AC_POINT_DEFINITION_H

/* *********************************************************************************************************************************************************
 * This defnition comes from Table 6.1.2 of the RVC-Specification - it is used for Generatore AC Status, Inverter AC Status, Charger AC Status and 
 * ATS AC Status
 * 
   ********************************************************************************************************************************************************* */
#include "Arduino.h"
#include "GenericDevice.h"
#include "PowerSensorView.h"
#include "Packet.h"
#include "elapsedMillis.h"

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
    OPEN_GROUND_NO_FAULT            = 0x00,  // xxxx xx00
    OPEN_GROUND_FAULT               = 0x01,  // xxxx xx01
    OPEN_NEUTRAL_NO_FAULT           = 0x00,  // xxxx 00xx
    OPEN_NEUTRAL_FAULT              = 0x04,  // xxxx 01xx
    REVERSE_POLARITY_NO_FAULT       = 0x00,  // xx00 xxxx
    REVERSE_POLARITY_FAULT          = 0x10,  // xx01 xxxx
    GROUND_CURRENT_NO_FAULT         = 0x00,  // 00xx xxxx
    GROUND_CURRENT_FAULT            = 0x40   // 01xx xxxx
} AC_POINT_FAULTS;

typedef enum {
    OPEN_GROUND_FAULT_MASK          = 0x03,  // 0000 0011
    OPEN_NEUTRAL_FAULT_MASK         = 0x0c,  // 0000 1100
    REVERSE_POLARITY_FAULT_MASK     = 0x30,  // 0011 0000
    GROUND_CURRENT_FAULT_MASK       = 0xc0  // 1100 0000
} AC_POINT_FAULT_MASK;

constexpr uint16_t BAD_DATA = 0xffff;
const float_t VAC_PRECISION = 0.05;
const float_t AAC_PRECISION = 0.05;
const uint16_t VAC_MAX = 3213;
const uint16_t AAC_MAX = 3213;
const uint16_t VAC_OFFSET = 0;
const uint16_t AAC_OFFSET = 1600;
const uint16_t AAC_ZERO = 0x7d00;
// const uint16_t MAX_INVALID_COUNT = 5;
const uint32_t MAX_READING_INTEVAL_MS = 300000; // 5 mins
#include "RVConstants.h"
// const float_t AAC_LOWER_LIMIT = -80.0;
// const float_t AAC_UPPER_LIMIT = 80.0;


class PowerSensor : public GenericDevice {
    private:
        uint16_t        lastValidVolts[3];
        // uint16_t invalidVoltCount;
        elapsedMillis   voltReadingElapsedTime;
        uint16_t        lastValidAmps[3];
        // uint16_t invalidAmpCount;
        elapsedMillis   ampReadingElapsedTime;

    protected:
        inline uint16_t validateVolts(uint8_t line, uint16_t volts) {
            uint16_t result = volts;
            float_t tmpVolts = static_cast<float>(volts);
            // printf("PowerSensor::validateVolts line %d, volts %d\n", line, volts);
            if (tmpVolts > ((VAC_MAX - VAC_OFFSET) * VAC_PRECISION)) {
                // printf("PowerSensor::validateVolts - volts %d out of range for line %d\n", volts, line);
                result = lastValidVolts[line];
            } else if (volts == 0) {
                // printf("PowerSensor::validateVolts - zero volt reading for line %d\n", line);
                if (voltReadingElapsedTime < MAX_READING_INTEVAL_MS) {
                    result = lastValidVolts[line];
                } else {
                    result = lastValidVolts[line] = 0; // volts are zero consistently
                    // printf("PowerSensor::validateVolts - zero volt reading for more than 5 mins - setting to 0\n");
                }
            } else {
                // printf("PowerSensor::validateVolts - valid volt reading for line %d, volts %d\n", line, volts);
                lastValidVolts[line] = volts;
                voltReadingElapsedTime = 0;
            }
            return result;
        }

        inline uint16_t validateAmps(uint8_t line, float_t amps) {
            uint16_t result = static_cast<uint16_t>(static_cast<int16_t>(amps));
            // printf("PowerSensor::validateAmps result %d\n", result);
            // IF NOT!!!!!!! in range!!
            if (!(amps >= AAC_LOWER_LIMIT && amps <= AAC_UPPER_LIMIT) || (amps == 0.0F)) {
                if (ampReadingElapsedTime < MAX_READING_INTEVAL_MS) {
                    // invalidAmpCount++;
                    result = lastValidAmps[line];
                } else {
                    result = lastValidAmps[line] = 0; // out of range
                    // printf("PowerSensor::rmsCurrent - out of range amp reading for more than 5 mins - setting to 0\n");
                }

            } else {
                result = lastValidAmps[line] = static_cast<uint16_t>(static_cast<int16_t>(amps));
                ampReadingElapsedTime = 0;
            }
            return result;
        }

        uint16_t getACPointValue(AC_POINT_DATA_INDECES msbIndex, AC_POINT_DATA_INDECES lsbIndex) const {
            uint8_t* rawData = getCurrentData();
            uint16_t result = BAD_DATA;
            if ((rawData != nullptr) && ((msbIndex > AC_POINT_INSTANCE_INDEX) && (lsbIndex < AC_POINT_FAULTS_INDEX) && ((lsbIndex - msbIndex) == 1))) {
                // printf("PowerSensor::getACPointValue - passed test\n");
                result = getLilEndian(rawData[msbIndex], rawData[lsbIndex]); // [lsbIndex]<<8 | rawData[msbIndex];
            }
            return result;
        }

        void setACPointValue(AC_POINT_DATA_INDECES msbIndex, AC_POINT_DATA_INDECES lsbIndex, uint16_t newValue) {
            uint8_t* rawData = getCurrentData();
            setLilEndian(rawData, msbIndex, lsbIndex, newValue);
        }

        friend class PowerSensorView;
        friend class GeneratorView;

        // power values
        virtual uint16_t rmsVoltage(uint8_t line=0) { 
            uint16_t value = getACPointValue(AC_POINT_RMS_VOLTAGE_MSB_INDEX, AC_POINT_RMS_VOLTAGE_LSB_INDEX);
            uint16_t result = 0;
            if (value <= VAC_MAX) {

                result = validateVolts(line, (value - VAC_OFFSET) * VAC_PRECISION);
            } 
            return result;
        }

        virtual uint16_t rmsCurrent(uint8_t line=0) { 
            uint16_t value = getACPointValue(AC_POINT_RMS_CURRENT_MSB_INDEX, AC_POINT_RMS_CURRENT_LSB_INDEX);
            // printf("PowerSensor::rmsCurrent value %d\n", value);
            float tmp = static_cast<float>((value - AAC_ZERO) * AAC_PRECISION);
            return validateAmps(line, tmp);
        }
            
        inline uint16_t frequency(uint8_t line=0) const  { return getACPointValue(AC_POINT_FREQUENCY_MSB_INDEX,   AC_POINT_FREQUENCY_LSB_INDEX); }

        // FAULTS
        boolean  isOpenGroundFault(uint8_t line=0) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t groundFault = rawData[AC_POINT_FAULTS_INDEX] & OPEN_GROUND_FAULT_MASK;
                result = groundFault == OPEN_GROUND_FAULT;
            }
            return result;
        }

        boolean  isOpenNeutralFault(uint8_t line=0) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t  neutralFault = rawData[AC_POINT_FAULTS_INDEX] & OPEN_NEUTRAL_FAULT_MASK;
                result = neutralFault == OPEN_NEUTRAL_FAULT;
            }
            return result;
        }


        boolean  isReversePolarityFault(uint8_t line=0) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t  reversePolarityFault = rawData[AC_POINT_FAULTS_INDEX] & REVERSE_POLARITY_FAULT_MASK;
                result = reversePolarityFault == REVERSE_POLARITY_FAULT;
            }
            return result;
        }


        boolean  isGroundCurrentFault(uint8_t line=0) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t  reversePolarityFault = rawData[AC_POINT_FAULTS_INDEX] & GROUND_CURRENT_FAULT_MASK;
                result = reversePolarityFault == GROUND_CURRENT_FAULT;
            }
            return result;
        }


        virtual void setData(RVC_DGN dgn, uint8_t* sourceData) {
            uint8_t* destinationData = getCurrentData();
            if ((sourceData != nullptr) && (destinationData != nullptr)) {
                for (int i = AC_POINT_INSTANCE_INDEX; i < 8; i++) {
                    destinationData[i] = sourceData[i];
                }
            }
        }

        virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the ATS - we listen only

    public:
        PowerSensor() : GenericDevice(), lastValidVolts{0,0,0}, /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps{0,0,0}, /* invalidAmpCount(0) */ ampReadingElapsedTime(0) {
            // Constructor implementation
        }

        PowerSensor(const PowerSensor& orig) : GenericDevice(orig), lastValidVolts{0,0,0}, /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps{0,0,0}, /* invalidAmpCount(0) */ ampReadingElapsedTime(0) {
            // Copy constructor implementation
        }

        PowerSensor(uint8_t address, uint8_t indx) : GenericDevice(address,indx), lastValidVolts{0,0,0}, /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps{0,0,0}, /* invalidAmpCount(0) */ ampReadingElapsedTime(0) { 
        }

        PowerSensor(uint8_t* data) : GenericDevice(data), lastValidVolts{0,0,0}, /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps{0,0,0}, /* invalidAmpCount(0) */ ampReadingElapsedTime(0) {
            // Constructor with parameters implementation
        }

        virtual ~PowerSensor() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS) = 0;


};






#endif // AC_POINT_DEFINITION_H
#endif // ifdef HOME_KIT_2