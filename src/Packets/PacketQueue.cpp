#include "PacketQueue.h"
#include "Packet.h"
#include "CoachESP32.h"
#include "CoachWifi.h"
elapsedMillis LAST_SEND_TIME(lastSendTime);
elapsedMillis LAST_RCV_TIME(lastRecvTime);
elapsedMillis PacketQueue::heartbeatTime;
// std::mutex PacketQueue::packetQueueMutex;
CAN_frame_t PacketQueue::packetQueue[sendQueueSize];
bool PacketQueue::packetShortGap[sendQueueSize];
uint16_t PacketQueue::packetQueueHead;
int16_t PacketQueue::packetQueueTail;
bool PacketQueue::doCANWrite = false;
elapsedMillis PacketQueue::lastPacketSendTime = LAST_SEND_TIME;
elapsedMillis PacketQueue::lastPacketRecvTime = LAST_RCV_TIME;
bool PacketQueue::sendIndicator;
bool PacketQueue::recvIndicator;

void PacketQueue::processPacketQueue() {
    // printf("PacketQueue::processPacketQueue Start\n");
	if (PacketQueue::packetQueueHead != PacketQueue::packetQueueTail) {
        // std::lock_guard<std::mutex> lock(packetQueueMutex);
		uint16_t nextIndex = (PacketQueue::packetQueueTail + 1) % sendQueueSize;
		uint32_t interval = (PacketQueue::packetShortGap[nextIndex]) ? minSendPacketIntervalmS : sendPacketIntervalmS;
        // printf("PacketQueue::processPacketQueue: nextIndex=%d, packetQueueTail=%d, interval=%d\n", nextIndex, packetQueueTail, interval);
		if (PacketQueue::lastPacketSendTime >= interval) {
			if (PacketQueue::doCANWrite) {
				// printf("PacketQueue::processPacketQueue sending packet over CAN-Bus\n");
				int result = ESP32Can.CANWriteFrame(&packetQueue[nextIndex]);
                // printf("PacketQueue::processPacketQueue: result = %d\n.         Sent packet with MsgID=%#x\n", result, PacketQueue::packetQueue[nextIndex].MsgID);
			}
			else {
				printf("%u: ***SIMULATE*** CAN-Bus Send Packet\n", (uint32_t)millis());
			}
			PacketQueue::packetQueueTail = nextIndex;
			PacketQueue::lastPacketSendTime = 0;
		}
	}
    // printf("PacketQueue::processPacketQueue Complete\n");
    // std::lock_guard<std::mutex> unlock(packetQueueMutex);
}

void PacketQueue::queuePacket(CAN_frame_t* packet, bool shortGap) {
    if (packet != nullptr) {
        
        // std::lock_guard<std::mutex> lock(packetQueueMutex);
        uint16_t nextIndex = (PacketQueue::packetQueueHead + 1) % sendQueueSize; 
        // printf("PacketQueue::queuePacket: Queuing packet with MsgID=%#x\n", packet->MsgID);
        printf("PacketQueue::queuePacket: nextIndex=%d, packetQueueTail=%d\n", nextIndex, PacketQueue::packetQueueTail);
        while (nextIndex == PacketQueue::packetQueueTail) {	// wait if queue is full
            printf("%u: PacketQueue::queuePacket: Queue is full, waiting...\n", (uint32_t)millis());
            PacketQueue::processPacketQueue();
        }

        PacketQueue::packetQueueHead = nextIndex;
        PacketQueue::packetQueue[packetQueueHead] = *packet;
        PacketQueue::packetShortGap[packetQueueHead] = shortGap;
        printf("%u: PacketQueue::queuePacket: Added packet to queue at index %d, packetQueueHead=%d, packetQueueTail=%d\n", (uint32_t)millis(), nextIndex, packetQueueHead, packetQueueTail);
    }
    else {
        printf("PacketQueue::queuePacket %u: Error: Attempted to queue a null packet.\n", (uint32_t)millis());
    }
}

void PacketQueue::adjustTimingOfPacketRecieve(void) {
    // std::lock_guard<std::mutex> lock(Packet::packetMutex);
    PacketQueue::sendIndicator = PacketQueue::lastPacketSendTime < packetBlinkTime;
    PacketQueue::recvIndicator = PacketQueue::lastPacketRecvTime < packetBlinkTime;
    
    if (sendIndicator || recvIndicator) {
    	PacketQueue::heartbeatTime = heartbeatBlinkTime;		// adjust next heartbeat time to avoid overlap with send/recv indicators
    }

    bool hearbeatIndicator = (PacketQueue::heartbeatTime % heatbeatRate) < heartbeatBlinkTime;

	if (!CoachWifi::isConnected()) {
		hearbeatIndicator = (PacketQueue::heartbeatTime % (heatbeatRate/4)) > heartbeatBlinkTime*5;
	}
	CoachESP32::adjustRGB(!sendIndicator, !hearbeatIndicator, !recvIndicator);
    // std::lock_guard<std::mutex> unlock(Packet::packetMutex);
}   


bool PacketQueue::initPacketQueue(CAN_device_t& CAN_cfg) 
{
    printf("PacketQueue::initPacketQueue Start\n");
    // std::lock_guard<std::mutex> lock(packetQueueMutex);
    PacketQueue::lastPacketSendTime = LAST_SEND_TIME;
    printf("%u: Init CAN module\n", (uint32_t)millis());
    PacketQueue::packetQueueHead = 0;
    PacketQueue::packetQueueTail = 0;
    PacketQueue::doCANWrite = true;
    bool inited = false;
    CAN_cfg.speed = CAN_SPEED_250KBPS;
    CAN_cfg.tx_pin_id = canTxPin;
    CAN_cfg.rx_pin_id = canRxPin;
    printf("before xQueueCreate with canTxPin %d, canRxPin %d\n", canTxPin, canRxPin);
    CAN_cfg.rx_queue = xQueueCreate(receiveQueueSize, sizeof(CAN_frame_t));
    printf("after xQueueCreate & before ESP32.CANInit\n");
    
    if (ESP32Can.CANInit() == 0) {
        inited = true;
        printf("%u: CAN module initialized successfully\n", (uint32_t)millis());
    } else {
        printf("%u: CAN module initialization failed\n", (uint32_t)millis());
    }
    printf("PacketQueue::initPacketQueue Complete\n");
    return inited;  
}

bool PacketQueue::packetReceived(CAN_device_t* CAN_cfg, CAN_frame_t* packet) {
    // printf("PacketQueue::packetReceived Start\n");
    bool received = false;
    if ((CAN_cfg != nullptr) && (packet != nullptr)) {
    	if (xQueueReceive(CAN_cfg->rx_queue, packet, 0) == pdTRUE) {
            // printf("PacketQueue::packetReceived: Packet received\n");
		    Packet::displayPacket(packet);
		    // debugPacket(&packet);
		    Packet::processPacket(packet);
            received = true;
	    }
    }
	processPacketQueue();
    // printf("PacketQueue::packetReceived Complete\n");
    return received;
}