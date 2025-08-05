#ifndef PACKET_H
#define PACKET_H
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
#include "ESP32CAN.h"
#include "CAN_config.h"
#include <mutex>
#include "PacketKit.h"
#include "DGN.h"
// #include "debug.h"

typedef enum {
	packetPrintNo = 0,
	packetPrintYes,
	packetPrintIfUnknown,
	packetPrintIfKnown
} PacketPrint ;

constexpr uint8_t SOURCE_ADDRESS = 145U; // Start bit for source address extraction

class Packet : protected PacketKit
{
    private:
        //static std::mutex packetMutex;
        static PacketPrint packetPrintMode;
        // static std::ostringstream oss;

        // @brief need to make sure this address is not being used elsewhere on the bus - this address will be the address of the ESP32/RV-Bridge from where messages will be sent.
        static const uint8_t sourceAddress = SOURCE_ADDRESS; 
        // constants for packets or esp32 - not sure yet - maybe mixing...
        static constexpr uint8_t  INSTANCE_INDEX = 0;
        static constexpr uint8_t  GROUP_INDEX = 1;
        static constexpr uint32_t packetBlinkTime = 25;
        static constexpr uint32_t heatbeatRate = 3000;
        static constexpr uint32_t heartbeatBlinkTime = 10;
        
        Packet() = delete; // Prevent instantiation
        Packet(const Packet&) = delete; // Prevent copy
        Packet& operator=(const Packet&) = delete; // Prevent assignment

        static boolean isRemoteTransmissionRequest(CAN_frame_t* packet); 

        static uint32_t makeMsg(uint32_t dgn, uint8_t sourceID=0, uint8_t priority=6);

        static inline void printRemoteTransmissionRequest(CAN_frame_t *packet) {
            if (isRemoteTransmissionRequest(packet)) {
                // std::ostringstream shortOss;
                //shortOss << "Packet::printRemoteTransmissionRequest" << std::endl;
                // LOGIT(VERBOSE_LOG_LEVEL, shortOss);
                printf("Packet::printRemoteTransmissionRequest\n");
            }
        }

        static inline RVC_DGN getDGN(CAN_frame_t *packet) {
            RVC_DGN dgn = ERROR;
            if ((packet != nullptr)) { // && (packet->FIR.B.FF == CAN_frame_std)) {
                dgn = (RVC_DGN) PacketKit::getMsgBits(packet->MsgID, DGN::startBit(), DGN::numBits());
            }
            return dgn; // Return ERROR if not a standard frame
        }         

        static inline uint8_t getSourceAddress(CAN_frame_t *packet) {
            uint8_t result =  0;
            if ((packet != nullptr)) { // && (packet->FIR.B.FF == CAN_frame_std)) {
                result = PacketKit::getMsgBits(packet->MsgID, SOURCE_ADDRESS_START_BIT, SOURCE_ADDRESS_NUM_BITS);
            }
            return result;
        }

        static inline uint8_t getPriority(CAN_frame_t *packet) {
            uint8_t result =  0;
            if ((packet != nullptr)) { //  && (packet->FIR.B.FF == CAN_frame_std)) {
                result = PacketKit::getMsgBits(packet->MsgID, PRIORITY_START_BIT, PRIORITY_NUM_BITS);
            }
            return result;
        }

        static inline uint8_t* getData(CAN_frame_t *packet) {
            uint8_t* result =  nullptr;
            if (packet != nullptr) {
                result = packet->data.u8;
            }
            return result;
        } 

        static void processPacketQueue(void);

    public:
        
        static void setPacketPrintMode(PacketPrint mode);

        static void adjustTimingOfPacketRecieve(void);
        static void processPacket(CAN_frame_t* packet);
        // static void queueItUpForSend(CAN_frame_t* packet, bool shortGap=false);
        static void displayPacket(CAN_frame_t* packet, PacketPrint printPacket = packetPrintNo);
        static void initPacket(CAN_frame_t* packet, uint8_t index, uint32_t msgID);
        static inline uint8_t getByte(uint8_t* data, uint8_t i) {
            uint8_t result = -1;
            if ((data != nullptr) && (i >= 0)) {
                result = data[i];
            }
            return result;
        }

        static inline uint8_t getAddress(CAN_frame_t* data) { return getSourceAddress(data); }
        static inline uint8_t getIndex(uint8_t* data) { return getByte(data, INSTANCE_INDEX); }

       
        static inline uint8_t getGroup(uint8_t* data) { return getByte(data, GROUP_INDEX); }
        
        static const char* parseBufferForValuePair(const char* buff, int16_t& val1, int16_t& val2);
        static const char* getNextValue(const char* buff, int16_t& val); 

        static void initialize(void); 
};



    


#endif
