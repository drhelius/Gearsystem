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

#ifndef VIDEO_H
#define	VIDEO_H

#include "definitions.h"

#define VDP_READ_VRAM_OPERATION 0x00
#define VDP_WRITE_VRAM_OPERATION 0x01
#define VDP_WRITE_REG_OPERATION 0x02
#define VDP_WRITE_CRAM_OPERATION 0x03

class Memory;
class Processor;
class Cartridge;

class Video
{
public:
    enum Overscan
    {
        OverscanDisabled,
        OverscanTopBottom,
        OverscanFull284,
        OverscanFull320
    };

public:
    Video(Memory* pMemory, Processor* pProcessor, Cartridge* pCartridge);
    ~Video();
    void Init();
    void Reset(bool bGameGear, bool bPAL);
    bool Tick(unsigned int clockCycles);
    u8 GetVCounter();
    u8 GetHCounter();
    u8 GetDataPort();
    u8 GetStatusFlags();
    bool IsExtendedMode224();
    bool IsSG1000Mode();
    void WriteData(u8 data);
    void WriteControl(u8 control);
    void LatchHCounter();
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);
    u8* GetVRAM();
    u8* GetCRAM();
    u8* GetRegisters();
    int GetSG1000Mode();
    u16 ColorFromPalette(int palette_color);
    u16* GetFrameBuffer();
    void Render24bit(u16* srcFrameBuffer, u8* dstFrameBuffer, GS_Color_Format pixelFormat, int size, bool overscan = false);
    void Render16bit(u16* srcFrameBuffer, u8* dstFrameBuffer, GS_Color_Format pixelFormat, int size, bool overscan = false);
    void SetOverscan(Overscan overscan);
    Overscan GetOverscan();

private:
    void ScanLine(int line);
    void RenderBackgroundSMSGG(int line);
    void RenderBackgroundSG1000(int line);
    void ParseSpritesSMSGG(int line);
    void RenderSpritesSMSGG(int line);
    void RenderSpritesSG1000(int line);
    void InitPalettes(const u8* src, u16* dest_565_rgb, u16* dest_555_rgb, u16* dest_565_bgr, u16* dest_555_bgr);

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Cartridge* m_pCartridge;
    u8* m_pInfoBuffer;
    u16* m_pFrameBuffer;
    u8* m_pVdpVRAM;
    u8* m_pVdpCRAM;
    bool m_bFirstByteInSequence;
    u8 m_VdpRegister[16];
    u8 m_VdpCode;
    u8 m_VdpBuffer;
    u16 m_VdpAddress;
    int m_iVCounter;
    int m_iHCounter;
    int m_iCycleCounter;
    u8 m_VdpStatus;
    int m_iVdpRegister10Counter;
    u8 m_ScrollX;
    u8 m_ScrollY;
    bool m_bGameGear;
    int m_iLinesPerFrame;
    bool m_bPAL;
    bool m_bExtendedMode224;
    Overscan m_Overscan;

    struct LineEvents 
    {
        bool vint;
        bool vintFlag;
        bool hint;
        bool scrollx;
        bool vcounter;
        bool render;
        bool display;
        bool spriteovr;
    };
    LineEvents m_LineEvents;

    int m_iRenderLine;
    int m_iScreenWidth;
    bool m_bSG1000;
    int m_iSG1000Mode;

    enum Timing
    {
        TIMING_VINT = 0,
        TIMING_XSCROLL = 1,
        TIMING_HINT = 2,
        TIMING_VCOUNT = 3,
        TIMING_FLAG_VINT = 4,
        TIMING_RENDER = 5,
        TIMING_DISPLAY = 6,
        TIMING_SPRITEOVR = 7
    };

    int m_Timing[8];
    int m_NextLineSprites[8];
    bool m_bDisplayEnabled;
    bool m_bSpriteOvrRequest;

    u16 m_SG1000_palette_565_rgb_normal[16];
    u16 m_SG1000_palette_555_rgb_normal[16];
    u16 m_SG1000_palette_565_bgr_normal[16];
    u16 m_SG1000_palette_555_bgr_normal[16];
    u16 m_SG1000_palette_565_rgb_sms[16];
    u16 m_SG1000_palette_555_rgb_sms[16];
    u16 m_SG1000_palette_565_bgr_sms[16];
    u16 m_SG1000_palette_555_bgr_sms[16];
};

inline u8* Video::GetVRAM()
{
    return m_pVdpVRAM;
}

inline u8* Video::GetCRAM()
{
    return m_pVdpCRAM;
}

inline u8* Video::GetRegisters()
{
    return m_VdpRegister;
}

inline int Video::GetSG1000Mode()
{
    return m_iSG1000Mode;
}

inline u16 Video::ColorFromPalette(int palette_color)
{
    if (m_bGameGear)
    {
        int palette_color_2 = palette_color << 1;
        return m_pVdpCRAM[palette_color_2] | ((m_pVdpCRAM[palette_color_2 + 1] & 0x0F) << 8);
    }
    else
    {
        return m_pVdpCRAM[palette_color];
    }
}

inline u16* Video::GetFrameBuffer()
{
    return m_pFrameBuffer;
}

const u8 kVdpHCounter[228] = {

  0xE9,0xEA,0xEA,0xEB,0xEC,0xED,0xED,0xEE,0xEF,0xF0,0xF0,0xF1,0xF2,0xF3,0xF3,0xF4,
  0xF5,0xF6,0xF6,0xF7,0xF8,0xF9,0xF9,0xFA,0xFB,0xFC,0xFC,0xFD,0xFE,0xFF,0xFF,
  0x00,0x01,0x02,0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B,0x0B,
  0x0C,0x0D,0x0E,0x0E,0x0F,0x10,0x11,0x11,0x12,0x13,0x14,0x14,0x15,0x16,0x17,0x17,
  0x18,0x19,0x1A,0x1A,0x1B,0x1C,0x1D,0x1D,0x1E,0x1F,0x20,0x20,0x21,0x22,0x23,0x23,
  0x24,0x25,0x26,0x26,0x27,0x28,0x29,0x29,0x2A,0x2B,0x2C,0x2C,0x2D,0x2E,0x2F,0x2F,
  0x30,0x31,0x32,0x32,0x33,0x34,0x35,0x35,0x36,0x37,0x38,0x38,0x39,0x3A,0x3B,0x3B,
  0x3C,0x3D,0x3E,0x3E,0x3F,0x40,0x41,0x41,0x42,0x43,0x44,0x44,0x45,0x46,0x47,0x47,
  0x48,0x49,0x4A,0x4A,0x4B,0x4C,0x4D,0x4D,0x4E,0x4F,0x50,0x50,0x51,0x52,0x53,0x53,
  0x54,0x55,0x56,0x56,0x57,0x58,0x59,0x59,0x5A,0x5B,0x5C,0x5C,0x5D,0x5E,0x5F,0x5F,
  0x60,0x61,0x62,0x62,0x63,0x64,0x65,0x65,0x66,0x67,0x68,0x68,0x69,0x6A,0x6B,0x6B,
  0x6C,0x6D,0x6E,0x6E,0x6F,0x70,0x71,0x71,0x72,0x73,0x74,0x74,0x75,0x76,0x77,0x77,
  0x78,0x79,0x7A,0x7A,0x7B,0x7C,0x7D,0x7D,0x7E,0x7F,0x80,0x80,0x81,0x82,0x83,0x83,
  0x84,0x85,0x86,0x86,0x87,0x88,0x89,0x89,0x8A,0x8B,0x8C,0x8C,0x8D,0x8E,0x8F,0x8F,
  0x90,0x91,0x92,0x92,0x93,
};

const u8 kSG1000_palette_888_normal[48] = {0,0,0, 0,0,0, 33,200,66, 94,220,120, 84,85,237, 125,118,252, 212,82,77, 66,235,245, 252,85,84, 255,121,120, 212,193,84, 230,206,128, 33,176,59, 201,91,186, 204,204,204, 255,255,255};
const u8 kSG1000_palette_888_sms[48] = {0,0,0, 0,0,0, 0,170,0, 0,255,0, 0,0,85, 0,0,255, 85,0,0, 0,255,255, 170,0,0, 255,0,0, 85,85,0, 255,255,0, 0,85,0, 255,0,255, 85,85,85, 255,255,255};
const u8 k2bitTo8bit[4] = {0,85,170,255};
const u8 k2bitTo5bit[4] = {0,10,21,31};
const u8 k2bitTo6bit[4] = {0,21,42,63};
const u8 k4bitTo8bit[16] = {0,17,34,51,68,86,102,119,136,153,170,187,204,221,238,255};
const u8 k4bitTo5bit[16] = {0,2,4,6,8,10,12,14,17,19,21,23,25,27,29,31};
const u8 k4bitTo6bit[16] = {0,4,8,13,17,21,25,29,34,38,42,46,50,55,59,63};

#endif	/* VIDEO_H */
