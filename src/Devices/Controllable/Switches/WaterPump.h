
#ifndef WATER_PUMP_H
#define WATER_PUMP_H // once I'm ready to define this, move this below ifndef
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


constexpr uint8_t WATER_PUMP_INDEX=0x00;

class WaterPump : public GenericDevice {
    private:
        const uint8_t PUMP_MASK = 0x03;
        const uint8_t COMPLIMENT_MASK = 0xfc;
        const uint8_t PUMP_ENABLED = 0x01;


        friend class WaterPumpView;

        boolean isPumpOn(void) {
           uint8_t* rawData = getCurrentData();
            boolean result = false;
            if (rawData != nullptr) { // set the current data to the new data 
                uint8_t pumpData = rawData[WATER_PUMP_INDEX] & PUMP_MASK;
                result = (pumpData == PUMP_ENABLED);
            }
            return result; 
        }

    protected:
        inline uint8_t* getCurrentData(void) const {
            return GenericDevice::getCurrentData();
        }
        
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                if (rawData != nullptr) {// set the current data to the new data
                    switch (dgn) {
                        case (WATER_PUMP_COMMAND):
                        case (WATER_PUMP_STATUS):
                            rawData[WATER_PUMP_INDEX] = data[WATER_PUMP_INDEX];
                            break;
                    
                        default:
                        // do nothing
                            break;
                    }
                }
            }
        }

        void turnPumpOn(boolean value) {
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint8_t onOff = value & PUMP_MASK;
                uint8_t otherData = rawData[WATER_PUMP_INDEX] & COMPLIMENT_MASK;
                rawData[WATER_PUMP_INDEX] = otherData | onOff;
                printf("WaterPump::turnPumpOn - rawData[WATER_PUMP_INDEX]=%#x\n", rawData[WATER_PUMP_INDEX]);
            }
        }
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);

    public:
        WaterPump() : GenericDevice() {
            // Constructor implementation
        }

        WaterPump(const WaterPump& orig) : GenericDevice(orig) {
            // Copy constructor implementation
        }

        WaterPump(uint8_t address) : GenericDevice(address, WATER_PUMP_INDEX) { 
        }

        WaterPump(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
        }

        virtual ~WaterPump() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS);
};

#endif
