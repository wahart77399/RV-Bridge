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

#include "elapsedMillis.h"
// #include "HomeSpan.h" 
// #include "ESP32CAN.h"
#include "CAN_config.h"

// #include "config.h"

#include <CoachWifi.h>
#include <CoachESP32.h>
#include <DeviceFactory.h>
#include <Packet.h>
#include <PacketQueue.h>
CAN_device_t CAN_cfg;

void setup() {
	printf("setup start\n");
	// put your setup code here, to run once:
  	pinMode(LED_BUILTIN, OUTPUT);
  	printf("Setup complete, LED pin set to OUTPUT.\n");
	
	constexpr uint8_t ERROR = 1;
	bool completed = false;
	CoachESP32* coachESP32 = CoachESP32::getInstance(&CAN_cfg);
	if (coachESP32 != nullptr) {
		printf("CoachESP32 instance created/obtained \n");
	
		// Initialize the CoachESP32 instance
		coachESP32->initialize();
		CoachWifi* coachWifi = CoachWifi::getInstance();
		if (coachWifi != nullptr) { 
			printf("CoachWifi instance created/obtained \n");
			
			coachWifi->initialize(); 
			printf("setup: DeviceFactory getting instance \n"); 
			DeviceFactory* factory = DeviceFactory::getInstance();
			if (factory != nullptr) {
				// printf("ERRROR: factory failed to create devices \n");
				completed = true;
				printf("%u: Init complete.\n", (uint32_t)millis());
				// setting it to print mode for now while I gather raw data so I can determine instance data
				Packet::setPacketPrintMode(packetPrintYes);
				Packet::initialize();

			} 
		}
	} 
	/*
	if (!completed) {
		printf("ERROR: *********initialization failure!!!************ \n");
		exit(ERROR);
	} */
	printf("setup complete\n");
}

int i = 0;
void loop() {
	PacketQueue::adjustTimingOfPacketRecieve();
	
	CoachWifi::pollSpan();
	// printf("looping %u\n", i);
	i++;
	// CAN_frame_t packet;
	CoachESP32::pollESP32();
	// put your main code here, to run repeatedly:
	//digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level
  	// printf("LED is ON.\n");
  	//delay(100);                       // wait for a second
  	//digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  	// printf("LED is OFF.\n");
  	//delay(100);                       // wait for a second

}
