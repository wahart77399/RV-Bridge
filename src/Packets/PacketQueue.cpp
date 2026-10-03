
#include "PacketQueue.h"
#include "Packet.h"

// -----------------------------------------------------------------------------
// Static storage (attributes live here; access only via private mediators)
// -----------------------------------------------------------------------------
namespace {
    bool gDriverInstalled = false;
    bool gDriverStarted   = false;
    uint8_t gSendHead     = 0;
    uint8_t gSendTail     = 0;
    uint8_t gSendCount    = 0;
    CAN_frame_t gSendQueue[kSendQueueSize];
    elapsedMillis gTimeSinceLastSend = 0;
    elapsedMillis gTimeSinceLastRecv = 0;
    elapsedMillis gTimeSinceAdjust   = 0;
}

// -----------------------------------------------------------------------------
// Private attribute mediators
// -----------------------------------------------------------------------------
bool& PacketQueue::driverInstalled(void) { return gDriverInstalled; }
bool& PacketQueue::driverStarted(void)   { return gDriverStarted; }
uint8_t& PacketQueue::sendHead(void)     { return gSendHead; }
uint8_t& PacketQueue::sendTail(void)     { return gSendTail; }
uint8_t& PacketQueue::sendCount(void)    { return gSendCount; }
CAN_frame_t* PacketQueue::sendQueue(void) { return gSendQueue; }
elapsedMillis& PacketQueue::timeSinceLastSend(void) { return gTimeSinceLastSend; }
elapsedMillis& PacketQueue::timeSinceLastRecv(void) { return gTimeSinceLastRecv; }
elapsedMillis& PacketQueue::timeSinceAdjust(void)   { return gTimeSinceAdjust; }

void PacketQueue::clearLastPacketReceiveTime(void) {
    timeSinceLastRecv() = 0;
}

// -----------------------------------------------------------------------------
// Protected: TWAI ↔ CAN_frame_t
// -----------------------------------------------------------------------------
void PacketQueue::canFrameFromTwai(const twai_message_t& in, CAN_frame_t& out) {
    out.MsgID = in.identifier;
    out.FIR.U = 0;
    out.FIR.B.DLC = in.data_length_code & 0x0F;
    out.FIR.B.FF  = in.extd ? CAN_frame_ext : CAN_frame_std;
    out.FIR.B.RTR = in.rtr ? CAN_RTR : CAN_no_RTR;

    for (uint8_t i = 0; i < 8; ++i) {
        out.data.u8[i] = (i < in.data_length_code) ? in.data[i] : 0;
    }
}

void PacketQueue::twaiFromCanFrame(const CAN_frame_t& in, twai_message_t& out) {
    out.identifier = in.MsgID;
    out.extd = (in.FIR.B.FF == CAN_frame_ext) ? 1 : 0;
    out.rtr  = (in.FIR.B.RTR == CAN_RTR) ? 1 : 0;
    out.ss = 0;
    out.self = 0;
    out.dlc_non_comp = 0;
    out.data_length_code = in.FIR.B.DLC & 0x0F;

    for (uint8_t i = 0; i < 8; ++i) {
        out.data[i] = (i < out.data_length_code) ? in.data.u8[i] : 0;
    }
}

bool PacketQueue::installTwaiDriver(gpio_num_t txPin,
                                    gpio_num_t rxPin,
                                    const twai_timing_config_t& timing) {
    bool ok = false;
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err = ESP_FAIL;

    general.rx_queue_len = kReceiveQueueSize;
    general.tx_queue_len = kSendQueueSize;

    do {
        if (driverInstalled()) {
            ok = true;
            break;
        }

        err = twai_driver_install(&general, &timing, &filter);
        if (err != ESP_OK) {
            printf("PacketQueue::installTwaiDriver install failed err=%d\n", (int)err);
            break;
        }

        driverInstalled() = true;
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::startTwaiDriver(void) {
    bool ok = false;
    esp_err_t err = ESP_FAIL;

    do {
        if (!driverInstalled()) {
            break;
        }
        if (driverStarted()) {
            ok = true;
            break;
        }

        err = twai_start();
        if (err != ESP_OK) {
            printf("PacketQueue::startTwaiDriver start failed err=%d\n", (int)err);
            break;
        }

        driverStarted() = true;
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::stopTwaiDriver(void) {
    bool ok = false;
    esp_err_t err = ESP_FAIL;

    do {
        if (!driverStarted()) {
            ok = true;
            break;
        }

        err = twai_stop();
        if (err != ESP_OK) {
            break;
        }

        driverStarted() = false;
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::receiveFrame(CAN_frame_t& outPacket) {
    bool ok = false;
    twai_message_t msg;
    esp_err_t err = ESP_FAIL;

    do {
        if (!driverStarted()) {
            break;
        }

        err = twai_receive(&msg, pdMS_TO_TICKS(kTwaiRxTimeoutMs));
        if (err != ESP_OK) {
            break;
        }

        canFrameFromTwai(msg, outPacket);
        timeSinceLastRecv() = 0;
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::transmitFrame(const CAN_frame_t& frame) {
    bool ok = false;
    twai_message_t msg;
    esp_err_t err = ESP_FAIL;

    do {
        if (!driverStarted()) {
            break;
        }

        twaiFromCanFrame(frame, msg);
        err = twai_transmit(&msg, pdMS_TO_TICKS(kTwaiTxTimeoutMs));
        if (err != ESP_OK) {
            break;
        }

        timeSinceLastSend() = 0;
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::pushSendQueue(const CAN_frame_t& frame) {
    bool ok = false;

    do {
        if (sendCount() >= kSendQueueSize) {
            break;
        }

        sendQueue()[sendHead()] = frame;
        sendHead() = static_cast<uint8_t>((sendHead() + 1) % kSendQueueSize);
        sendCount() = static_cast<uint8_t>(sendCount() + 1);
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::popSendQueue(CAN_frame_t& outFrame) {
    bool ok = false;

    do {
        if (sendCount() == 0) {
            break;
        }

        outFrame = sendQueue()[sendTail()];
        sendTail() = static_cast<uint8_t>((sendTail() + 1) % kSendQueueSize);
        sendCount() = static_cast<uint8_t>(sendCount() - 1);
        ok = true;
    } while (false);

    return ok;
}

// -----------------------------------------------------------------------------
// Public behaviors
// -----------------------------------------------------------------------------
bool PacketQueue::initialize(gpio_num_t txPin,
                             gpio_num_t rxPin,
                             twai_timing_config_t timing) {
    bool ok = false;

    do {
        if (!installTwaiDriver(txPin, rxPin, timing)) {
            break;
        }
        if (!startTwaiDriver()) {
            break;
        }

        sendHead() = 0;
        sendTail() = 0;
        sendCount() = 0;
        timeSinceLastSend() = 0;
        timeSinceLastRecv() = 0;
        timeSinceAdjust() = 0;
        ok = true;
    } while (false);

    return ok;
}

bool PacketQueue::initPacketQueue(CAN_device_t& cfg) {
    bool ok = false;
    gpio_num_t tx = kCanTxPin;
    gpio_num_t rx = kCanRxPin;

    do {
        // Prefer pins from legacy CAN_device_t when set
        if (cfg.tx_pin_id != (gpio_num_t)0) {
            tx = cfg.tx_pin_id;
        }
        if (cfg.rx_pin_id != (gpio_num_t)0) {
            rx = cfg.rx_pin_id;
        }

        // RV-C is 250 kbit/s
        ok = initialize(tx, rx, TWAI_TIMING_CONFIG_250KBITS());
    } while (false);

    return ok;
}

bool PacketQueue::packetReceived(CAN_device_t* cfg, CAN_frame_t* outPacket) {
    bool received = false;
    CAN_frame_t frame;

    do {
        if (outPacket == nullptr) {
            break;
        }
        // cfg retained for call-site compatibility; TWAI owns the bus
        (void)cfg;

        if (!receiveFrame(frame)) {
            break;
        }

        *outPacket = frame;
        Packet::processPacket(&frame);
        received = true;
    } while (false);

    processPacketQueue();
    return received;
}

bool PacketQueue::queuePacket(const CAN_frame_t& frame) {
    bool ok = false;

    do {
        ok = pushSendQueue(frame);
    } while (false);

    return ok;
}

void PacketQueue::processPacketQueue(void) {
    CAN_frame_t frame;
    bool haveFrame = false;

    do {
        if (sendCount() == 0) {
            break;
        }
        if (timeSinceLastSend() < kMinSendPacketIntervalMs) {
            break;
        }

        haveFrame = popSendQueue(frame);
        if (!haveFrame) {
            break;
        }

        if (!transmitFrame(frame)) {
            // Best-effort: drop on TX failure to avoid stalling queue
            printf("PacketQueue::processPacketQueue TX failed\n");
            break;
        }
    } while (false);
}

void PacketQueue::adjustTimingOfPacketRecieve(void) {
    // Placeholder for legacy LED/heartbeat timing hooks; single exit
    do {
        if (timeSinceAdjust() < kPacketBlinkTimeMs) {
            break;
        }
        timeSinceAdjust() = 0;
        // Optional: activity LED based on timeSinceLastRecv() / timeSinceLastSend()
    } while (false);
}
