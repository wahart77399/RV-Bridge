#pragma once
#include <cstdint>

constexpr uint8_t CAN_frame_ext = 1;
constexpr uint8_t CAN_RTR = 1;

struct CAN_frame_t {
    struct FrameInfo {
        struct FrameBits {
            uint8_t FF = 0;
            uint8_t RTR = 0;
        } B;
    } FIR;
};