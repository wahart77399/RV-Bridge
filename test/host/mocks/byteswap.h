#pragma once

#include <stdint.h>

static inline uint16_t __bswap16(uint16_t value) {
    return static_cast<uint16_t>((value >> 8) | ((value & 0xFFu) << 8));
}

static inline uint32_t __bswap32(uint32_t value) {
    return ((value & 0x000000FFu) << 24) |
           ((value & 0x0000FF00u) << 8) |
           ((value & 0x00FF0000u) >> 8) |
           ((value & 0xFF000000u) >> 24);
}

static inline uint64_t __bswap64(uint64_t value) {
    return ((value & 0x00000000000000FFULL) << 56) |
           ((value & 0x000000000000FF00ULL) << 40) |
           ((value & 0x0000000000FF0000ULL) << 24) |
           ((value & 0x00000000FF000000ULL) << 8) |
           ((value & 0x000000FF00000000ULL) >> 8) |
           ((value & 0x0000FF0000000000ULL) >> 24) |
           ((value & 0x00FF000000000000ULL) >> 40) |
           ((value & 0xFF00000000000000ULL) >> 56);
}

#ifndef bswap_16
#define bswap_16(x) __bswap16(x)
#endif

#ifndef bswap_32
#define bswap_32(x) __bswap32(x)
#endif

#ifndef bswap_64
#define bswap_64(x) __bswap64(x)
#endif
