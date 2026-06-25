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
        uint16_t        lastValidVolts;
        // uint16_t invalidVoltCount;
        elapsedMillis   voltReadingElapsedTime;
        uint16_t        lastValidAmps;
        // uint16_t invalidAmpCount;
        elapsedMillis   ampReadingElapsedTime;

    protected:
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
        inline uint16_t rmsVoltage(void) { 
            uint16_t value = getACPointValue(AC_POINT_RMS_VOLTAGE_MSB_INDEX, AC_POINT_RMS_VOLTAGE_LSB_INDEX);
            uint16_t result = 0;
            if (value <= VAC_MAX) {
                uint16_t tmpLastValidVolts = (value - VAC_OFFSET) * VAC_PRECISION;
                if ((tmpLastValidVolts > 0) && (tmpLastValidVolts <= ((VAC_MAX - VAC_OFFSET) * VAC_PRECISION))) { // valid voltage range
                    result = lastValidVolts = tmpLastValidVolts;
                    // invalidVoltCount = 0;
                    voltReadingElapsedTime = 0;
                } else if (tmpLastValidVolts == 0) { // zero volts is valid but we don't want to keep setting it to 0 if it is noise
                    if (voltReadingElapsedTime < MAX_READING_INTEVAL_MS) {
                        // invalidVoltCount++;
                        result = lastValidVolts;
                    } else {
                        result = lastValidVolts = 0; // volts are zero consistently
                        printf("PowerSensor::rmsVoltage - zero volt reading for more than 5 mins - setting to 0\n");
                    }
                }
            } 
            return result;
        }
        inline uint16_t rmsCurrent(void)  { 
            uint16_t value = getACPointValue(AC_POINT_RMS_CURRENT_MSB_INDEX, AC_POINT_RMS_CURRENT_LSB_INDEX);
            uint16_t result = 0;
            // printf("PowerSensor::rmsCurrent value %d\n", value);
            float tmp = (value - AAC_ZERO) * AAC_PRECISION;
            // printf("PowerSensor::rmsCurrent result %d\n", result);
            // IF NOT!!!!!!! in range!!
            if (!(tmp >= AAC_LOWER_LIMIT && tmp <= AAC_UPPER_LIMIT) || (tmp == 0.0F)) {
                if (ampReadingElapsedTime < MAX_READING_INTEVAL_MS) {
                    // invalidAmpCount++;
                    result = lastValidAmps;
                } else {
                    result = lastValidAmps = 0; // out of range
                    printf("PowerSensor::rmsCurrent - out of range amp reading for more than 5 mins - setting to 0\n");
                }

            } else {
                result = lastValidAmps = static_cast<uint16_t>(static_cast<int16_t>(tmp));
                ampReadingElapsedTime = 0;
            }
            return result;
        }
            
        inline uint16_t frequency(void) const  { return getACPointValue(AC_POINT_FREQUENCY_MSB_INDEX,   AC_POINT_FREQUENCY_LSB_INDEX); }

        // FAULTS
        boolean  isOpenGroundFault(void) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t groundFault = rawData[AC_POINT_FAULTS_INDEX] & OPEN_GROUND_FAULT_MASK;
                result = groundFault == OPEN_GROUND_FAULT;
            }
            return result;
        }

        boolean  isOpenNeutralFault(void) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t  neutralFault = rawData[AC_POINT_FAULTS_INDEX] & OPEN_NEUTRAL_FAULT_MASK;
                result = neutralFault == OPEN_NEUTRAL_FAULT;
            }
            return result;
        }


        boolean  isReversePolarityFault(void) const {
            boolean result = false;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t  reversePolarityFault = rawData[AC_POINT_FAULTS_INDEX] & REVERSE_POLARITY_FAULT_MASK;
                result = reversePolarityFault == REVERSE_POLARITY_FAULT;
            }
            return result;
        }


        boolean  isGroundCurrentFault(void) const {
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
        PowerSensor() : GenericDevice(), lastValidVolts(0), /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps(0), /* invalidAmpCount(0) */ ampReadingElapsedTime(0) {
            // Constructor implementation
        }

        PowerSensor(const PowerSensor& orig) : GenericDevice(orig), lastValidVolts(0), /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps(0), /* invalidAmpCount(0) */ ampReadingElapsedTime(0) {
            // Copy constructor implementation
        }

        PowerSensor(uint8_t address, uint8_t indx) : GenericDevice(address,indx), lastValidVolts(0), /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps(0), /* invalidAmpCount(0) */ ampReadingElapsedTime(0) { 
        }

        PowerSensor(uint8_t* data) : GenericDevice(data), lastValidVolts(0), /* invalidVoltCount(0), */ voltReadingElapsedTime(0), lastValidAmps(0), /* invalidAmpCount(0) */ ampReadingElapsedTime(0) {
            // Constructor with parameters implementation
        }

        virtual ~PowerSensor() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS) = 0;


};






#endif // AC_POINT_DEFINITION_H
#endif // ifdef HOME_KIT_2