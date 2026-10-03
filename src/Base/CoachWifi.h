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

#define CUSTOM_CHAR_HEADER  // this must be done prior to #include of HomeSpan call anywhere. 
#include "HomeSpan.h"
#include "WifiPortal.h"
#ifdef WIFI_CRED_H
/*
boot
  → load NVS
  → if ssid present: STA + homeSpan (current path)
  → if missing OR (optional) long fail: PROVISIONING
        AP "RV-Bridge-Setup"
        DNS * → 192.168.4.1
        GET /  → form
        POST /save → NVS → reboot
*/

#include "wifi-creds.h"
#endif

// homeSpan is a global in the HomeSpan framework - it isused to manage the connection to homekit
// enum HS_STATUS : int16_t;

// #include "debug.h"
/* 
class CoachWifi {
    private:
        #ifdef WIFI_CRED_H
        const char* ssid = WIFI_SSID;
        const char* password = WIFI_PASSWORD;
        #else
        String ssid_;
        String password_;
        #endif

        

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

        #ifndef WIFI_CRED_H
        void loadCredentials();
        void pollSerial();
        void processSerialLine(const String& line);
        #endif

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
*/
class CoachWifi {
private:
    static CoachWifi* instance;
    static bool       wifiConnected;
    static bool       hadWifiConnection;

    String      ssid_;
    String      password_;
    bool        provisioning_;
    WifiPortal* portal_;

    CoachWifi(void);
    CoachWifi(const CoachWifi&) = delete;
    CoachWifi& operator=(const CoachWifi&) = delete;
    CoachWifi(CoachWifi&&) = delete;
    CoachWifi& operator=(CoachWifi&&) = delete;

    // private attribute access
    const String& ssid() const { return ssid_; }
    void          ssid(const String& value) { ssid_ = value; }
    const String& password() const { return password_; }
    void          password(const String& value) { password_ = value; }
    bool          provisioning() const { return provisioning_; }
    void          provisioning(bool value) { provisioning_ = value; }
    WifiPortal*   portal() const { return portal_; }
    void          portal(WifiPortal* value) { portal_ = value; }

    static void     wifiStatusChanged(HS_STATUS status);
    static void     wifiReady(void);
    static uint64_t millis64(void);

    void loadCredentials();
    void startHomeSpan();
    void startProvisioning();
    void pollSerial();
    void processSerialLine(const String& line);

public:
    static bool       isProvisioning(void);
    static CoachWifi* getInstance(void);
    static void       initialize(void);
    static bool       isConnected(void);
    static void       pollSpan(void);
    static void       verify(void);

    ~CoachWifi() = default;
};

#endif // wifi
