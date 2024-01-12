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
    InitPointer(m_pOPLL);
    m_iCycleCounter = 0;
    m_iSampleCounter = 0;
    m_iBufferIndex = 0;
    m_ElapsedCycles = 0;
    m_iClockRate = 0;
    m_RegisterF2 = 0;
    m_CurrentSample = 0;
}

YM2413::~YM2413()
{
    OPLL_delete(m_pOPLL);
    SafeDeleteArray(m_pBuffer);
}

void YM2413::Init(int clockRate)
{
    m_pBuffer = new s16[GS_AUDIO_BUFFER_SIZE];
    m_pOPLL = OPLL_new();
    OPLL_setChipType(m_pOPLL, 0);
    Reset(clockRate);
}

void YM2413::Reset(int clockRate)
{
    m_iClockRate = clockRate;
    m_ElapsedCycles = 0;
    m_CurrentSample = 0;

    OPLL_reset(m_pOPLL);

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
    {
        m_pBuffer[i] = 0;
    }
}

void YM2413::Write(u8 port, u8 value)
{
    Sync();

    if (port == 0xF2)
    {
        m_RegisterF2 = value & 0x03;
    }
    else
    {
        OPLL_writeIO(m_pOPLL, port & 0x01, value);
    }
}

u8 YM2413::Read(u8 port)
{
    return (port == 0xF2) ? m_RegisterF2 : 0xFF;
}

void YM2413::Tick(unsigned int clockCycles)
{
    m_ElapsedCycles += clockCycles;
}

int YM2413::EndFrame(s16* pSampleBuffer)
{
    Sync();

    int ret = 0;

    if (IsValidPointer(pSampleBuffer))
    {
        ret = m_iBufferIndex;

        for (int i = 0; i < m_iBufferIndex; i++)
        {
            pSampleBuffer[i] = m_pBuffer[i];
        }
    }

    m_iBufferIndex = 0;

    return ret;
}

void YM2413::Sync()
{
    for (int i = 0; i < m_ElapsedCycles; i++)
    {
        m_iCycleCounter ++;
        if (m_iCycleCounter >= 72)
        {
            m_iCycleCounter -= 72;
            m_CurrentSample = OPLL_calc(m_pOPLL);
        }

        m_iSampleCounter++;
        int cyclesPerSample = m_iClockRate / GS_AUDIO_SAMPLE_RATE;
        if (m_iSampleCounter >= cyclesPerSample)
        {
            m_iSampleCounter -= cyclesPerSample;

            m_pBuffer[m_iBufferIndex] = m_CurrentSample;
            m_pBuffer[m_iBufferIndex + 1] = m_CurrentSample;
            m_iBufferIndex += 2;

            if (m_iBufferIndex >= GS_AUDIO_BUFFER_SIZE)
            {
                Log("YM2413 Audio buffer overflow");
                m_iBufferIndex = 0;
            }
        }
    }

    m_ElapsedCycles = 0;
}

void YM2413::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_iCycleCounter), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_iSampleCounter), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_iBufferIndex), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_ElapsedCycles), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_iClockRate), sizeof(int));
    stream.write(reinterpret_cast<const char*>(&m_RegisterF2), sizeof(u8));
    stream.write(reinterpret_cast<const char*>(m_pBuffer), sizeof(s16) * GS_AUDIO_BUFFER_SIZE);
    stream.write(reinterpret_cast<const char*>(&m_CurrentSample), sizeof(s16));
}

void YM2413::LoadState(std::istream& stream)
{
    stream.read(reinterpret_cast<char*>(&m_iCycleCounter), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iSampleCounter), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iBufferIndex), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_ElapsedCycles), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iClockRate), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_RegisterF2), sizeof(u8));
    stream.read(reinterpret_cast<char*>(m_pBuffer), sizeof(s16) * GS_AUDIO_BUFFER_SIZE);
    stream.read(reinterpret_cast<char*>(&m_CurrentSample), sizeof(s16));
}
