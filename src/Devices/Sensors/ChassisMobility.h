
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

constexpr uint8_t DEFAULT_CHASSIS_INDEX = 0;
// constexpr uint8_t CHASSIS_GROUP=0;

class ChassisMobility : public GenericDevice {
    private:
        const uint8_t CHASSIS_MOBILITY_BRAKE_INDEX=4;
        const uint8_t BRAKE_MASK = 0x03;
        const uint8_t BRAKE_ENGAGED = 0x01;
        static ChassisMobility* instance;
    

        boolean isBrakeEngaged(void) {
            uint8_t* rawData = getCurrentData();
            boolean result = false;
            if (rawData != nullptr) { // set the current data to the new data 
                uint8_t brakeData = rawData[CHASSIS_MOBILITY_BRAKE_INDEX] & BRAKE_MASK;
                result = (brakeData == BRAKE_ENGAGED);
            }
            return result;
        }

    protected:
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            printf("ChassisMobility::setData: dgn=%d\n",dgn);
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                if (rawData != nullptr) { // set the current data to the new data
                    switch (dgn) {
                        case (CHASSIS_MOBILITY_COMMAND):
                            printf("ChassisMobility::setData: **************** WE DO NOT SUPPORT CHASSIS MOBILITY COMMAND ******************\n");
                            break;
                        case CHASSIS_MOBILITY_STATUS:
                            printf("ChassisMobility::setData: CHASSIS_MOBILITY_STATUS = %d", data[CHASSIS_MOBILITY_BRAKE_INDEX]);
                            rawData[CHASSIS_MOBILITY_BRAKE_INDEX] = data[CHASSIS_MOBILITY_BRAKE_INDEX];
                            break;
                        case CHASSIS_MOBILITY_STATUS_2:
                            printf("ChassisMobility::setData: **************** WE DO NOT SUPPORT CHASSIS MOBILITY STATUS 2 ******************\n");
                            break;
                        default:
                        // do nothing
                            break;
                    }
                }
            }
        }
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn) { return nullptr; /* do nothing */ }


        ChassisMobility() : GenericDevice() {
            // Constructor implementation
            ;
        }
        ChassisMobility(const ChassisMobility& orig) : GenericDevice(orig) {
            // Copy constructor implementation
            ;
        }

        ChassisMobility(uint8_t address) 
            : GenericDevice(address, DEFAULT_CHASSIS_INDEX) {} 

        ChassisMobility(uint8_t* data) : GenericDevice(data) {
            ; // Constructor with parameters implementation
        }

    public:
        boolean isChassisMobilityDGN(RVC_DGN dgn) { 
            boolean result = false;
            switch (dgn) {
                case (CHASSIS_MOBILITY_COMMAND):
                    break;
                case (CHASSIS_MOBILITY_STATUS):
                    break;
                case (CHASSIS_MOBILITY_STATUS_2):
                    result = true;
                    break;
                default:
                    break;
            }
            return result;
        }

        

        static ChassisMobility* getInstance(void) {
            if (instance == nullptr) {
                instance = new ChassisMobility(148); 
            }
            return instance;
        }

        virtual ~ChassisMobility() {
            // Destructor implementation
            
        } 

        static boolean isParked(void) {
            boolean result = false;
            ChassisMobility* myInstance = ChassisMobility::getInstance();
            if (instance != nullptr)
                result = instance->isBrakeEngaged();
            return result;
        }


        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* sendData = nullptr, uint8_t sAddress = SOURCE_ADDRESS); // {} // do nothing
};

#endif
