/*
Platformio.ini will need to be modified
; -----------------------------------------------------------------------------
; HomeSpan 2.x path (Arduino-ESP32 3 / IDF 5) — use after TWAI PacketQueue is in
; -----------------------------------------------------------------------------
[env:HomeSpan2]
build_type = release
platform = espressif32 @ 6.11.0
; Pin Arduino 3.x if the platform default is still 2.x:
; platform_packages =
;     framework-arduinoespressif32 @ 3.0.7
lib_deps =
    elapsedMillis
    homespan/HomeSpan@^2.1.8
build_flags =
    ${env.build_flags}
*/

#pragma once

#include "Arduino.h"
#include "elapsedMillis.h"
#include "CanFrameTypes.h"
#include <driver/gpio.h>
#include <driver/twai.h>

// Timing / sizing (explicit constants)
constexpr uint16_t kReceiveQueueSize       = 10;
constexpr uint16_t kSendQueueSize          = 8;
constexpr uint32_t kSendPacketIntervalMs   = 50;
constexpr uint32_t kMinSendPacketIntervalMs = 5;
constexpr uint32_t kPacketBlinkTimeMs      = 25;
constexpr uint32_t kHeartbeatRateMs        = 3000;
constexpr uint32_t kHeartbeatBlinkTimeMs   = 10;
constexpr gpio_num_t kCanTxPin             = CAN_TX; // static_cast<gpio_num_t>(11);
constexpr gpio_num_t kCanRxPin             = CAN_RX; // static_cast<gpio_num_t>(12);
constexpr uint32_t kTwaiRxTimeoutMs        = 0;   // non-blocking poll
constexpr uint32_t kTwaiTxTimeoutMs        = 10;

/**
 * PacketQueue
 * Owns TWAI install/start, RX poll → CAN_frame_t, and a software TX queue.
 * Language policy: non-copyable, non-assignable (static facade + deleted instance ops).
 */
class PacketQueue {
public:
    // --- Language behaviors (explicit) ---
    PacketQueue() = delete;
    ~PacketQueue() = delete;
    PacketQueue(const PacketQueue&) = delete;
    PacketQueue& operator=(const PacketQueue&) = delete;
    PacketQueue(PacketQueue&&) = delete;
    PacketQueue& operator=(PacketQueue&&) = delete;

    // --- Behaviors (public) ---
    static bool initialize(gpio_num_t txPin = kCanTxPin,
                           gpio_num_t rxPin = kCanRxPin,
                           twai_timing_config_t timing = TWAI_TIMING_CONFIG_250KBITS());

    /** Legacy entry used by CoachESP32 — configures pins from CAN_device_t then initialize(). */
    static bool initPacketQueue(CAN_device_t& cfg);

    /** Poll one TWAI frame into outPacket; on success runs Packet::processPacket path via caller. */
    static bool packetReceived(CAN_device_t* cfg, CAN_frame_t* outPacket);

    /** Queue a frame for rate-limited transmit. */
    static bool queuePacket(const CAN_frame_t& frame);

    /** Drain software TX queue to TWAI (call from poll loop). */
    static void processPacketQueue(void);

    /** Legacy timing helper used from main loop. */
    static void adjustTimingOfPacketRecieve(void);

    static void clearLastPacketReceiveTime(void);

protected:
    // --- Internal behaviors (class-only) ---
    static bool installTwaiDriver(gpio_num_t txPin, gpio_num_t rxPin, const twai_timing_config_t& timing);
    static bool startTwaiDriver(void);
    static bool stopTwaiDriver(void);

    static void canFrameFromTwai(const twai_message_t& in, CAN_frame_t& out);
    static void twaiFromCanFrame(const CAN_frame_t& in, twai_message_t& out);

    static bool transmitFrame(const CAN_frame_t& frame);
    static bool receiveFrame(CAN_frame_t& outPacket);

    static bool pushSendQueue(const CAN_frame_t& frame);
    static bool popSendQueue(CAN_frame_t& outFrame);

private:
    // --- Attribute mediation only ---
    static bool& driverInstalled(void);
    static bool& driverStarted(void);
    static uint8_t& sendHead(void);
    static uint8_t& sendTail(void);
    static uint8_t& sendCount(void);
    static CAN_frame_t* sendQueue(void);
    static elapsedMillis& timeSinceLastSend(void);
    static elapsedMillis& timeSinceLastRecv(void);
    static elapsedMillis& timeSinceAdjust(void);
};

