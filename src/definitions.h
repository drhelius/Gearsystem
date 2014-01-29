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

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <iostream>
#include <fstream>

//#define DEBUG_GEARSYSTEM 1

#ifdef DEBUG_GEARSYSTEM
#define DISASM_GEARSYSTEM 1
#endif

#define GEARSYSTEM_TITLE "Gearsystem 1.0"
#define GEARSYSTEM_VERSION 1.0

#ifndef NULL
#define NULL 0
#endif

#ifdef _WIN32
#define BLARGG_USE_NAMESPACE 1
#endif

#define SafeDelete(pointer) if(pointer != NULL) {delete pointer; pointer = NULL;}
#define SafeDeleteArray(pointer) if(pointer != NULL) {delete [] pointer; pointer = NULL;}

#define InitPointer(pointer) ((pointer) = NULL)
#define IsValidPointer(pointer) ((pointer) != NULL)

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

#define FLAG_CARRY 0x01
#define FLAG_NEGATIVE 0x02
#define FLAG_PARITY 0x04
#define FLAG_X 0x08
#define FLAG_HALF 0x10
#define FLAG_Y 0x20
#define FLAG_ZERO 0x40
#define FLAG_SIGN 0x80
#define FLAG_NONE 0

#define GS_SMS_WIDTH 256
#define GS_SMS_HEIGHT 224
#define GS_GG_WIDTH 160
#define GS_GG_HEIGHT 144
#define GS_GG_X_OFFSET 48
#define GS_GG_Y_OFFSET 24
#define GS_BORDER_LEFT_RIGHT 64
#define GS_BORDER_TOP_BOTTOM 48
#define GS_SCREEN_WIDTH (GS_SMS_WIDTH + (GS_BORDER_LEFT_RIGHT * 2))
#define GS_SCREEN_HEIGHT (GS_SMS_HEIGHT + (GS_BORDER_TOP_BOTTOM * 2))

#define GS_MASTER_CLOCK_NTSC 3579545
#define GS_LINES_PER_FRAME_NTSC 262
#define GS_FRAMES_PER_SECOND_NTSC 60
#define GS_CYCLES_PER_LINE 228

#define GS_MASTER_CLOCK_PAL 3546893
#define GS_LINES_PER_FRAME_PAL 313
#define GS_FRAMES_PER_SECOND_PAL 50

struct GS_Color
{
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
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
    Joypad_1,
    Joypad_2
};

enum GS_System
{
    System_SMS_NTSC_USA,
    System_SMS_NTSC_JAP,
    System_SMS_PAL,
    System_GG
};

#ifdef DEBUG_GEARSYSTEM
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

#endif	/* DEFINITIONS_H */

