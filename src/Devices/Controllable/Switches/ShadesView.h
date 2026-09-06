#include "RVConstants.h"
#ifdef HOME_KIT_2
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

constexpr float    DOOR_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC            = 15000.0F;  // time to fully close door night shade 14 seconds
constexpr float    DOOR_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC             = 15000.0F;  // time to fully open door night shade 14 seconds
constexpr float    FRONT_WINDOW_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC    = 44000.0F;  // time to fully close front window night shade 44 seconds
constexpr float    FRONT_WINDOW_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC     = 46000.0F;  // time to fully open front window night shade 46 seconds
constexpr float    DRIVERS_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC         = 44000.0F;  // time to fully close drivers night shade 44 seconds
constexpr float    DRIVERS_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC          = 46000.0F;  // time to fully open drivers night shade 46 seconds
constexpr uint8_t  SHADES_FULLY_CLOSED_PCT                          = 100;       // fully close shade percentage
constexpr uint8_t  SHADES_FULLY_OPEN_PCT                            = 0;         // fully open shade percentage
constexpr uint8_t  SHADES_STEP_PCT                                  = 1;         // Shade step percentage



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


                // desired commands
                boolean            closeCommand = false;
                boolean            openCommand = false;
                boolean            stopCommand = false;

                // timing variables
                //float               timeToFullyClose;
                //float               timeToFullyOpen;
                float               targetClosedPosition;              // percentage of Shades closed 0.0 to 100.0
                float               currentClosedPosition;            // percentage of Shades closed 0.0 to 100.0
                elapsedMillis       startTime;                        // time when the shades started moving 
                uint16_t            travelTime;                       // time it takes to open or close the shades to the target position
                boolean             shadeMoving;                      // is the shades currently moving
                

                // functional variables for opening and closing the shades
                //ShadeState shadeState = 0;
	            //float currentShadePosition = homeKitShadeOpenValue;
	            // float currentShadeTarget = homeKitShadeClosedValue;

	            // uint64_t targetUpdateAtTime = 0;
	            // uint64_t noUpdateBeforeTime = 0;

	            // uint8_t openIndex;
	            // uint8_t closeIndex;

                ShadesController(ShadesView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::WindowCovering(),
                                    targetClosedPosition(0.0F), currentClosedPosition(0.0F), startTime(0), travelTime(0), shadeMoving(false) {
                    currentState = new Characteristic::CurrentPosition(SHADES_FULLY_OPEN_PCT); // default to fully open
                    currentState->setRange(SHADES_FULLY_OPEN_PCT, SHADES_FULLY_CLOSED_PCT, SHADES_STEP_PCT);
                    currentState->setDescription("Shades Current Position");
                    targetState = new Characteristic::TargetPosition(SHADES_FULLY_OPEN_PCT);   // default to fully open
                    targetState->setRange(SHADES_FULLY_OPEN_PCT, SHADES_FULLY_CLOSED_PCT, SHADES_STEP_PCT);
                    targetState->setDescription("Shades Target Position");
                    obstructionDetected = new Characteristic::ObstructionDetected();

                    this->model = (Shades* )mdl;
                    view = vw;
                }
                // prep commands
                void readyToClose(void) { closeCommand = true; openCommand = false; stopCommand = false; }
                boolean isReadyToClose(void) { return closeCommand; }
                void readyToOpen(void) { closeCommand = false; openCommand = true; stopCommand = false; }
                boolean isReadyToOpen(void) { return openCommand; }
                void readyToStop(void)   { closeCommand = false; openCommand = false; stopCommand = true; }
                boolean isReadyToStop(void) { 
                    unsigned long stime = startTime;
                    if ((isShadeInMotion() && (stime >= travelTime)) || (stopCommand == true)) {
                        // printf("ShadesView::ShadesController::isReadyToStop stime=%d, travelTime =%d, stopCommand =%d \n", stime, travelTime, stopCommand);
                        clearCommands();
                        stopCommand = true; // we are done moving
                        stopMoving();
                    }
                    return stopCommand; 
                }
                void clearCommands(void) { closeCommand = false; openCommand = false; stopCommand = false; }

                void startMoving(void) {
                    startTime = 0;
                    shadeMoving = true;
                }
                void stopMoving(void) {
                    startTime = 0;
                    travelTime = 0;
                    shadeMoving = false;
                    clearCommands();
                }


                void openShade(float amt = static_cast<float>(SHADES_MIN_PERCENT));
                void closeShade(float amt = static_cast<float>(SHADES_MAX_PERCENT));
                void stopShade(void);

                boolean update(void); 
                void loop(void);

                                // clarification of status
                //boolean isShadeClosed(void); 
                //boolean isShadeOpen(void);
                boolean isShadeInMotion(void)          { return shadeMoving; }
                // boolean isShadeFullyClosed(void);
                // boolean isShadeFullyOpen(void);
                boolean isDown(void) { return (currentState->getVal() == SHADES_FULLY_CLOSED_PCT); }
                // void extending(boolean val) { out->setVal(val); PacketQueue::clearLastPacketReceiveTime();}
                //void retracting(void) { out->setVal(false);retractAwning(AWNING_MIN_PERCENT); }

                boolean isShadeOpen(void) { return targetState->getNewVal() == ShadesView::FULLY_OPEN_SHADE; }
                boolean isShadeClosed(void) { return targetState->getNewVal() == ShadesView::CLOSED_SHADE; }
                //boolean isShadeInMotion(void);  
                void setShadeObstruction(boolean val) { obstructionDetected->setVal(val); }

        }; 
        ShadesController* controller;

        struct ShadesDownController: Service::Switch {
            Shades*             model;
            ShadesView*         view;
            ShadesController*   controller;
            SpanCharacteristic* down;

            ShadesDownController(ShadesView* vw, GenericDevice* mdl, ShadesController* ctrl, const char* spanDeviceName) : Service::Switch() {
                down = new Characteristic::On(false);
                down->setDescription("Shades Down/Closed");
                this->model = (Shades* )mdl;
                view = vw;
                this->controller = ctrl;
            }
            void closing(boolean val) { down->setVal(val); PacketQueue::clearLastPacketReceiveTime();}
            boolean update(void); 
            void loop(void);

        };
        /*
        struct ShadesUpController: Service::Switch {
            Shades*             model;
            ShadesView*         view;
            ShadesController*   controller;
            SpanCharacteristic* up;

            ShadesUpController(ShadesView* vw, GenericDevice* mdl, ShadesController* ctrl, const char* spanDeviceName) : Service::Switch() {
                up = new Characteristic::On(false);
                up->setDescription("Shades Up/Opened");
                this->model = (Shades* )mdl;
                view = vw;
                this->controller = ctrl;
            }
            void opening(boolean val) { up->setVal(val); PacketQueue::clearLastPacketReceiveTime();}
            boolean update(void); 
            void loop(void);

        };
        */
/*
        struct ShadesStopController: Service::Switch {
            Shades*             model;
            ShadesView*         view;
            ShadesController*   controller;
            SpanCharacteristic* stop;

            ShadesStopController(ShadesView* vw, GenericDevice* mdl, ShadesController* ctrl, const char* spanDeviceName) : Service::Switch() {
                stop = new Characteristic::On(false);
                stop->setDescription("Shades Stop");
                this->model = (Shades* )mdl;
                view = vw;
                this->controller = ctrl;
            }
            void stopping(boolean val) { stop->setVal(val); PacketQueue::clearLastPacketReceiveTime();}
            boolean update(void); 
            void loop(void);

        };
        */

        ShadesDownController* downController;
        // ShadesUpController*   upController;
        // ShadesStopController* stopController;
        void setController(ShadesController* ctrl) {controller = ctrl; }
        void setDownController(ShadesDownController* ctrl) {downController = ctrl; }
        // void setUpController(ShadesUpController* ctrl) {upController = ctrl; }
        //void setStopController(ShadesStopController* ctrl) {stopController = ctrl; }


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
