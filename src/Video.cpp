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
    for (int i = 0; i < 16; i++)
        m_VdpRegister[i] = 0;
    m_VdpCode = 0;
    m_VdpBuffer = 0;
    m_VdpAddress = 0;
    m_iVCounter = 0;
    m_iHCounter = 0;
    m_iCycleCounter = 0;
    m_VdpStatus = 0;
    m_iVdpRegister10Counter = 0;
    m_ScrollX = 0;
    m_ScrollY = 0;
    m_bGameGear = false;
    m_iLinesPerFrame = 0;
    m_bPAL = false;
    m_bVIntReached = false;
    m_bVCounterIncremented = false;
    m_bHIntReached = false;
    m_bScrollXLatched = false;
    m_bVIntFlagSet = false;
    m_bReg10CounterDecremented = false;
    m_bExtendedMode224 = false;
    m_iRenderLine = 0;
    m_iScreenWidth = 0;
    m_bSG1000 = false;
}

Video::~Video()
{
    SafeDeleteArray(m_pInfoBuffer);
    SafeDeleteArray(m_pVdpVRAM);
    SafeDeleteArray(m_pVdpCRAM);
}

void Video::Init()
{
    m_pInfoBuffer = new u8[GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT];
    m_pVdpVRAM = new u8[0x4000];
    m_pVdpCRAM = new u8[0x40];
    Reset(false, false);
}

void Video::Reset(bool bGameGear, bool bPAL)
{
    m_bGameGear = bGameGear;
    m_bPAL = bPAL;
    m_iLinesPerFrame = bPAL ? GS_LINES_PER_FRAME_PAL : GS_LINES_PER_FRAME_NTSC;
    m_bFirstByteInSequence = true;
    m_VdpBuffer = 0;
    m_iVCounter = 0;
    m_iHCounter = 0;
    m_VdpCode = 0;
    m_VdpBuffer = 0;
    m_VdpAddress = 0;
    m_VdpStatus = 0;
    m_bReg10CounterDecremented = false;
    m_ScrollX = 0;
    m_ScrollY = 0;
    for (int i = 0; i < (GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT); i++)
        m_pInfoBuffer[i] = 0;
    for (int i = 0; i < 0x4000; i++)
        m_pVdpVRAM[i] = 0;
    for (int i = 0; i < 0x40; i++)
        m_pVdpCRAM[i] = 0;

    m_VdpRegister[0] = 0x36; // Mode
    m_VdpRegister[1] = 0x80; // Mode
    m_VdpRegister[2] = 0xFF; // Screen Map Table Base
    m_VdpRegister[3] = 0xFF; // Always $FF
    m_VdpRegister[4] = 0xFF; // Always $FF
    m_VdpRegister[5] = 0xFF; // Sprite Table Base
    m_VdpRegister[6] = 0xFB; // Sprite Pattern Table Base
    m_VdpRegister[7] = 0x00; // Border color #0
    m_VdpRegister[8] = 0x00; // Scroll-X
    m_VdpRegister[9] = 0x00; // Scroll-Y
    m_VdpRegister[10] = 0xFF; // H-line interrupt ($FF=OFF)

    for (int i = 11; i < 16; i++)
        m_VdpRegister[i] = 0;

    m_bExtendedMode224 = false;
    m_bVIntReached = false;
    m_bVCounterIncremented = false;
    m_bHIntReached = false;
    m_bScrollXLatched = false;
    m_bVIntFlagSet = false;
    m_iCycleCounter = 0;
    m_iVCounter = m_iLinesPerFrame - 1;
    m_iVdpRegister10Counter = m_VdpRegister[10];
    m_iRenderLine = 0;

    m_iScreenWidth = m_bGameGear ? GS_RESOLUTION_GG_WIDTH : GS_RESOLUTION_SMS_WIDTH;

    m_bSG1000 = false;
}

bool Video::Tick(unsigned int clockCycles, GS_Color* pColorFrameBuffer)
{
    int max_height = m_bExtendedMode224 ? 224 : 192;
    bool return_vblank = false;
    m_pColorFrameBuffer = pColorFrameBuffer;

    m_iCycleCounter += clockCycles;

    ///// VINT /////
    if (!m_bVIntReached && (m_iCycleCounter >= 222))
    {
        m_bVIntReached = true;
        if ((m_iRenderLine == (max_height + 1)) && (IsSetBit(m_VdpRegister[1], 5)))
            m_pProcessor->RequestINT(true);
    }

    ///// XSCROLL /////
    if (!m_bScrollXLatched && (m_iCycleCounter >= 211))
    {
        m_bScrollXLatched = true;
        m_ScrollX = m_VdpRegister[8];   // latch scroll X
    }

    ///// HINT /////
    if (!m_bHIntReached && (m_iCycleCounter >= 224))
    {
        m_bHIntReached = true;
        if (m_iRenderLine <= max_height)
        {
            m_iVdpRegister10Counter--;
            if (m_iVdpRegister10Counter < 0)
            {
                m_iVdpRegister10Counter = m_VdpRegister[10];
                if (!m_bSG1000 && IsSetBit(m_VdpRegister[0], 4))
                    m_pProcessor->RequestINT(true);
            }
        }
        else
            m_iVdpRegister10Counter = m_VdpRegister[10];
    }

    ///// VCOUNT /////
    if (!m_bVCounterIncremented && (m_iCycleCounter >= 222))
    {
        m_bVCounterIncremented = true;
        m_iVCounter++;
        if (m_iVCounter >= m_iLinesPerFrame)
        {
            m_ScrollY = m_VdpRegister[9];   // latch scroll Y
            m_iVCounter = 0;
        }
    }

    ///// FLAG VINT /////
    if (!m_bVIntFlagSet && (m_iCycleCounter >= 222))
    {
        m_bVIntFlagSet = true;
        if (m_iRenderLine == (max_height + 1))
            m_VdpStatus = SetBit(m_VdpStatus, 7);
    }

    ///// RENDER LINE /////
    if (m_iCycleCounter >= GS_CYCLES_PER_LINE)
    {
        if ((m_iRenderLine < max_height) && IsValidPointer(m_pColorFrameBuffer))
        {
            ScanLine(m_iRenderLine);
        }
        else if (m_iRenderLine == max_height)
        {
            return_vblank = true;
        }
        m_iRenderLine++;
        m_iRenderLine %= m_iLinesPerFrame;
        m_iCycleCounter -= GS_CYCLES_PER_LINE;
        m_bVIntReached = false;
        m_bHIntReached = false;
        m_bVCounterIncremented = false;
        m_bScrollXLatched = false;
        m_bVIntFlagSet = false;
    }

    return return_vblank;
}

void Video::LatchHCounter()
{
    m_iHCounter = kVdpHCounter[m_iCycleCounter % 228];
}

u8 Video::GetVCounter()
{
    if (m_bPAL)
    {
        if (m_bExtendedMode224)
        {
            // 224 lines
            if (m_iVCounter > 0x102)
                return m_iVCounter - 0x39;
            else if (m_iVCounter > 0xFF)
                return m_iVCounter - 0x100;
            else
                return m_iVCounter;
        }
        else
        {
            // 192 lines
            if (m_iVCounter > 0xF2)
                return m_iVCounter - 0x39;
            else
                return m_iVCounter;
        }
    }
    else
    {
        // NTSC
        if (m_bExtendedMode224)
        {
            // 224 lines
            if (m_iVCounter > 0xEA)
                return m_iVCounter - 0x06;
            else
                return m_iVCounter;
        }
        else
        {
            // 192 lines
            if (m_iVCounter > 0xDA)
                return m_iVCounter - 0x06;
            else
                return m_iVCounter;
        }
    }
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
    u8 ret = m_VdpStatus | (m_bSG1000 ? 0 : 0x1F);
    m_bFirstByteInSequence = true;
    m_VdpStatus = 0x00;
    m_pProcessor->RequestINT(false);
    return ret;
}

bool Video::IsExtendedMode224()
{
    return m_bExtendedMode224;
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
        m_VdpAddress = (m_VdpAddress & 0xFF00) | control;
    }
    else
    {
        m_bFirstByteInSequence = true;
        m_VdpCode = (control >> 6) & 0x03;
        m_VdpAddress = (m_VdpAddress & 0x00FF) | ((control & 0x3F) << 8);

        if (m_bSG1000 && (m_VdpCode == VDP_WRITE_CRAM_OPERATION))
            Log("--> ** SG-1000 Attempting to write on CRAM");

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
                u8 reg = control & (m_bSG1000 ? 0x07 : 0x0F);
                m_VdpRegister[reg] = (m_VdpAddress & 0x00FF);

                if (reg < 2)
                {
                    m_bExtendedMode224 = ((m_VdpRegister[0] & 0x06) == 0x06) && ((m_VdpRegister[1] & 0x18) == 0x10);
                    m_bSG1000 = ((m_VdpRegister[0] & 0x06) == 0x02) && ((m_VdpRegister[1] & 0x18) == 0x00);

                    //Log("--> ** Video mode R0: $%X R1: $%X", m_VdpRegister[0] & 0x06, m_VdpRegister[1] & 0x18);
                }
                else if (reg > 10)
                {
                    Log("--> ** Attempting to write on VDP REG %d: %X", reg, control);
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
        if (m_bSG1000)
        {
            RenderBackgroundSG1000(line);
            RenderSpritesSG1000(line);
        }
        else
        {
            RenderBackgroundSMSGG(line);
            RenderSpritesSMSGG(line);
        }
    }
    else
    {
        // DISPLAY OFF
        int line_width = line * m_iScreenWidth;

        for (int scx = 0; scx < m_iScreenWidth; scx++)
        {
            GS_Color final_color;
            final_color.red = 0;
            final_color.green = 0;
            final_color.blue = 0;
            final_color.alpha = 0xFF;
            m_pColorFrameBuffer[line_width + scx] = final_color;
        }
    }
}

void Video::RenderBackgroundSMSGG(int line)
{
    if (m_bGameGear && ((line < GS_RESOLUTION_GG_Y_OFFSET) || (line >= (GS_RESOLUTION_GG_Y_OFFSET + GS_RESOLUTION_GG_HEIGHT))))
        return;

    int scy_adjust = m_bGameGear ? GS_RESOLUTION_GG_Y_OFFSET : 0;
    int scy = line;
    int line_width = (line - scy_adjust) * m_iScreenWidth;
    int origin_x = m_ScrollX;
    if ((line < 16) && IsSetBit(m_VdpRegister[0], 6))
        origin_x = 0;
    int origin_y = m_ScrollY;

    u16 map_address = (m_VdpRegister[2] & (m_bExtendedMode224 ? 0x0C : 0x0E)) << 10;
    int map_y = scy + origin_y;

    if (m_bExtendedMode224)
    {
        map_address |= 0x700;
        map_y &= 0xFF;
    }
    else if (map_y >= 224)
            map_y -= 224;

    int tile_y = map_y >> 3;
    int tile_y_offset = map_y & 7;

    int palette_color = 0;
    u8 info = 0;

    int scx_begin = m_bGameGear ? GS_RESOLUTION_GG_X_OFFSET : 0;
    int scx_end = scx_begin + m_iScreenWidth;

    for (int scx = scx_begin; scx < scx_end; scx++)
    {
        if (IsSetBit(m_VdpRegister[0], 5) && scx < 8)
        {
            palette_color = (m_VdpRegister[7] & 0x0F) + 16;
        }
        else
        {
            if (IsSetBit(m_VdpRegister[0], 7) && (scx >= 192))
            {
                map_y = scy;
                tile_y = map_y >> 3;
                tile_y_offset = map_y & 7;
            }
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

        int pixel = line_width + scx - scx_begin;

        m_pColorFrameBuffer[pixel] = ConvertTo8BitColor(palette_color);
        m_pInfoBuffer[pixel] = info;
    }
}

void Video::RenderSpritesSMSGG(int line)
{
    if (m_bGameGear && ((line < GS_RESOLUTION_GG_Y_OFFSET) || (line >= (GS_RESOLUTION_GG_Y_OFFSET + GS_RESOLUTION_GG_HEIGHT))))
        return;

    int sprite_collision = false;
    int sprite_count = 0;
    int scy_adjust = m_bGameGear ? GS_RESOLUTION_GG_Y_OFFSET : 0;
    int line_width = (line - scy_adjust) * m_iScreenWidth;
    int sprite_height = IsSetBit(m_VdpRegister[1], 1) ? 16 : 8;
    int sprite_shift = IsSetBit(m_VdpRegister[0], 3) ? 8 : 0;
    u16 sprite_table_address = (m_VdpRegister[5] << 7) & 0x3F00;
    u16 sprite_table_address_2 = sprite_table_address + 0x80;
    u16 sprite_tiles_address = (m_VdpRegister[6] << 11) & 0x2000;

    int scx_begin = m_bGameGear ? GS_RESOLUTION_GG_X_OFFSET : 0;
    int scx_end = scx_begin + m_iScreenWidth;

    int max_sprite = 63;

    for (int sprite = 0; sprite < 64; sprite++)
    {
        int sprite_y = m_pVdpVRAM[sprite_table_address + sprite];
        if (!m_bExtendedMode224 && (sprite_y == 0xD0))
        {
            max_sprite = sprite - 1;
            break;
        }
    }

    for (int sprite = max_sprite; sprite >= 0; sprite--)
    {
        u8 sprite_y = m_pVdpVRAM[sprite_table_address + sprite] + 1;
        if ((sprite_y > line) || ((sprite_y + sprite_height) <= line))
            continue;

        sprite_count++;
        if (sprite_count > 8)
            m_VdpStatus = SetBit(m_VdpStatus, 6);

        u16 sprite_info_address = sprite_table_address_2 + (sprite << 1);
        int sprite_x = m_pVdpVRAM[sprite_info_address] - sprite_shift;
        if (sprite_x >= GS_RESOLUTION_MAX_WIDTH)
            continue;

        int sprite_tile = m_pVdpVRAM[sprite_info_address + 1];
        sprite_tile &= (sprite_height == 16) ? 0xFE : 0xFF;
        int sprite_tile_addr = sprite_tiles_address + (sprite_tile << 5) + ((line - sprite_y) << 2);

        for (int tile_x = 0; tile_x < 8; tile_x++)
        {
            int sprite_pixel_x = sprite_x + tile_x;
            if (sprite_pixel_x >= scx_end)
                break;
            if (sprite_pixel_x < scx_begin)
                continue;
            if (IsSetBit(m_VdpRegister[0], 5) && (sprite_pixel_x < 8))
                continue;

            int pixel = line_width + sprite_pixel_x - scx_begin;

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

void Video::RenderBackgroundSG1000(int line)
{
    int line_width = line * m_iScreenWidth;

    int name_table_addr = (m_VdpRegister[2] & 0x0F) << 10;
    int pattern_table_addr = (m_VdpRegister[4] & 0x04) << 11;
    int color_table_addr = (m_VdpRegister[3] & 0x80) << 6;
    int region = (m_VdpRegister[4] & 0x03) << 8;
    int backdrop_color = m_VdpRegister[7] & 0x0F;

    int tile_y = line >> 3;
    int tile_y_offset = line & 7;

    for (int scx = 0; scx < m_iScreenWidth; scx++)
    {
        int tile_x = scx >> 3;
        int tile_x_offset = scx & 7;

        int tile_number = ((tile_y << 5) + tile_x);

        int name_tile_addr = name_table_addr + tile_number;

        int name_tile = m_pVdpVRAM[name_tile_addr] | (region & 0x300 & tile_number);

        u8 pattern_line = m_pVdpVRAM[pattern_table_addr + (name_tile << 3) + tile_y_offset];

        u8 color_line = m_pVdpVRAM[color_table_addr + (name_tile << 3) + tile_y_offset];

        int bg_color = color_line & 0x0F;
        int fg_color = color_line >> 4;

        int pixel = line_width + scx;

        int final_color = IsSetBit(pattern_line, 7 - tile_x_offset) ? fg_color : bg_color;

        m_pColorFrameBuffer[pixel] = kSG1000_palette[(final_color > 0) ? final_color : backdrop_color];
        m_pInfoBuffer[pixel] = 0x00;
    }
}

void Video::RenderSpritesSG1000(int line)
{
    int sprite_collision = false;
    int sprite_count = 0;
    int line_width = line * m_iScreenWidth;
    int sprite_size = IsSetBit(m_VdpRegister[1], 1) ? 16 : 8;
    bool sprite_zoom = IsSetBit(m_VdpRegister[1], 0);
    if (sprite_zoom)
        sprite_size *= 2;
    u16 sprite_attribute_addr = (m_VdpRegister[5] & 0x7F) << 7;
    u16 sprite_pattern_addr = (m_VdpRegister[6] & 0x07) << 11;

    int max_sprite = 31;

    for (int sprite = 0; sprite <= max_sprite; sprite++)
    {
        if (m_pVdpVRAM[sprite_attribute_addr + (sprite << 2)] == 0xD0)
        {
            max_sprite = sprite - 1;
            break;
        }
    }

    for (int sprite = 0; sprite <= max_sprite; sprite++)
    {
        int sprite_attribute_offset = sprite_attribute_addr + (sprite << 2);
        int sprite_y = (m_pVdpVRAM[sprite_attribute_offset] + 1) & 0xFF;

        if (sprite_y >= 0xE0)
            sprite_y = -(0x100 - sprite_y);

        if ((sprite_y > line) || ((sprite_y + sprite_size) <= line))
            continue;

        sprite_count++;
        if (!SetBit(m_VdpStatus, 6) && (sprite_count > 4))
        {
            m_VdpStatus = SetBit(m_VdpStatus, 6);
            m_VdpStatus = (m_VdpStatus & 0xE0) | sprite;
        }

        int sprite_color = m_pVdpVRAM[sprite_attribute_offset + 3] & 0x0F;

        if (sprite_color == 0)
            continue;

        int sprite_shift = (m_pVdpVRAM[sprite_attribute_offset + 3] & 0x80) ? 32 : 0;
        int sprite_x = m_pVdpVRAM[sprite_attribute_offset + 1] - sprite_shift;

        if (sprite_x >= GS_RESOLUTION_MAX_WIDTH)
            continue;

        int sprite_tile = m_pVdpVRAM[sprite_attribute_offset + 2];
        int sprite_line_addr = sprite_pattern_addr + (sprite_tile << 3) + ((line - sprite_y ) >> (sprite_zoom ? 1 : 0));

        for (int tile_x = 0; tile_x < sprite_size; tile_x++)
        {
            int sprite_pixel_x = sprite_x + tile_x;
            if (sprite_pixel_x >= m_iScreenWidth)
                break;
            if (sprite_pixel_x < 0)
                continue;

            int pixel = line_width + sprite_pixel_x;

            bool sprite_pixel = false;

            int tile_x_adjusted = tile_x >> (sprite_zoom ? 1 : 0);

            if (tile_x_adjusted < 8)
                sprite_pixel = IsSetBit(m_pVdpVRAM[sprite_line_addr], 7 - tile_x_adjusted);
            else
                sprite_pixel = IsSetBit(m_pVdpVRAM[sprite_line_addr + 16], 15 - tile_x_adjusted);

            if (sprite_pixel && (sprite_count < 5) && ((m_pInfoBuffer[pixel] & 0x08) == 0))
            {
                m_pColorFrameBuffer[pixel] = kSG1000_palette[sprite_color];
                m_pInfoBuffer[pixel] |= 0x08;
            }

            if ((m_pInfoBuffer[pixel] & 0x04) != 0)
                sprite_collision = true;
            else
                m_pInfoBuffer[pixel] |= 0x04;
        }
    }

    if (sprite_collision)
        m_VdpStatus = SetBit(m_VdpStatus, 5);
}

void Video::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_pInfoBuffer), GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT);
    stream.write(reinterpret_cast<const char*> (m_pVdpVRAM), 0x4000);
    stream.write(reinterpret_cast<const char*> (m_pVdpCRAM), 0x40);
    stream.write(reinterpret_cast<const char*> (&m_bFirstByteInSequence), sizeof(m_bFirstByteInSequence));
    stream.write(reinterpret_cast<const char*> (m_VdpRegister), sizeof(m_VdpRegister));
    stream.write(reinterpret_cast<const char*> (&m_VdpCode), sizeof(m_VdpCode));
    stream.write(reinterpret_cast<const char*> (&m_VdpBuffer), sizeof(m_VdpBuffer));
    stream.write(reinterpret_cast<const char*> (&m_VdpAddress), sizeof(m_VdpAddress));
    stream.write(reinterpret_cast<const char*> (&m_iVCounter), sizeof(m_iVCounter));
    stream.write(reinterpret_cast<const char*> (&m_iHCounter), sizeof(m_iHCounter));
    stream.write(reinterpret_cast<const char*> (&m_iCycleCounter), sizeof(m_iCycleCounter));
    stream.write(reinterpret_cast<const char*> (&m_VdpStatus), sizeof(m_VdpStatus));
    stream.write(reinterpret_cast<const char*> (&m_iVdpRegister10Counter), sizeof(m_iVdpRegister10Counter));
    stream.write(reinterpret_cast<const char*> (&m_ScrollX), sizeof(m_ScrollX));
    stream.write(reinterpret_cast<const char*> (&m_ScrollY), sizeof(m_ScrollY));
    stream.write(reinterpret_cast<const char*> (&m_iLinesPerFrame), sizeof(m_iLinesPerFrame));
    stream.write(reinterpret_cast<const char*> (&m_bReg10CounterDecremented), sizeof(m_bReg10CounterDecremented));
    stream.write(reinterpret_cast<const char*> (&m_bExtendedMode224), sizeof(m_bExtendedMode224));
    stream.write(reinterpret_cast<const char*> (&m_bVIntFlagSet), sizeof(m_bVIntFlagSet));
    stream.write(reinterpret_cast<const char*> (&m_bVIntReached), sizeof(m_bVIntReached));
    stream.write(reinterpret_cast<const char*> (&m_bHIntReached), sizeof(m_bHIntReached));
    stream.write(reinterpret_cast<const char*> (&m_bScrollXLatched), sizeof(m_bScrollXLatched));
    stream.write(reinterpret_cast<const char*> (&m_bVCounterIncremented), sizeof(m_bVCounterIncremented));
    stream.write(reinterpret_cast<const char*> (&m_iRenderLine), sizeof(m_iRenderLine));
}

void Video::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pInfoBuffer), GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT);
    stream.read(reinterpret_cast<char*> (m_pVdpVRAM), 0x4000);
    stream.read(reinterpret_cast<char*> (m_pVdpCRAM), 0x40);
    stream.read(reinterpret_cast<char*> (&m_bFirstByteInSequence), sizeof(m_bFirstByteInSequence));
    stream.read(reinterpret_cast<char*> (m_VdpRegister), sizeof(m_VdpRegister));
    stream.read(reinterpret_cast<char*> (&m_VdpCode), sizeof(m_VdpCode));
    stream.read(reinterpret_cast<char*> (&m_VdpBuffer), sizeof(m_VdpBuffer));
    stream.read(reinterpret_cast<char*> (&m_VdpAddress), sizeof(m_VdpAddress));
    stream.read(reinterpret_cast<char*> (&m_iVCounter), sizeof(m_iVCounter));
    stream.read(reinterpret_cast<char*> (&m_iHCounter), sizeof(m_iHCounter));
    stream.read(reinterpret_cast<char*> (&m_iCycleCounter), sizeof(m_iCycleCounter));
    stream.read(reinterpret_cast<char*> (&m_VdpStatus), sizeof(m_VdpStatus));
    stream.read(reinterpret_cast<char*> (&m_iVdpRegister10Counter), sizeof(m_iVdpRegister10Counter));
    stream.read(reinterpret_cast<char*> (&m_ScrollX), sizeof(m_ScrollX));
    stream.read(reinterpret_cast<char*> (&m_ScrollY), sizeof(m_ScrollY));
    stream.read(reinterpret_cast<char*> (&m_iLinesPerFrame), sizeof(m_iLinesPerFrame));
    stream.read(reinterpret_cast<char*> (&m_bReg10CounterDecremented), sizeof(m_bReg10CounterDecremented));
    stream.read(reinterpret_cast<char*> (&m_bExtendedMode224), sizeof(m_bExtendedMode224));
    stream.read(reinterpret_cast<char*> (&m_bVIntFlagSet), sizeof(m_bVIntFlagSet));
    stream.read(reinterpret_cast<char*> (&m_bVIntReached), sizeof(m_bVIntReached));
    stream.read(reinterpret_cast<char*> (&m_bHIntReached), sizeof(m_bHIntReached));
    stream.read(reinterpret_cast<char*> (&m_bScrollXLatched), sizeof(m_bScrollXLatched));
    stream.read(reinterpret_cast<char*> (&m_bVCounterIncremented), sizeof(m_bVCounterIncremented));
    stream.read(reinterpret_cast<char*> (&m_iRenderLine), sizeof(m_iRenderLine));
}
