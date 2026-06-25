#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef TANKS_H
#define TANKS_H // once I'm ready to define this, move this below ifndef


#include "Arduino.h"
#include "GenericDevice.h"
#include "TanksDefinition.h"



// TANKS is only (for now) a sensor, not a controller where we can start/stop it
// Initially, we are only going to monitor line output of TANKS, but we may add status of the TANKS in general at a later point
class Tanks : public GenericDevice {
    public: 

        static const uint8_t TANKS_INDEX; //                  = 0; 
        static const uint8_t TANKS_RELATIVE_LEVEL; //         = 1;  
        static const uint8_t TANKS_RESOLUTION; //             = 2;  
        static const uint8_t TANKS_ABSOLUTE_LEVEL_MSB; //     = 3;
        static const uint8_t TANKS_ABSOLUTE_LEVEL_LSB; //     = 4;
        static const uint8_t TANKS_SIZE_MSB; //               = 5;
        static const uint8_t TANKS_SIZE_LSB; //               = 6; 
        static const uint8_t FRESH_WATER_INSTANCE; //         = 0;
        static const uint8_t BLACK_WATER_INSTANCE; //         = 1;
        static const uint8_t GRAY_WATER_INSTANCE; //          = 2;
        static const uint8_t LPG_INSTANCE; //                 = 3;
        static const uint8_t FRESH_WATER_SECOND_INSTANCE; //  = 16;
        static const uint8_t BLACK_WATER_SECOND_INSTANCE; //  = 17;
        static const uint8_t GRAY_WATER_SECOND_INSTANCE; //   = 18;
        static const uint8_t LPG_SECOND_INSTANCE; //          = 19;

    private:
        friend class TanksView;

        uint16_t size(void) const { 
            uint16_t result = INVALID_SIZE;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint16_t tankSize = getLilEndian(rawData[TANKS_SIZE_MSB], rawData[TANKS_SIZE_LSB]);
                if (tankSize < INVALID_TANK_SIZE)
                    result = tankSize;
                else 
                    result = NEWMAR_ESSEX_2022_FRESH;
            }
            return result;
        }

        uint16_t level(void) const {
            uint16_t result = INVALID_TANK_SIZE;
            uint8_t* rawData = getCurrentData();
            if (rawData != nullptr) {
                uint16_t tankLevel = getLilEndian(rawData[TANKS_ABSOLUTE_LEVEL_MSB], rawData[TANKS_ABSOLUTE_LEVEL_LSB]);
                if (tankLevel < INVALID_TANK_SIZE)
                    result = tankLevel;
                }
            return result;
        }


    protected:
        
        // virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the TANKS - we listen only
        virtual void setData(RVC_DGN dgn, uint8_t* sourceData) {
                    uint8_t* destinationData = getCurrentData();
                    if ((sourceData != nullptr) && (destinationData != nullptr)) {
                        for (int i = 0; i < 8; i++) {
                            destinationData[i] = sourceData[i];
                        }
                    }
                }
        
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the TANKS - we listen only


    public:

        Tanks() : GenericDevice() {
            // Constructor implementation
        }

        Tanks(const Tanks& orig) : GenericDevice(orig) {
            // Copy constructor implementation
        }

        Tanks(uint8_t address, uint8_t index) : GenericDevice(address, index) { 
        }

        Tanks(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
        }

        virtual ~Tanks() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // this is only to deal with TANKS dgns

};


#endif // TANKS_H
#endif // ifdef HOME_KIT_2