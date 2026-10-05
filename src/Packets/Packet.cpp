
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
#include "debug.h"

// #define CUSTOM_CHAR_HEADER
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
    boolean result = false;
    if (packet != nullptr) {
		// RV_PRINTF("Packet::isRemoteTransmissionRequest: packet != nullptr\n");
		// RV_PRINTF("Packet::isRemoteTransmissionRequest: RTR is %d\n", packet->FIR.B.RTR);
        result = (packet->FIR.B.RTR == CAN_RTR);
		// RV_PRINTF("Packet::isRemoteTransmissionRequest: RTR is %s\n", result ? "true" : "false");
    }
    return result;
}
#include "Thermostat.h"
void Packet::displayPacket(CAN_frame_t* packet, PacketPrint printPacket) {
	if (isRemoteTransmissionRequest(packet)) {
		RV_PRINTF("Packet::displayPacket: Remote Transmission Request received with ID 0x%08X, DLC %d\n", packet->MsgID, packet->FIR.B.DLC);
		printRemoteTransmissionRequest(packet);	
	} else if (packet != nullptr) {
		// RV_PRINTF("Packet::displayPacket: Packet received with ID 0x%08X, DLC %d\n", packet->MsgID, packet->FIR.B.DLC);

		uint8_t sourceAddr = getSourceAddress(packet);
		uint8_t* data = getData(packet);
		uint8_t index = getIndex(data);
		RVC_DGN dgn = DGN::getDGN(packet);
		// if ((dgn == GENERATOR_AC_STATUS_1) || (dgn == GENERATOR_AC_STATUS_2) || (dgn == GENERATOR_AC_STATUS_3) || (dgn == GENERATOR_AC_STATUS_4)) {
			// RV_PRINTF("Packet::displayPacket: Generator DGN received with DGN=0x%08X, ID 0x%08X, DLC %d, data 0x%08X \n",dgn, packet->MsgID, packet->FIR.B.DLC, *((uint32_t*)data));
		// if ((dgn == WINDOW_SHADE_CONTROL_COMMAND) || (dgn == WINDOW_SHADE_CONTROL_STATUS)) {  //  || 
			// RV_PRINTF("Packet::displayPacket: Window Shade DGN received with DGN=0x%08X, ID 0x%08X, DLC %d, data 0x%08X \n",dgn, packet->MsgID, packet->FIR.B.DLC, *((uint32_t*)data));
		// if ((dgn == AWNING_COMMAND) || (dgn == AWNING_COMMAND_2) || (dgn == AWNING_STATUS) || (dgn == AWNING_STATUS_2)) { //  || 
		//	GOOD -> (dgn == FLOOR_HEAT_COMMAND) || (dgn == FLOOR_HEAT_STATUS)) { 
		// if ((dgn==DC_DIMMER_COMMAND_2) || (dgn==DC_DIMMER_COMMAND) || (dgn == DC_DIMMER_STATUS_1) || (dgn == DC_DIMMER_STATUS_2) || (dgn == DC_DIMMER_STATUS_3)) { // rvc dgns are this or better
		// if (((dgn == LOCK_COMMAND) || (dgn == LOCK_STATUS)) && (index == 1)) {
		// NO FANS FOUND if ((dgn == ROOF_FAN_COMMAND_1) || (dgn == ROOF_FAN_COMMAND_2) || (dgn == ROOF_FAN_STATUS_1) || (dgn == ROOF_FAN_STATUS_2)) {
		// NO BATTERY information on RVC if ((dgn == BATTERY_COMMAND) || (dgn == BATTERY_STATUS_1) || (dgn == BATTERY_STATUS_2) || (dgn == BATTERY_STATUS_3)) {
		// if ((dgn == WATER_PUMP_COMMAND) || (dgn == WATER_PUMP_STATUS)) {
		// if (((dgn == THERMOSTAT_COMMAND_1) || (dgn == THERMOSTAT_STATUS_1)) && (index < 5)) {
		if (((dgn == DC_SOURCE_STATUS_1) || (dgn == DC_SOURCE_STATUS_2) )) { // ||  (dgn == DC_SOURCE_STATUS_3) || (dgn == DC_SOURCE_STATUS_4) */ )) { // || 
		//	(dgn == DC_SOURCE_STATUS_5) || (dgn == DC_SOURCE_STATUS_6) || (dgn == DC_SOURCE_STATUS_7) || (dgn == DC_SOURCE_STATUS_8) ||
		//	(dgn == DC_SOURCE_STATUS_9) || (dgn == DC_SOURCE_STATUS_10) || (dgn == DC_SOURCE_STATUS_11) || (dgn == DC_SOURCE_STATUS_12) || 
		//	(dgn == DC_SOURCE_STATUS_13))) {
			// || ((dgn == ATS_AC_STATUS_1) || (dgn == ATS_AC_STATUS_2) || (dgn == ATS_AC_STATUS_3) || (dgn == ATS_AC_STATUS_4))
		// if /*( */(dgn == INVERTER_AC_STATUS_1) { //|| (dgn == INVERTER_STATUS)) { /* || (dgn == INVERTER_AC_STATUS_2) || (dgn == INVERTER_AC_STATUS_3) || (dgn == INVERTER_AC_STATUS_4)) { */
		// if (dgn == TANK_STATUS) {
		// if ((dgn == ATS_AC_STATUS_1)) { //  || (dgn == ATS_AC_STATUS_2) || (dgn == ATS_AC_STATUS_3) || (dgn == ATS_AC_STATUS_4) || (dgn == ATS_COMMAND)) {
			uint8_t priority = getPriority(packet);
			
        	if ((data != nullptr)) { //  && ( (index == 2)))  {
				//uint16_t* temperatureData = (uint16_t*) &(data[3]);
				//uint16_t tempHot = data[4];
				// uint16_t resultH = HVAC_Thermostat::convToTempC(data[4]<<8 | data[3]);
				// uint16_t tempCool = data[6];
				// uint16_t resultC = HVAC_Thermostat::convToTempC(data[6]<<8 | data[5]);
				// std::ostringstream oss;
				
				if (data[4] != 0x00) // && (data[0] != 0x41))
				// RV_PRINTF("RV_PRINTF Packet::displayPacket: DGN %#x, index = %#x, Source Address %#x, Data : d[0]=%#x, d[1]=%#x, d[2]=%#x, d[3]=%#x, d[4]=%#x, d[5]=%#x, d[6]=%#x, d[7]=%#x\n", 
				// 		dgn, index, sourceAddr, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
				; // do nothing - just a place to put a breakpoint if needed
			}
        }
    }
}

#include "ChassisMobility.h"
#include "LearnMode.h"
#include "BridgeDiagnostics.h"
#include "LearnRecord.h"
#include <cstring>
void Packet::processPacket(CAN_frame_t *packet) 
{
	if ((packet != nullptr) && (!isRemoteTransmissionRequest(packet))) {

		RVC_DGN dgn = DGN::getDGN(packet);
		uint8_t* rawData = getData(packet);
		if (rawData != nullptr) {
			uint8_t sourceAddress = getSourceAddress(packet);
			LearnMode::observe(dgn, sourceAddress, rawData);
			DeviceFactory* factory = DeviceFactory::getInstance();
			if (factory != nullptr) {
				GenericDevice* device = factory->getDeviceByData(dgn, rawData, sourceAddress);
				if (device != nullptr) {
					bool handled = device->executeCommand(dgn, rawData, sourceAddress);
					BridgeDiagnostics::observeDevice(device, dgn, sourceAddress, handled);
				} else {
					uint8_t instanceIndex = rawData[0];
					bool instanceVerified = false;
					const char* dgnName = LearnTable::dgnName(dgn);
					if (dgn == DM_RV) {
						if (packet->FIR.B.DLC > 1) {
							instanceIndex = rawData[1];
							instanceVerified = true;
						} else {
							instanceIndex = 0xFF;
						}
					} else if ((dgn == GENERATOR_STATUS_1) || (dgn == GENERATOR_STATUS_2) ||
					           (dgn == GENERATOR_DEMAND_STATUS)) {
						instanceIndex = 0xFF;
					} else if (dgnName != nullptr && std::strcmp(dgnName, "UNKNOWN_DGN") != 0) {
						uint8_t decodedIndex = instanceIndex;
						instanceVerified = DeviceFactory::instanceFromData(dgn, rawData, decodedIndex);
						if (instanceVerified) instanceIndex = decodedIndex;
					}
					uint8_t dataLength = packet->FIR.B.DLC > 8 ? 8 : packet->FIR.B.DLC;
					BridgeDiagnostics::observeUnmapped(dgn, sourceAddress, instanceIndex,
					                                   instanceVerified, rawData, dataLength);
				}
			}
		}
	}
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
