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
    m_vVBlankInterrupt = false;
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
    m_vVBlankInterrupt = false;
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
    
    m_iCycleCounter += clockCycles;

    if(m_iCycleCounter >= GS_CYCLES_PER_LINE_NTSC)
    {
        ScanLine(m_iVCounter);

        m_iCycleCounter -= GS_CYCLES_PER_LINE_NTSC;
        m_iVCounter++;
        
        if (m_iVCounter == 192)
        {
            m_VdpStatus = SetBit(m_VdpStatus, 7);
            m_vVBlankInterrupt = true;
        }
        
        if (m_vVBlankInterrupt && IsSetBit(m_VdpRegister[1], 5) && (m_iVCounter >= 192) && (m_iVCounter < 223))
        {
            m_pProcessor->RequestINT(true);
        }

        if (m_iVCounter >= GS_LINES_PER_FRAME_NTSC)
        {
            m_iVCounter = 0;
            vblank = true;
        }
    }

    m_pColorFrameBuffer = pColorFrameBuffer;

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
    m_vVBlankInterrupt = false;
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
    RenderBG(line);
}

void Video::RenderBG(int line)
{
    if (line >= 192)
        return;
    // 32x24 tiles on screen (256x192 pixels)
    u16 map_address = 0x3800;//(m_VdpRegister[2] << 10) & 0x3800;
    u16 tile_address = 0;//(m_VdpRegister[6] << 11) & 0x2000;

    int tile_y = line / 8;
    int tile_y_offset = line % 8;
    
    int pixel_cur = 0;
    for (int tile_x = 0; tile_x < 32; tile_x++)
    {
        int tile_cur_addr = (((tile_y * 32) + tile_x) * 2) + map_address;
        int tile_cur = (m_pVdpVRAM[tile_cur_addr] & 0xFF) + ((m_pVdpVRAM[tile_cur_addr + 1] & 0x01) << 8);
        
        bool second_pallete = IsSetBit(m_pVdpVRAM[tile_cur_addr + 1], 3);
        
        int tile_data_addr = tile_address + (tile_cur * 32);
        int tile_data_offset = tile_data_addr + (tile_y_offset * 4);
        
        for (int pixel_x = 0; pixel_x < 8; pixel_x++)
        {
            int pixel = ((m_pVdpVRAM[tile_data_offset] >> (7-pixel_x)) & 0x01) +
            (((m_pVdpVRAM[tile_data_offset + 1] >> (7-pixel_x)) & 0x01) << 1) +
            (((m_pVdpVRAM[tile_data_offset + 2] >> (7-pixel_x)) & 0x01) << 2) +
            (((m_pVdpVRAM[tile_data_offset + 3] >> (7-pixel_x)) & 0x01) << 3);
            
            if (second_pallete)
                pixel += 16;
            int r = m_pVdpCRAM[pixel] & 0x03;
            int g = (m_pVdpCRAM[pixel] >> 2) & 0x03;
            int b = (m_pVdpCRAM[pixel] >> 4) & 0x03;
            
            GS_Color color;
            
            color.red = (r * 255) / 3;
            color.green = (g * 255) / 3;
            color.blue = (b * 255) / 3;
            color.alpha = 0xFF;
            
            m_pColorFrameBuffer[(line * 256) + (tile_x * 8) + pixel_x] = color;  
            
            pixel_cur++;
        }
    }
}

void Video::RenderSprites(int line)
{

}

