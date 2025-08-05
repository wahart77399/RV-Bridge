
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

PacketPrint Packet::packetPrintMode = packetPrintNo;


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
	// Packet::oss << "Packet::isRemoteTransmissionRequest Start\n";
	// LOGIT(VERBOSE_LOG_LEVEL, Packet::oss);
    boolean result = false;
    if (packet != nullptr) {
		// printf("Packet::isRemoteTransmissionRequest: packet != nullptr\n");
		// printf("Packet::isRemoteTransmissionRequest: RTR is %d\n", packet->FIR.B.RTR);
        result = (packet->FIR.B.RTR == CAN_RTR);
		// printf("Packet::isRemoteTransmissionRequest: RTR is %s\n", result ? "true" : "false");
    }
	//Packet::oss << "Packet::isRemoteTransmissionRequest End: " <<  result << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, Packet::oss);
    return result;
}
#include "Thermostat.h"
void Packet::displayPacket(CAN_frame_t* packet, PacketPrint printPacket) {
	//Packet::oss << "Packet::displayPacket start" << std::endl;
	// LOGIT(VERBOSE_LOG_LEVEL, oss);
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
		if (((dgn == THERMOSTAT_COMMAND_1) || (dgn == THERMOSTAT_STATUS_1)) && (index < 5)) {
			uint8_t priority = getPriority(packet);
			
        	if (data != nullptr) {
				// uint16_t* temperatureData = (uint16_t*) &(data[3]);
				// uint16_t tempHot = data[4];
				// uint16_t resultH = HVAC_Thermostat::convToTempC(data[4]<<8 | data[3]);
				// uint16_t tempCool = data[6];
				// uint16_t resultC = HVAC_Thermostat::convToTempC(data[6]<<8 | data[5]);
				// std::ostringstream oss;
				/**
				Packet::oss << "Packet::displayPacket: DGN: " << std::hex << dgn << ", Source Address: " << sourceAddr << ", index: " << index << ", Priority: " << priority  
					<< "Data : d[0]=" << std::hex << data[0] << "d[1]=" << std::hex << data[1] << "d[2]=" << std::hex << data[2] 
					<< "d[3]=" << std::hex << data[3] << "d[4]=" << std::hex << data[4] << "d[5]=" << std::hex << data[5]
					<< "d[6]=" << std::hex << data[6] << "d[7]=" << std::hex << data[7] << std::endl;
				LOGIT(MIN_LOG_LEVEL, Packet::oss); 
				*/
				printf("printf Packet::displayPacket: DGN %#x, Source Address %#x, Data : d[0]=%d, d[1]=%#x, d[2]=%#x, d[3]=%#x, d[4]=%#x, d[5]=%#x, d[6]=%#x, d[7]=%#x\n", 
				 		dgn, sourceAddr, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
			}
        }
    }
	//Packet::oss << "Packet::displayPacket end" << std::endl;
	//LOGIT(VERBOSE_LOG_LEVEL, oss);
}

#include "ChassisMobility.h"
void Packet::processPacket(CAN_frame_t *packet) 
{
	//Packet::oss << "Packet::processPacket Start\n";
	//LOGIT(VERBOSE_LOG_LEVEL, Packet::oss);
	// std::lock_guard<std::mutex> lock(Packet::packetMutex);
	if ((packet != nullptr) && (!isRemoteTransmissionRequest(packet))) {

		RVC_DGN dgn = DGN::getDGN(packet);
		uint8_t* rawData = getData(packet);
		//Packet::oss << "Packet::processPacket: DGN: " << std::hex << dgn << ", Source Address: << " << getSourceAddress(packet) << ", Index: " << getIndex(rawData) << std::endl; 
		//		dgn, getSourceAddress(packet), getIndex(rawData));
		// LOGIT(NOT_SO_VERBOSE_LOG_LEVEL, Packet::oss);
		if (rawData != nullptr) {
			DeviceFactory* factory = DeviceFactory::getInstance();
			if (factory != nullptr) {
				GenericDevice* device = factory->getDeviceByData(dgn, rawData);
				if (device != nullptr) {
					device->executeCommand(dgn, rawData, getSourceAddress(packet));
					
				}
				
			} else { 
				//Packet::oss << "Packet::processPacket ERROR: factory null" << std::endl;
				// LOGIT(NOT_SO_VERBOSE_LOG_LEVEL,Packet::oss); 
			}
		}
	}
	// std::lock_guard<std::mutex> unlock(Packet::packetMutex);
	//Packet::oss << "Packet::processPacket End" << std::endl;
	//LOGIT(VERBOSE_LOG_LEVEL, Packet::oss);
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

void Packet::initialize(void) { 
    ;
}
