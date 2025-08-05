#include "Arduino.h"
#include "CoachESP32.h"
#include "ESP32CAN.h"
#include "CAN_config.h"

#include "Packet.h"
#include "PacketQueue.h"

// std::ostringstream CoachESP32::oss;

CoachESP32::PinSetup CoachESP32::pinList[] = {
    { CoachESP32::indicatorPinR, OUTPUT, HIGH },
    { CoachESP32::indicatorPinG, OUTPUT, HIGH },
    { CoachESP32::indicatorPinB, OUTPUT, HIGH },
};

void CoachESP32::initPins(void) {
	for (PinSetup pin : pinList) {
		pinMode(pin.pinNumber, pin.mode);
		digitalWrite(pin.pinNumber, pin.state);
	}
}

void CoachESP32::flashPin(uint8_t pin, uint16_t count, uint16_t period) {
	for (auto i=0; i<count; i++) {
		digitalWrite(pin, LOW);
		delay(period/2);
		digitalWrite(pin, HIGH);
		delay(period/2);
	}
}

CoachESP32::CoachESP32(CAN_device_t* cfg) : CAN_cfg(cfg) {
    
    // CoachESP32::CAN_cfg = new CAN_device_t;
}

// create the mutex
// std::mutex CoachESP32::esp32Mutex;
CoachESP32* CoachESP32::instance = nullptr;


CoachESP32* CoachESP32::getInstance(CAN_device_t* cfg) {
    // std::lock_guard<std::mutex> lock(esp32Mutex);
    if (!CoachESP32::instance) {
        CoachESP32::instance = new CoachESP32(cfg);
    }
    // std::lock_guard<std::mutex> unlock(esp32Mutex);
    return CoachESP32::instance;
}

void CoachESP32::initialize(void) {
    //CoachESP32::oss<< "CoachESP32::initialize() called" << std::endl;
    //LOGIT(VERBOSE_LOG_LEVEL, CoachESP32::oss);
    initPins();
    flashPin(indicatorPinB, 20, 200);
    Serial.begin(115200);
	//CoachESP32::oss << "CoachESP32::initialize - RV Bridge - Startup"  << std::endl;
    //LOGIT(VERBOSE_LOG_LEVEL, oss);
    /* ******
	#ifdef OVERRIDE_MAC_ADDRESS
 		uint8_t newMACAddress[] = OVERRIDE_MAC_ADDRESS;
 		esp_base_mac_addr_set(&newMACAddress[0]);
 		printf("%u: MAC address updated to: %s\n", (uint32_t)millis(), WiFi.macAddress().c_str());
 	#endif
    ****** */
	//CoachESP32::oss << "Init CAN module" << std::endl;
    //LOGIT(VERBOSE_LOG_LEVEL, oss);
    if (CoachESP32::CAN_cfg != nullptr) {
        PacketQueue::initPacketQueue(*CoachESP32::CAN_cfg);
    }
    // Reduce processor frequency to lower current consumption
	setCpuFrequencyMhz(160);
    //CoachESP32::oss << "CoachESP32 initialize complete" << std::endl;
    //LOGIT(VERBOSE_LOG_LEVEL, oss);
}

void CoachESP32::adjustRGB(bool red, bool green, bool blue) {
    digitalWrite(indicatorPinG, green);
	digitalWrite(indicatorPinR, red);
	digitalWrite(indicatorPinB, blue);
}

void CoachESP32::pollESP32(void) {
    //CoachESP32::oss << "CoachESP32::pollESP32 started" << std::endl;
    //LOGIT(VERBOSE_LOG_LEVEL, oss);
    // std::lock_guard<std::mutex> lock(esp32Mutex);
    CoachESP32* coach = CoachESP32::getInstance();
    if (coach != nullptr) {
        coach->processQueue();
    }
    PacketQueue::processPacketQueue();
    // std::lock_guard<std::mutex> unlock(esp32Mutex);
    //CoachESP32::oss << "CoachESP32::pollESP32 completed" << std::endl;
    // LOGIT(VERBOSE_LOG_LEVEL, oss);
}


void CoachESP32::processQueue(void) {
    // CoachESP32::oss << "CoachESP32::processQueue started" << std::endl;
    if (CoachESP32::CAN_cfg != nullptr) {
        CAN_frame_t packet;
        // CoachESP32::oss << "CoachESP32::processQueue started" << std::endl;
        if (PacketQueue::packetReceived(CoachESP32::CAN_cfg, &packet)) {
            // printf("INFO: Packet received \n");
        }
    }
    //CoachESP32::oss << "CoachESP32::processQueue completed" << std::endl;
    //LOGIT(VERBOSE_LOG_LEVEL, oss);
}
