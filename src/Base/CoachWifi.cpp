#include "CoachWifi.h"
#include "HomeSpan.h"

uint64_t CoachWifi::millis64(void) {
	volatile static uint32_t low32 = 0, high32 = 0;
	uint32_t new_low32 = millis();

	if (new_low32 < low32)
		high32++;

	low32 = new_low32;

	return (uint64_t) high32 << 32 | low32;
}

// create the mutex
std::mutex CoachWifi::wifiMutex;
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
    printf("CoachWifi::initialized started \n");
	homeSpan.setWifiCredentials((char* )WIFI_SSID,(char* )WIFI_PASSWORD);
    homeSpan.setSketchVersion(versionString);
    homeSpan.setWifiCallback(CoachWifi::wifiReady);
    homeSpan.setStatusCallback(CoachWifi::wifiStatusChanged);
	printf("%u homeSpan.begin() called with Bridges\n", Category::Bridges);
    homeSpan.begin(Category::Bridges, "RV-Bridge", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
	printf("CoachWifi::initialized completed \n");
}


void CoachWifi::wifiStatusChanged(HS_STATUS status) {
	printf("CoachWifi::wifiStatusChanged called with status: %d\n", status);
    // std::lock_guard<std::mutex> lock(wifiMutex);
	if (status == HS_WIFI_CONNECTING) {
		wifiConnected = false;
		if (hadWifiConnection) {
			printf("%u: WIFI: Lost Connection..\n", (uint32_t)millis());
		}
	}
    // std::lock_guard<std::mutex> unlock(wifiMutex);
	printf("CoachWifi::wifiStatusChanged completed\n");
}

void CoachWifi::wifiReady() {
	printf("CoachWifi::wifiReady started\n");
    // std::lock_guard<std::mutex> lock(wifiMutex);
	wifiConnected = true;
	hadWifiConnection = true;
	printf("%u: WIFI: Ready..\n", (uint32_t)millis());
    // std::lock_guard<std::mutex> unlock(wifiMutex);
	printf("CoachWifi::wifiReady completed\n");
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