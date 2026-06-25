
#ifdef HOME_KIT
#ifndef SHADES_VIEW_H
#define SHADES_VIEW_H

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
#include "PacketQueue.h"
#include "DGN.h"

class Shades;


constexpr float homeKitShadeOpenValue = 100.0;
constexpr float homeKitShadeClosedValue = 0.0;
typedef uint16_t   ShadeState;
constexpr uint16_t shadeStateClosing = 1 << 3;
constexpr uint16_t shadeStateOpening = 1 << 4;
constexpr uint16_t shadeStateMoving = shadeStateClosing | shadeStateOpening;

constexpr uint16_t shadeStateUserAction = 1 < 6;
constexpr uint16_t shadeStateHomeKitAction = 1 << 7;


// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class ShadesView : public SpanView  {
    private:
        static const uint8_t FULLY_OPEN_SHADE; //  = 100;
        static const uint8_t CLOSED_SHADE; //      = 0;   
        static const uint8_t STEP_SHADE; //        = 10; // step value for HomeKit position changes
        
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        friend class Shades;

        struct ShadesController: Service::WindowCovering {

                Shades*             model;
                ShadesView*         view;
                SpanCharacteristic* currentState;
                SpanCharacteristic* targetState;
                SpanCharacteristic* obstructionDetected;

                // functional variables for opening and closing the shades
                ShadeState shadeState = 0;
	            float currentShadePosition = homeKitShadeOpenValue;
	            float currentShadeTarget = homeKitShadeClosedValue;

	            uint64_t targetUpdateAtTime = 0;
	            uint64_t noUpdateBeforeTime = 0;

	            uint8_t openIndex;
	            uint8_t closeIndex;

	            // uint32_t openTimeMS;
	            // uint32_t rollOpenTimeMS;
	            // uint32_t closeTimeMS;
	            // uint32_t rollCloseTimeMS;

	            // bool extendState = false;
	            // bool retractState = false;

	            // uint64_t extendOffTime = 0;
	            // uint64_t retractOffTime = 0;



                ShadesController(ShadesView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::WindowCovering() {
                    currentState = new Characteristic::CurrentPosition(FULLY_OPEN_SHADE); // default to fully open
                    currentState->setRange(CLOSED_SHADE, FULLY_OPEN_SHADE, STEP_SHADE);
                    currentState->setDescription("Shades Current Position");
                    targetState = new Characteristic::TargetPosition(FULLY_OPEN_SHADE);   // default to fully open
                    targetState->setRange(CLOSED_SHADE, FULLY_OPEN_SHADE, STEP_SHADE);
                    targetState->setDescription("Shades Target Position");
                    obstructionDetected = new Characteristic::ObstructionDetected();
                    this->model = (Shades* )mdl;
                    view = vw;
                }
                void openShade(void);
                void closeShade(void);
                void stopShade(void);

                boolean update(void); 
                void loop(void);

                boolean isShadeOpen(void) { return targetState->getNewVal() == ShadesView::FULLY_OPEN_SHADE; }
                boolean isShadeClosed(void) { return targetState->getNewVal() == ShadesView::CLOSED_SHADE; }
                boolean isShadeInMotion(void) {
                    // { return (currentState->getVal() != targetState->getVal()); }
                    boolean inMotion = false;
                    if (model = nullptr) {
                        inMotion = model->isMotorInForward() || model->isMOtorInReverse();
                    }
                    return inMotion;
                }
                void setShadeObstruction(boolean val) { obstructionDetected->setVal(val); }

        }; 
        ShadesController controller;

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        ShadesView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~ShadesView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createShadesView(GenericDevice* model, const char* spanDevName); 
        
        
};
#endif // SHADES_VIEW_H 
#endif // HOME_KIT
