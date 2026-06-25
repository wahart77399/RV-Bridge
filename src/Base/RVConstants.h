#ifndef RV_CONSTANTS_H
#define RV_CONSTANTS_H
/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2023 Randy Ubillos
 *  
 *  https://github.com/rubillos/RV-Bridge
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
////////////////////////////////////////////////////////////////
//                                                            //
//    RV-Bridge: A HomeKit to RV-C interface for the ESP32    //
//                                                            //
////////////////////////////////////////////////////////////////
#include "Arduino.h"

#define HOME_KIT_1
// #define HOME_KIT_2
constexpr uint8_t INVALID_SIZE = 255;
constexpr uint16_t INVALID_TANK_SIZE = 0x01f4;
constexpr float_t INVALID_TEMPERATURE = -273.0F;
constexpr uint8_t OUT_OF_RANGE_DATA = 255;
constexpr uint16_t BAD_UINT16_DATA = 0xffff;
constexpr uint8_t ZERO_PERCENT_DEGREE_F = 0;
constexpr uint8_t ONE_HUNDRED_PERCENT_DEGREE_F = 100;
constexpr uint8_t ZERO_AMP_DEGREE_F = 0;
constexpr uint8_t SIXTY_AMP_DEGREE_F = 60;
constexpr uint8_t ZERO_VOLT_DEGREE_F = 0;
constexpr uint8_t ONE_TWENTY_VOLT_DEGREE_F = 120;
constexpr uint8_t TWO_FORTY_VOLT_DEGREE_F = 240;
constexpr uint8_t MAX_RVC_PERCENT = 250;
constexpr float_t RVC_PERCENT_PRECISION = 0.5;

constexpr double  tempCOffset = -273.0;
constexpr double  tempCScale = 0.03125;
constexpr double  tempCRoundingOffset = -0.25;

const float_t AAC_LOWER_LIMIT = -80.0;
const float_t AAC_UPPER_LIMIT = 80.0;

typedef enum {
    INVERTER_INPUT =  0x00,
    INVERTER_OUTPUT = 0x01
} INVERTER_IO_TYPE;

typedef enum {
    INVERTER_LINE_1_TYPE = 0x00, // line 1
    INVERTER_LINE_2_TYPE = 0x01  // line 2
} INVERTER_LINE_TYPE;


// Tank Capaciity should be in a CSV file that is read on input so recompiles are not needed for new coaches
typedef enum {
    NEWMAR_ESSEX_2022_FRESH     = 398, // liters
    NEWMAR_ESSEX_2022_GRAY      = 302,
    NEWMAR_ESSEX_2022_BLACK     = 227,
    NEWMAR_ESSEX_2022_LPG       = 0xffff 
} TANK_CAPACITIES;

#include "byteswap.h"

inline uint16_t getLilEndian(uint8_t msb, uint8_t lsb) { 
    return (lsb << 8 | msb);
    // uint16_t* origVal = (uint16_t* ) &msb; // msb and lsb are contiguous
    // uint16_t result = __bswap16(*origVal);
    // return result;
}
inline uint8_t* setLilEndian(uint8_t* vector, uint8_t msbIndex, uint8_t lsbIndex, uint16_t val ) { 
    if ((vector != nullptr)  && ((lsbIndex - msbIndex) == 1) && (val != BAD_UINT16_DATA)) {
        vector[lsbIndex] = val >> 8;
        vector[msbIndex] = val & 0xff;
    }
    return vector;
}

inline float degCfromDegF(float degF) {
	return degF / 1.8;
}

inline float tempCfromTempF(float tempF) {
	return degCfromDegF(tempF - 32.0);
}

inline double convToTempC(uint16_t value) {
	        return tempCOffset + value * tempCScale + tempCRoundingOffset;  
}

inline uint16_t convFromTempC(double tempC) {
	        return (tempC - tempCOffset + 0.5) / tempCScale;
}





#endif