
#ifndef THERMOSTAT_H
#define THERMOSTAT_H // once I'm ready to define this, move this below ifndef
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
#include "Arduino.h"
#include "GenericDevice.h"
#include "DGN.h"
#include "Packet.h"

#include "HVAC_defintions.h"

constexpr double  tempCOffset = -273.0;
constexpr double  tempCScale = 0.03125;
constexpr double  tempCRoundingOffset = -0.25;

class HVAC_Thermostat : public GenericDevice {

    private:
        // for THERMOSTAT_STATUS_1 which will be our base data
        const uint8_t THERMOSTAT_INDEX = 0;
        const uint8_t THERMOSTAT_OPERATING_MODE = 1;
        const uint8_t THERMOSTAT_FAN_SPEED = 2;
        const uint8_t THERMOSTAT_HEAT_TEMP_MSB = 3;
        const uint8_t THERMOSTAT_HEAT_TEMP_LSB = 4;
        const uint8_t THERMOSTAT_COOL_TEMP_MSB = 5;
        const uint8_t THERMOSTAT_COOL_TEMP_LSB = 6;
        const uint8_t OPERATING_MODE_MASK = 0x0f;
        const uint8_t FAN_MODE_MASK = 0x30;
        const uint8_t SCHEDULE_MODE_MASK = 0xc0;
         // (degF− 32) × 5/9 = degC | (\°C × 9/5) + 32 = °F
        const uint8_t MAX_TEMP = 100;


        // uint16_t*   temperatureData;
        const uint8_t HEAT_TEMP_INDEX = 0;
        const uint8_t COLD_TEMP_INDEX = 1;
    public:
        static inline double convToTempC(uint16_t value) {
	        return tempCOffset + value * tempCScale + tempCRoundingOffset;  
        }
        static inline uint16_t convFromTempC(double tempC) {
	        return (tempC - tempCOffset + 0.5) / tempCScale;
        }

        // uint16_t*   getTemperatureData(void) const { return temperatureData; }
        //void setTemperatureData(uint8_t* rawData) {
        //    if (rawData != nullptr)
        //        temperatureData = (uint16_t*) &(rawData[THERMOSTAT_HEAT_TEMP_MSB]);
        //}

        

    private:
        friend class ThermostatView;

        void setFanMode(const RVCFanMode setting) {
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t mode = rawData[THERMOSTAT_OPERATING_MODE];
                uint8_t mask = SCHEDULE_MODE_MASK | OPERATING_MODE_MASK;
                rawData[THERMOSTAT_OPERATING_MODE] = (mode & mask) | setting;
            }
        }
        void setOperatingMode(const RVCMode md) {
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t mode = rawData[THERMOSTAT_OPERATING_MODE];
                uint8_t mask = SCHEDULE_MODE_MASK | FAN_MODE_MASK;
                rawData[THERMOSTAT_OPERATING_MODE] = (mode & mask) | md;
            }
        }

        void setFanSpeed(const RVCForcedFan speed) {
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                rawData[THERMOSTAT_FAN_SPEED] = speed;
            }
        }

        void setCoolTemp(const uint16_t tempC) {
            // uint16_t* tempData = getTemperatureData();
            uint8_t* tempData = getCurrentData();
            if (tempData != nullptr) {
                // tempData[COLD_TEMP_INDEX] = temp;
                uint16_t temp = convFromTempC(tempC);
                tempData[6] = temp >> 8;
                tempData[5] = temp & 0xff;
                printf("HVAC_Thermostat::setCoolTemp end - temp = tempData[5] %d, tempData[6] %d, Temp = %d", tempData[5], tempData[6], tempC);
            }
        }

        // all temps coming from °C to °C (storing in °C)
        void setHeatTemp(const uint16_t tempC) {
            uint8_t* tempData = getCurrentData();
            if (tempData != nullptr) {
                // tempData[COLD_TEMP_INDEX] = temp;
                uint16_t temp = convFromTempC(tempC);
                tempData[4] = temp >> 8;
                tempData[3] = temp & 0xff;
            }
        }

        const RVCFanMode getFanMode(void) const {
            RVCFanMode modeResult = AUTO_FAN_MODE;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                modeResult = (RVCFanMode) (rawData[THERMOSTAT_OPERATING_MODE] & FAN_MODE_MASK);
            }
            return modeResult;
        }

        const RVCForcedFan getFanSpeed(void) const {
            RVCForcedFan speed = AUTO_FAN;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                speed = (RVCForcedFan) (rawData[THERMOSTAT_FAN_SPEED]);
            }
            return speed;
        }

        const RVCMode getOperatingMode(void) const {
            RVCMode modeResult = AUTO;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                modeResult = (RVCMode) (rawData[THERMOSTAT_OPERATING_MODE] & OPERATING_MODE_MASK);
            }
            return modeResult;
        }

        const uint16_t getCoolTemp(void) const {
            uint16_t temp = DEFAULT_TEMP;
            // uint8_t* temp8 = &temp;
            uint8_t* tempData = getCurrentData();
            // uint16_t* tempData = getTemperatureData();
            if (tempData != nullptr) {
                uint16_t tmp = convToTempC(tempData[THERMOSTAT_COOL_TEMP_LSB]<<8 | tempData[THERMOSTAT_COOL_TEMP_MSB]); 
                temp = (tmp < MAX_TEMP) ? tmp : DEFAULT_TEMP; //convFromTempC((double) tempC);

            }
            return temp;
        }

        const uint16_t getHeatTemp(void) const {
                       uint16_t temp = DEFAULT_TEMP;
            // uint8_t* temp8 = &temp;
            uint8_t* tempData = getCurrentData();
            // uint16_t* tempData = getTemperatureData();
            if (tempData != nullptr) {
                uint16_t tmp = convToTempC(tempData[THERMOSTAT_HEAT_TEMP_LSB]<<8 | tempData[THERMOSTAT_HEAT_TEMP_MSB]); 
                temp = (tmp < MAX_TEMP) ? tmp : DEFAULT_TEMP; //convFromTempC((double) tempC);
            }
            return temp;
        }

        boolean isFanAuto(void) const { return getFanMode() == AUTO_FAN_MODE; }

    protected:
        inline uint8_t* getCurrentData(void) const {
            return GenericDevice::getCurrentData();
        }
        
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                uint8_t i = 1;
                if (rawData != nullptr) {// set the current data to the new data
                    switch (dgn) {
                        // COMMAND_1 and STATUS_1 have the same data format
                        case (THERMOSTAT_COMMAND_1):
                        case (THERMOSTAT_STATUS_1):
                            for (i = 1; i < 8; i++) // 8 bytes in a data payload
                                if (((i == 3) || (i == 4) || (i == 5) || (i==6)) && (data[i] == 0xff)) { // deals with out of bounds data
                                    rawData[i] = 0x24;
                                } else {
                                    rawData[i] = data[i];
                                }
                            break;
                    
                        default:
                            printf("********** WARNING: THESE THERMOSTAT DGNs ARE NOT SUPPORTED************* \n");
                            break;
                    }
                }
            }
        }

        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);

    public:
        HVAC_Thermostat() : GenericDevice() { //, temperatureData(nullptr) {
            // Constructor implementation
            // setTemperatureData(getCurrentData());
        }

        HVAC_Thermostat(const HVAC_Thermostat& orig) : GenericDevice(orig) { // , temperatureData(nullptr) {
            // Copy constructor implementation
            // setTemperatureData(getCurrentData());
        }

        HVAC_Thermostat(uint8_t address, uint8_t instance, uint8_t grp, std::list <RVC_DGN> dgns) : GenericDevice(address, instance, grp, dgns) { // , temperatureData(nullptr) {     
            // setTemperatureData(getCurrentData());
        }

        HVAC_Thermostat(uint8_t* data) : GenericDevice(data) { //  temperatureData(nullptr) {
            // Constructor with parameters implementation
            // setTemperatureData(getCurrentData());
        }

        virtual ~HVAC_Thermostat() {
            // Destructor implementation
        } 
        // virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* sendData = nullptr, uint8_t sAddress = SOURCE_ADDRESS); 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS);
};

#endif