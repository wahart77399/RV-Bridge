#include "RVConstants.h"
#ifdef HOME_KIT_2
#define AWNNIG_VIEW_H
#ifndef AWNING_VIEW_H

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
// #include <stdint.h>
#include "HomeSpan.h"
#include "PacketQueue.h"
#include "DGN.h"
#include "AwningDefinitions.h"

class Awning;

constexpr float    DOOR_AWNING_EXTEND_CYCLE_TIME_SEC    = 14000.0F; // time to fully extend door awning 14 seconds
constexpr float    DOOR_AWNING_RETRACT_CYCLE_TIME_SEC   = 14000.0F; // time to fully retract door awning 14 seconds
constexpr float    FRONT_AWNING_EXTEND_CYCLE_TIME_SEC   = 44000.0F; // time to fully extend front awning 44 seconds
constexpr float    FRONT_AWNING_RETRACT_CYCLE_TIME_SEC  = 46000.0F; // time to fully retract front awning 46 seconds
constexpr float    BACK_AWNING_EXTEND_CYCLE_TIME_SEC    = 44000.0F; // time to fully extend back awning 44 seconds
constexpr float    BACK_AWNING_RETRACT_CYCLE_TIME_SEC   = 46000.0F; // time to fully retract back awning 46 seconds
constexpr uint8_t  AWNING_FULLY_RETRACTED_PCT           = 100;        // fully retracted awning percentage
constexpr uint8_t  AWNING_FULLY_EXTENDED_PCT            = 0;      // fully extended awning percentage
constexpr uint8_t  AWNING_STEP_PCT                      = 1;        // awning step percentage


// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class AwningView : public SpanView  {
    private:
        
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        friend class Awning;

        struct AwningController: Service::WindowCovering {

                Awning*             model;
                AwningView*         view;
                SpanCharacteristic* currentState;
                SpanCharacteristic* targetState;
                SpanCharacteristic* obstructionDetected;

                // desired commands
                boolean            extendCommand = false;
                boolean            retractCommand = false;
                boolean            stopCommand = false;

                // timing variables
                float               timeToFullyExtend;
                float               timeToFullyRetract;
                float               targetExtendedPosition;           // percentage of awning extended 0.0 to 100.0
                float               currentExtendedPosition;          // percentage of awning extended 0.0 to 100.0
                elapsedMillis       startTime;                        // time when the awning started moving 
                uint16_t            travelTime;                       // time it takes to open or close the awning to the target position
                boolean             awningMoving;                     // is the awning currently moving
                
                // remember fully retracted in HOMEKIT window coverings is 100
                AwningController(AwningView* vw, GenericDevice* mdl, const char* spanDeviceName, float extendTime, float retractTime) : Service::WindowCovering(), 
                                    timeToFullyExtend(extendTime), timeToFullyRetract(retractTime), targetExtendedPosition(0.0F), currentExtendedPosition(100.0F), startTime(0), travelTime(0), awningMoving(false) {
                    currentState = new Characteristic::CurrentPosition(AWNING_FULLY_RETRACTED_PCT); // default to fully retracted
                    currentState->setRange(AWNING_FULLY_EXTENDED_PCT, AWNING_FULLY_RETRACTED_PCT, AWNING_STEP_PCT);
                    currentState->setDescription("Awning Current Position");
                    targetState = new Characteristic::TargetPosition(AWNING_FULLY_RETRACTED_PCT);   // default to fully open
                    targetState->setRange(AWNING_FULLY_EXTENDED_PCT, AWNING_FULLY_RETRACTED_PCT, AWNING_STEP_PCT);
                    targetState->setDescription("Awning Target Position");
                    obstructionDetected = new Characteristic::ObstructionDetected();
                    this->model = (Awning* )mdl;
                    view = vw;
                }

                // prep commands
                void readyToExtend(void) { extendCommand = true; retractCommand = false; stopCommand = false; }
                boolean isReadyToExtend(void) { return extendCommand; }
                void readyToRetract(void) { extendCommand = false; retractCommand = true; stopCommand = false; }
                boolean isReadyToRetract(void) { return retractCommand; }
                void readyToStop(void)   { extendCommand = false; retractCommand = false; stopCommand = true; }
                boolean isReadyToStop(void) { 
                    unsigned long stime = startTime;
                    if ((isAwningInMotion() && (stime >= travelTime)) || (stopCommand == true)) {
                        printf("AwningView::AwningController::isReadyToStop stime=%d, travelTime =%d, stopCommand =%d \n", stime, travelTime, stopCommand);
                        clearCommands();
                        stopCommand = true; // we are done moving
                        stopMoving();
                    }
                    return stopCommand; 
                }
                void clearCommands(void) { extendCommand = false; retractCommand = false; stopCommand = false; }

                void startMoving(void) {
                    startTime = 0;
                    awningMoving = true;
                }
                void stopMoving(void) {
                    startTime = 0;
                    travelTime = 0;
                    awningMoving = false;
                    clearCommands();
                }


                void extendAwning(float amt);
                void retractAwning(float amt);
                void stopAwning(void);

                // void targetUpdate(void);
                boolean update(void); 
                void loop(void);

                // clarification of status
                boolean isAwningExtended(void); 
                boolean isAwningRetracted(void);
                boolean isAwningInMotion(void)          { return awningMoving; }
                boolean isAwningFullyExtended(void);
                boolean isAwningFullyRetracted(void);
                void setAwningObstruction(boolean val)  { obstructionDetected->setVal(val); }

        }; 
        AwningController*           controller;

        void setController(AwningController* ctrl) {controller = ctrl; }

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        AwningView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~AwningView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createAwningView(GenericDevice* model, const char* spanDevName, 
                                     float extTime = FRONT_AWNING_EXTEND_CYCLE_TIME_SEC, 
                                     float retTime = FRONT_AWNING_RETRACT_CYCLE_TIME_SEC); 
        
        
};
#endif  // ifndef AWNING_VIEW_H
#endif // ifdef HOME_KIT_2
