#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef AWNING_H
#define AWNING_H // once I'm ready to define this, move this below ifndef


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
#include "GenericDevice.h"
#include "AwningDefinitions.h"

class Awning : public GenericDevice {
    private:

        uint8_t*        commandData;
        AWNING_MOTION   motion; 

        // void movingAwning(AWNING_MOTION mooving) { motion = mooving; }
        // const boolean isAwningMoving(void) { return motion != NO_MOTION; }

        uint8_t* getCommandData(void) const { return commandData; }


        friend class AwningView;

        // status information
        /*
        const boolean isExtending(void) const {
            // boolean extending = false;
            boolean extending = (motion == EXTENDING);
            return extending;
        }
        */
        // command 
        void  extend(uint8_t percent = AWNING_MAX_PERCENT) {
            uint8_t* rawData = getCommandData();
            if (rawData != nullptr) {
                rawData[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_EXTEND_COMMAND;
                motion = EXTENDING;
                if ((percent >= AWNING_MIN_PERCENT) && (percent <= AWNING_MAX_PERCENT)) {
                    rawData[AWNING_COMMAND_POSITION_INDEX] = ((float_t) percent) / AWNING_PERCENT_PRECISION; // set the extend percentage

                    // printf("Awning::extend - percent good %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                } else if (percent < AWNING_MIN_PERCENT) {
                    rawData[AWNING_COMMAND_POSITION_INDEX] = AWNING_MIN_PERCENT;
                    // printf("Awning::extend - percent min %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);

                } else {
                    rawData[AWNING_COMMAND_POSITION_INDEX] = ((float_t)AWNING_MAX_PERCENT)/AWNING_PERCENT_PRECISION;
                    // printf("Awning::extend - percent calc %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);

                // updateViews(); // update all views
                }
                executeCommand(AWNING_COMMAND, rawData);
            }
        }
        // status information
        /*
        const uint8_t extendedAmount(void) const {
            uint8_t result = 0;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[AWNING_STATUS_POSITION_INDEX] * AWNING_PERCENT_PRECISION;
                result = (tmp > AWNING_MAX_PERCENT) ? AWNING_MAX_PERCENT : tmp;
            }
            return result;
        }
        */
        void clearExtendedAmount(void) { 
            uint8_t* data = getCurrentData();
            if (data != nullptr)
                data[AWNING_STATUS_POSITION_INDEX] = AWNING_MIN_PERCENT;
        }
        

        // command
        void  stop(void) {
            uint8_t* rawData = getCommandData();
            if (rawData != nullptr) {
                rawData[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_STOP_COMMAND;
                // printf("Awning::stop %d", rawData[AWNING_COMMAND_DIRECTION_INDEX] );
                // updateViews(); // update all views
                executeCommand(AWNING_COMMAND, rawData);
                motion = NO_MOTION;
            }
        }
        // command
        // retracting 100% is setting it to 0%
        void  retract(uint8_t percent = 0) {
            uint8_t* rawData = getCommandData();
            if (rawData != nullptr) {
                rawData[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_RETRACT_COMMAND;
                motion = RETRACTING;
                if ((percent >= AWNING_MIN_PERCENT) && (percent <= AWNING_MAX_PERCENT)) {
                    rawData[AWNING_COMMAND_POSITION_INDEX] = ((float_t)(percent)) / AWNING_PERCENT_PRECISION; // set the retract percentage
                    // printf("Awning::retract - percent good %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                } else if (percent < AWNING_MIN_PERCENT) {
                    rawData[AWNING_COMMAND_POSITION_INDEX] = AWNING_MIN_PERCENT;
                    // printf("Awning::retract - percent max %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                } else {
                    rawData[AWNING_COMMAND_POSITION_INDEX] = ((float_t)AWNING_MIN_PERCENT)/AWNING_PERCENT_PRECISION;
                    // printf("Awning::retract - percent calc %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                // updateViews(); // update all views
                }
                executeCommand(AWNING_COMMAND, rawData);
            }
        }
        // status information
        /*
        const uint8_t retractedAmount(void) const {
            uint8_t result = 0;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[AWNING_STATUS_POSITION_INDEX] * AWNING_PERCENT_PRECISION;
                result = (tmp > AWNING_MAX_PERCENT) ? AWNING_MIN_PERCENT : (AWNING_MAX_PERCENT - tmp);
            }
            return result;
        }

        // status information
        const boolean isRetracting(void) const {
            boolean retracting = (motion == RETRACTING); // = false;
            return retracting;
        }
        // status information
        const boolean isStopped(void) const {
            boolean stopped = (motion == NO_MOTION); // false;
            return stopped;
        }
        // status information
        const boolean isExtended(void) const {  
            boolean extended = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                extended = (data[AWNING_STATUS_POSITION_INDEX] >= (AWNING_MIN_PERCENT));
            }
            return extended;

        }
        // status information
        const boolean isRetracted(void) const {
            boolean retracted = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                retracted = (data[AWNING_STATUS_POSITION_INDEX] <= (AWNING_MIN_PERCENT));
            }
            return retracted;
        }
        // status information
        const boolean isInMotion(void) const {
            boolean inMotion = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                inMotion = (data[AWNING_STATUS_MOTION_INDEX] != NO_MOTION);
            }
            return inMotion;
        }
        */
 

        // Add private members and methods specific to Awning here
    protected:
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                uint8_t* cData = getCommandData();
                if ((rawData != nullptr) && (cData != nullptr))// set the current data to the new data
                switch (dgn) {
                    case AWNING_COMMAND_2:
                        printf("Awning::setData: ********WARNING ****** AWNING_COMMAND_2 received\n");
                        break;
                    case AWNING_COMMAND:
                        // printf("Awning::setData: AWNING_COMMAND received\n");
                        for (uint8_t i = 1; i < 8; i++) {
                            cData[i] = data[i]; // copy the command data
                        }
                        break;
                    case AWNING_STATUS:
                        // printf("Awning::setData: AWNING_STATUS received\n");
                        for (uint8_t i = 1; i < 8; i++) {
                            rawData[i] = data[i]; // copy the status data
                        }
                        break;
                    case AWNING_STATUS_2:
                        printf("Awning::setData: AWNING_STATUS_2 ************ WARNING *************** received\n");
                    default:
                        // do nothing
                        break;
                }
            }
        }

    public:
        Awning() : GenericDevice(), motion(NO_MOTION) {
            // Constructor implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
            clearExtendedAmount();
        }

        Awning(const Awning& orig) : GenericDevice(orig), motion(NO_MOTION) {
            // Copy constructor implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            for (uint8_t i = 0; i < 8; i++) {
                commandData[i] = orig.commandData[i]; // initialize to invalid data
            }
        }

        Awning(uint8_t address, uint8_t instance) : GenericDevice(address, instance), motion(NO_MOTION) {     
            // Constructor with parameters implementation
                        // availableDGNs = dgns;
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
            clearExtendedAmount();
        }

        Awning(uint8_t* data) : GenericDevice(data), motion(NO_MOTION) {
            // Constructor with parameters implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
            clearExtendedAmount();
        }

        virtual ~Awning() {
            // Destructor implementation
            
        }
                 
        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* sendData = nullptr, uint8_t sAddress = SOURCE_ADDRESS); // execute command based on DGN and data received from the controller  

};

#endif  // ifndef AWNING_H
#endif // ifdef HOME_KIT_2