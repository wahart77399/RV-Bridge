#pragma once
// #ifndef LIGHTDEVICECMD_H
// #define LIGHTDEVICECMD_H
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
#include "RVConstants.h"
#ifdef HOME_KIT_1
/*******************
 * 
 * enum for on/off and dimmer switches to be used throughout the hierarchy of DC_Switch and potentially other switches
 *  
 */
/*
enum class LightDeviceCMD:uint8_t {
	LightDeviceCmdSetBrightness = 0,
	LightDeviceCmdOnDuration,
	LightDeviceCmdOnDelay,
	LightDeviceCmdOff,
	LightDeviceCmdStop,
	LightDeviceCmdToggle,
	LightDeviceCmdMemoryOff,
	LightDeviceCmdRampBrightness,
	LightDeviceCmdRampToggle,
	LightDeviceCmdRampUp,
	LightDeviceCmdRampDown,
	LightDeviceCmdRampUpDown,
	LightDeviceCmdLock,
	LightDeviceCmdUnlock,
	LightDeviceCmdFlash,
	LightDeviceCmdFlashMomentarily,

	LightDeviceCmdNA = 255
};
*/
constexpr uint8_t DIMMER_STATUS_3_SWITCH_OFF = 0x6;
constexpr uint8_t DIMMER_QUARTER_BRIGHTNESS = 25U;
// #endif
#endif //HOME_KIT_1
