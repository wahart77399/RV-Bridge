#ifndef DOOR_LOCK_VIEW_H
#define DOOR_LOCK_VIEW_H
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

#include "RVConstants.h"
#ifdef HOME_KIT_1

#include "SpanView.h"
#include "HomeSpan.h"
#include "PacketQueue.h"
#include "DGN.h"

class DoorLock;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class DoorLockView : public SpanView  {
    private:
        
        static const char  LOCK_COMMAND_ID;
        // static const char* LOCK_COMMAND_DESCRIPTION; // = "<index>=<state:0-1>,... - send lock/unlock to <index>";
        // static const char  LOCK_STATUS_ID; // = 'O';
        // static const char* LOCK_STATUS_DESCRIPTION; // = "<index> to retrieve current state in HomeSpan";

        static const uint8_t VALID_LOCK_STATE; //  = 0x01; // valid lock state
        static const uint8_t VALID_UNLOCKED_STATE; 
        
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        friend class DoorLock;

        struct DoorLockController: Service::LockMechanism {

                DoorLock*           model;
                DoorLockView*       view;
                SpanCharacteristic* currentState;
                SpanCharacteristic* targetState;



                DoorLockController(DoorLockView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::LockMechanism() {
                    currentState = new Characteristic::LockCurrentState();
                    targetState = new Characteristic::LockTargetState();
                    this->model = (DoorLock* )mdl;
                    view = vw;
                }
                boolean update(void); 
                boolean isLocked(void) { return targetState->getNewVal() == DoorLockView::VALID_LOCK_STATE; }
                void lockIt(void) { currentState->setVal(DoorLockView::VALID_LOCK_STATE);PacketQueue::clearLastPacketReceiveTime();}
                void unLockIt(void) {currentState->setVal(DoorLockView::VALID_UNLOCKED_STATE);PacketQueue::clearLastPacketReceiveTime();}
        }; 
        DoorLockController controller;

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        DoorLockView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~DoorLockView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createDoorLockView(GenericDevice* model, const char* spanDevName); 
        
        
};
#endif 

#endif // HOME_KIT_1