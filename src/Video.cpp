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
    InitPointer(m_pInfoBuffer);
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
    m_iCycleAdjustment = 0;
    m_VdpStatus = 0;
    m_iHBlankCycles = 0;
    m_bHBlankInterrupt = false;
    m_HBlankCounter = 0;
    m_ScrollV = 0;
    m_bGameGear = false;
    m_iCyclesPerLine = 0;
    m_iCyclesAdjustmentLine = 0;
    m_iCyclesPerLineLeft = 0;
    m_iLinesPerFrame = 0;
}

Video::~Video()
{
    SafeDeleteArray(m_pInfoBuffer);
    SafeDeleteArray(m_pVdpVRAM);
    SafeDeleteArray(m_pVdpCRAM);
}

void Video::Init()
{
    m_pInfoBuffer = new u8[GS_SMS_WIDTH * GS_SMS_HEIGHT];
    m_pVdpVRAM = new u8[0x4000];
    m_pVdpCRAM = new u8[0x40];
    Reset(false, false);
}

void Video::Reset(bool bGameGear, bool bPAL)
{
    m_bGameGear = bGameGear;
    m_iCyclesPerLine = bPAL ? GS_CYCLES_PER_LINE_PAL : GS_CYCLES_PER_LINE_NTSC;
    m_iCyclesAdjustmentLine = bPAL ? GS_CYCLES_ADJUSTMENT_LINE_PAL : GS_CYCLES_ADJUSTMENT_LINE_NTSC;
    m_iCyclesPerLineLeft = bPAL ? GS_CYCLES_PER_LINE_LEFT_PAL : GS_CYCLES_PER_LINE_LEFT_NTSC;
    m_iLinesPerFrame = bPAL ? GS_LINES_PER_FRAME_PAL : GS_LINES_PER_FRAME_NTSC;
    m_bFirstByteInSequence = true;
    m_VdpBuffer = 0;
    m_iVCounter = 0;
    m_iHCounter = 0;
    m_VdpLatch = 0;
    m_VdpCode = 0;
    m_VdpBuffer = 0;
    m_VdpAddress = 0;
    m_iCycleCounter = 0;
    m_iCycleAdjustment = 0;
    m_VdpStatus = 0;
    m_iHBlankCycles = 0;
    m_bHBlankInterrupt = false;
    m_HBlankCounter = 0xFF;
    m_ScrollV = 0;
    for (int i = 0; i < (GS_SMS_WIDTH * GS_SMS_HEIGHT); i++)
        m_pInfoBuffer[i] = 0;
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

    for (int i = 11; i < 16; i++)
        m_VdpRegister[i] = 0;
}

bool Video::Tick(unsigned int &clockCycles, GS_Color* pColorFrameBuffer)
{
    bool vblank = false;
    m_pColorFrameBuffer = pColorFrameBuffer;
    m_iCycleCounter += clockCycles;

    if (m_iHBlankCycles > 0)
    {
        m_iHBlankCycles -= clockCycles;

        if (m_iHBlankCycles <= 0)
        {
            m_iHBlankCycles = 0;
            if (m_bHBlankInterrupt && IsSetBit(m_VdpRegister[0], 4))
                m_pProcessor->RequestINT(true);
        }
    }

    if (m_iCycleCounter >= m_iCyclesPerLine)
    {
        m_iCycleAdjustment++;
        if (m_iCycleAdjustment >= m_iCyclesAdjustmentLine)
        {
            m_iCycleAdjustment = 0;
            m_iCycleCounter += m_iCyclesPerLineLeft;
        }
        m_iCycleCounter -= m_iCyclesPerLine;

        if (m_iVCounter < 192)
        {
            ScanLine(m_iVCounter);

            if (m_iVCounter == 191)
            {
                m_VdpStatus = SetBit(m_VdpStatus, 7);
                if (IsSetBit(m_VdpRegister[1], 5))
                    m_pProcessor->RequestINT(true);
            }
        }

        m_iVCounter++;
        if (m_iVCounter >= m_iLinesPerFrame)
        {
            m_ScrollV = m_VdpRegister[9];
            m_iVCounter = 0;
            vblank = true;
        }

        if ((m_iVCounter >= 0) && (m_iVCounter < 193))
        {
            m_HBlankCounter--;
            if (m_HBlankCounter == 0xFF)
            {
                m_HBlankCounter = m_VdpRegister[10];
                m_bHBlankInterrupt = true;
                m_iHBlankCycles = 110;
            }
        }
        else
            m_HBlankCounter = m_VdpRegister[10];
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
    // Not implemented
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
    m_VdpStatus = 0x00;
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
            m_pVdpCRAM[m_VdpAddress & (m_bGameGear ? 0x3F : 0x1F)] = data;
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
        RenderSprites(line);
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
    int scy = line;
    int scy_256 = scy << 8;
    int origin_x = m_VdpRegister[8];
    if ((line < 16) && IsSetBit(m_VdpRegister[0], 6))
        origin_x = 0;
    int origin_y = m_ScrollV;

    u16 map_address = (m_VdpRegister[2] << 10) & 0x3800;
    int map_y = scy + origin_y;
    if (map_y >= 224)
        map_y -= 224;

    int tile_y = map_y >> 3;
    int tile_y_offset = map_y & 7;

    int palette_color = 0;
    u8 info = 0;

    for (int scx = 0; scx < 256; scx++)
    {
        if (IsSetBit(m_VdpRegister[0], 5) && scx < 8)
        {
            palette_color = (m_VdpRegister[7] & 0x0F) + 16;
        }
        else
        {
            if (IsSetBit(m_VdpRegister[0], 7) && (scx >= 192))
                origin_y = 0;
            u8 map_x = scx - origin_x;

            int tile_x = map_x >> 3;
            int tile_x_offset = map_x & 7;

            int tile_addr = map_address + (((tile_y << 5) + tile_x) << 1);
            int tile_index = m_pVdpVRAM[tile_addr];
            int tile_info = m_pVdpVRAM[tile_addr + 1];
            if (IsSetBit(tile_info, 0))
                tile_index = (tile_index | 0x0100) & 0x1FF;

            bool hflip = IsSetBit(tile_info, 1);
            bool vflip = IsSetBit(tile_info, 2);
            int palette_offset = IsSetBit(tile_info, 3) ? 16 : 0;
            bool priority = IsSetBit(tile_info, 4);

            int tile_data_addr = tile_index << 5;
            tile_data_addr += ((vflip ? 7 - tile_y_offset : tile_y_offset) << 2);

            int tile_pixel_x = 7 - tile_x_offset;
            if (hflip)
                tile_pixel_x = tile_x_offset;

            palette_color = ((m_pVdpVRAM[tile_data_addr] >> tile_pixel_x) & 0x01) +
                    (((m_pVdpVRAM[tile_data_addr + 1] >> tile_pixel_x) & 0x01) << 1) +
                    (((m_pVdpVRAM[tile_data_addr + 2] >> tile_pixel_x) & 0x01) << 2) +
                    (((m_pVdpVRAM[tile_data_addr + 3] >> tile_pixel_x) & 0x01) << 3) +
                    palette_offset;
            info = 0x01 | ((priority && ((palette_color - palette_offset) != 0)) ? 0x02 : 0);
        }

        int pixel = scy_256 + scx;
        if (m_bGameGear && ((scx < GS_GG_X_OFFSET) || (scx >= (GS_GG_X_OFFSET + GS_GG_WIDTH)) || (scy < GS_GG_Y_OFFSET) || (scy >= (GS_GG_Y_OFFSET + GS_GG_HEIGHT))))
        {
            GS_Color final_color;
            final_color.red = 0;
            final_color.green = 0;
            final_color.blue = 0;
            final_color.alpha = 0xFF;
            m_pColorFrameBuffer[pixel] = final_color;
        }
        else
            m_pColorFrameBuffer[pixel] = ConvertTo8BitColor(palette_color);
        m_pInfoBuffer[pixel] = info;
    }
}

void Video::RenderSprites(int line)
{
    int sprite_collision = false;
    int sprite_count = 0;
    int scy = line;
    int scy_256 = scy << 8;
    int sprite_height = IsSetBit(m_VdpRegister[1], 1) ? 16 : 8;
    int sprite_shift = IsSetBit(m_VdpRegister[0], 3) ? 8 : 0;
    u16 sprite_table_address = (m_VdpRegister[5] << 7) & 0x3F00;
    u16 sprite_table_address_2 = sprite_table_address + 0x80;
    u16 sprite_tiles_address = (m_VdpRegister[6] << 11) & 0x2000;

    int max_sprite = 63;

    for (int sprite = 0; sprite < 64; sprite++)
    {
        int sprite_y = m_pVdpVRAM[sprite_table_address + sprite];
        if (sprite_y == 0xD0)
        {
            max_sprite = sprite - 1;
            break;
        }
    }

    for (int sprite = max_sprite; sprite >= 0; sprite--)
    {
        int sprite_y = m_pVdpVRAM[sprite_table_address + sprite] + 1;
        if ((sprite_y > line) || ((sprite_y + sprite_height) <= line))
            continue;

        sprite_count++;
        if (sprite_count > 8)
            m_VdpStatus = SetBit(m_VdpStatus, 6);

        u16 sprite_info_address = sprite_table_address_2 + (sprite << 1);
        int sprite_x = m_pVdpVRAM[sprite_info_address] - sprite_shift;
        if (sprite_x >= GS_SMS_WIDTH)
            continue;

        int sprite_tile = m_pVdpVRAM[sprite_info_address + 1];
        int sprite_tile_addr = sprite_tiles_address + (sprite_tile << 5) + ((line - sprite_y) << 2);

        for (int tile_x = 0; tile_x < 8; tile_x++)
        {
            int sprite_pixel_x = sprite_x + tile_x;
            if (sprite_pixel_x >= GS_SMS_WIDTH)
                break;
            if (sprite_pixel_x < 0)
                continue;
            if (IsSetBit(m_VdpRegister[0], 5) && (sprite_pixel_x < 8))
                continue;

            int pixel = scy_256 + sprite_pixel_x;

            if (m_pInfoBuffer[pixel] & 0x02)
                continue;

            int tile_pixel_x = 7 - tile_x;

            int palette_color = ((m_pVdpVRAM[sprite_tile_addr] >> tile_pixel_x) & 0x01) +
                    (((m_pVdpVRAM[sprite_tile_addr + 1] >> tile_pixel_x) & 0x01) << 1) +
                    (((m_pVdpVRAM[sprite_tile_addr + 2] >> tile_pixel_x) & 0x01) << 2) +
                    (((m_pVdpVRAM[sprite_tile_addr + 3] >> tile_pixel_x) & 0x01) << 3);
            if (palette_color == 0)
                continue;

            palette_color += 16;

            if (m_bGameGear && ((sprite_pixel_x < GS_GG_X_OFFSET) || (sprite_pixel_x >= (GS_GG_X_OFFSET + GS_GG_WIDTH)) || (scy < GS_GG_Y_OFFSET) || (scy >= (GS_GG_Y_OFFSET + GS_GG_HEIGHT))))
            {
                GS_Color final_color;
                final_color.red = 0;
                final_color.green = 0;
                final_color.blue = 0;
                final_color.alpha = 0xFF;
                m_pColorFrameBuffer[pixel] = final_color;
            }
            else
                m_pColorFrameBuffer[pixel] = ConvertTo8BitColor(palette_color);

            if ((m_pInfoBuffer[pixel] & 0x04) != 0)
                sprite_collision = true;
            else
                m_pInfoBuffer[pixel] |= 0x04;
        }
    }

    if (sprite_collision)
        m_VdpStatus = SetBit(m_VdpStatus, 5);
}

