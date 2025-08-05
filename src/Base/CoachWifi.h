#ifndef COACHWIFI_H
#define COACHWIFI_H
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
#include "HomeSpan.h"
#include <mutex>

#include "wifi-creds.h"

// homeSpan is a global in the HomeSpan framework - it isused to manage the connection to homekit
// enum HS_STATUS : int16_t;

// #include "debug.h"

class CoachWifi {
    private:
        const char* ssid = WIFI_SSID;
        const char* password = WIFI_PASSWORD;
        

        // static std::mutex wifiMutex;
        // static std::ostringstream oss;
        static CoachWifi* instance;
        static bool wifiConnected ;
        static bool hadWifiConnection;


        CoachWifi (void);
        CoachWifi(CoachWifi& wifi) = delete;
        CoachWifi& operator=(CoachWifi&) = delete;

        static void wifiStatusChanged(HS_STATUS status);
        static void wifiReady(void);
        static uint64_t millis64(void);

    public:
        static CoachWifi* getInstance(void);
        static void initialize(void);
        static bool isConnected() {
            return CoachWifi::wifiConnected;
        }
        static void pollSpan(void);
        static void verify(void);
        ~CoachWifi() {}

};

#endif // wifi
