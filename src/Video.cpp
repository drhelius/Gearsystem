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
    InitPointer(m_pVRAM);
    InitPointer(m_pCRAM);
    m_bFirstByteInSequence = false;
    m_CachedByte = 0x00;
    for (int i = 0; i < 16; i++)
        m_VDPRegister[i] = 0;
    m_Operation = 0;
    m_VCounter = 0;
    m_HCounter = 0;
}

Video::~Video()
{
    SafeDeleteArray(m_pFrameBuffer);
    SafeDeleteArray(m_pVRAM);
    SafeDeleteArray(m_pCRAM);
}

void Video::Init()
{
    m_pFrameBuffer = new u8[GS_WIDTH * GS_HEIGHT];
    m_pVRAM = new u8[0x4000];
    m_pCRAM = new u8[0x40];
    Reset();
}

void Video::Reset()
{
    m_bFirstByteInSequence = true;
    m_VCounter = 0;
    m_HCounter = 0;
    m_CachedByte = 0x00;
    m_Operation = 0x00;
    for (int i = 0; i < (GS_WIDTH * GS_HEIGHT); i++)
        m_pFrameBuffer[i] = 0;
    for (int i = 0; i < 0x4000; i++)
        m_pVRAM[i] = 0;
    for (int i = 0; i < 0x40; i++)
        m_pCRAM[i] = 0;
    for (int i = 0; i < 16; i++)
        m_VDPRegister[i] = 0;
}

u8 Video::GetVCounter()
{
    return m_VCounter;
}

u8 Video::GetHCounter()
{
    return m_HCounter;
}

u8 Video::GetDataPort()
{
    m_bFirstByteInSequence = true;

    switch (m_Operation)
    {
        case VDP_READ_VRAM_OPERATION:
        case VDP_WRITE_VRAM_OPERATION:
        case VDP_WRITE_REG_OPERATION:
        {
            // VRAM read
            return m_pVRAM[m_Address];
            m_Address++;
            if (m_Address >= 0x4000)
                m_Address = 0x0000;
            break;
        }
        case VDP_WRITE_CRAM_OPERATION:
        default:
        {
            Log("--> ** Attempting to read data with VDP operation %d", m_Operation);
            return 0x00;
        }
    }
}

u8 Video::GetStatusFlags()
{
    m_bFirstByteInSequence = true;
    return 0x00;
}

void Video::WriteData(u8 data)
{
    m_bFirstByteInSequence = true;
}

void Video::WriteControl(u8 control)
{
    if (m_bFirstByteInSequence)
    {
        m_bFirstByteInSequence = false;
        m_CachedByte = control;
    }
    else
    {
        m_bFirstByteInSequence = true;
        m_Operation = (control >> 6) & 0x03;

        switch (m_Operation)
        {
            case VDP_WRITE_REG_OPERATION:
            {
                u8 value = m_CachedByte;
                u8 reg = control & 0x0F;
                m_VDPRegister[reg] = value;
                if (reg > 10)
                {
                    Log("--> ** Attempting to write on VDP REG %d: %X", reg, value);
                }
                break;
            }
            case VDP_READ_VRAM_OPERATION:
            case VDP_WRITE_VRAM_OPERATION:
            {
                m_Address = ((control & 0x3F) << 8) + m_CachedByte;
                break;
            }
            case VDP_WRITE_CRAM_OPERATION:
            {
                break;
            }
        }
    }
}

bool Video::Tick(unsigned int &clockCycles, GS_Color* pColorFrameBuffer)
{
    m_pColorFrameBuffer = pColorFrameBuffer;

    bool vblank = false;


    return vblank;
}
