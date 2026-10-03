// #include "CoachWifi.h"
// #include "HomeSpan.h"
// #include "RVConstants.h"
// #include "debug.h"

/*
uint64_t CoachWifi::millis64(void) {
	volatile static uint32_t low32 = 0, high32 = 0;
	uint32_t new_low32 = millis();

	if (new_low32 < low32)
		high32++;

	low32 = new_low32;

	return (uint64_t) high32 << 32 | low32;
}

// create the mutex
// std::mutex CoachWifi::wifiMutex;
CoachWifi* CoachWifi::instance = nullptr;
bool CoachWifi::wifiConnected = false;
bool CoachWifi::hadWifiConnection = false;

CoachWifi* CoachWifi::getInstance(void) {
	RV_PRINTF("CoachWifi::getInstance() started\n");
    if (!CoachWifi::instance) {
        CoachWifi::instance = new CoachWifi();
    }
	RV_PRINTF("CoachWifi::getInstance() completed\n");
    return instance;
}

#ifdef WIFI_CRED_H
CoachWifi::CoachWifi (void) : ssid(WIFI_SSID), password(WIFI_PASSWORD) {
}

constexpr const char* versionString = "v3.0.0";
#else
CoachWifi::CoachWifi(void)
    : ssid_("")
    , password_("")
    , provisioning_(false)
    , portal_(nullptr)
{
}

#include "WifiCredentials.h"
void CoachWifi::loadCredentials() {
    String loadedSsid;
    String loadedPass;
    bool   loaded = WifiCredentials::load(loadedSsid, loadedPass);

    if (loaded) {
        ssid(loadedSsid);
        password(loadedPass);
    } else {
        ssid("");
        password("");
    }
}

void CoachWifi::startProvisioning() {
    provisioning(true);
    if (portal() == nullptr) {
        portal(new WifiPortal("RV-Bridge-Setup"));
    }
    portal()->begin();
}

constexpr const char* versionString = "v3.1.0";
#endif


void CoachWifi::initialize() {
	#ifdef WIFI_CRED_H
    // // oss << "CoachWifi::initialized started" << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, oss); 
	homeSpan.setWifiCredentials((char* )WIFI_SSID,(char* )WIFI_PASSWORD);

	#else
	CoachWifi* self = CoachWifi::getInstance();
    if (self == nullptr) {
        return;
    }

    self->loadCredentials();

    // HomeSpan still owns association; we only supply credentials
    if (self->ssid_.length() > 0) {
        homeSpan.setWifiCredentials(
            (char*)self->ssid_.c_str(),
            (char*)self->password_.c_str());
    }
	#endif
	homeSpan.setSketchVersion(versionString);
    homeSpan.setWifiCallback(CoachWifi::wifiReady);
    homeSpan.setStatusCallback(CoachWifi::wifiStatusChanged);
	
	// oss << "homeSpan.begin() called with Bridges" << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, oss);
	#ifdef HOME_KIT_1
    homeSpan.begin(Category::Bridges, "RV-Bridge", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
	#endif
	#ifdef HOME_KIT_2
    homeSpan.begin(Category::Bridges, "RV-Bridge-2", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
	#endif
	//oss << "CoachWifi::initialized completed \n";
	//LOGIT(VERBOSE_LOG_LEVEL, oss);
}


void CoachWifi::wifiStatusChanged(HS_STATUS status) {
	//oss << "CoachWifi::wifiStatusChanged called with status: " << status << std::endl; 
	//LOGIT(VERBOSE_LOG_LEVEL, oss);
    // std::lock_guard<std::mutex> lock(wifiMutex);
	if (status == HS_WIFI_CONNECTING) {
		wifiConnected = false;
		if (hadWifiConnection) {
			RV_PRINTF("CoachWifi::wifiStatusChanged - lost connection\n");
		}
	}
    // std::lock_guard<std::mutex> unlock(wifiMutex);
	//oss << "CoachWifi::wifiStatusChanged completed" << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, oss);
}

void CoachWifi::wifiReady() {
	//oss << "CoachWifi::wifiReady started\n";
	// LOGIT(VERBOSE_LOG_LEVEL, oss);
    // std::lock_guard<std::mutex> lock(wifiMutex);
	wifiConnected = true;
	hadWifiConnection = true;
	// DEBUG("%u: WIFI: Ready..\n", (uint32_t)millis());
    // std::lock_guard<std::mutex> unlock(wifiMutex);
	//oss << "CoachWifi::wifiReady completed\n";
	// LOGIT(VERBOSE_LOG_LEVEL, oss);
}

void CoachWifi::pollSpan(void) {
    homeSpan.poll();
	verify();
}

#include <WiFi.h>

void CoachWifi::verify(void) {
    	// HomeSpan does not call the wifi callback after the first connection
	// We do that manually here
	if (hadWifiConnection && !wifiConnected) {
		static uint64_t nextWifiCheck = 0;
		uint64_t time = millis64();

		if (time >= nextWifiCheck) {
			if (WiFi.status()==WL_CONNECTED) { // from WiFi.h
				wifiReady();
			}

			nextWifiCheck = time + 500;
		}
	}
}
//////////////////////////////////////////////
#ifndef WIFI_CRED_H
void CoachWifi::pollSerial() {
    static String line;

    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (line.length() > 0) {
                processSerialLine(line);
                line = "";
            }
        } else {
            line += c;
            if (line.length() > 200) {
                line = "";
            }
        }
    }
}
void CoachWifi::processSerialLine(const String& line) {
    String work = line;
    work.trim();

    if (work.equalsIgnoreCase("WIFI clear")) {
        WifiCredentials::clear();
        ssid_ = "";
        password_ = "";
        Serial.println("WIFI cleared - reboot required");
        delay(100);
        ESP.restart();
    } else if (work.startsWith("WIFI ")) {
        String rest = work.substring(5);
        rest.trim();

        String newSsid;
        String newPass;
        int ssidPos = rest.indexOf("ssid=");
        int passPos = rest.indexOf("pass=");

        if (ssidPos >= 0) {
            int ssidStart = ssidPos + 5;
            int ssidEnd = rest.length();
            if (passPos > ssidStart) {
                ssidEnd = passPos;
            }
            newSsid = rest.substring(ssidStart, ssidEnd);
            newSsid.trim();
        }
        if (passPos >= 0) {
            newPass = rest.substring(passPos + 5);
            newPass.trim();
        }

		if (newSsid.length() > 0) {
            if (WifiCredentials::save(newSsid, newPass)) {
                Serial.println("WIFI saved, rebooting");
                delay(100);
                ESP.restart();
            } else {
                Serial.println("WIFI save failed");
            }
        } else {
            Serial.println("WIFI usage: WIFI ssid=Name pass=Secret");
            Serial.println("WIFI usage: WIFI clear");
        }
    }
}

#endif
*/
#include "CoachWifi.h"
#include "WifiCredentials.h"
#include "HomeSpan.h"
#include "RVConstants.h"
#include <WiFi.h>

CoachWifi* CoachWifi::instance          = nullptr;
bool       CoachWifi::wifiConnected     = false;
bool       CoachWifi::hadWifiConnection = false;

uint64_t CoachWifi::millis64(void) {
    volatile static uint32_t low32 = 0;
    volatile static uint32_t high32 = 0;
    uint32_t new_low32 = millis();

    if (new_low32 < low32) {
        high32++;
    }
    low32 = new_low32;
    return ((uint64_t)high32 << 32) | low32;
}


bool CoachWifi::isProvisioning(void) {
    CoachWifi* self = CoachWifi::getInstance();
    return (self != nullptr) && self->provisioning();
}

CoachWifi* CoachWifi::getInstance(void) {
    if (CoachWifi::instance == nullptr) {
        CoachWifi::instance = new CoachWifi();
    }
    return CoachWifi::instance;
}

CoachWifi::CoachWifi(void)
    : ssid_("")
    , password_("")
    , provisioning_(false)
    , portal_(nullptr)
{
}

void CoachWifi::loadCredentials() {
    String loadedSsid;
    String loadedPass;
    bool   loaded = WifiCredentials::load(loadedSsid, loadedPass);

    if (loaded) {
        ssid(loadedSsid);
        password(loadedPass);
    } else {
        ssid("");
        password("");
    }
}

void CoachWifi::startProvisioning() {
    provisioning(true);
    if (portal() == nullptr) {
		printf("CoachWifi::startProvisioning \n");
        portal(new WifiPortal("SmartCoach-Setup"));
    }
    portal()->begin();
}

void CoachWifi::startHomeSpan() {
    constexpr const char* versionString = "v2.0.0";

    provisioning(false);

    if (ssid().length() > 0) {
        homeSpan.setWifiCredentials(
            (char*)ssid().c_str(),
            (char*)password().c_str());
    }

    homeSpan.setSketchVersion(versionString);
    homeSpan.setConnectionCallback(CoachWifi::connectionEstablished);
    homeSpan.setStatusCallback(CoachWifi::wifiStatusChanged);

/* *
#ifdef HOME_KIT_1
    homeSpan.begin(Category::Bridges, "RV-Bridge", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
#endif
#ifdef HOME_KIT_2
    homeSpan.begin(Category::Bridges, "RV-Bridge-2", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
#endif
*/

    homeSpan.begin(Category::Bridges, "SmartCoach", DEFAULT_HOST_NAME, "SmartCoach-ESP32-S3");
}

void CoachWifi::initialize() {
    CoachWifi* self = CoachWifi::getInstance();

    if (self != nullptr) {
        self->loadCredentials();

        if (self->ssid().length() == 0) {
            self->startProvisioning();
        } else {
            self->startHomeSpan();
        }
    }
}

bool CoachWifi::isConnected(void) {
    return CoachWifi::wifiConnected;
}

void CoachWifi::wifiStatusChanged(HS_STATUS status) {
    if (status == HS_WIFI_CONNECTING) {
        wifiConnected = false;
    }
}

void CoachWifi::wifiReady() {
    wifiConnected = true;
    hadWifiConnection = true;
}

void CoachWifi::connectionEstablished(int /*count*/) {
    wifiReady();
}

void CoachWifi::pollSpan(void) {
    CoachWifi* self = CoachWifi::getInstance();

    if (self != nullptr) {
        self->pollSerial();

        if (self->provisioning()) {
            if (self->portal() != nullptr) {
                self->portal()->poll();
            }
        } else {
            homeSpan.poll();
            verify();
        }
    }
}

void CoachWifi::verify(void) {
    if (hadWifiConnection && !wifiConnected) {
        static uint64_t nextWifiCheck = 0;
        uint64_t time = millis64();

        if (time >= nextWifiCheck) {
            if (WiFi.status() == WL_CONNECTED) {
                wifiReady();
            }
            nextWifiCheck = time + 500;
        }
    }
}

void CoachWifi::pollSerial() {
    static String line;

    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (line.length() > 0) {
                processSerialLine(line);
                line = "";
            }
        } else {
            line += c;
            if (line.length() > 200) {
                line = "";
            }
        }
    }
}

void CoachWifi::processSerialLine(const String& line) {
    String work = line;
    work.trim();

    if (work.equalsIgnoreCase("WIFI clear")) {
        WifiCredentials::clear();
        ssid("");
        password("");
        delay(100);
        ESP.restart();
    } else if (work.startsWith("WIFI ")) {
        String rest = work.substring(5);
        rest.trim();

        String newSsid;
        String newPass;
        int    ssidPos = rest.indexOf("ssid=");
        int    passPos = rest.indexOf("pass=");
        bool   haveSsid = false;
        bool   saved = false;

        if (ssidPos >= 0) {
            int ssidStart = ssidPos + 5;
            int ssidEnd = rest.length();
            if (passPos > ssidStart) {
                ssidEnd = passPos;
            }
            newSsid = rest.substring(ssidStart, ssidEnd);
            newSsid.trim();
            haveSsid = (newSsid.length() > 0);
        }

        if (passPos >= 0) {
            newPass = rest.substring(passPos + 5);
            newPass.trim();
        } else {
            newPass = "";
        }

        if (haveSsid) {
            saved = WifiCredentials::save(newSsid, newPass);
        }

        if (saved) {
            delay(100);
            ESP.restart();
        }
    } else if (!provisioning()) {
        // our reader drains Serial, so HomeSpan's CLI never sees input otherwise
        homeSpan.processSerialCommand(work.c_str());
    }
}
