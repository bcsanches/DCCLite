// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef LITE_DECODER_H
#define LITE_DECODER_H

#include <Arduino.h>

#if  defined(ARDUINO_AVR_ADK)       
    #define ARDUINO_TYPE "Mega Adk"
#elif defined(ARDUINO_AVR_BT)    // Bluetooth
    #define ARDUINO_TYPE "Bt"
#elif defined(ARDUINO_AVR_DUEMILANOVE)       
    #define ARDUINO_TYPE "Duemilanove"
#elif defined(ARDUINO_AVR_ESPLORA)       
    #define ARDUINO_TYPE "Esplora"
#elif defined(ARDUINO_AVR_ETHERNET)       
    #define ARDUINO_TYPE "Ethernet"
#elif defined(ARDUINO_AVR_FIO)       
    #define ARDUINO_TYPE "Fio"
#elif defined(ARDUINO_AVR_GEMMA)
    #define ARDUINO_TYPE "Gemma"
#elif defined(ARDUINO_AVR_LEONARDO)       
    #define ARDUINO_TYPE "Leonardo"
#elif defined(ARDUINO_AVR_LILYPAD)
    #define ARDUINO_TYPE "Lilypad"
#elif defined(ARDUINO_AVR_LILYPAD_USB)
    #define ARDUINO_TYPE "Lilypad Usb"
#elif defined(ARDUINO_AVR_MEGA)       
    #define ARDUINO_TYPE "Mega"
#elif defined(ARDUINO_AVR_MEGA2560)       
    #define ARDUINO_TYPE "Mega 2560"
#elif defined(ARDUINO_AVR_MICRO)       
    #define ARDUINO_TYPE "Micro"
#elif defined(ARDUINO_AVR_MINI)       
    #define ARDUINO_TYPE "Mini"
#elif defined(ARDUINO_AVR_NANO)       
    #define ARDUINO_TYPE "Nano"
#elif defined(ARDUINO_AVR_NG)       
    #define ARDUINO_TYPE "NG"
#elif defined(ARDUINO_AVR_PRO)       
    #define ARDUINO_TYPE "Pro"
#elif defined(ARDUINO_AVR_ROBOT_CONTROL)       
    #define ARDUINO_TYPE "Robot Ctrl"
#elif defined(ARDUINO_AVR_ROBOT_MOTOR)       
    #define ARDUINO_TYPE "Robot Motor"
#elif defined(ARDUINO_AVR_UNO)       
    #define ARDUINO_TYPE "Uno"
#elif defined(ARDUINO_AVR_YUN)       
    #define ARDUINO_TYPE "Yun"

// These ARDUINO_TYPEs must be installed separately:
#elif defined(ARDUINO_SAM_DUE)       
    #define ARDUINO_TYPE "Due"
#elif defined(ARDUINO_SAMD_ZERO)       
    #define ARDUINO_TYPE "Zero"
#elif defined(ARDUINO_ARC32_TOOLS)       
    #define ARDUINO_TYPE "101"
#elif defined(BCS_ARDUINO_EMULATOR)
	#define ARDUINO_TYPE "BCSEmul"
#else
    #error "Unknown ARDUINO_TYPE"
#endif

#endif
