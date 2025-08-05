
#ifndef DOOR_LOCKS_H
#define DOOR_LOCKS_H 
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
#include "DoorLockView.h"
#include "Packet.h"


class DoorLock : public GenericDevice {
    private:
        static const uint8_t LOCK_INDEX = 1; // index of the lock state in the data

        friend class DoorLockView; // allow DoorLockView to access private members

        const uint8_t LOCK_IT = 0x01;
        const uint8_t UNLOCK_IT = 0x00;
        // boolean locked = false; // flag to indicate if the door is locked or not
        
        inline void setLockedFlag(bool lock) {
            //locked = lock;
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) {
                rawData[LOCK_INDEX] = (lock ? LOCK_IT : UNLOCK_IT);
            }
            updateViews(); // update all views
        } 

        inline boolean getLockedFlag(void) const {
            boolean locked = false;
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) {
                locked = (rawData[LOCK_INDEX] ? LOCK_IT : UNLOCK_IT);
            }
            return locked;
        }   

        
        inline const boolean isLocked(void) const {
            return getLockedFlag();
        }   
       

    protected:
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                if (rawData != nullptr)// set the current data to the new data
                switch (dgn) {
                    case LOCK_COMMAND:
                        rawData[LOCK_INDEX] = data[LOCK_INDEX];
                        break;
                    case LOCK_STATUS:
                        rawData[LOCK_INDEX] = data[LOCK_INDEX];
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn) override;    

    public:
        DoorLock(uint8_t address, uint8_t instance) : GenericDevice(address, instance) { 
            printf("DoorLock constructor called with address=%d, instance=%d, group=%d\n", address, instance); 
            // Constructor with parameters implementation
        }

        DoorLock(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
        }
        virtual ~DoorLock() {
            // Destructor implementation
            
        } 
         
        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* sendData = nullptr, uint8_t sAddress = SOURCE_ADDRESS); // execute command based on DGN and data received from the controller  
};
#endif
