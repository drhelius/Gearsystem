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
#define GS_DEBUG 1
#endif

#if defined(PS2) || defined(PSP)
#define PERFORMANCE
#endif

#if !defined(EMULATOR_BUILD)
    #define EMULATOR_BUILD "undefined"
#endif

#define GS_TITLE "Gearsystem"
#define GS_VERSION EMULATOR_BUILD
#define GS_TITLE_ASCII "" \
"   ____                               _                  \n" \
"  / ___| ___  __ _ _ __ ___ _   _ ___| |_ ___ _ __ ___   \n" \
" | |  _ / _ \\/ _` | '__/ __| | | / __| __/ _ \\ '_ ` _ \\  \n" \
" | |_| |  __/ (_| | |  \\__ \\ |_| \\__ \\ ||  __/ | | | | | \n" \
"  \\____|\\___|\\__,_|_|  |___/\\__, |___/\\__\\___|_| |_| |_| \n" \
"                            |___/                        \n"

#ifndef NULL
#define NULL 0
#endif

#ifdef _WIN32
#define BLARGG_USE_NAMESPACE 1
#endif

//#define GS_DISABLE_DISASSEMBLER

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

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(value, min, max) MIN(MAX(value, min), max)

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

#define GS_MAX_GAMEPADS 2

#define GS_RESOLUTION_MAX_WIDTH 256
#define GS_RESOLUTION_MAX_HEIGHT 224

#define GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN 320
#define GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN 288

#define GS_RESOLUTION_SMS_WIDTH 256
#define GS_RESOLUTION_SMS_HEIGHT 192
#define GS_RESOLUTION_SMS_HEIGHT_EXTENDED 224
#define GS_RESOLUTION_SMS_OVERSCAN_H_320_L 32
#define GS_RESOLUTION_SMS_OVERSCAN_H_320_R 32
#define GS_RESOLUTION_SMS_OVERSCAN_H_284_L 14
#define GS_RESOLUTION_SMS_OVERSCAN_H_284_R 14
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

#define GS_AUDIO_SAMPLE_RATE 44100
#define GS_AUDIO_BUFFER_SIZE 4096
#define GS_AUDIO_QUEUE_SIZE 4096

#define GS_SAVESTATE_MAGIC 0x03121220
#define GS_SAVESTATE_VERSION 100
#define GS_SAVESTATE_VERSION_V1 1

enum GS_Color_Format
{
    GS_PIXEL_RGB565,
    GS_PIXEL_RGB555,
    GS_PIXEL_RGBA8888,
    GS_PIXEL_BGR565,
    GS_PIXEL_BGR555,
    GS_PIXEL_BGRA8888
};

enum GS_Keys
{
    Key_None = 0x00,
    Key_Up = 0x01,
    Key_Down = 0x02,
    Key_Left = 0x04,
    Key_Right = 0x08,
    Key_1 = 0x10,
    Key_2 = 0x20,
    Key_Start = 0x80,
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

struct GS_SaveState_Header
{
    u32 magic;
    u32 version;
    u32 size;
    s64 timestamp;
    char rom_name[128];
    u32 rom_crc;
    u32 screenshot_size;
    u16 screenshot_width;
    u16 screenshot_height;
    char emu_build[32];
};

struct GS_SaveState_Header_Libretro
{
    u32 magic;
    u32 version;
};

struct GS_SaveState_Screenshot
{
    u32 width;
    u32 height;
    u32 size;
    u8* data;
};

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

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    #define GS_BIG_ENDIAN
#else
    #define GS_LITTLE_ENDIAN
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define INLINE inline __attribute__((always_inline))
    #define NO_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define INLINE __forceinline
    #define NO_INLINE __declspec(noinline)
#else
    #define INLINE inline
    #define NO_INLINE
#endif

#if !defined(GS_DEBUG)
    #if defined(__GNUC__) || defined(__clang__)
        #if !defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
            #warning "Compiling without optimizations."
            #define GS_NO_OPTIMIZATIONS
        #endif
    #elif defined(_MSC_VER)
        #if !defined(NDEBUG)
            #pragma message("Compiling without optimizations.")
            #define GS_NO_OPTIMIZATIONS
        #endif
    #endif
#endif

#endif	/* DEFINITIONS_H */
