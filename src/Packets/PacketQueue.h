#ifndef RVC_PACKET_QUEUE_H
#define RVC_PACKET_QUEUE_H
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

constexpr uint16_t receiveQueueSize = 10;
constexpr uint16_t sendQueueSize = 8;

constexpr uint32_t sendPacketIntervalmS = 50;
constexpr uint32_t minSendPacketIntervalmS = 5;

constexpr unsigned long lastSendTime = 1000UL;
constexpr unsigned long lastRecvTime = 1000UL;
constexpr uint32_t packetBlinkTime = 25;
constexpr uint32_t heatbeatRate = 3000;
constexpr uint32_t heartbeatBlinkTime = 10;
constexpr gpio_num_t canTxPin = GPIO_NUM_25;
constexpr gpio_num_t canRxPin = GPIO_NUM_26;


class PacketQueue {
    private:
        // static std::mutex packetQueueMutex;
        static CAN_frame_t packetQueue[sendQueueSize];
        static bool packetShortGap[sendQueueSize];
        static uint16_t packetQueueHead;
        static int16_t packetQueueTail;
        static bool doCANWrite;
        static elapsedMillis lastPacketSendTime;
        static elapsedMillis lastPacketRecvTime;
        static bool sendIndicator;
	    static bool recvIndicator;
        static elapsedMillis heartbeatTime;

        PacketQueue() = delete; // Prevent instantiation
        PacketQueue(const PacketQueue&) = delete; // Prevent copy
        PacketQueue& operator=(const PacketQueue&) = delete; // Prevent assignment


    public:
        static void adjustTimingOfPacketRecieve(void);
        static bool initPacketQueue(CAN_device_t& );
        static void processPacketQueue(void);
        static void queuePacket(CAN_frame_t* packet, bool shortGap=false);
        static bool packetReceived(CAN_device_t* CAN_cfg, CAN_frame_t*);
        static void clearLastPacketReceiveTime(void) {PacketQueue::lastPacketRecvTime=0;}
};
#endif // PACKET_QUEUE_H