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
// #include "CAN_config.h"
#include "CanFrameTypes.h"

#include <CoachWifi.h>
#include <CoachESP32.h>
#ifdef FUTURE
#include <SmartCoachWeb.h>
#endif
#include <WifiCredentials.h>
#include <DeviceFactory.h>
#include <Packet.h>
#include <PacketQueue.h>
#include <debug.h>

CAN_device_t CAN_cfg;

void setup() {
	RV_PRINTF("setup start\n");
	pinMode(LED_BUILTIN, OUTPUT);

	CoachESP32* coachESP32 = CoachESP32::getInstance(&CAN_cfg);
	if (coachESP32 == nullptr) {
		RV_PRINTF("ERROR: CoachESP32 init failed\n");
		return;
	}
	RV_PRINTF("CoachESP32 instance created/obtained\n");
	coachESP32->initialize();

	CoachWifi* coachWifi = CoachWifi::getInstance();
	if (coachWifi == nullptr) {
		RV_PRINTF("ERROR: CoachWifi init failed\n");
		return;
	}
	RV_PRINTF("CoachWifi instance created/obtained\n");
	coachWifi->initialize();
	if (CoachWifi::isProvisioning()) {
        RV_PRINTF("setup: Wi-Fi provisioning - devices and CAN not started\n");
        return;
    }

	// First call creates the singleton; subsequent calls just return it
	#ifdef FUTURE
	String ssid;
	String passPhrase;
	WifiCredentials::load(ssid, passPhrase);
    SmartCoachWebServer& server = SmartCoachWebServer::instance(ssid.c_str(), passPhrase.c_str());
    server.begin();
	#endif

	RV_PRINTF("setup: DeviceFactory getting instance\n");
	DeviceFactory* factory = DeviceFactory::getInstance();
	if (factory == nullptr) {
		RV_PRINTF("ERROR: DeviceFactory init failed\n");
		return;
	}

	RV_PRINTF("%u: Init complete.\n", (uint32_t)millis());
	// Packet dump on while gathering raw RV-C instance data
	Packet::setPacketPrintMode(packetPrintYes);
	Packet::initialize();
	RV_PRINTF("setup complete\n");
}

void loop() {
	PacketQueue::adjustTimingOfPacketRecieve();
	CoachWifi::pollSpan();
	if (!CoachWifi::isProvisioning()) {
		CoachESP32::pollESP32();
	}
	#ifdef FUTURE
	SmartCoachWebServer::instance().handleClient();
	#endif
}
