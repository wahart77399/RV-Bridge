#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef ATS_DEFINITIONS_H
#define ATS_DEFINITIONS_H

// --- Raw bit masks for Byte 0 of ATS_AC_STATUS_* ---
static const uint8_t ATS_INSTANCE_MASK = 0x07;  // bits 0-2
static const uint8_t ATS_IOTYPE_MASK   = 0x08;  // bit 3
static const uint8_t ATS_SOURCE_MASK   = 0x70;  // bits 4-6
static const uint8_t ATS_LEG_MASK      = 0x80;  // bit 7

static const uint8_t ATS_IOTYPE_SHIFT  = 3;
static const uint8_t ATS_SOURCE_SHIFT  = 4;
static const uint8_t ATS_LEG_SHIFT     = 7;

// Normalized logical values (after mask + shift)
enum class ATSInstance : uint8_t {
    Invalid0 = 0,
    Inst1 = 1,
    Inst2 = 2,
    Inst3 = 3,
    Inst4 = 4,
    Inst5 = 5,
    Inst6 = 6,
    Invalid7 = 7
};


enum class ATS_IO_Type : uint8_t {
    Input  = 0,  // bit3 = 0
    Output = 1   // bit3 = 1
};


enum class ATSSource : uint8_t {
    Primary   = 0,  // bits4-6 = 000
    Secondary = 1,  // bits4-6 = 001  (0x10 >> 4)
    // 2..6 reserved / vendor
    NoData    = 7   // bits4-6 = 111  (0x70 >> 4)
};


enum class ATSLeg : uint8_t {
    Leg1 = 0,  // bit7 = 0
    Leg2 = 1   // bit7 = 1
};



// Keep old names as aliases if other files still use them
using ATS_IO_TYPE     = ATS_IO_Type;
using ATS_SOURCE_TYPE = ATSSource;
using ATS_LEG_TYPE    = ATSLeg;

static const uint8_t ATS_INSTANCE_0_INVALID = 0;
static const uint8_t ATS_INSTANCE_7_INVALID = 7;

#endif
#endif