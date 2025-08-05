#include "CoachWifi.h"
#include "HomeSpan.h"

// std::ostringstream CoachWifi::oss;
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
	printf("CoachWifi::getInstance() started\n");
    // std::lock_guard<std::mutex> lock(CoachWifi::wifiMutex);
    if (!CoachWifi::instance) {
        CoachWifi::instance = new CoachWifi();
    }
    // std::lock_guard<std::mutex> unlock(CoachWifi::wifiMutex);
	printf("CoachWifi::getInstance() completed\n");
    return instance;
}

CoachWifi::CoachWifi (void) : ssid(WIFI_SSID), password(WIFI_PASSWORD) {
}

constexpr const char* versionString = "v2.0.0";

void CoachWifi::initialize() {
    // // oss << "CoachWifi::initialized started" << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, oss); 
	homeSpan.setWifiCredentials((char* )WIFI_SSID,(char* )WIFI_PASSWORD);
    homeSpan.setSketchVersion(versionString);
    homeSpan.setWifiCallback(CoachWifi::wifiReady);
    homeSpan.setStatusCallback(CoachWifi::wifiStatusChanged);
	
	// oss << "homeSpan.begin() called with Bridges" << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, oss);
    homeSpan.begin(Category::Bridges, "RV-Bridge", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
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
			// oss << (uint32_t)millis() << ": WIFI: Lost Connection." << std::endl;
			// LOGIT(NOT_SO_VERBOSE_LOG_LEVEL, oss);
			printf("CoachWifi::wifiStatusChanged - lost connection\n");
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
