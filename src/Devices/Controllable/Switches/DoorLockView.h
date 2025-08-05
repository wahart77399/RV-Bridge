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

#include "SpanView.h"
#include "HomeSpan.h"
// #include <mutex>
#include "DGN.h"

class DoorLock;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class DoorLockView : public SpanView , public SpanService {
    private:
        
        static const char* name; //  = "LockMechanism"; // from Span.h
        static const char* type; //  = "45"; // from Span.h
        static const char  LOCK_COMMAND_ID;
        static const char* LOCK_COMMAND_DESCRIPTION; // = "<index>=<state:0-1>,... - send lock/unlock to <index>";
        static const char  LOCK_STATUS_ID; // = 'O';
        static const char* LOCK_STATUS_DESCRIPTION; // = "<index> to retrieve current state in HomeSpan";

        static const uint8_t VALID_LOCK_STATE = 0x01; // valid lock state
        
        
        SpanCharacteristic* spanCharCurrentLockState;
        SpanCharacteristic* spanCharTargetLockState;


        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        

        // void sendOnOff(int16_t);

        // creating cohesion between the DoorLock and this class so that the model can friend the static callback cmdSendOnOff
        friend class DoorLock;
        

        // static void prepUserCommands(void);

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

        virtual boolean update(void); 

        static void createDoorLockView(GenericDevice* model, const char* spanDevName); 
        
        
};
#endif 
