#pragma once
#include <cstdint>

using esp_err_t = int;
constexpr esp_err_t ESP_OK = 0;
constexpr esp_err_t ESP_FAIL = -1;
constexpr int TWAI_STATE_STOPPED = 0;
constexpr int TWAI_STATE_RUNNING = 1;
constexpr int TWAI_STATE_BUS_OFF = 2;

struct twai_status_info_t {
    int state = TWAI_STATE_STOPPED;
    uint32_t tx_error_counter = 0;
    uint32_t rx_error_counter = 0;
    uint32_t tx_failed_count = 0;
    uint32_t rx_missed_count = 0;
    uint32_t rx_overrun_count = 0;
    uint32_t arb_lost_count = 0;
    uint32_t bus_error_count = 0;
};

inline twai_status_info_t fakeTwaiStatus;
inline esp_err_t fakeTwaiStatusResult = ESP_OK;
inline uint32_t fakeTwaiStatusReads = 0;

inline esp_err_t twai_get_status_info(twai_status_info_t* status) {
    ++fakeTwaiStatusReads;
    *status = fakeTwaiStatus;
    return fakeTwaiStatusResult;
}