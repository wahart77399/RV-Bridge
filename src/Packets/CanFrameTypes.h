#pragma once
#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Standard vs extended (29-bit) frame — RV-C uses extended. */
typedef enum {
    CAN_frame_std = 0,
    CAN_frame_ext = 1
} CAN_frame_format_t;

typedef enum {
    CAN_no_RTR = 0,
    CAN_RTR    = 1
} CAN_RTR_t;

/** Frame info (layout compatible with prior miwagner CAN_frame_t usage). */
typedef union {
    uint32_t U;
    struct {
        uint8_t DLC : 4;
        unsigned int unknown_2 : 2;
        CAN_RTR_t RTR : 1;
        CAN_frame_format_t FF : 1;
        unsigned int reserved_24 : 24;
    } B;
} CAN_FIR_t;

/** One CAN / RV-C frame. */
typedef struct {
    CAN_FIR_t FIR;
    uint32_t  MsgID;
    union {
        uint8_t  u8[8];
        uint32_t u32[2];
        uint64_t u64;
    } data;
} CAN_frame_t;

/** Optional legacy speed tags (TWAI uses its own timing config). */
typedef enum {
    CAN_SPEED_100KBPS = 100,
    CAN_SPEED_125KBPS = 125,
    CAN_SPEED_250KBPS = 250,
    CAN_SPEED_500KBPS = 500,
    CAN_SPEED_800KBPS = 800,
    CAN_SPEED_1000KBPS = 1000
} CAN_speed_t;

/**
 * Legacy device config used by main/CoachESP32.
 * rx_queue is unused under TWAI (driver owns RX); kept so call sites compile.
 */
typedef struct {
    CAN_speed_t  speed;
    gpio_num_t   tx_pin_id;
    gpio_num_t   rx_pin_id;
    QueueHandle_t rx_queue;
} CAN_device_t;


constexpr gpio_num_t CAN_TX = static_cast<gpio_num_t>(11);  // Connects to CTX
constexpr gpio_num_t CAN_RX = static_cast<gpio_num_t>(12);  // Connects to CRX

#ifdef __cplusplus
}
#endif

