#ifndef PACKETKIT_H
#define PACKETKIT_H 

//#include "DGN.h"

class PacketKit {
    protected:
        static constexpr uint8_t SOURCE_ADDRESS_START_BIT = 7; // Start bit for source address extraction
        static constexpr uint8_t SOURCE_ADDRESS_NUM_BITS = 8; // Number of bits for source address extraction
        static constexpr int8_t PRIORITY_START_BIT = 28; // Start bit for PRIORITY extraction
        static constexpr int8_t PRIORITY_NUM_BITS = 3; // Number of bits for PRIORITY extraction

        // Helper function to extract bits from a message
        // This function extracts 'numBits' bits from 'msg' starting at 'startBit'.
        // The bits are extracted in a right-to-left manner, meaning the least significant bit is
        // at 'startBit' and the most significant bit is at 'startBit - numBits + 1'.
        // For example, if startBit is 24 and numBits is 17, it extracts bits 24 to 8 from the message.
        // The function returns the extracted bits as a 32-bit unsigned integer.
        // If startBit is less than numBits, it will shift the message to the right by the appropriate amount and apply a mask to get
        // the desired bits.
        // If startBit is greater than or equal to numBits, it will shift the message to the right by (startBit - numBits + 1) and apply a mask to get
        // the desired bits.
        // If startBit is negative, it will return 0.
        // If numBits is negative, it will return 0.
        // If startBit is greater than 31, it will return 0.
        // If numBits is greater than 32, it will return 0.
        // If startBit is less than 0, it will return 0.
        // If numBits is less than 0, it will return 0.
        // If startBit is greater than 31, it will return 0.
        // If numBits is greater than 32, it will return 0.
        // If startBit is less than 0, it will return 0.
        // If numBits is less than 0, it will return 0.
        // If startBit is greater than 31, it will return 0.
        // If numBits is greater than 32, it will return 0.
        // If startBit is less than 0, it will return 0.
        // If numBits is less than 0, it will return 0.
        static inline uint32_t getMsgBits(uint32_t msg, int8_t startBit, int8_t numBits) {
            // printf("PacketKit::getMsgBits Start: msg=%#x, startBit=%d, numBits=%d\n", msg, startBit, numBits);
	        int8_t shift = startBit - numBits + 1;
	        uint32_t mask = 0xFFFFFFFF >> (32 - numBits);
            uint32_t result = (msg >> shift) & mask;
            // printf("PacketKit::getMsgBits: shift=%d, mask=%#x\n", shift, mask);
            // printf("PacketKit::getMsgBits End: result=%#x\n", result);
	        return result;
        }

        PacketKit() {} 
        PacketKit(const PacketKit&) {} // Prevent copy
        PacketKit& operator=(const PacketKit&) {return *this; } // Prevent assignment
};
#endif // PACKETKIT_H