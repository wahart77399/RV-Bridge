#pragma once
#include <cstdint>
inline uint32_t esp_random() {
    static uint32_t value = 0x12340000;
    return ++value;
}