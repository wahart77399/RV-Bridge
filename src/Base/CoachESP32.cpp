#include "Arduino.h" 
#include "CoachESP32.h"
#include "CanFrameTypes.h"
// #include "ESP32CAN.h"
// #include "CAN_config.h"

#include "Packet.h"
#include "PacketQueue.h"


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
CoachESP32* CoachESP32::instance = nullptr;


CoachESP32* CoachESP32::getInstance(CAN_device_t* cfg) {
    if (!CoachESP32::instance) {
        CoachESP32::instance = new CoachESP32(cfg);
    }
    return CoachESP32::instance;
}

void CoachESP32::initialize(void) {
    initPins();
    flashPin(indicatorPinB, 20, 200);
    Serial.begin(115200);
    if (CoachESP32::CAN_cfg != nullptr) {
        CoachESP32::CAN_cfg->tx_pin_id = kCanTxPin; 
        CoachESP32::CAN_cfg->rx_pin_id = kCanRxPin;
        CoachESP32::CAN_cfg->speed     = CAN_SPEED_250KBPS;
        PacketQueue::initPacketQueue(*CoachESP32::CAN_cfg);
    }
    // Reduce processor frequency to lower current consumption
	setCpuFrequencyMhz(160);
}

void CoachESP32::adjustRGB(bool red, bool green, bool blue) {
    digitalWrite(indicatorPinG, green);
	digitalWrite(indicatorPinR, red);
	digitalWrite(indicatorPinB, blue);
}

void CoachESP32::pollESP32(void) {
    CoachESP32* coach = CoachESP32::getInstance();
    if (coach != nullptr) {
        coach->processQueue();
    }
    PacketQueue::processPacketQueue();
}


void CoachESP32::processQueue(void) {
    if (CoachESP32::CAN_cfg != nullptr) {
        CAN_frame_t packet;
        if (PacketQueue::packetReceived(CoachESP32::CAN_cfg, &packet)) {

        }
    }
}
