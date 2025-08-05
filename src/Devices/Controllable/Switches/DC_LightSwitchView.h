#ifndef DC_LIGHTSWITCHVIEW_H
#define DC_LIGHTSWITCHVIEW_H

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
#include <mutex>
#include "DGN.h"

class DC_Switch;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class DC_LightSwitchView : public SpanView , public SpanService {
    private:
        const uint8_t Lamp = 0; // not a dimmable switch
        static const char* name; //  = "LightBulb"; // from Span.h
        static const char* type; //  = "43"; // from Span.h
        static const char  ON_OFF_COMMAND; //  = 'o';
        static const char* ON_OFF_COMMAND_DESCRIPTION; // = "<index>=<state:0-1>,... - send onOff to <index>";
        static const char  ON_OFF_STATUS; // = 'O';
        static const char* ON_OFF_STATUS_DESCRIPTION; // = "<index> to retrieve current state in HomeSpan";

        SpanCharacteristic* spanCharOn;


        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 


        // creating cohesion between the DC_Switch and this class so that the model can friend the static callback cmdSendOnOff
        friend class DC_Switch;


    protected:
        inline void turnOnLight(void) {
            if (spanCharOn != nullptr) {
                spanCharOn->setVal(true); // turn on the light
            }
        }
        inline void turnOffLight(void) {
            if (spanCharOn != nullptr) {
                spanCharOn->setVal(false); // turn off the light
            }
        }
        inline const char* getSpanDeviceName(void) const { return spanDeviceName; } // return the name of the device for this view
        

        const uint8_t RVCBrightMax = 200;
        // @brief need to review this... doesn't seem right SpanService(type, name)
        DC_LightSwitchView(GenericDevice* model, const char* spanDevName);
    public:

        
        /// @brief destructor
        virtual ~DC_LightSwitchView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        virtual boolean update(void); 

        static void createDC_LightSwitchView(GenericDevice* model, const char* spanDevName); 
};
#endif // DC_SWITCHVIEW_H
