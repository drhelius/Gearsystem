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
#include "Cartridge.h"

Video::Video(Memory* pMemory, Processor* pProcessor, Cartridge* pCartridge)
{
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    m_pCartridge = pCartridge;
    InitPointer(m_pInfoBuffer);
    InitPointer(m_pFrameBuffer);
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
    m_Phaser.x = 0;
    m_Phaser.y = 0;
    m_Phaser.enabled = false;
    m_Phaser.detected = false;
    m_LineEvents.hint = false;
    m_LineEvents.scrollx = false;
    m_LineEvents.vcounter = false;
    m_LineEvents.vint = false;
    m_LineEvents.vintFlag = false;
    m_bExtendedMode224 = false;
    m_iRenderLine = 0;
    m_iScreenWidth = 0;
    m_bTMS9918 = false;
    m_iTMS9918Mode = 0;
    m_bDisplayEnabled = false;
    m_bSpriteOvrRequest = false;
    m_Overscan = OverscanDisabled;
    m_HideLeftBar = HideLeftBarNo;
    m_iHideLeftBarOffset = 0;
}

Video::~Video()
{
    SafeDeleteArray(m_pInfoBuffer);
    SafeDeleteArray(m_pFrameBuffer);
    SafeDeleteArray(m_pVdpVRAM);
    SafeDeleteArray(m_pVdpCRAM);
}

void Video::Init()
{
    m_pFrameBuffer = new u16[GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN];
    m_pInfoBuffer = new u8[GS_RESOLUTION_MAX_WIDTH * GS_LINES_PER_FRAME_PAL];
    m_pVdpVRAM = new u8[0x4000];
    m_pVdpCRAM = new u8[0x40];
    InitPalettes(kSG1000_palette_888_normal,
        m_SG1000_palette_565_rgb_normal,
        m_SG1000_palette_555_rgb_normal,
        m_SG1000_palette_565_bgr_normal,
        m_SG1000_palette_555_bgr_normal);
    InitPalettes(kSG1000_palette_888_sms,
        m_SG1000_palette_565_rgb_sms,
        m_SG1000_palette_555_rgb_sms,
        m_SG1000_palette_565_bgr_sms,
        m_SG1000_palette_555_bgr_sms);
    Reset(false, false);
}

void Video::Reset(bool bGameGear, bool bPAL)
{
    m_bGameGear = bGameGear;
    m_bPAL = bPAL;
    m_iLinesPerFrame = bPAL ? GS_LINES_PER_FRAME_PAL : GS_LINES_PER_FRAME_NTSC;
    m_bFirstByteInSequence = true;
    m_VdpBuffer = 0;
    m_iVCounter = m_iLinesPerFrame - 1;
    m_iHCounter = 0;
    m_VdpCode = 0;
    m_VdpBuffer = 0;
    m_VdpAddress = 0;
    m_VdpStatus = 0;
    m_ScrollX = 0;
    m_ScrollY = 0;

    for (int i = 0; i < (GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN); i++)
        m_pFrameBuffer[i] = 0;
    for (int i = 0; i < (GS_RESOLUTION_MAX_WIDTH * GS_LINES_PER_FRAME_PAL); i++)
        m_pInfoBuffer[i] = 0;
    for (int i = 0; i < 0x4000; i++)
        m_pVdpVRAM[i] = 0;
    for (int i = 0; i < 0x40; i++)
        m_pVdpCRAM[i] = 0;

    m_VdpRegister[0] = 0x36;
    m_VdpRegister[1] = (m_pCartridge->GetFeatures() & GS_DB_FEATURE_INITIAL_VINT) ? 0xA0 : 0x80;
    m_VdpRegister[2] = 0xFF; // Screen Map Table Base
    m_VdpRegister[3] = 0xFF; // Always $FF
    m_VdpRegister[4] = 0xFF; // Always $FF
    m_VdpRegister[5] = 0xFF; // Sprite Table Base
    m_VdpRegister[6] = 0xFB; // Sprite Pattern Table Base
    m_VdpRegister[7] = 0x00; // Border color #0
    m_VdpRegister[8] = 0x00; // Scroll-X
    m_VdpRegister[9] = 0x00; // Scroll-Y
    m_VdpRegister[10] = 0xFF; // H-line interrupt ($FF=OFF)

    m_bDisplayEnabled = false;
    m_bSpriteOvrRequest = false;

    for (int i = 11; i < 16; i++)
        m_VdpRegister[i] = 0;

    m_bExtendedMode224 = false;

    m_Phaser.x = 0;
    m_Phaser.y = 0;
    m_Phaser.enabled = false;
    m_Phaser.detected = false;

    m_LineEvents.hint = false;
    m_LineEvents.scrollx = false;
    m_LineEvents.vcounter = false;
    m_LineEvents.vint = false;
    m_LineEvents.vintFlag = false;
    m_LineEvents.render = false;

    m_iCycleCounter = 0;
    m_iVdpRegister10Counter = m_VdpRegister[10];
    m_iRenderLine = 0;

    m_iScreenWidth = m_bGameGear ? GS_RESOLUTION_GG_WIDTH : GS_RESOLUTION_SMS_WIDTH;

    m_bTMS9918 = false;
    m_iTMS9918Mode = 0;

    if (m_bGameGear)
    {
        m_Timing[TIMING_VINT] = 27;
        m_Timing[TIMING_XSCROLL] = 16;
        m_Timing[TIMING_HINT] = 30;
        m_Timing[TIMING_VCOUNT] = 28;
        m_Timing[TIMING_FLAG_VINT] = 27;
        m_Timing[TIMING_RENDER] = 186;
        m_Timing[TIMING_DISPLAY] = 20;
        m_Timing[TIMING_SPRITEOVR] = 27;
    }
    else
    {
        m_Timing[TIMING_VINT] = 25;
        m_Timing[TIMING_XSCROLL] = 14;
        m_Timing[TIMING_HINT] = 27;
        m_Timing[TIMING_VCOUNT] = 25;
        m_Timing[TIMING_FLAG_VINT] = 25;
        m_Timing[TIMING_RENDER] = 195;
        m_Timing[TIMING_DISPLAY] = 37;
        m_Timing[TIMING_SPRITEOVR] = 25;
    }

    for (int i = 0; i < 8; i++)
    {
        m_NextLineSprites[i] = -1;
    }
}

bool Video::Tick(unsigned int clockCycles)
{
    int max_height = m_bExtendedMode224 ? 224 : 192;
    bool return_vblank = false;

    m_iCycleCounter += clockCycles;

    ///// PHASER /////
    if (m_Phaser.enabled && !m_Phaser.detected)
    {
        CheckPhaser();
    }

    ///// VINT /////
    if (!m_LineEvents.vint && (m_iCycleCounter >= m_Timing[TIMING_VINT]))
    {
        m_LineEvents.vint = true;
        if ((m_iRenderLine == (max_height + 1)) && (IsSetBit(m_VdpRegister[1], 5)))
            m_pProcessor->RequestINT(true);
    }

    ///// DISPLAY ON/OFF /////
    if (!m_LineEvents.display && (m_iCycleCounter >= m_Timing[TIMING_DISPLAY]))
    {
        m_LineEvents.display = true;
        m_bDisplayEnabled = IsSetBit(m_VdpRegister[1], 6);
    }

    ///// SCROLLX /////
    if (!m_LineEvents.scrollx && (m_iCycleCounter >= m_Timing[TIMING_XSCROLL]))
    {
        m_LineEvents.scrollx = true;
        m_ScrollX = m_VdpRegister[8];   // latch scroll X
    }

    ///// HINT /////
    if (!m_LineEvents.hint && (m_iCycleCounter >= m_Timing[TIMING_HINT]))
    {
        m_LineEvents.hint = true;
        if (m_iRenderLine <= max_height)
        {
            if (m_iVdpRegister10Counter == 0)
            {
                m_iVdpRegister10Counter = m_VdpRegister[10];
                if (!m_bTMS9918 && IsSetBit(m_VdpRegister[0], 4))
                    m_pProcessor->RequestINT(true);
            }
            else
            {
                m_iVdpRegister10Counter--;
            }
        }
        else
            m_iVdpRegister10Counter = m_VdpRegister[10];
    }

    ///// VCOUNT /////
    if (!m_LineEvents.vcounter && (m_iCycleCounter >= m_Timing[TIMING_VCOUNT]))
    {
        m_LineEvents.vcounter = true;
        m_iVCounter++;
        if (m_iVCounter >= m_iLinesPerFrame)
        {
            m_ScrollY = m_VdpRegister[9];   // latch scroll Y
            m_iVCounter = 0;
        }
    }

    ///// FLAG VINT /////
    if (!m_LineEvents.vintFlag && (m_iCycleCounter >= m_Timing[TIMING_FLAG_VINT]))
    {
        m_LineEvents.vintFlag = true;
        if (m_iRenderLine == (max_height + 1))
            m_VdpStatus = SetBit(m_VdpStatus, 7);
    }

    ///// SPRITE OVR /////
    if (!m_LineEvents.spriteovr && (m_iCycleCounter >= m_Timing[TIMING_SPRITEOVR]) && !m_bTMS9918)
    {
        m_LineEvents.spriteovr = true;

        if (m_bSpriteOvrRequest)
        {
            m_bSpriteOvrRequest = false;
            m_VdpStatus = SetBit(m_VdpStatus, 6);
        }
    }

    ///// RENDER /////
    if (!m_LineEvents.render && (m_iCycleCounter >= m_Timing[TIMING_RENDER]))
    {
        if (m_iRenderLine == 0)
        {
            m_iHideLeftBarOffset = 0;

            if ((m_HideLeftBar != HideLeftBarNo) && (!m_bGameGear) && (!m_bTMS9918) && ((m_Overscan == OverscanDisabled) || (m_Overscan == OverscanTopBottom)))
            {
                if (m_HideLeftBar == HideLeftBarAlways)
                    m_iHideLeftBarOffset = 8;
                else if (m_HideLeftBar == HideLeftBarAuto)
                    m_iHideLeftBarOffset = IsSetBit(m_VdpRegister[0], 5) ? 8 : 0;
            }
        }

        m_LineEvents.render = true;
        ScanLine(m_iRenderLine);
    }

    ///// END OF LINE /////
    if (m_iCycleCounter >= GS_CYCLES_PER_LINE)
    {
        if (m_iRenderLine == (max_height - 1))
        {
            return_vblank = true;
        }
        m_iRenderLine++;
        m_iRenderLine %= m_iLinesPerFrame;
        m_iCycleCounter -= GS_CYCLES_PER_LINE;
        m_LineEvents.hint = false;
        m_LineEvents.scrollx = false;
        m_LineEvents.vcounter = false;
        m_LineEvents.vint = false;
        m_LineEvents.vintFlag = false;
        m_LineEvents.render = false;
        m_LineEvents.display = false;
        m_LineEvents.spriteovr = false;
        m_Phaser.detected = false;
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
    m_VdpAddress = (m_VdpAddress + 1) & 0x3FFF;
    return ret;
}

u8 Video::GetStatusFlags()
{
    u8 ret = m_VdpStatus | (m_bTMS9918 ? 0 : 0x1F);
    m_bFirstByteInSequence = true;
    m_VdpStatus = 0x00;
    m_pProcessor->RequestINT(false);
    return ret;
}

bool Video::IsExtendedMode224()
{
    return m_bExtendedMode224;
}

bool Video::IsSG1000Mode()
{
    return m_bTMS9918;
}

void Video::WriteData(u8 data)
{
    m_bFirstByteInSequence = true;
    m_VdpBuffer = data;

    if (m_VdpCode == 0x03)
        m_pVdpCRAM[m_VdpAddress & (m_bGameGear ? 0x3F : 0x1F)] = data;
    else
        m_pVdpVRAM[m_VdpAddress] = data;

    m_VdpAddress = (m_VdpAddress + 1) & 0x3FFF;
}

void Video::WriteControl(u8 data)
{
    if (m_bFirstByteInSequence)
    {
        m_bFirstByteInSequence = false;
        m_VdpAddress = (m_VdpAddress & 0x3F00) | data;
    }
    else
    {
        m_bFirstByteInSequence = true;

        m_VdpCode = (data >> 6) & 0x03;
        m_VdpAddress = ((data & 0x3F) << 8) | (m_VdpAddress & 0x00FF);

        switch (data & 0xC0)
        {
            case 0x00:
            {
                m_VdpBuffer = m_pVdpVRAM[m_VdpAddress];
                m_VdpAddress = (m_VdpAddress + 1) & 0x3FFF;
                break;
            }
            case 0x80:
            {
                u8 reg = data & 0x0F;
                m_VdpRegister[reg] = (m_VdpAddress & 0x00FF);

                if (reg < 2)
                {
                    m_bExtendedMode224 = ((m_VdpRegister[0] & 0x06) == 0x06) && ((m_VdpRegister[1] & 0x18) == 0x10);
                    m_iTMS9918Mode = CalculateVideoMode();
                    m_bTMS9918 = !m_bGameGear && (m_iTMS9918Mode != 4);
                }
                else if (reg > 10)
                {
                    Debug("--> ** Attempting to write on VDP REG %d: %X", reg, data);
                }
                break;
            }
        }
    }
}

void Video::ScanLine(int line)
{
    int max_height = m_bExtendedMode224 ? 224 : 192;
    int next_line = line + 1;
    next_line %= m_iLinesPerFrame;

    if (!m_bTMS9918)
    {
        ParseSpritesSMSGG(next_line);
    }

    if (m_bDisplayEnabled)
    {
        // DISPLAY ON
        if (m_bTMS9918)
        {
            if (line < max_height)
            {
                RenderBackgroundTMS9918(line);
                RenderSpritesTMS9918(line);
            }
        }
        else
        {
            RenderBackgroundSMSGG(line);
            RenderSpritesSMSGG(next_line);
        }
    }
    else
    {
        // DISPLAY OFF
        if (line < max_height)
        {
            u16 color = 0;
            if (m_bTMS9918)
                color = m_VdpRegister[7] & 0x0F;
            else
                color = ColorFromPalette((m_VdpRegister[7] & 0x0F) + 16);

            int line_width = line * m_iScreenWidth;

            for (int scx = 0; scx < m_iScreenWidth; scx++)
            {
                int pixel = line_width + scx;
                m_pFrameBuffer[pixel] = color;
                m_pInfoBuffer[pixel] = 0;
            }
        }
    }
}

void Video::RenderBackgroundSMSGG(int line)
{
    int y_offset = m_bExtendedMode224 ? GS_RESOLUTION_GG_Y_OFFSET_EXTENDED : GS_RESOLUTION_GG_Y_OFFSET;
    int scy_adjust = m_bGameGear ? y_offset : 0;
    int scy = line;
    int line_width_info = line * (m_iScreenWidth - m_iHideLeftBarOffset);
    int line_width_screen = (line - scy_adjust) * (m_iScreenWidth - m_iHideLeftBarOffset);
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

    int scx_begin = m_bGameGear ? GS_RESOLUTION_GG_X_OFFSET : m_iHideLeftBarOffset;
    int scx_end = scx_begin + m_iScreenWidth - m_iHideLeftBarOffset;

    int max_height = m_bExtendedMode224 ? 224 : 192;

    for (int scx = scx_begin; scx < scx_end; scx++)
    {
        int scx_diff = scx - scx_begin;
        int pixel_screen = line_width_screen + scx_diff;
        int pixel_info = line_width_info + scx_diff;

        if (line < max_height)
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

                bool final_priority = priority && ((palette_color - palette_offset) != 0);

                if ((m_pInfoBuffer[pixel_info] & 0x01) && !final_priority)
                {
                    m_pInfoBuffer[pixel_info] = 0;
                    continue;
                }
            }

            if (m_bGameGear)
            {
                if ((line >= y_offset) && (line < (y_offset + GS_RESOLUTION_GG_HEIGHT)))
                    m_pFrameBuffer[pixel_screen] = ColorFromPalette(palette_color);
            }
            else
            {
                m_pFrameBuffer[pixel_screen] = ColorFromPalette(palette_color);
            }
        }

        m_pInfoBuffer[pixel_info] = 0;
    }
}

void Video::ParseSpritesSMSGG(int line)
{
    u16 sprite_table_address = (m_VdpRegister[5] << 7) & 0x3F00;
    int buffer_index = 0;
    int max_height = m_bExtendedMode224 ? 224 : 192;

    for (int sprite = 0; sprite < 64; sprite++)
    {
        int sprite_index = sprite_table_address + sprite;

        if (!m_bExtendedMode224 && (m_pVdpVRAM[sprite_index] == 0xD0))
        {
            break;
        }

        int sprite_y = m_pVdpVRAM[sprite_index] + 1;
        int sprite_height = IsSetBit(m_VdpRegister[1], 1) ? 16 : 8;
        bool sprite_zoom = IsSetBit(m_VdpRegister[1], 0);
        if (sprite_zoom)
            sprite_height <<= 1;
        int sprite_y2 = sprite_y + sprite_height;

        int sprite_y_offscreen = ((sprite_y > 240) && (sprite_y <= 256)) ? sprite_y - 256 : sprite_y;
        int sprite_y_offscreen2 = sprite_y_offscreen + sprite_height;

        if (((line >= sprite_y) && (line < sprite_y2)) || ((line >= sprite_y_offscreen) && (line < sprite_y_offscreen2)))
        {
            if (buffer_index > 7)
            {
                if (line < max_height)
                    m_bSpriteOvrRequest = true;
                break;
            }

            m_NextLineSprites[buffer_index] = sprite;

            buffer_index++;
        }
    }

    for (int i = buffer_index; i < 8; i++)
    {
        m_NextLineSprites[i] = -1;
    }
}

void Video::RenderSpritesSMSGG(int line)
{
    int max_height = m_bExtendedMode224 ? 224 : 192;

    if ((line >= max_height) && (line < 240))
        return;

    int y_offset = m_bExtendedMode224 ? GS_RESOLUTION_GG_Y_OFFSET_EXTENDED : GS_RESOLUTION_GG_Y_OFFSET;
    u16 sprite_table_address = (m_VdpRegister[5] << 7) & 0x3F00;
    u16 sprite_table_address_2 = sprite_table_address + 0x80;
    int sprite_collision = false;
    int scy_adjust = m_bGameGear ? y_offset : 0;
    int line_width_info = line * (m_iScreenWidth - m_iHideLeftBarOffset);
    int line_width_screen = (line - scy_adjust) * (m_iScreenWidth - m_iHideLeftBarOffset);
    int sprite_width = 8;
    bool sprite_height_16 = IsSetBit(m_VdpRegister[1], 1);
    bool sprite_zoom = IsSetBit(m_VdpRegister[1], 0);
    if (sprite_zoom)
    {
        sprite_width <<= 1;
    }
    int sprite_shift = IsSetBit(m_VdpRegister[0], 3) ? 8 : 0;
    u16 sprite_tiles_address = (m_VdpRegister[6] << 11) & 0x2000;

    int scx_begin = m_bGameGear ? GS_RESOLUTION_GG_X_OFFSET : m_iHideLeftBarOffset;
    int scx_end = scx_begin + (m_iScreenWidth - m_iHideLeftBarOffset);

    for (int i = 7; i >= 0; i--)
    {
        if (m_NextLineSprites[i] < 0)
            continue;

        int sprite = m_NextLineSprites[i];

        u16 sprite_info_address = sprite_table_address_2 + (sprite << 1);
        int sprite_index = sprite_table_address + sprite;

        int sprite_y = m_pVdpVRAM[sprite_index] + 1;

        if ((sprite_y > 240) && (sprite_y <= 256) && (line < max_height))
        {
            sprite_y -= 256;
        }

        int sprite_x = m_pVdpVRAM[sprite_info_address] - sprite_shift;
        if (sprite_x >= GS_RESOLUTION_MAX_WIDTH)
            continue;

        int sprite_tile = m_pVdpVRAM[sprite_info_address + 1];
        sprite_tile &= sprite_height_16 ? 0xFE : 0xFF;
        int sprite_tile_addr = sprite_tiles_address + (sprite_tile << 5) +  (((line - sprite_y) >> (sprite_zoom ? 1 : 0)) << 2);

        for (int tile_x = 0; tile_x < sprite_width; tile_x++)
        {
            int sprite_pixel_x = sprite_x + tile_x;
            if (sprite_pixel_x >= scx_end)
                break;
            if (sprite_pixel_x < scx_begin)
                continue;
            if (IsSetBit(m_VdpRegister[0], 5) && (sprite_pixel_x < 8))
                continue;

            int x_diff = sprite_pixel_x - scx_begin;
            int pixel_screen = line_width_screen + x_diff;
            int pixel_info = line_width_info + x_diff;

            int tile_x_adjusted = tile_x >> (sprite_zoom ? 1 : 0);
            int tile_pixel_x = 0;

            if (tile_x_adjusted < 8)
                tile_pixel_x = 7 - tile_x_adjusted;
            else
                tile_pixel_x = 15 - tile_x_adjusted;

            int palette_color = ((m_pVdpVRAM[sprite_tile_addr] >> tile_pixel_x) & 0x01) +
                    (((m_pVdpVRAM[sprite_tile_addr + 1] >> tile_pixel_x) & 0x01) << 1) +
                    (((m_pVdpVRAM[sprite_tile_addr + 2] >> tile_pixel_x) & 0x01) << 2) +
                    (((m_pVdpVRAM[sprite_tile_addr + 3] >> tile_pixel_x) & 0x01) << 3);
            if (palette_color == 0)
                continue;

            palette_color += 16;

            if (m_bGameGear)
            {
                if ((line >= y_offset) && (line < (y_offset + GS_RESOLUTION_GG_HEIGHT)))
                    m_pFrameBuffer[pixel_screen] = ColorFromPalette(palette_color);
            }
            else
            {
                if (line < max_height)
                    m_pFrameBuffer[pixel_screen] = ColorFromPalette(palette_color);
            }

            if ((m_pInfoBuffer[pixel_info] & 0x01) != 0)
                sprite_collision = true;

            m_pInfoBuffer[pixel_info] |= 0x01;
        }
    }

    if (sprite_collision)
        m_VdpStatus = SetBit(m_VdpStatus, 5);
}

void Video::RenderBackgroundTMS9918(int line)
{
    int line_offset = line * m_iScreenWidth;

    int name_table_addr = m_VdpRegister[2] << 10;
    int color_table_addr = m_VdpRegister[3] << 6;
    int pattern_table_addr = m_VdpRegister[4] << 11;
    int region_mask = ((m_VdpRegister[4] & 0x03) << 8) | 0xFF;
    int color_mask = ((m_VdpRegister[3] & 0x7F) << 3) | 0x07;
    int backdrop_color = m_VdpRegister[7] & 0x0F;
    backdrop_color = (backdrop_color > 0) ? backdrop_color : 1;

    int tile_y = line >> 3;
    int tile_y_offset = line & 7;
    int region = 0;

    switch (m_iTMS9918Mode)
    {
        case 1:
        {
            int fg_color = (m_VdpRegister[7] >> 4) & 0x0F;
            int bg_color = backdrop_color;
            fg_color = (fg_color > 0) ? fg_color : backdrop_color;

            for (int i = 0; i < 8; i++)
            {
                int pixel = line_offset + i;
                m_pFrameBuffer[pixel] = bg_color;
                m_pFrameBuffer[pixel + 248] = bg_color;
                m_pInfoBuffer[pixel] = 0x00;
                m_pInfoBuffer[pixel + 248] = 0x00;
            }

            for (int tile_x = 0; tile_x < 40; tile_x++)
            {
                int tile_number = (tile_y * 40) + tile_x;
                int name_tile_addr = name_table_addr + tile_number;
                int name_tile = m_pVdpVRAM[name_tile_addr];
                u8 pattern_line = m_pVdpVRAM[pattern_table_addr + (name_tile << 3) + tile_y_offset];

                int screen_offset = line_offset + (tile_x * 6) + 8;

                for (int tile_pixel = 0; tile_pixel < 6; tile_pixel++)
                {
                    int pixel = screen_offset + tile_pixel;
                    m_pFrameBuffer[pixel] = IsSetBit(pattern_line, 7 - tile_pixel) ? fg_color : bg_color;
                    m_pInfoBuffer[pixel] = 0x00;
                }
            }
            return;
        }
        case 2:
        {
            pattern_table_addr &= 0x2000;
            color_table_addr &= 0x2000;
            region = (tile_y & 0x18) << 5;
            break;
        }
        case 3:
        {
            pattern_table_addr &= 0x2000;
            break;
        }
    }

    for (int tile_x = 0; tile_x < 32; tile_x++)
    {
        int tile_number = (tile_y << 5) + tile_x;
        int name_tile_addr = name_table_addr + tile_number;
        int name_tile = m_pVdpVRAM[name_tile_addr];
        u8 pattern_line = 0;
        u8 color_line = 0;

        if (m_iTMS9918Mode == 3)
        {
            int offset_color = pattern_table_addr + (name_tile << 3) + ((tile_y & 0x03) << 1) + (line & 0x04 ? 1 : 0);
            color_line = m_pVdpVRAM[offset_color];

            int left_color = color_line >> 4;
            int right_color = color_line & 0x0F;
            left_color = (left_color > 0) ? left_color : backdrop_color;
            right_color = (right_color > 0) ? right_color : backdrop_color;

            int screen_offset = line_offset + (tile_x << 3);

            for (int tile_pixel = 0; tile_pixel < 4; tile_pixel++)
            {
                int pixel = screen_offset + tile_pixel;
                m_pFrameBuffer[pixel] = left_color;
                m_pInfoBuffer[pixel] = 0x00;
            }

            for (int tile_pixel = 4; tile_pixel < 8; tile_pixel++)
            {
                int pixel = screen_offset + tile_pixel;
                m_pFrameBuffer[pixel] = right_color;
                m_pInfoBuffer[pixel] = 0x00;
            }

            continue;
        }
        else if (m_iTMS9918Mode == 0)
        {
            pattern_line = m_pVdpVRAM[pattern_table_addr + (name_tile << 3) + tile_y_offset];
            color_line = m_pVdpVRAM[color_table_addr + (name_tile >> 3)];
        }
        else if (m_iTMS9918Mode == 2)
        {
            name_tile += region;
            pattern_line = m_pVdpVRAM[pattern_table_addr + ((name_tile & region_mask) << 3) + tile_y_offset];
            color_line = m_pVdpVRAM[color_table_addr + ((name_tile & color_mask) << 3) + tile_y_offset];
        }

        int fg_color = color_line >> 4;
        int bg_color = color_line & 0x0F;
        fg_color = (fg_color > 0) ? fg_color : backdrop_color;
        bg_color = (bg_color > 0) ? bg_color : backdrop_color;

        int screen_offset = line_offset + (tile_x << 3);

        for (int tile_pixel = 0; tile_pixel < 8; tile_pixel++)
        {
            int pixel = screen_offset + tile_pixel;
            m_pFrameBuffer[pixel] = IsSetBit(pattern_line, 7 - tile_pixel) ? fg_color : bg_color;
            m_pInfoBuffer[pixel] = 0x00;
        }
    }
}

void Video::RenderSpritesTMS9918(int line)
{
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
        if (!IsSetBit(m_VdpStatus, 6) && (sprite_count > 4))
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
        sprite_tile &= IsSetBit(m_VdpRegister[1], 1) ? 0xFC : 0xFF;

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

            if (sprite_pixel && (sprite_count < 5))
            {
                if (!IsSetBit(m_pInfoBuffer[pixel], 0) && (sprite_color > 0))
                {
                    m_pFrameBuffer[pixel] = sprite_color;
                    m_pInfoBuffer[pixel] = SetBit(m_pInfoBuffer[pixel], 0);
                }

                if (IsSetBit(m_pInfoBuffer[pixel], 1))
                {
                     m_VdpStatus = SetBit(m_VdpStatus, 5);
                }
                else
                {
                    m_pInfoBuffer[pixel] = SetBit(m_pInfoBuffer[pixel], 1);
                }
            }
        }
    }
}

void Video::Render32bit(u16* srcFrameBuffer, u8* dstFrameBuffer, GS_Color_Format pixelFormat, int size, bool overscan)
{
    int x = 0;
    int y = 0;
    int overscan_h_l = 0;
    int overscan_v = 0;
    int overscan_content_v = 0;
    int overscan_content_h = 0;
    int overscan_total_width = GS_RESOLUTION_MAX_WIDTH - m_iHideLeftBarOffset;
    int overscan_total_height = 0;
    bool overscan_enabled = false;
    int overscan_color = m_bTMS9918 ? m_VdpRegister[7] & 0x0F : ColorFromPalette((m_VdpRegister[7] & 0x0F) + 16);
    int buffer_size = size * 4;    
    int shift_g = m_bGameGear ? 4 : 2;
    int shift_b = m_bGameGear ? 8 : 4;
    int mask = m_bGameGear ? 0x0F : 0x03;
    bool bgr = (pixelFormat == GS_PIXEL_BGRA8888);
    const u8* lut = m_bGameGear ? k4bitTo8bit : k2bitTo8bit;
    const u8* sg1000_palette = m_pCartridge->IsSG1000() ? kSG1000_palette_888_normal : kSG1000_palette_888_sms;

    if (m_bGameGear)
        overscan = false;

    if (overscan && (m_Overscan != OverscanDisabled))
    {
        overscan_enabled = true;
        overscan_content_v = m_bExtendedMode224 ? GS_RESOLUTION_SMS_HEIGHT_EXTENDED : GS_RESOLUTION_SMS_HEIGHT;
        overscan_v = m_bPAL ? GS_RESOLUTION_SMS_OVERSCAN_V_PAL : GS_RESOLUTION_SMS_OVERSCAN_V;
        overscan_v -= m_bExtendedMode224 ? 16 : 0;
        overscan_total_height = overscan_content_v + (overscan_v * 2);
    }

    if (overscan && (m_Overscan == OverscanFull320))
    {
        overscan_content_h = GS_RESOLUTION_MAX_WIDTH;
        overscan_h_l = GS_RESOLUTION_SMS_OVERSCAN_H_320_L;
        overscan_total_width = overscan_content_h + overscan_h_l + GS_RESOLUTION_SMS_OVERSCAN_H_320_R;
    }

    if (overscan && (m_Overscan == OverscanFull284))
    {
        overscan_content_h = GS_RESOLUTION_MAX_WIDTH;
        overscan_h_l = GS_RESOLUTION_SMS_OVERSCAN_H_284_L;
        overscan_total_width = overscan_content_h + overscan_h_l + GS_RESOLUTION_SMS_OVERSCAN_H_284_R;
    }

    for (int i = 0, j = 0; j < buffer_size; j += 4)
    {
        u16 src_color = 0;
        if (overscan_enabled)
        {
            bool is_h_overscan = (overscan_h_l > 0) && (x < overscan_h_l || x >= (overscan_h_l + overscan_content_h));
            bool is_v_overscan = (overscan_v > 0) && (y < overscan_v || y >= (overscan_v + overscan_content_v));

            if (is_h_overscan || is_v_overscan)
                src_color = m_bTMS9918 ? overscan_color * 3 : overscan_color;
            else
                src_color = m_bTMS9918 ? srcFrameBuffer[i++] * 3 : srcFrameBuffer[i++];

            if (++x == overscan_total_width)
            {
                x = 0;
                if (++y == overscan_total_height)
                {
                    y = 0;
                }
            }
        }
        else
            src_color = m_bTMS9918 ? srcFrameBuffer[i++] * 3 : srcFrameBuffer[i++];

        if (m_bTMS9918)
        {
            dstFrameBuffer[j + 0] = bgr ? sg1000_palette[src_color + 2] : sg1000_palette[src_color];
            dstFrameBuffer[j + 1] = sg1000_palette[src_color + 1];
            dstFrameBuffer[j + 2] = bgr ? sg1000_palette[src_color] : sg1000_palette[src_color + 2];
            dstFrameBuffer[j + 3] = 0xFF;
        }
        else
        {
            dstFrameBuffer[j + 0] = lut[bgr ? (src_color >> shift_b) & mask : src_color & mask];
            dstFrameBuffer[j + 1] = lut[(src_color >> shift_g) & mask];
            dstFrameBuffer[j + 2] = lut[bgr ? src_color & mask : (src_color >> shift_b) & mask];
            dstFrameBuffer[j + 3] = 0xFF;
        }
    }
}

void Video::Render16bit(u16* srcFrameBuffer, u8* dstFrameBuffer, GS_Color_Format pixelFormat, int size, bool overscan)
{
    int x = 0;
    int y = 0;
    int overscan_h_l = 0;
    int overscan_v = 0;
    int overscan_content_v = 0;
    int overscan_content_h = 0;
    int overscan_total_width = GS_RESOLUTION_MAX_WIDTH - m_iHideLeftBarOffset;
    int overscan_total_height = 0;
    bool overscan_enabled = false;
    int overscan_color = m_bTMS9918 ? m_VdpRegister[7] & 0x0F : ColorFromPalette((m_VdpRegister[7] & 0x0F) + 16);
    int buffer_size = size * 2;    
    int shift_g = m_bGameGear ? 4 : 2;
    int shift_b = m_bGameGear ? 8 : 4;
    int mask = m_bGameGear ? 0x0F : 0x03;
    bool bgr = ((pixelFormat == GS_PIXEL_BGR555) || (pixelFormat == GS_PIXEL_BGR565));
    bool green_6bit = (pixelFormat == GS_PIXEL_RGB565) || (pixelFormat == GS_PIXEL_BGR565);
    const u8* lut = m_bGameGear ? k4bitTo5bit : k2bitTo5bit;
    const u8* lut_g = m_bGameGear ? (green_6bit ? k4bitTo6bit : k4bitTo5bit) : (green_6bit ? k2bitTo6bit : k2bitTo5bit);
    int shift = green_6bit ? 11 : 10;
    const u16* pal;

    if (m_pCartridge->IsSG1000())
    {
        if (bgr)
            pal = green_6bit ? m_SG1000_palette_565_bgr_normal : m_SG1000_palette_555_bgr_normal;
        else
            pal = green_6bit ? m_SG1000_palette_565_rgb_normal : m_SG1000_palette_555_rgb_normal;
    }
    else
    {
        if (bgr)
            pal = green_6bit ? m_SG1000_palette_565_bgr_sms : m_SG1000_palette_555_bgr_sms;
        else
            pal = green_6bit ? m_SG1000_palette_565_rgb_sms : m_SG1000_palette_555_rgb_sms;
    }

    if (m_bGameGear)
        overscan = false;

    if (overscan && (m_Overscan != OverscanDisabled))
    {
        overscan_enabled = true;
        overscan_content_v = m_bExtendedMode224 ? GS_RESOLUTION_SMS_HEIGHT_EXTENDED : GS_RESOLUTION_SMS_HEIGHT;
        overscan_v = m_bPAL ? GS_RESOLUTION_SMS_OVERSCAN_V_PAL : GS_RESOLUTION_SMS_OVERSCAN_V;
        overscan_v -= m_bExtendedMode224 ? 16 : 0;
        overscan_total_height = overscan_content_v + (overscan_v * 2);
    }

    if (overscan && (m_Overscan == OverscanFull320))
    {
        overscan_content_h = GS_RESOLUTION_MAX_WIDTH;
        overscan_h_l = GS_RESOLUTION_SMS_OVERSCAN_H_320_L;
        overscan_total_width = overscan_content_h + overscan_h_l + GS_RESOLUTION_SMS_OVERSCAN_H_320_R;
    }

    if (overscan && (m_Overscan == OverscanFull284))
    {
        overscan_content_h = GS_RESOLUTION_MAX_WIDTH;
        overscan_h_l = GS_RESOLUTION_SMS_OVERSCAN_H_284_L;
        overscan_total_width = overscan_content_h + overscan_h_l + GS_RESOLUTION_SMS_OVERSCAN_H_284_R;
    }

    for (int i = 0, j = 0; j < buffer_size; j += 2)
    {
        u16 src_color = 0;
        if (overscan_enabled)
        {
            bool is_h_overscan = (overscan_h_l > 0) && (x < overscan_h_l || x >= (overscan_h_l + overscan_content_h));
            bool is_v_overscan = (overscan_v > 0) && (y < overscan_v || y >= (overscan_v + overscan_content_v));

            if (is_h_overscan || is_v_overscan)
                src_color = overscan_color;
            else
                src_color = srcFrameBuffer[i++];

            if (++x == overscan_total_width)
            {
                x = 0;
                if (++y == overscan_total_height)
                {
                    y = 0;
                }
            }
        }
        else
            src_color = srcFrameBuffer[i++];

        if (m_bTMS9918)
        {
            *(u16*)(&dstFrameBuffer[j]) = pal[src_color];
        }
        else
        {
            u8 red, green, blue;

            red = bgr ? (src_color >> shift_b) & mask : src_color & mask;
            green = (src_color >> shift_g) & mask;
            blue = bgr ? src_color & mask : (src_color >> shift_b) & mask;

            *(u16*)(&dstFrameBuffer[j]) = (lut[red] << shift) | (lut_g[green] << 5) | lut[blue];
        }
    }
}

void Video::SetOverscan(Overscan overscan)
{
    m_Overscan = overscan;
}

Video::Overscan Video::GetOverscan()
{
    return m_Overscan;
}

void Video::SetHideLeftBar(HideLeftBar hideLeftBar)
{
    m_HideLeftBar = hideLeftBar;
}

Video::HideLeftBar Video::GetHideLeftBar()
{
    return m_HideLeftBar;
}

int Video::GetHideLeftBarOffset()
{
    return m_iHideLeftBarOffset;
}

void Video::SetPhaserCoordinates(int x, int y)
{
    m_Phaser.x = x;
    m_Phaser.y = y;
    m_Phaser.enabled = true;
}

bool Video::IsPhaserDetected()
{
    return m_Phaser.detected;
}

void Video::InitPalettes(const u8* src, u16* dest_565_rgb, u16* dest_555_rgb, u16* dest_565_bgr, u16* dest_555_bgr)
{
    for (int i=0,j=0; i<16; i++,j+=3)
    {
        u8 red = src[j];
        u8 green = src[j+1];
        u8 blue = src[j+2];

        u8 red_5 = red * 31 / 255;
        u8 green_5 = green * 31 / 255;
        u8 green_6 = green * 63 / 255;
        u8 blue_5 = blue * 31 / 255;

        dest_565_rgb[i] = red_5 << 11 | green_6 << 5 | blue_5;
        dest_555_rgb[i] = red_5 << 10 | green_5 << 5 | blue_5;
        dest_565_bgr[i] = blue_5 << 11 | green_6 << 5 | red_5;
        dest_555_bgr[i] = blue_5 << 10 | green_5 << 5 | red_5;
    }
}

int Video::CalculateVideoMode()
{
    if (IsSetBit(m_VdpRegister[0], 2))
        return 4;
    else if (IsSetBit(m_VdpRegister[1], 4))
        return 1;
    else if (IsSetBit(m_VdpRegister[1], 3))
        return 3;
    else if (IsSetBit(m_VdpRegister[0], 1))
        return 2;
    else
        return 0;
}

void Video::CheckPhaser()
{
    int phaser_x_adj = (((m_Phaser.x + m_iHideLeftBarOffset) * 170) / 256) + 66;
    int phaser_y_adj = m_Phaser.y - 4;
    bool xmatch = abs(m_iCycleCounter - phaser_x_adj) <= 10;
    bool ymatch = abs(m_iRenderLine - phaser_y_adj) <= 4;

    if (xmatch && ymatch)
    {
        LatchHCounter();
        m_Phaser.detected = true;
    }
}

void Video::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_pInfoBuffer), GS_RESOLUTION_MAX_WIDTH * GS_LINES_PER_FRAME_PAL);
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
    bool bogus = false;
    stream.write(reinterpret_cast<const char*> (&bogus), sizeof(bogus));
    stream.write(reinterpret_cast<const char*> (&m_bExtendedMode224), sizeof(m_bExtendedMode224));
    stream.write(reinterpret_cast<const char*> (&m_LineEvents), sizeof(m_LineEvents));
    stream.write(reinterpret_cast<const char*> (&m_iRenderLine), sizeof(m_iRenderLine));

    stream.write(reinterpret_cast<const char*> (&m_bGameGear), sizeof(m_bGameGear));
    stream.write(reinterpret_cast<const char*> (&m_bPAL), sizeof(m_bPAL));
    stream.write(reinterpret_cast<const char*> (&m_iScreenWidth), sizeof(m_iScreenWidth));
    stream.write(reinterpret_cast<const char*> (&m_bTMS9918), sizeof(m_bTMS9918));
    stream.write(reinterpret_cast<const char*> (&m_iTMS9918Mode), sizeof(m_iTMS9918Mode));
    stream.write(reinterpret_cast<const char*> (&m_Timing), sizeof(m_Timing));
    stream.write(reinterpret_cast<const char*> (&m_NextLineSprites), sizeof(m_NextLineSprites));
    stream.write(reinterpret_cast<const char*> (&m_bDisplayEnabled), sizeof(m_bDisplayEnabled));
    stream.write(reinterpret_cast<const char*> (&m_bSpriteOvrRequest), sizeof(m_bSpriteOvrRequest));
    stream.write(reinterpret_cast<const char*> (&m_Phaser), sizeof(m_Phaser));
}

void Video::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pInfoBuffer), GS_RESOLUTION_MAX_WIDTH * GS_LINES_PER_FRAME_PAL);
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
    bool bogus;
    stream.read(reinterpret_cast<char*> (&bogus), sizeof(bogus));
    stream.read(reinterpret_cast<char*> (&m_bExtendedMode224), sizeof(m_bExtendedMode224));
    stream.read(reinterpret_cast<char*> (&m_LineEvents), sizeof(m_LineEvents));
    stream.read(reinterpret_cast<char*> (&m_iRenderLine), sizeof(m_iRenderLine));

    stream.read(reinterpret_cast<char*> (&m_bGameGear), sizeof(m_bGameGear));
    stream.read(reinterpret_cast<char*> (&m_bPAL), sizeof(m_bPAL));
    stream.read(reinterpret_cast<char*> (&m_iScreenWidth), sizeof(m_iScreenWidth));
    stream.read(reinterpret_cast<char*> (&m_bTMS9918), sizeof(m_bTMS9918));
    stream.read(reinterpret_cast<char*> (&m_iTMS9918Mode), sizeof(m_iTMS9918Mode));
    stream.read(reinterpret_cast<char*> (&m_Timing), sizeof(m_Timing));
    stream.read(reinterpret_cast<char*> (&m_NextLineSprites), sizeof(m_NextLineSprites));
    stream.read(reinterpret_cast<char*> (&m_bDisplayEnabled), sizeof(m_bDisplayEnabled));
    stream.read(reinterpret_cast<char*> (&m_bSpriteOvrRequest), sizeof(m_bSpriteOvrRequest));
    stream.read(reinterpret_cast<char*> (&m_Phaser), sizeof(m_Phaser));
}
