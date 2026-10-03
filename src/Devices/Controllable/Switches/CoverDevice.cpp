#include "RVConstants.h"
#include "Arduino.h"
#include "CoverDevice.h"
#include "CanFrameTypes.h"
#include "Packet.h"
#include "DGN.h"
#include "PacketQueue.h"
#include "debug.h"

CoverDevice::CoverDevice(uint8_t address, uint8_t instance, CoverKind k)
    : GenericDevice(address, instance)
    , kind_(k)
    , motion_(CoverMotion::Stopped)
    , extended_(false)
    , extendSec_(DEFAULT_EXTEND_TIME_SEC)
    , retractSec_(DEFAULT_RETRACT_TIME_SEC)
{
    commandData_ = new uint8_t[8];
    commandData_[0] = index();
    for (uint8_t i = 1; i < 8; ++i) commandData_[i] = INVALID_DATA;
    clearExtendedAmount();
}

CoverDevice::~CoverDevice() {
    delete[] commandData_;
    commandData_ = nullptr;
}

void CoverDevice::configureTimings(uint16_t extendMs, uint16_t retractMs) {
    extendSec(static_cast<float>(extendMs));
    retractSec(static_cast<float>(retractMs));
}

void CoverDevice::extend(uint8_t percent) {
    uint8_t* cmd = getCommandData();
    if (!cmd) return;

    if (kind() == CoverKind::Awning) {
        extended(true);
        motion(CoverMotion::Closing);               // extending
        cmd[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_EXTEND_COMMAND;

        if (percent >= (AWNING_MIN_PERCENT + COVER_PCT_FUDGE) &&
            percent <= (AWNING_MAX_PERCENT - COVER_PCT_FUDGE)) {
            cmd[AWNING_COMMAND_POSITION_INDEX] =
                static_cast<uint8_t>(round(percent / COVER_PERCENT_PRECISION));
        } else if (percent < AWNING_MIN_PERCENT) {
            cmd[AWNING_COMMAND_POSITION_INDEX] = AWNING_MIN_PERCENT;
        } else {
            cmd[AWNING_COMMAND_POSITION_INDEX] =
                static_cast<uint8_t>(round(AWNING_MAX_PERCENT / COVER_PERCENT_PRECISION));
        }
        executeCommand(AWNING_COMMAND, cmd);
    } else { // Shade – close
        /*
	closed(true);
        motion(CoverMotion::Closing);

        // Build a complete shade command frame
        cmd[SHADE_INSTANCE_INDEX]   = index();
        cmd[SHADE_GROUP_INDEX]      = SHADE_GROUP_NONE;          // or the correct group if you use one
        cmd[SHADE_MOTOR_DUTY_INDEX] = 0x64;                      // 100 % duty (adjust if needed)
        cmd[SHADE_COMMAND_INDEX]    = SHADE_COMMAND_TOGGLE_REVERSE;
        cmd[SHADE_DURATION_INDEX]   = 0x00;
        cmd[SHADE_INTERLOCK_INDEX]  = 0x00;
        */
        printf("CoverDevice::extend() - Shade close command\n");
        closed(true);
        motion(CoverMotion::Closing);

        // Only the command byte is written – everything else stays as initialized
        // (instance already in cmd[0], rest INVALID_DATA). This is what worked before.
        cmd[SHADE_COMMAND_INDEX] = SHADE_COMMAND_TOGGLE_REVERSE;
        executeCommand(WINDOW_SHADE_CONTROL_COMMAND, cmd);
    }
}

void CoverDevice::retract(uint8_t percent) {
    uint8_t* cmd = getCommandData();
    if (!cmd) return;

    if (kind() == CoverKind::Awning) {
        motion(CoverMotion::Opening);               // retracting
        cmd[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_RETRACT_COMMAND;

        if (percent >= (AWNING_MIN_PERCENT + COVER_PCT_FUDGE) &&
            percent <= (AWNING_MAX_PERCENT - COVER_PCT_FUDGE)) {
            cmd[AWNING_COMMAND_POSITION_INDEX] =
                static_cast<uint8_t>(round(percent / COVER_PERCENT_PRECISION));
        } else {
            extended(false);
            cmd[AWNING_COMMAND_POSITION_INDEX] = AWNING_MIN_PERCENT;
        }
        executeCommand(AWNING_COMMAND, cmd);
    } else { // Shade – open
        printf("CoverDevice::retract() - Shade open command\n");
	    closed(false);

        motion(CoverMotion::Opening);

        // Only the command byte is written – matches the original working code
        cmd[SHADE_COMMAND_INDEX] = SHADE_COMMAND_TOGGLE_FORWARD;
	/*
	closed(false);
        motion(CoverMotion::Opening);

        cmd[SHADE_INSTANCE_INDEX]   = index();
        cmd[SHADE_GROUP_INDEX]      = SHADE_GROUP_NONE;
        cmd[SHADE_MOTOR_DUTY_INDEX] = 0x64;
        cmd[SHADE_COMMAND_INDEX]    = SHADE_COMMAND_TOGGLE_FORWARD;
        cmd[SHADE_DURATION_INDEX]   = 0x00;
        cmd[SHADE_INTERLOCK_INDEX]  = 0x00;
	*/
        executeCommand(WINDOW_SHADE_CONTROL_COMMAND, cmd);
    }
}

void CoverDevice::stop() {
    uint8_t* cmd = getCommandData();
    if (!cmd) return;

    if (kind() == CoverKind::Awning) {
        cmd[AWNING_COMMAND_DIRECTION_INDEX] = AWNING_STOP_COMMAND;
        motion(CoverMotion::Stopped);
        executeCommand(AWNING_COMMAND, cmd);
    } else {
	/*
	SHADES_COMMANDS c = SHADE_COMMAND_TOGGLE_REVERSE;
        if (!isClosed() || motion() == CoverMotion::Opening)
            c = SHADE_COMMAND_TOGGLE_FORWARD;

        cmd[SHADE_INSTANCE_INDEX]   = index();
        cmd[SHADE_GROUP_INDEX]      = SHADE_GROUP_NONE;
        cmd[SHADE_MOTOR_DUTY_INDEX] = 0x00;          // stop usually uses 0 duty
        cmd[SHADE_COMMAND_INDEX]    = c;
        cmd[SHADE_DURATION_INDEX]   = 0x00;
        cmd[SHADE_INTERLOCK_INDEX]  = 0x00;

        motion(CoverMotion::Stopped);
	*/
        printf("CoverDevice::stop() - Shade stop command\n");
	    SHADES_COMMANDS c = SHADE_COMMAND_TOGGLE_REVERSE;
        if (!isClosed() || motion() == CoverMotion::Opening) {
            c = SHADE_COMMAND_TOGGLE_FORWARD;
        }

        // Only the command byte is written
        cmd[SHADE_COMMAND_INDEX] = c;
        motion(CoverMotion::Stopped);
        executeCommand(WINDOW_SHADE_CONTROL_COMMAND, cmd);
    }
}

CAN_frame_t* CoverDevice::buildCommand(RVC_DGN dgn) {
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn);
    if (!frame) return nullptr;

    uint8_t* d = frame->data.u8;
    const uint8_t* cData = getCommandData();

    if (kind() == CoverKind::Awning) {
        switch (dgn) {
            case AWNING_COMMAND:
                if (d && cData) {
                    for (int i = 1; i < 8; ++i) d[i] = cData[i];
                }
                break;
            case AWNING_STATUS:
                setData(dgn, getCurrentData());
                break;
            default:
                break;
        }
    } else { // Shade – CRITICAL FIX: use command buffer
        switch (dgn) {
            case WINDOW_SHADE_CONTROL_COMMAND:
                if (d && cData) {
                    for (int i = 0; i < 8; ++i) d[i] = cData[i];
                }
                break;
            case WINDOW_SHADE_CONTROL_STATUS:
                setData(dgn, getCurrentData());
                break;
            default:
                break;
        }
    }
    return frame;
}

void CoverDevice::setData(RVC_DGN dgn, uint8_t* data) {
    if (!data) return;
    uint8_t* raw = getCurrentData();
    uint8_t* cmd = getCommandData();
    if (!raw || !cmd) return;

    if (kind() == CoverKind::Awning) {
        switch (dgn) {
            case AWNING_COMMAND:
                for (uint8_t i = 1; i < 8; ++i) cmd[i] = data[i];
                break;
            case AWNING_STATUS:
                for (uint8_t i = 1; i < 8; ++i) raw[i] = data[i];
                break;
            default:
                break;
        }
    } else {
        switch (dgn) {
            case WINDOW_SHADE_CONTROL_COMMAND:
                for (uint8_t i = 1; i < 8; ++i) cmd[i] = data[i];
                break;
            case WINDOW_SHADE_CONTROL_STATUS:
                for (uint8_t i = 1; i < 8; ++i) raw[i] = data[i];
                break;
            default:
                break;
        }
    }
}

boolean CoverDevice::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    boolean done = GenericDevice::executeCommand(dgn, data);
    if (done) return true;

    CAN_frame_t* frame = nullptr;
    uint8_t* raw = const_cast<uint8_t*>(data);

    if (kind() == CoverKind::Awning) {
        switch (dgn) {
            case AWNING_COMMAND:
                if (sAddress == SOURCE_ADDRESS) {
                    frame = buildCommand(dgn);
                    if (frame) PacketQueue::queuePacket(*frame);
                } else {
                    updateViews();
                }
                done = true;
                break;
            case AWNING_STATUS:
                setData(dgn, raw);
                updateViews();
                done = true;
                break;
            default:
                break;
        }
    } else {
        switch (dgn) {
            case WINDOW_SHADE_CONTROL_COMMAND:
                if (sAddress == SOURCE_ADDRESS) {
                    setData(dgn, raw);
                    frame = buildCommand(dgn);
                    if (frame) PacketQueue::queuePacket(*frame);
                } else {
                    updateViews();
                }
                done = true;
                break;
            case WINDOW_SHADE_CONTROL_STATUS:
                setData(dgn, raw);
                updateViews();
                done = true;
                break;
            default:
                break;
        }
    }
    return done;
}
