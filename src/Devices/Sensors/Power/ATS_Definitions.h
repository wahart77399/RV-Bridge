#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef ATS_DEFINITIONS_H
#define ATS_DEFINITIONS_H
typedef enum {
    ATS_INSTANCE_0_INVALID = 0x00,
    ATS_INSTANCE_1          = 0x01,  // 0000 0001
    ATS_INSTANCE_2          = 0x02,  // 0000 0010
    ATS_INSTANCE_3          = 0x03,  // 0000 0011
    ATS_INSTANCE_4          = 0x04,  // 0000 0100
    ATS_INSTANCE_5          = 0x05,  // 0000 0101
    ATS_INSTANCE_6          = 0x06,  // 0000 0110
    ATS_INSTANCE_7_INVALID  = 0x07,  // 0000 0111

    ATS_INPUT               = 0x00,
    ATS_OUTPUT              = 0x08,  // 0000 1000

    ATS_SOURCE_PRIMARY      = 0x00,
    ATS_SOURCE_SECONDARY    = 0x10,  // 0001 0000
    ATS_SOURCE_NO_DATA      = 0x70,  // 0111 0000

    ATS_LEG_1               = 0x00,
    ATS_LEG_2               = 0x80   // 1000 0000
} ATS_STATUS_BYTE_0_DEFINITION;

typedef enum {
    ATS_INPUT_TYPE = ATS_INPUT,
    ATS_OUTPUT_TYPE = ATS_OUTPUT
} ATS_IO_TYPE;

typedef enum {
    ATS_SOURCE_PRIMARY_TYPE = ATS_SOURCE_PRIMARY,
    ATS_SOURCE_SECONDARY_TYPE = ATS_SOURCE_SECONDARY,
    ATS_SOURCE_NO_DATA_TYPE = ATS_SOURCE_NO_DATA 
} ATS_SOURCE_TYPE;

typedef enum {
    ATS_LEG_1_TYPE = 0,
    ATS_LEG_2_TYPE = 1
} ATS_LEG_TYPE;
#endif
#endif