#ifndef DC_DIMMABLESWITCHVIEW_H
#define DC_DIMMABLESWITCHVIEW_H

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

#include "DC_LightSwitchView.h"


class DC_Switch;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class DC_DimmableSwitchView : public DC_LightSwitchView {
    private:
        const uint8_t Lamp = 0; // not a dimmable switch
        boolean needToUpdateView = true;
        static const char* name; //  = "DimmableBulb"; // from Span.h
        static const char* type; //  = "43"; // from Span.h
        static const char  DIMMABLE_FXN_COMMAND; //  = 'o';
        static const char* DIMMABLE_FXN_COMMAND_DESCRIPTION; // = "<index>=<state:0-1>,... - send onOff to <index>";
        // static const char  DIMMABLE_FXN_STATUS; // = 'O';
        // static const char* DIMMABLE_FXN_STATUS_DESCRIPTION; // = "<index> to retrieve current state in HomeSpan";

        SpanCharacteristic* spanCharBrightness;

        static SpanUserCommand* setBrightness;
        

        

        // void sendOnOff(int16_t);

        // creating cohesion between the DC_Switch and this class so that the model can friend the static callback cmdSendOnOff
        friend class DC_DimmableSwitch;


        // cmd callback for HomeSpan
        static void cmdSetBrightness(const char* buff);

        static void prepUserCommands(void);
        inline void updateTheView(void) { needToUpdateView = true; } // set the flag to indicate view needs to be updated   
        inline void dontUpdateTheView(void) { needToUpdateView = false; } // reset the flag to indicate view does not need to be updated
        inline const boolean isNeedToUpdateView(void) const { return needToUpdateView; } // return the flag to indicate view needs to be updated

    protected:
        const uint8_t RVCBrightMax = 200;
        // @brief need to review this... doesn't seem right SpanService(type, name)
        DC_DimmableSwitchView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~DC_DimmableSwitchView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        virtual boolean update(void); 

        static void createDC_DimmableSwitchView(GenericDevice* model, const char* spanDevName); 
};
#endif // DC_SWITCHVIEW_H