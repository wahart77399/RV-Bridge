#ifndef CHASSIS_MOBILITY_VIEW_H
#define CHASSIS_MOBILITY_VIEW_H
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
// #define CUSTOM_CHAR_HEADER
#define CUSTOM_CHAR_HEADER
#include "HomeSpan.h"
#include "RVConstants.h"
#include "PacketQueue.h"
#include "DGN.h"

typedef enum {
    NOT_IN_MOTION = 0,
    IN_MOTION = 1
} MOTION_STATE;

class ChassisMobility;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class ChassisMobilityView : public SpanView  {
    private:
        
        //const char* spanDeviceName;
        //static bool bridgeCreated;
        //static void createBridge(void); 

        friend class ChassisMobility;

        struct ChassisMobilityMotion: Service::MotionSensor {

                ChassisMobility*           model;
                ChassisMobilityView*       view;
                SpanCharacteristic*        inMotion;

                ChassisMobilityMotion(ChassisMobilityView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::MotionSensor() {
                    inMotion = new Characteristic::MotionDetected(0); // not detected initially
                    this->model = (ChassisMobility* )mdl;
                    view = vw;
                    inMotion->setDescription("Chassis Mobility Motion Sensor");
                }
                void detected(MOTION_STATE val) { inMotion->setVal(val); }
                boolean isInMotion(void) { return (inMotion->getVal() == IN_MOTION); }
        }; 
        ChassisMobilityMotion* chassisMobilitySensor;
        void setSensorView(ChassisMobilityMotion* vw) { chassisMobilitySensor = vw; }


    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        ChassisMobilityView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~ChassisMobilityView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createChassisMobilityView(GenericDevice* model, const char* spanDevName); 
        
        
};
#endif 