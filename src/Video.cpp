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

#include "Video.h"
#include "Memory.h"
#include "Processor.h"

Video::Video(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    InitPointer(m_pFrameBuffer);
    InitPointer(m_pColorFrameBuffer);
    InitPointer(m_pVdpVRAM);
    InitPointer(m_pVdpCRAM);
    m_bFirstByteInSequence = false;
    m_VdpLatch = 0;
    for (int i = 0; i < 16; i++)
        m_VdpRegister[i] = 0;
    m_VdpCode = 0;
    m_VdpBuffer = 0;
    m_VdpAddress = 0;
    m_iVCounter = 0;
    m_iHCounter = 0;
    m_iCycleCounter = 0;
    m_VdpStatus = 0;
    m_bVBlankInterrupt = false;
    m_bHBlankInterrupt = false;
    m_HBlankCounter = 0;
}

Video::~Video()
{
    SafeDeleteArray(m_pFrameBuffer);
    SafeDeleteArray(m_pVdpVRAM);
    SafeDeleteArray(m_pVdpCRAM);
}

void Video::Init()
{
    m_pFrameBuffer = new u8[GS_SMS_WIDTH * GS_SMS_HEIGHT];
    m_pVdpVRAM = new u8[0x4000];
    m_pVdpCRAM = new u8[0x40];
    Reset();
}

void Video::Reset()
{
    m_bFirstByteInSequence = true;
    m_VdpBuffer = 0;
    m_iVCounter = 0;
    m_iHCounter = 0;
    m_VdpLatch = 0;
    m_VdpCode = 0;
    m_VdpBuffer = 0;
    m_iCycleCounter = 0;
    m_VdpStatus = 0;
    m_bVBlankInterrupt = false;
    m_bHBlankInterrupt = false;
    m_HBlankCounter = 0xFF;
    for (int i = 0; i < (GS_SMS_WIDTH * GS_SMS_HEIGHT); i++)
        m_pFrameBuffer[i] = 0;
    for (int i = 0; i < 0x4000; i++)
        m_pVdpVRAM[i] = 0;
    for (int i = 0; i < 0x40; i++)
        m_pVdpCRAM[i] = 0;

    m_VdpRegister[0] = 0x36; // Mode
    m_VdpRegister[1] = 0xA0; // Mode
    m_VdpRegister[2] = 0xFF; // Screen Map Table Base
    m_VdpRegister[3] = 0xFF; // Always $FF
    m_VdpRegister[4] = 0xFF; // Always $FF
    m_VdpRegister[5] = 0xFF; // Sprite Table Base
    m_VdpRegister[6] = 0xFB; // Sprite Pattern Table Base
    m_VdpRegister[7] = 0x00; // Border color #0
    m_VdpRegister[8] = 0x00; // Scroll-H
    m_VdpRegister[9] = 0x00; // Scroll-V
    m_VdpRegister[10] = 0xFF; // H-line interrupt ($FF=OFF)
}

bool Video::Tick(unsigned int &clockCycles, GS_Color* pColorFrameBuffer)
{
    bool vblank = false;
    m_pColorFrameBuffer = pColorFrameBuffer;
    m_iCycleCounter += clockCycles;

    if (m_iCycleCounter >= GS_CYCLES_PER_LINE_NTSC)
    {
        m_iCycleCounter -= GS_CYCLES_PER_LINE_NTSC;

        ScanLine(m_iVCounter);

        if (m_iVCounter < 192)
        {
            m_HBlankCounter--;
            if (m_HBlankCounter == 0xFF)
            {
                m_HBlankCounter = m_VdpRegister[10];
                m_bHBlankInterrupt = true;
            }

            if (m_iVCounter == 191)
            {
                m_VdpStatus = SetBit(m_VdpStatus, 7);
                m_bVBlankInterrupt = true;
            }
        }
        else // m_iVCounter >= 192
            m_HBlankCounter = m_VdpRegister[10];

        if ((m_bVBlankInterrupt && IsSetBit(m_VdpRegister[1], 5)) ||
                (m_bHBlankInterrupt && IsSetBit(m_VdpRegister[0], 4)))
            m_pProcessor->RequestINT(true);

        m_iVCounter++;

        if (m_iVCounter >= GS_LINES_PER_FRAME_NTSC)
        {
            m_iVCounter = 0;
            vblank = true;
        }
    }

    return vblank;
}

u8 Video::GetVCounter()
{
    // NTSC
    if (m_iVCounter > 0xDA)
        return m_iVCounter - 0x06;
    else
        return m_iVCounter;

    // PAL
    //if (m_iVCounter > 0xF2)
    //    return m_iVCounter - 0x39;
    //else
    //    return m_iVCounter;
}

u8 Video::GetHCounter()
{
    return m_iHCounter;
}

u8 Video::GetDataPort()
{
    m_bFirstByteInSequence = true;
    u8 ret = m_VdpBuffer;
    m_VdpBuffer = m_pVdpVRAM[m_VdpAddress];
    m_VdpAddress++;
    m_VdpAddress &= 0x3FFF;
    return ret;
}

u8 Video::GetStatusFlags()
{
    u8 ret = m_VdpStatus;
    m_bFirstByteInSequence = true;
    m_VdpStatus = UnsetBit(m_VdpStatus, 7);
    m_bVBlankInterrupt = false;
    m_bHBlankInterrupt = false;
    m_pProcessor->RequestINT(false);
    return ret;
}

void Video::WriteData(u8 data)
{
    m_bFirstByteInSequence = true;
    m_VdpBuffer = data;
    switch (m_VdpCode)
    {
        case VDP_READ_VRAM_OPERATION:
        case VDP_WRITE_VRAM_OPERATION:
        case VDP_WRITE_REG_OPERATION:
        {
            m_pVdpVRAM[m_VdpAddress] = data;
            break;
        }
        case VDP_WRITE_CRAM_OPERATION:
        {
            m_pVdpCRAM[m_VdpAddress & 0x1F] = data;
            break;
        }
    }
    m_VdpAddress++;
    m_VdpAddress &= 0x3FFF;
}

void Video::WriteControl(u8 control)
{
    if (m_bFirstByteInSequence)
    {
        m_bFirstByteInSequence = false;
        m_VdpLatch = control;
    }
    else
    {
        m_bFirstByteInSequence = true;
        m_VdpCode = (control >> 6) & 0x03;
        m_VdpAddress = ((control & 0x3F) << 8) + m_VdpLatch;

        switch (m_VdpCode)
        {
            case VDP_READ_VRAM_OPERATION:
            {
                m_VdpBuffer = m_pVdpVRAM[m_VdpAddress];
                m_VdpAddress++;
                m_VdpAddress &= 0x3FFF;
                break;
            }
            case VDP_WRITE_REG_OPERATION:
            {
                u8 reg = control & 0x0F;
                m_VdpRegister[reg] = m_VdpLatch;
                if (reg > 10)
                {
                    Log("--> ** Attempting to write on VDP REG %d: %X", reg, m_VdpLatch);
                }
                break;
            }
        }
    }
}

void Video::ScanLine(int line)
{
    if (IsSetBit(m_VdpRegister[1], 6))
    {
        // DISPLAY ON
        RenderBG(line);
    }
    else
    {
        // DISPLAY OFF
        for (int scx = 0; scx < 256; scx++)
        {
            GS_Color final_color;
            final_color.red = 0;
            final_color.green = 0;
            final_color.blue = 0;
            final_color.alpha = 0xFF;
            m_pColorFrameBuffer[(line * 256) + scx] = final_color;
        }
    }
}

void Video::RenderBG(int line)
{
    if (line >= 192)
        return;

    int origin_x = m_VdpRegister[8];
    if (IsSetBit(m_VdpRegister[0], 6) && line < 16)
        origin_x = 0;
    int origin_y = m_VdpRegister[9];
    int scy = line;
    int map_y = scy + origin_y;

    if (map_y >= 224)
        map_y -= 224;

    int palette_color = 0;

    for (int scx = 0; scx < 256; scx++)
    {
        if (IsSetBit(m_VdpRegister[0], 5) && scx < 8)
        {
            palette_color = (m_VdpRegister[7] & 0x0F) + 16;
        }
        else
        {
            if (IsSetBit(m_VdpRegister[0], 7) && scx >= 192)
                origin_y = 0;
            u8 map_x = scx - origin_x;
            u16 map_address = (m_VdpRegister[2] << 10) & 0x3800;

            int tile_x = map_x / 8;
            int tile_x_offset = map_x % 8;
            int tile_y = map_y / 8;
            int tile_y_offset = map_y % 8;

            int tile_addr = map_address + (((tile_y * 32) + tile_x) * 2);
            int tile_index = m_pVdpVRAM[tile_addr];
            int tile_info = m_pVdpVRAM[tile_addr + 1];
            if (IsSetBit(tile_info, 0))
                tile_index = (tile_index | 0x0100) & 0x1FF;

            bool hflip = IsSetBit(tile_info, 1);
            bool vflip = IsSetBit(tile_info, 2);
            int palette_offset = IsSetBit(tile_info, 3) ? 16 : 0;
            bool priotirty = IsSetBit(tile_info, 4);

            int tile_data_addr = tile_index * 32;
            tile_data_addr += ((vflip ? 7 - tile_y_offset : tile_y_offset) * 4);

            int tile_pixel_x = 7 - tile_x_offset;
            if (hflip)
                tile_pixel_x = tile_x_offset;

            palette_color = ((m_pVdpVRAM[tile_data_addr] >> tile_pixel_x) & 0x01) +
                    (((m_pVdpVRAM[tile_data_addr + 1] >> tile_pixel_x) & 0x01) << 1) +
                    (((m_pVdpVRAM[tile_data_addr + 2] >> tile_pixel_x) & 0x01) << 2) +
                    (((m_pVdpVRAM[tile_data_addr + 3] >> tile_pixel_x) & 0x01) << 3) +
                    palette_offset;
        }

        int r = m_pVdpCRAM[palette_color] & 0x03;
        int g = (m_pVdpCRAM[palette_color] >> 2) & 0x03;
        int b = (m_pVdpCRAM[palette_color] >> 4) & 0x03;

        GS_Color final_color;

        final_color.red = (r * 255) / 3;
        final_color.green = (g * 255) / 3;
        final_color.blue = (b * 255) / 3;
        final_color.alpha = 0xFF;

        m_pColorFrameBuffer[(scy * 256) + scx] = final_color;
    }
}

void Video::RenderSprites(int line)
{

}

