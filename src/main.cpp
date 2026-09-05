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
#include "CAN_config.h"

#include <CoachWifi.h>
#include <CoachESP32.h>
#include <DeviceFactory.h>
#include <Packet.h>
#include <PacketQueue.h>

CAN_device_t CAN_cfg;

void setup() {
	printf("setup start\n");
	pinMode(LED_BUILTIN, OUTPUT);

	CoachESP32* coachESP32 = CoachESP32::getInstance(&CAN_cfg);
	if (coachESP32 == nullptr) {
		printf("ERROR: CoachESP32 init failed\n");
		return;
	}
	printf("CoachESP32 instance created/obtained\n");
	coachESP32->initialize();

	CoachWifi* coachWifi = CoachWifi::getInstance();
	if (coachWifi == nullptr) {
		printf("ERROR: CoachWifi init failed\n");
		return;
	}
	printf("CoachWifi instance created/obtained\n");
	coachWifi->initialize();

	printf("setup: DeviceFactory getting instance\n");
	DeviceFactory* factory = DeviceFactory::getInstance();
	if (factory == nullptr) {
		printf("ERROR: DeviceFactory init failed\n");
		return;
	}

	printf("%u: Init complete.\n", (uint32_t)millis());
	// Packet dump on while gathering raw RV-C instance data
	Packet::setPacketPrintMode(packetPrintYes);
	Packet::initialize();
	printf("setup complete\n");
}

void loop() {
	PacketQueue::adjustTimingOfPacketRecieve();
	CoachWifi::pollSpan();
	CoachESP32::pollESP32();
}
