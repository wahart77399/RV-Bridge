
#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef BATTERY_H
#define BATTERY_H // once I'm ready to define this, move this below ifndef

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

//#define CUSTOM_CHAR_HEADER
#include "Arduino.h"
#include "GenericDevice.h"
#include "RVConstants.h"
#include "BatteryDefinitions.h"
//#include "Thermostat.h"
#include "byteswap.h"

class Battery : public GenericDevice {
    private:
         
        uint8_t* source2Data;
        uint8_t* source3Data;
        uint8_t* source4Data;

        const uint8_t TIME_TO_EMPTY = 0x00;
        const uint8_t TIME_TO_FULL = 0x01;
        friend class BatteryView;
        uint8_t* getSource1Data(void) const { return getCurrentData();} 
        uint8_t* getSource2Data(void) const { return source2Data; }
        uint8_t* getSource3Data(void) const { return source3Data; }
        uint8_t* getSource4Data(void) const { return source4Data; }

        void setDataFromSource(uint8_t* source, uint8_t* destination) {
            if ((source != nullptr) && (destination != nullptr)) {
                for (int i = DC_SOURCE_PRIORITY_INDEX; i < (DC_SOURCE_CURRENT_LSB_2_INDEX + 1); i++)
                    destination[i] = source[i];
            }
        }

        const DC_SOURCE_INSTANCE_DEFINITION getSourceDefinition(void) const {
            DC_SOURCE_INSTANCE_DEFINITION result = INVALID_SOURCE;
            uint8_t* data = getCurrentData();
            if (data != nullptr)
                result = (DC_SOURCE_INSTANCE_DEFINITION) data[DC_SOURCE_INSTANCE_INDEX];
            return result;
        }

        const DC_SOURCE_PRIORITY_DEFINTION getPirorityDefinition(void) const {
            DC_SOURCE_PRIORITY_DEFINTION result = NO_PRIORITY_ALWAYS_RPT;
            uint8_t* data = getCurrentData();
            if (data != nullptr)
                result = (DC_SOURCE_PRIORITY_DEFINTION) data[DC_SOURCE_PRIORITY_INDEX];
            return result;
        }

        // SOURCE 1 STATUS Data
        const uint16_t directCurrentVoltage(void) const {
            uint8_t* data = getSource1Data();
            uint16_t value = 0;
            uint16_t result = 0;
            if (data != nullptr) {
                value = getLilEndian(data[DC_SOURCE_VOLTAGE_MSB_INDEX], data[DC_SOURCE_VOLTAGE_LSB_INDEX]);
                if (value < VDC_MAX)
                    result = (value - VDC_OFFSET) * VDC_PRECISION;
            }
            return result;
        }

        const uint32_t directCurrentAmperage(void) const {
            uint8_t* data = getSource1Data();
            uint32_t result = 0;
            uint32_t value = 0;
            if (data != nullptr) {
                //uint8_t* result8 = (uint8_t* ) (&result);
                uint32_t* data32 = (uint32_t* )&(data[DC_SOURCE_CURRENT_MSB_1_INDEX]);
                // uint8_t*  tmp = (uint8_t* ) (&value); 

                // printf("Battery::directCurrentAmperage d[4]=%8x, d[5]=%8x, d[6]=%8x, d[7]=%8x\n", data[4], data[5], data[6], data[7]);
                // value = ((*data32 & 0xFF000000) >> 24) |
                //          ((*data32 & 0x00FF0000) >> 8) |
                //         ((*data32 & 0x0000FF00) << 8) |
                //         ((*data32 & 0x000000FF) << 24);
                // int32_t tmp2 = (int32_t) value;
                // printf("Battery::directCurrentAmperage data32 = %d\n", *data32);
                // result = (value - ADC_ZERO) * ADC_PRECISION
                int32_t conversion = (*data32 - ADC_ZERO); // * ADC_PRECISION;
                // printf("Battery::directCurrentAmperage value %d, conversion %d\n", *data32, conversion);
                conversion = conversion * ADC_PRECISION;
                // printf("Battery::directCurrentAmperage result %d\n", conversion);
                result = conversion;
            }
            return result;
        }

        // SOURCE 2 STATUS DATA
        const uint16_t temperature(void) const {
            uint8_t* data = getSource2Data();
            uint16_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = convToTempC(getLilEndian(data[DC_SOURCE_TEMPERATURE_MSB_INDEX], data[DC_SOURCE_TEMPERATURE_LSB_INDEX]));
                // printf("Battery::temperature = %d\n", result);
            }
            return result;
        }

        const uint8_t level(void) const {
            uint8_t* data = getSource2Data();
            uint8_t result = MAX_RVC_PERCENT * RVC_PERCENT_PRECISION; // default to 125%
            if (data != nullptr) {
                uint8_t tmp = data[DC_SOURCE_STATE_OF_CHARGE_INDEX];
                result = (tmp <= MAX_RVC_PERCENT) ? tmp : MAX_RVC_PERCENT;
                result = result * RVC_PERCENT_PRECISION;
                // printf("Battery::level = %d\n", result);
            }
            return result;
        }
        /* REMOVED - not currently used by Newmar
        const uint16_t timeRemaining(void) const {
            uint8_t* data = getSource2Data();
            uint16_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                uint8_t tmp = getLilEndian(data[DC_SOURCE_TIME_REMAINING_MSB_INDEX], data[DC_SOURCE_TIME_REMAINING_LSB_INDEX]);
                if (tmp != 0xffff) {
                    result = tmp;
                    printf("Battery::timeRemaining = %d\n", result);
                }
            } else {
                printf("Battery::timeRemaining data is not valid\n");
            }
            return result;
        }
        
        const boolean timeToEmpty(void) const {
            boolean empty = true;
            uint8_t* data = getSource2Data();
            if (data != nullptr) {
                empty = data[DC_SOURCE_TIME_INTERPRETATION] == TIME_TO_EMPTY;
                // printf("Battery::timeToEmpty = %d\n", empty);
            }
            return empty;
        } 
        */

        // SOURCE 3 STATUS DATA
        /* REMOVED - not currently used by Newmar
        const uint8_t health(void) const {
            uint8_t* data = getSource3Data();
            uint8_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = data[DC_SOURCE_HEALTH_STATE_INDEX];
                printf("Battery::health = %d\n", result);
            } else {
                printf("Battery::health data is not valid\n");
            }
            return result;
        }
        */

        const uint16_t remainingCapacity(void) const {
            uint8_t* data = getSource3Data();
            uint16_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = convFromTempC(getLilEndian(data[DC_SOURCE_CAP_REMAINS_MSB_INDEX], data[DC_SOURCE_CAP_REMAINS_LSB_INDEX]));
                // printf("Battery::remaining capacity = %d\n", result);
            } else {
                printf("Battery::remaining capacity data is not valid\n");
            }
            return result;
        }

        /* REMOVED - not currently used by Newmar
        const uint8_t relativeCapacity(void) const {
            uint8_t* data = getSource3Data();
            uint8_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = data[DC_SOURCE_RELATIVE_CAP_INDEX];
                printf("Battery:relativeCapacity percenatge = %d\n", result);
            } else {
                printf("Battery::relativeCapacity data is not valid\n");
            }
            return result;
        }
        */
        const uint16_t rmsRipple(void) const {
            uint8_t* data = getSource3Data();
            uint16_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = convFromTempC(getLilEndian(data[DC_SOURCE_AC_RIPPLE_MSB_INDEX], data[DC_SOURCE_AC_RIPPLE_LSB_INDEX]));
                // printf("Battery::rmsRipple = %d\n", result);
            }
            return result;
        }

        // SOURCE 4 STATUS DATA
        /* REMOVED - not currently used by Newmar
        const CHARGE_STATE chargeState(void) const {
            uint8_t* data = getSource4Data();
            CHARGE_STATE result = UNDEFINED_CHARGE_STATE;
            if (data != nullptr) {
                result = (CHARGE_STATE) data[DC_SOURCE_DESORED_CHARGE_STATE_INDEX];
                printf("Battery::chargeState = %d\n", result);
            } else {
                printf("Battery::chargeState data is not valid\n");
            }
            return result;
        }

        const uint16_t desiredVoltage(void) const {
            uint8_t* data = getSource4Data();
            uint16_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = getLilEndian(data[DC_SOURCE_DESIRED_VOLT_MSB_INDEX], data[DC_SOURCE_DESIRED_VOLT_LSB_INDEX]);
                printf("Battery::desiredVoltage = %d\n", result);
            } else {
                printf("Battery::desiredVoltage data is not valid\n");
            }
            return result;
        }

        const uint16_t desiredCurrent(void) const {
            uint8_t* data = getSource4Data();
            uint16_t result = OUT_OF_RANGE_DATA;
            if (data != nullptr) {
                result = getLilEndian(data[DC_SOURCE_DESIRED_AMP_MSB_INDEX], data[DC_SOURCE_DESIRED_AMP_LSB_INDEX]);
                printf("Battery::desiredCurrent = %d\n", result);
            } else {
                printf("Battery::desiredCurrent data is not valid\n");
            }
            return result;
        }

        const BATTERY_TYPE type(void) const {
            uint8_t* data = getSource4Data();
            BATTERY_TYPE result = FLOODED_BATTERY;
            if (data != nullptr) {
                result = (BATTERY_TYPE) data[DC_SOURCE_BATTERY_TYPE_INDEX];
                printf("Battery::battery type = %d\n", result);
            } else {
                printf("Battery::battery type data is not valid\n");
            }
            return result;  
        }
        */

    protected:
        
        // virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the ATS - we listen only
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                switch (dgn) {
                    case (DC_SOURCE_STATUS_1):
                        setDataFromSource(data, getSource1Data());
                        break;
                    case (DC_SOURCE_STATUS_2):
                        setDataFromSource(data, getSource2Data());
                        break;
                    case (DC_SOURCE_STATUS_3):
                        setDataFromSource(data, getSource3Data());
                        break;
                    case (DC_SOURCE_STATUS_4):
                        setDataFromSource(data, getSource4Data());
                        break;
                    default:
                        printf("Battery::setData failed = no matching dgn - dgn= %0x\n", dgn);
                }
            }
        }

        virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the BATTERY - we listen only


    public:
        Battery() : GenericDevice() {
            // Constructor implementation
        }


        Battery(const Battery& orig) : GenericDevice(orig) {
            // Copy constructor implementation
        }


        Battery(uint8_t address, uint8_t index, uint8_t pri) : GenericDevice(address, index) { 
            uint8_t* data = getCurrentData();
            if (data != nullptr)
                data[DC_SOURCE_PRIORITY_INDEX] = pri;
            source2Data = new uint8_t[sizeof(uint8_t) * 8];
            source2Data[DC_SOURCE_INSTANCE_INDEX] = index;
            source2Data[DC_SOURCE_PRIORITY_INDEX] = pri;
            for (uint8_t i = 2; i < 8; i++) {
                source2Data[i] = OUT_OF_RANGE_DATA; // initialize to invalid data
            }
            source3Data = new uint8_t[sizeof(uint8_t) * 8];
            source3Data[DC_SOURCE_INSTANCE_INDEX] = index;
            source3Data[DC_SOURCE_PRIORITY_INDEX] = pri;
            for (uint8_t i = 2; i < 8; i++) {
                source3Data[i] = OUT_OF_RANGE_DATA; // initialize to invalid data
            }
            source4Data = new uint8_t[sizeof(uint8_t) * 8];
            source4Data[DC_SOURCE_INSTANCE_INDEX] = index;
            source4Data[DC_SOURCE_PRIORITY_INDEX] = pri;
            for (uint8_t i = 2; i < 8; i++) {
                source4Data[i] = OUT_OF_RANGE_DATA; // initialize to invalid data
            }
        }


        Battery(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
        }


        virtual ~Battery() {
            // Destructor implementation
        } 


        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // this is only to deal with Batetery dgns
};

#endif
#endif // #endif // BATTERY_H // HOME_KIT_2