#ifndef COACHESP32_H
#define COACHESP32_H
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
#include <mutex>
#include "CAN_config.h"
// #include "debug.h"

class CoachESP32 {
    private:

        // constants
        static const uint8_t indicatorPinR = 2; // FROM THE ESP32 BOARD
        static const uint8_t indicatorPinG = 15;
        static const uint8_t indicatorPinB = 4;
        static const uint8_t sourceAddress = 145;
        static const gpio_num_t canTxPin = GPIO_NUM_25;
        static const gpio_num_t canRxPin = GPIO_NUM_26;
        // static std::ostringstream oss;
        

        

        typedef struct {        // from the ESP32CAN library
	        uint8_t pinNumber;
	        uint8_t mode;
	        uint8_t state;
        } PinSetup;

        static PinSetup pinList[];

        // static std::mutex esp32Mutex;
        static CoachESP32* instance;
        CAN_device_t* CAN_cfg;


        CoachESP32 (CAN_device_t* cfg);
        CoachESP32 (void) = delete;
        CoachESP32(CoachESP32& wifi) = delete;
        CoachESP32& operator=(CoachESP32&) = delete;

        void initPins(void);
        void flashPin(uint8_t pin, uint16_t count, uint16_t period);
        void processQueue(void);

        static CoachESP32* getInstance(void) { return CoachESP32::instance; }


    public:
        static CoachESP32* getInstance(CAN_device_t* cfg);
        void initialize(void);
        static void adjustRGB(bool red, bool green, bool blue);
        static void pollESP32(void);
        ~CoachESP32() {}

};


#endif
