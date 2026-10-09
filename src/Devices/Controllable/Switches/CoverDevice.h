/*
#include "RVConstants.h"
#include "debug.h"
#ifdef HOME_KIT_2
#pragma once


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
 ********************************************************************************
 
////////////////////////////////////////////////////////////////
//                                                            //
//    RV-Bridge: A HomeKit to RV-C interface for the ESP32    //
//                                                            //
////////////////////////////////////////////////////////////////
#include "GenericDevice.h"
#include "CoverDeviceDefinitions.h"


class CoverView;

class CoverDevice : public GenericDevice {
    private:

        uint8_t*        commandData;
	    CoverKind	    kind_;
        CoverMotion     motion_; 
        boolean         extended_; // is the awning extended or retracted
	    float   	    extendSec_;
	    float   	    retractSec_;

	    // set/get methods
	    CoverKind	kind(void) const { return kind_; }
	    CoverMotion	motion(void) const { return motion_; }
	    void		motion(CoverMotion m) { motion_ = m; }
	    float   	extendSec(void) { return extendSec_; }
	    void		extendSec(float v) { extendSec_ = v; }
	    float   	retractSec(void) { return retractSec_; }
	    void		retractSec(float v) { retractSec_ = v; }

        // shades extended means closed, and retracted means opened
        // while awngings are just the opposite
        boolean     isClosed() {return !extended_; }
        void        closed(boolean closed) { extended_ = !closed; }
        void        extended(boolean ext) { extended_ = ext; }

        // void movingCoverDevice(AWNING_MOTION mooving) { motion = mooving; }
        // const boolean isCoverDeviceMoving(void) { return motion != NO_MOTION; }

        uint8_t* getCommandData(void) const { return commandData; }


        friend class CoverView;

        // status information

        void clearExtendedAmount(void) { 
            uint8_t* data = getCurrentData();
            if (data != nullptr)
                data[AWNING_STATUS_POSITION_INDEX] = AWNING_MIN_PERCENT;
        }
        
    protected:
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);

        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                uint8_t* cData = getCommandData();
                if (kind() == CoverKind::Awning) {
                    if ((rawData != nullptr) && (cData != nullptr))// set the current data to the new data
                    switch (dgn) {
                        case AWNING_COMMAND_2:
                            RV_PRINTF("CoverDevice::setData: ********WARNING ****** AWNING_COMMAND_2 received\n");
                            break;
                        case AWNING_COMMAND:
                            // RV_PRINTF("CoverDevice::setData: AWNING_COMMAND received\n");
                            for (uint8_t i = 1; i < 8; i++) {
                                cData[i] = data[i]; // copy the command data
                            }
                            break;
                        case AWNING_STATUS:
                        // RV_PRINTF("CoverDevice::setData: AWNING_STATUS received\n");
                            for (uint8_t i = 1; i < 8; i++) {
                                rawData[i] = data[i]; // copy the status data
                            }
                            break;
                        case AWNING_STATUS_2:
                            RV_PRINTF("CoverDevice::setData: AWNING_STATUS_2 ************ WARNING *************** received\n");
                        default:
                            // do nothing
                            break;
                    }
                } else {
                    if ((rawData != nullptr) && (cData != nullptr)) {// set the current data to the new data
                        switch (dgn) {
                            case WINDOW_SHADE_CONTROL_COMMAND:
                                for (uint8_t i = 1; i < 8; i++) {
                                    cData[i] = data[i]; // copy the command data
                                }
                                break;
                            case WINDOW_SHADE_CONTROL_STATUS:
                                for (uint8_t i = 1; i < 8; i++) {
                                    rawData[i] = data[i]; // copy the status data
                                }
                                break;
                            default:
                                // do nothing
                                break;
                        } 
                    }
                }
            }
        }

    public:

        // command 
        void  extend(uint8_t percent = AWNING_MAX_PERCENT) {
            uint8_t* rawData = getCommandData();
            if (rawData != nullptr) {
                extended(true);
                if (kind() == CoverKind::Awning) {
                    rawData[AWNING_COMMAND_DIRECTION_INDEX] =AWNING_EXTEND_COMMAND ;
                    motion(static_cast<CoverMotion>(EXTENDING));
                    extended(true);
                    if ((percent >= (AWNING_MIN_PERCENT + AWNING_PCT_FUDGE_FACTOR)) && (percent <= (AWNING_MAX_PERCENT - AWNING_PCT_FUDGE_FACTOR))) {
                        rawData[AWNING_COMMAND_POSITION_INDEX] = static_cast<uint8_t>(round((static_cast<float>(percent))/ AWNING_PERCENT_PRECISION)); // set the extend percentage

                        // RV_PRINTF("CoverDevice::extend - percent good %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                    } else if (percent < AWNING_MIN_PERCENT) {
                        rawData[AWNING_COMMAND_POSITION_INDEX] = AWNING_MIN_PERCENT;
                        // RV_PRINTF("CoverDevice::extend - percent min %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);

                    } else {
                        rawData[AWNING_COMMAND_POSITION_INDEX] = static_cast<uint8_t>(round(static_cast<float>(AWNING_MAX_PERCENT)/AWNING_PERCENT_PRECISION));
                        // RV_PRINTF("CoverDevice::extend - percent calc %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                        // updateViews(); // update all views
                    }
                    executeCommand(AWNING_COMMAND, rawData);
                } else {
                    closed(true);
                    rawData[SHADE_COMMAND_INDEX] = SHADE_COMMAND_TOGGLE_REVERSE; // set the command to close
                    motion(static_cast<CoverMotion>(CLOSING));
                    executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
                }
            }
        }

	// command
        void  stop(void) {
            uint8_t* rawData = getCommandData();
            if (rawData != nullptr) {
                if (kind() == CoverKind::Awning) {
                    rawData[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_STOP_COMMAND;
                    // RV_PRINTF("Awning::stop %d", rawData[AWNING_COMMAND_DIRECTION_INDEX] );
                    // updateViews(); // update all views
                    executeCommand(AWNING_COMMAND, rawData);
                    motion(static_cast<CoverMotion>(NO_MOTION));
                } else {
                    SHADES_COMMANDS cmd = SHADE_COMMAND_TOGGLE_REVERSE;
                    if ((!isClosed()) || (motion() == static_cast<CoverMotion>(OPENING))) {
                        cmd = SHADE_COMMAND_TOGGLE_FORWARD;
                    } 
                    rawData[SHADE_COMMAND_INDEX] = cmd; // set the command to stop
                    motion(static_cast<CoverMotion>(STOPPED));
                    executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
                }
            }
        }

        // command
        // retracting 100% is setting it to 0%
        void  retract(uint8_t percent = 0) {
            uint8_t* rawData = getCommandData();
            if (rawData != nullptr) {
                if (kind() == CoverKind::Awning) {
                    rawData[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_RETRACT_COMMAND;
                    motion(static_cast<CoverMotion>(RETRACTING));
                    if ((percent >= (AWNING_MIN_PERCENT + AWNING_PCT_FUDGE_FACTOR)) && (percent <= (AWNING_MAX_PERCENT - AWNING_PCT_FUDGE_FACTOR))) {
                        rawData[AWNING_COMMAND_POSITION_INDEX] = static_cast<uint8_t>(round((static_cast<float>(percent)) / AWNING_PERCENT_PRECISION)); // set the retract percentage
                        // RV_PRINTF("Awning::retract - percent good %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                    } else if (percent < (AWNING_MIN_PERCENT + AWNING_PCT_FUDGE_FACTOR)) {
                        rawData[AWNING_COMMAND_POSITION_INDEX] = AWNING_MIN_PERCENT;
                        extended(false);
                    // RV_PRINTF("Awning::retract - percent max %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                    } else {
                        extended(false);
                        rawData[AWNING_COMMAND_POSITION_INDEX] = static_cast<uint8_t>(round(static_cast<float>(AWNING_MIN_PERCENT)/AWNING_PERCENT_PRECISION));
                        // RV_PRINTF("Awning::retract - percent calc %d\n", rawData[AWNING_COMMAND_POSITION_INDEX]);
                    // updateViews(); // update all views
                    }
                    executeCommand(AWNING_COMMAND, rawData);
                } else {
                    closed(false);
                    rawData[SHADE_COMMAND_INDEX] = SHADE_COMMAND_TOGGLE_FORWARD; // set the command to reverse
                    motion(static_cast<CoverMotion>(OPENING));
                    executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
                }
            }
        }

	    // void moveTo(uint8_t percent);

        boolean isExtended(void) const { return extended_; }
        void configureTimings(uint16_t extend, uint16_t retract);

	    // language behaviors

	    CoverDevice() = delete;

        CoverDevice(const CoverDevice& orig) = delete;

        CoverDevice(uint8_t address, uint8_t instance, CoverKind k) 
            : GenericDevice(address, instance), 
              kind_(k),
              motion_(CoverMotion::Stopped), 
              extended_(false),
              extendSec_(DEFAULT_EXTEND_TIME_SEC),
              retractSec_(DEFAULT_RETRACT_TIME_SEC) {     
            // Constructor with parameters implementation
                        // availableDGNs = dgns;
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
            clearExtendedAmount();
        }

	    CoverDevice(CoverDevice&& ) = delete;
	    CoverDevice& operator=(CoverDevice&& ) = delete;

        ~CoverDevice() = default;
                 
        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* sendData = nullptr, uint8_t sAddress = SOURCE_ADDRESS); // execute command based on DGN and data received from the controller  

};

#endif
*/
#include "RVConstants.h"
#include "debug.h"
#pragma once

#include "GenericDevice.h"
#include "CoverDeviceDefinitions.h"

class CoverView;

class CoverDevice : public GenericDevice {
private:
    uint8_t*     commandData_;
    CoverKind    kind_;
    CoverMotion  motion_;
    bool         extended_;          // true = physically out / closed (shade)
    float        extendSec_;
    float        retractSec_;
    bool         awningStatusPending_ = false;

    // Private mediation
    CoverKind    kind() const               { return kind_; }
    CoverMotion  motion() const             { return motion_; }
    void         motion(CoverMotion m)      { motion_ = m; }
    float        extendSec() const          { return extendSec_; }
    void         extendSec(float v)         { extendSec_ = v; }
    float        retractSec() const         { return retractSec_; }
    void         retractSec(float v)        { retractSec_ = v; }
    bool         isClosed() const           { return kind_ == CoverKind::Shade ? extended_ : !extended_; }
    void         closed(bool c)             { extended_ = c; }
    void         extended(bool e)           { extended_ = e; }
    uint8_t*     getCommandData() const     { return commandData_; }

    void clearExtendedAmount() {
        uint8_t* data = getCurrentData();
        if (data) data[AWNING_STATUS_POSITION_INDEX] = AWNING_MIN_PERCENT;
    }

    friend class CoverView;

protected:
    virtual CAN_frame_t* buildCommand(RVC_DGN dgn) override;

    virtual void setData(RVC_DGN dgn, uint8_t* data) override;

public:
    // Commands (public intent)
    void extend(uint8_t percent = AWNING_MAX_PERCENT);
    void retract(uint8_t percent = 0);
    void stop();

    bool isExtended() const { return extended_; }
    void configureTimings(uint16_t extendMs, uint16_t retractMs);

    // Rule of Five
    CoverDevice() = delete;
    CoverDevice(const CoverDevice&) = delete;
    CoverDevice& operator=(const CoverDevice&) = delete;
    CoverDevice(CoverDevice&&) = delete;
    CoverDevice& operator=(CoverDevice&&) = delete;

    CoverDevice(uint8_t address, uint8_t instance, CoverKind k);
    ~CoverDevice();

    virtual boolean executeCommand(RVC_DGN dgn,
                                   const uint8_t* sendData = nullptr,
                                   uint8_t sAddress = SOURCE_ADDRESS) override;
};
