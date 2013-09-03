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

class Video
{
public:
    Video(Memory* pMemory, Processor* pProcessor);
    ~Video();
    void Init();
    void Reset();
    bool Tick(unsigned int &clockCycles, GS_Color* pColorFrameBuffer);
    u8 GetVCounter();
    u8 GetHCounter();
    u8 GetDataPort();
    u8 GetStatusFlags();
    void WriteData(u8 data);
    void WriteControl(u8 control);

private:
    

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    u8* m_pFrameBuffer;
    GS_Color* m_pColorFrameBuffer;
    u8* m_pVdpVRAM;
    u8* m_pVdpCRAM;
    bool m_bFirstByteInSequence;
    u8 m_VdpLatch;
    u8 m_VdpRegister[16];
    u8 m_VdpCode;
    u8 m_VdpBuffer;
    u16 m_VdpAddress;
    u8 m_VCounter;
    u8 m_HCounter;
};

#endif	/* VIDEO_H */

