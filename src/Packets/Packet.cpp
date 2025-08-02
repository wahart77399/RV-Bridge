
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

#include "Packet.h"
#include "DGN.h"
#include "DeviceFactory.h"
#include "GenericDevice.h"

#include "CoachWifi.h"
#include "CoachESP32.h"

// std::mutex Packet::packetMutex;
PacketPrint Packet::packetPrintMode = packetPrintNo;
// elapsedMillis Packet::heartbeatTime;
// elapsedMillis Packet::lastPacketSendTime = 1000;
// elapsedMillis Packet::lastPacketRecvTime = 1000;
// uint16_t Packet::packetQueueHead = 0;
// uint16_t Packet::packetQueueTail = 0;
// bool Packet::doCANWrite = true;
//bool Packet::sendIndi cator;
//bool Packet::recvIndicator;
// CAN_frame_t Packet ::packetQueue[Packet::sendQueueSize];
// bool Packet::packetShortGap[Packet::sendQueueSize];

uint32_t Packet::makeMsg(uint32_t dgn, uint8_t sourceID, uint8_t priority) {
	if (sourceID == 0) {
		sourceID = Packet::sourceAddress;
	}

	return (priority<<26) | (dgn << 8) | sourceID;	
}

void Packet::initPacket(CAN_frame_t* packet, uint8_t index, uint32_t msgID) {
	packet->FIR.B.DLC = 8;
	packet->FIR.B.RTR = CAN_no_RTR;
	packet->FIR.B.FF = CAN_frame_ext;
	packet->MsgID = Packet::makeMsg(msgID);
	packet->data.u8[0] = index;
	for (auto i=1; i<8; i++) {
		packet->data.u8[i] = 0xFF;
	}
}

boolean Packet::isRemoteTransmissionRequest(CAN_frame_t* packet) {
	//printf("Packet::isRemoteTransmissionRequest Start\n");
    boolean result = false;
    if (packet != nullptr) {
		// printf("Packet::isRemoteTransmissionRequest: packet != nullptr\n");
		// printf("Packet::isRemoteTransmissionRequest: RTR is %d\n", packet->FIR.B.RTR);
        result = (packet->FIR.B.RTR == CAN_RTR);
		// printf("Packet::isRemoteTransmissionRequest: RTR is %s\n", result ? "true" : "false");
    }
	// printf("Packet::isRemoteTransmissionRequest End: %s\n", result ? "true" : "false");
    return result;
}
#include "Thermostat.h"
void Packet::displayPacket(CAN_frame_t* packet, PacketPrint printPacket) {
	// printf("Packet::displayPacket start\n");
	// std::lock_guard<std::mutex> lock(Packet::packetMutex);
	if (isRemoteTransmissionRequest(packet)) {
		printf("Packet::displayPacket: Remote Transmission Request received with ID 0x%08X, DLC %d\n", packet->MsgID, packet->FIR.B.DLC);
		printRemoteTransmissionRequest(packet);	
	} else if (packet != nullptr) {
		// printf("Packet::displayPacket: Packet received with ID 0x%08X, DLC %d\n", packet->MsgID, packet->FIR.B.DLC);

		uint8_t sourceAddr = getSourceAddress(packet);
		uint8_t* data = getData(packet);
		uint8_t index = getIndex(data);
		RVC_DGN dgn = DGN::getDGN(packet);
		// if ((dgn==DC_DIMMER_COMMAND_2) || (dgn==DC_DIMMER_COMMAND) || (dgn == DC_DIMMER_STATUS_1) || (dgn == DC_DIMMER_STATUS_2) || (dgn == DC_DIMMER_STATUS_3)) { // rvc dgns are this or better
		// if (((dgn == LOCK_COMMAND) || (dgn == LOCK_STATUS)) && (index == 1)) {
		// NO FANS FOUND if ((dgn == ROOF_FAN_COMMAND_1) || (dgn == ROOF_FAN_COMMAND_2) || (dgn == ROOF_FAN_STATUS_1) || (dgn == ROOF_FAN_STATUS_2)) {
		// NO BATTERY information on RVC if ((dgn == BATTERY_COMMAND) || (dgn == BATTERY_STATUS_1) || (dgn == BATTERY_STATUS_2) || (dgn == BATTERY_STATUS_3)) {
		// if ((dgn == WATER_PUMP_COMMAND) || (dgn == WATER_PUMP_STATUS)) {
		if (((dgn == THERMOSTAT_COMMAND_1) || (dgn == THERMOSTAT_STATUS_1)) && (index < 4)) {
			uint8_t priority = getPriority(packet);
			
			//printf("Packet::displayPacket: Data: ");
			
			// printf("Packet::displayPacket: DGN: %#x, Source Address: %d, Index: %d, Priority: %d\n", 
			// 	    dgn, sourceAddr, index, priority);
        	if (data != nullptr) {
				// uint16_t* temperatureData = (uint16_t*) &(data[3]);
				// uint16_t tempHot = data[4];
				// uint16_t resultH = HVAC_Thermostat::convToTempC(data[4]<<8 | data[3]);
				// uint16_t tempCool = data[6];
				// uint16_t resultC = HVAC_Thermostat::convToTempC(data[6]<<8 | data[5]);

				printf("Packet::displayPacket: DGN %#x, Source Address %#x, Data : d[0]=%d, d[1]=%#x, d[2]=%#x, d[3]=%#x, d[4]=%#x, d[5]=%#x, d[6]=%#x, d[7]=%#x\n", 
						dgn, sourceAddr, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
				// printf("Packet::displayPacket - heat = %d, cool = %d\n", resultH, resultC);
				// printf("Packet::displayPacket: data != nullptr\n"); 
			}
        }
    }
	// std::lock_guard<std::mutex> unlock(Packet::packetMutex);
	//printf("Packet::displayPacket end\n");
}

#include "ChassisMobility.h"
void Packet::processPacket(CAN_frame_t *packet) 
{
	//printf("Packet::processPacket Start\n");
	// std::lock_guard<std::mutex> lock(Packet::packetMutex);
	if ((packet != nullptr) && (!isRemoteTransmissionRequest(packet))) {

		RVC_DGN dgn = DGN::getDGN(packet);
		uint8_t* rawData = getData(packet);
		// printf("Packet::processPacket: DGN: %#x, Source Address: %d, Index: %d\n", 
		//		dgn, getSourceAddress(packet), getIndex(rawData));
		if (rawData != nullptr) {
			DeviceFactory* factory = DeviceFactory::getInstance();
			if (factory != nullptr) {
				GenericDevice* device = factory->getDeviceByData(dgn, rawData);
				if (device != nullptr) {
					// printf("Packet::processPacket: Device found: \n");
					// if ((dgn != WATER_PUMP_COMMAND) && (dgn != WATER_PUMP_STATUS)) {
					device->executeCommand(dgn, rawData, getSourceAddress(packet));
					// } else {
					//. device->executeCommand(dgn, rawData, getSourceAddress(packet));
					// }
				}
				
			} else { 
				printf("ERROR: factory null \n "); 
			}
		}
	}
	// std::lock_guard<std::mutex> unlock(Packet::packetMutex);
	//printf("Packet::processPacket End\n");
}

const char* Packet::parseBufferForValuePair(const char* buff, int16_t& val1, int16_t& val2) {
	val1 = -1;
	val2 = -2;

	const char* equals = strchr(buff, '=');

	if (equals) {
		equals += 1;
		val1 = atoi(buff);
		val2 = atoi(equals);

		buff = strchr(equals, ',');
		if (buff) {
			buff += 1;
		}
	}
	else {
		buff = NULL;
	}

	return buff;
}

const char* Packet::getNextValue(const char* buff, int16_t& val) {
	val = -1;
	if (buff && buff[0]) {
		val = atoi(buff);
		buff = strchr(buff, ',');
		if (buff) {
			buff += 1;
		}
	}
	else {
		buff = NULL;
	}
	return buff;
}

void Packet::setPacketPrintMode(PacketPrint mode) {
    // std::lock_guard<std::mutex> lock(Packet::packetMutex);
    Packet::packetPrintMode = mode;
    // std::lock_guard<std::mutex> unlock(Packet::packetMutex);
}

/**
void Packet::adjustTimingOfPacketRecieve(void) {
    // std::lock_guard<std::mutex> lock(Packet::packetMutex);
    sendIndicator = lastPacketSendTime < packetBlinkTime;
    recvIndicator = lastPacketRecvTime < packetBlinkTime;
    
    if (sendIndicator || recvIndicator) {
    	heartbeatTime = heartbeatBlinkTime;		// adjust next heartbeat time to avoid overlap with send/recv indicators
    }

    bool hearbeatIndicator = (heartbeatTime % heatbeatRate) < heartbeatBlinkTime;

	if (!CoachWifi::isConnected()) {
		hearbeatIndicator = (heartbeatTime % (heatbeatRate/4)) > heartbeatBlinkTime*5;
	}
	CoachESP32::adjustRGB(!sendIndicator, !hearbeatIndicator, !recvIndicator);
    // std::lock_guard<std::mutex> unlock(Packet::packetMutex);
}   */

void Packet::initialize(void) { 
    // std::lock_guard<std::mutex> lock(Packet::packetMutex);
    // Packet::lastPacketSendTime = 1000;
    // Packet::lastPacketRecvTime = 1000;
    // std::lock_guard<std::mutex> unlock(Packet::packetMutex);
}
/**
void Packet::queueItUpForSend(CAN_frame_t* packet, bool shortGap) {
	// std::lock_guard<std::mutex> lock(Packet::packetMutex);
	printf("Packet::queueItUpForSend: Queuing packet with MsgID=%#x\n", packet->MsgID);
	uint16_t nextIndex = (Packet::packetQueueHead + 1) % Packet::sendQueueSize;
	printf("Packet::queueItUpForSend: nextIndex=%d, packetQueueTail=%d\n", nextIndex, Packet::packetQueueTail);
	while (nextIndex == Packet::packetQueueTail) {	// wait if queue is full
		printf("Packet::queueItUpForSend: Queue is full, waiting...\n");
		Packet::processPacketQueue();
	}
	printf("Packet::queueItUpForSend: Adding packet to queue at index %d\n", nextIndex);
	Packet::packetQueueHead = nextIndex;
	Packet::packetQueue[packetQueueHead] = *packet;
	Packet::packetShortGap[packetQueueHead] = shortGap;
	// std::lock_guard<std::mutex> unlock(Packet::packetMutex);
}
	/
	if (packet->FIR.B.RTR != CAN_RTR) { // CAN-RTR is a remote transmission request -> It's a bit within a CAN message that indicates whether the message is a data frame or a remote frame. When the RTR bit is set to dominant (0), it signifies a data frame containing data. When set to recessive (1), it indicates a remote frame requesting data from another node. 
		uint32_t dgn = getMsgBits(packet->MsgID, 24, 17);
		// uint8_t sourceAddr = getMsgBits(packet->MsgID, 7, 8);
		// uint8_t priority = getMsgBits(packet->MsgID, 28, 3);

		uint8_t* d = packet->data.u8;

		if (dgn == DC_DIMMER_STATUS_3) {
			uint8_t instance = d[0];
			// uint8_t group = d[1];
			uint8_t brightness = min(d[2], RVCBrightMax);
			// uint8_t enable = (d[3] >> 6) & 3;
			// uint8_t delayDuration = d[4];
			// DCDimmerCmd lastCmd = (DCDimmerCmd)d[5];
			// uint8_t status = (d[6] >> 2) & 3;

			// if (instance == 4) {
			// 	displayPacket(packet, packetPrintYes);
			// }
			setSwitchLevel(instance, brightness);
		}
		else if (dgn == THERMOSTAT_AMBIENT_STATUS) {
			uint8_t instance = d[0];
			double tempC = convToTempC(d[2]<<8 | d[1]);
			
			setAmbientTemp(instance, tempC);
		}
		else if (dgn == THERMOSTAT_STATUS_1) {
			uint8_t instance = d[0];
			ThermostatMode opMode = (ThermostatMode)(d[1] & 0x0F);
			FanMode fanMode = (FanMode)((d[1]>>4) & 0x03);
			// uint8_t scheduleEnabled = ((d[1]>>6) & 0x03);
			uint8_t fanSpeed = d[2];
			double heatTemp = convToTempC(d[4]<<8 | d[3]);
			double coolTemp = convToTempC(d[6]<<8 | d[5]);
			
			setThermostatInfo(instance, opMode, fanMode, fanSpeed, heatTemp, coolTemp);
		}
		else if (dgn == BATTERY_STATUS) {
			#ifdef CREATE_BATTERIES
			uint8_t instance = d[0];
			uint16_t v_int = d[3]<<8 | d[2];
			float voltage = v_int * 0.050;
			setBatteryVoltage(instance, voltage);		
			#endif
		}
	}
}
		*/






/* *
		if (dgn == DC_DIMMER_STATUS_3) {
			uint8_t instance = d[0];
			uint8_t group = d[1];
			uint8_t brightness = min(d[2], RVCBrightMax);
			uint8_t enable = (d[3] >> 6) & 3;
			uint8_t delayDuration = d[4];
			DCDimmerCmd lastCmd = (DCDimmerCmd)d[5];
			uint8_t status = (d[6] >> 2) & 3;

			if (printPacket==packetPrintYes || printPacket==packetPrintIfKnown) {
				printf("%u: DC_DIMMER_STATUS_3: inst=%d, grp=0X%02X, bright=%d, enable=%d, dur=%d, last cmd=%d, status=0X%02X\n", (uint32_t)millis(), instance, group, brightness, enable, delayDuration, lastCmd, status);				
			}
		}
		else if (dgn == DC_DIMMER_COMMAND_2) {
			uint8_t instance = d[0];
			uint8_t group = d[1];
			uint8_t brightness = d[2];
			DCDimmerCmd cmd = (DCDimmerCmd)d[3];
			uint8_t delayDuration = d[4];

			// if (instance!=32 && instance!=43 && instance!=44 && instance!=53 && instance!=54 && instance!=55 && instance!=56) {
			// 	printPacket = packetPrintYes;
			// }

			if (printPacket==packetPrintYes || printPacket==packetPrintIfKnown) {
				printf("%u: DC_DIMMER_COMMAND_2: inst=%d, grp=0X%02X, bright=%d, cmd=0X%02X, dur=%d\n", (uint32_t)millis(), instance, group, brightness, cmd, delayDuration);	
				printPacket = packetPrintYes;			
			}
		}
		else if (dgn == THERMOSTAT_AMBIENT_STATUS) {
			uint8_t instance = d[0];
			double tempC = convToTempC(d[2]<<8 | d[1]);
			
			if (printPacket==packetPrintYes || printPacket==packetPrintIfKnown) {
				printf("%u: THERMOSTAT_AMBIENT_STATUS: #%d, temp=%0.1fºC\n", (uint32_t)millis(), instance, tempC);
			}
		}
		else if (dgn == THERMOSTAT_STATUS_1 || dgn == THERMOSTAT_COMMAND_1) {
			uint8_t instance = d[0];
			ThermostatMode opMode = (ThermostatMode)(d[1] & 0x0F);
			FanMode fanMode = (FanMode)((d[1]>>4) & 0x03);
			// uint8_t scheduleEnabled = ((d[1]>>6) & 0x03);
			uint8_t fanSpeed = d[2];
			double heatTemp = convToTempC(d[4]<<8 | d[3]);
			double coolTemp = convToTempC(d[6]<<8 | d[5]);

			const char* name = (dgn == THERMOSTAT_COMMAND_1) ? "THERMOSTAT_COMMAND_1" : "THERMOSTAT_STATUS_1";
			
			if (printPacket==packetPrintYes || printPacket==packetPrintIfKnown) {
				printf("%u: %s: inst=%d, opMode=%d, fanMode=%d, fanSpeed=%d, heatTemp=%0.1fºC, coolTemp=%0.1fºC\n", (uint32_t)millis(), name, instance, opMode, fanMode, fanSpeed, heatTemp, coolTemp);
			}
		}
		else if (dgn == FURNACE_STATUS || dgn == FURNACE_COMMAND) {
			uint8_t instance = d[0];
			ACMode opMode = (ACMode)(d[1] & 0x03);
			uint8_t fanSpeed = d[2];
			uint8_t heatOutput = d[3];

			const char* name = (dgn == FURNACE_COMMAND) ? "FURNACE_COMMAND" : "FURNACE_STATUS";

			if (printPacket==packetPrintYes || printPacket==packetPrintIfKnown) {
				printf("%u: %s: inst=%d, opMode=%d, fanSpeed=%d, heatOutput=%d\n", (uint32_t)millis(), name, instance, opMode, fanSpeed, heatOutput);
			}
		}
		else if (dgn == DC_LOAD_COMMAND) {
			uint8_t instance = d[0];
			uint8_t group = d[1];
			uint8_t brightness = min(d[2], RVCBrightMax);
			uint8_t direction = (d[3] >> 4) & 0xF;
			uint8_t command = d[4];

			printf("%u: DC_LOAD_COMMAND: inst=%d, grp=0X%02X, bright=%d, dir=%d, cmd=%d\n", (uint32_t)millis(), instance, group, brightness, direction, command);				
		}
		else if (dgn == AWNING_COMMAND) {
			uint8_t instance = d[0];
			uint8_t direction = d[2];
			uint8_t position = d[3];
			uint8_t retract = d[5];

			printf("%u: AWNING_COMMAND: inst=%d, dir=%d, pos=%d, retract=%d\n", (uint32_t)millis(), instance, direction, position, retract);			
		}
		else if (dgn == AIR_CONDITIONER_STATUS) { }
		else if (dgn == AWNING_STATUS) { }
		else if (dgn == AWNING_STATUS_2) { }
		else if (dgn == DATE_TIME_STATUS) { }
		else if (dgn == GENERIC_INDICATOR_COMMAND) { }
		else if (dgn == GENERIC_CONFIGURATION_STATUS) { }
		else if (dgn == DC_SOURCE_STATUS_1) {
			uint8_t instance = d[0];
			uint8_t priority = d[1];
			uint16_t v_int = d[3]<<8 | d[2];
			uint32_t i_int = d[7]<<24 | d[6]<<16 | d[5]<<8 | d[4];
			float voltage = v_int * 0.050;
			float current = -2000000.0 + i_int * 0.001;

			printf("%u: DC_SOURCE_STATUS_1: inst=%d, pri=%d, voltage=%0.1f, current=%0.1f\n", (uint32_t)millis(), instance, priority, voltage, current);		
		}
		else if (dgn == BATTERY_STATUS) { }
		else if (dgn == TANK_STATUS) { }
		else if (dgn == GENERATOR_STATUS_1) { }
		else if (dgn == 0x0FECA) { }
		else if (dgn == 0x0E8FF) { }
		else if (dgn == 0x0EAFF) { }
		else if (dgn == 0x15FCE) { }
		else if (dgn == 0x1BBFD) { }
		else if (dgn == 0x1FACE) { }
		else if (dgn == 0x1FACF) { }
		else if (dgn == 0x1FBDA) { }
		else {
			knownPacket = false;
		}

		if (printPacket==packetPrintYes || (!knownPacket && printPacket==packetPrintIfUnknown) || (knownPacket && printPacket==packetPrintIfKnown)) {
			if (packet->FIR.B.FF == CAN_frame_std) {
				printf("%u: Std - ", (uint32_t)millis());
			}
			else {
				printf("%u: Ext - ", (uint32_t)millis());
			}

			printf("dgn=%05X, src=%02X, pri=%d, Data: ", dgn, sourceAddr, priority);

			for (auto i = 0; i < packet->FIR.B.DLC; i++) {
				printf("%02X ", packet->data.u8[i]);
			}
			printf("\n");
		}
	}
}
*/