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

#include "YM2413.h"

YM2413::YM2413()
{
    InitPointer(m_pBuffer);
    m_iCycleCounter = 0;
    m_iSampleCounter = 0;
    m_iBufferIndex = 0;
    m_ElapsedCycles = 0;
    m_iClockRate = 0;
    m_RegisterF2 = 0;
}

YM2413::~YM2413()
{
    SafeDeleteArray(m_pBuffer);
}

void YM2413::Init(int clockRate)
{
    m_pBuffer = new s16[GS_AUDIO_BUFFER_SIZE];
    Reset(clockRate);
}

void YM2413::Reset(int clockRate)
{
    m_iClockRate = clockRate;
    m_ElapsedCycles = 0;

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
    {
        m_pBuffer[i] = 0;
    }
}

void YM2413::Write(u8 port, u8 value)
{
    if (port == 0xF2)
    {
        m_RegisterF2 = value & 0x03;
    }
    else
    {
        //OPLL_writeIO(opll, port & 0x01, value);
    }
}

u8 YM2413::Read(u8 port)
{
    return (port == 0xF2) ? m_RegisterF2 : 0xFF;
}

void YM2413::Tick(unsigned int clockCycles)
{
    m_iCycleCounter += clockCycles;
    m_ElapsedCycles += clockCycles;
}

int YM2413::EndFrame(s16* pSampleBuffer)
{
    int iSamples = 0;

    for (int i = 0; i < iSamples; i++)
    {
        pSampleBuffer[i] = m_pBuffer[i];
    }

    return iSamples;
}

void YM2413::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_iCycleCounter), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_iSampleCounter), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_iBufferIndex), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_ElapsedCycles), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_iClockRate), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_RegisterF2), sizeof(u8));
}

void YM2413::LoadState(std::istream& stream)
{
    stream.read(reinterpret_cast<char*>(&m_iCycleCounter), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iSampleCounter), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iBufferIndex), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_ElapsedCycles), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iClockRate), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_RegisterF2), sizeof(u8));
}
