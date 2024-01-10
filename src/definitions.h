/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef DEBUG
#define DEBUG_GEARSYSTEM 1
#endif

#if defined(PS2) || defined(PSP)
#define PERFORMANCE
#endif

#define GEARSYSTEM_TITLE "Gearsystem"
#define GEARSYSTEM_VERSION "3.5.0"

#ifndef EMULATOR_BUILD
#define EMULATOR_BUILD "undefined"
#endif

#ifndef NULL
#define NULL 0
#endif

#ifdef _WIN32
#define BLARGG_USE_NAMESPACE 1
#endif

//#define GEARSYSTEM_DISABLE_DISASSEMBLER

#define MAX_ROM_SIZE 0x800000

#define SafeDelete(pointer) if(pointer != NULL) {delete pointer; pointer = NULL;}
#define SafeDeleteArray(pointer) if(pointer != NULL) {delete [] pointer; pointer = NULL;}

#define InitPointer(pointer) ((pointer) = NULL)
#define IsValidPointer(pointer) ((pointer) != NULL)

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN
#else
#define IS_LITTLE_ENDIAN
#endif

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

typedef void (*RamChangedCallback) (void);

#define FLAG_CARRY 0x01
#define FLAG_NEGATIVE 0x02
#define FLAG_PARITY 0x04
#define FLAG_X 0x08
#define FLAG_HALF 0x10
#define FLAG_Y 0x20
#define FLAG_ZERO 0x40
#define FLAG_SIGN 0x80
#define FLAG_NONE 0

#define GS_RESOLUTION_MAX_WIDTH 256
#define GS_RESOLUTION_MAX_HEIGHT 224

#define GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN 320
#define GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN 288

#define GS_RESOLUTION_SMS_WIDTH 256
#define GS_RESOLUTION_SMS_HEIGHT 192
#define GS_RESOLUTION_SMS_HEIGHT_EXTENDED 224
#define GS_RESOLUTION_SMS_OVERSCAN_H 32
#define GS_RESOLUTION_SMS_OVERSCAN_V 24
#define GS_RESOLUTION_SMS_OVERSCAN_V_PAL 48

#define GS_RESOLUTION_GG_WIDTH 160
#define GS_RESOLUTION_GG_HEIGHT 144
#define GS_RESOLUTION_GG_X_OFFSET 48
#define GS_RESOLUTION_GG_Y_OFFSET 24
#define GS_RESOLUTION_GG_Y_OFFSET_EXTENDED 40

#define GS_MASTER_CLOCK_NTSC 3579545
#define GS_LINES_PER_FRAME_NTSC 262
#define GS_FRAMES_PER_SECOND_NTSC 60
#define GS_CYCLES_PER_LINE 228

#define GS_MASTER_CLOCK_PAL 3546893
#define GS_LINES_PER_FRAME_PAL 313
#define GS_FRAMES_PER_SECOND_PAL 50

#define GS_AUDIO_BUFFER_SIZE 4096

#define GS_SAVESTATE_MAGIC 0x28011983

enum GS_Color_Format
{
    GS_PIXEL_RGB565,
    GS_PIXEL_RGB555,
    GS_PIXEL_RGB888,
    GS_PIXEL_BGR565,
    GS_PIXEL_BGR555,
    GS_PIXEL_BGR888
};

enum GS_Keys
{
    Key_Up = 0,
    Key_Down = 1,
    Key_Left = 2,
    Key_Right = 3,
    Key_1 = 4,
    Key_2 = 5,
    Key_Start = 6,
};

enum GS_Joypads
{
    Joypad_1 = 0,
    Joypad_2 = 1
};

enum GS_System
{
    System_SMS_NTSC_USA,
    System_SMS_NTSC_JAP,
    System_SMS_PAL,
    System_GG
};

enum GS_Region
{
    Region_NTSC,
    Region_PAL
};

struct GS_RuntimeInfo
{
    int screen_width;
    int screen_height;
    GS_Region region;
};

#ifdef DEBUG_GEARSYSTEM
#ifdef __ANDROID__
        #include <android/log.h>
        #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "GEARSYSTEM", __VA_ARGS__);
    #endif
#define Log(msg, ...) (Log_func(msg, ##__VA_ARGS__))
#else
#define Log(msg, ...)
#endif

inline void Log_func(const char* const msg, ...)
{
    static int count = 1;
    char szBuf[512];

    va_list args;
    va_start(args, msg);
    vsprintf(szBuf, msg, args);
    va_end(args);

    printf("%d: %s\n", count, szBuf);
    fflush(stdout);

    count++;
}

inline u8 SetBit(const u8 value, const u8 bit)
{
    return value | (0x01 << bit);
}

inline u8 UnsetBit(const u8 value, const u8 bit)
{
    return value & (~(0x01 << bit));
}

inline bool IsSetBit(const u8 value, const u8 bit)
{
    return (value & (0x01 << bit)) != 0;
}

inline u8 FlipBit(const u8 value, const u8 bit)
{
    return value ^ (0x01 << bit);
}

inline u8 ReverseBits(const u8 value)
{
    u8 ret = value;
    ret = (ret & 0xF0) >> 4 | (ret & 0x0F) << 4;
    ret = (ret & 0xCC) >> 2 | (ret & 0x33) << 2;
    ret = (ret & 0xAA) >> 1 | (ret & 0x55) << 1;
    return ret;
}

inline int AsHex(const char c)
{
   return c >= 'A' ? c - 'A' + 0xA : c - '0';
}

inline unsigned int Pow2Ceil(u16 n)
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    ++n;
    return n;
}

#endif	/* DEFINITIONS_H */
