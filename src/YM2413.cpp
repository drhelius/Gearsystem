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
    m_CurrentSample = 0;
    m_bEnabled = false;
    m_iCyclesPerSample = 0;
}

YM2413::~YM2413()
{
    SafeDeleteArray(m_pBuffer);
}

void YM2413::Init(int clockRate)
{
    m_pBuffer = new s16[GS_AUDIO_BUFFER_SIZE];
    YM2413Init();
    Reset(clockRate);
}

void YM2413::Reset(int clockRate)
{
    m_iClockRate = clockRate;
    m_iCyclesPerSample = m_iClockRate / GS_AUDIO_SAMPLE_RATE;
    m_ElapsedCycles = 0;
    m_CurrentSample = 0;
    m_iCycleCounter = 0;
    m_iSampleCounter = 0;
    m_iBufferIndex = 0;
    m_RegisterF2 = 0;
    m_CurrentSample = 0;
    m_bEnabled = false;

    YM2413ResetChip();

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
    {
        m_pBuffer[i] = 0;
    }
}

void YM2413::Write(u8 port, u8 value)
{
    if (port & 0x01)
    {
        Sync();
    }

    YM2413Write(port, value);
}

u8 YM2413::Read()
{
    return YM2413Read();
}

void YM2413::Tick(unsigned int clockCycles)
{
    m_ElapsedCycles += clockCycles;
}

int YM2413::EndFrame(s16* pSampleBuffer)
{
    if (!m_bEnabled)
    {
        m_iBufferIndex = 0;
        return 0;
    }

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

void YM2413::Enable(bool bEnabled)
{
    m_bEnabled = bEnabled;
}

void YM2413::Sync()
{
    if (!m_bEnabled)
    {
        m_ElapsedCycles = 0;
        return;
    }

    for (int i = 0; i < m_ElapsedCycles; i++)
    {
        m_iCycleCounter ++;
        if (m_iCycleCounter >= 72)
        {
            m_iCycleCounter -= 72;
            m_CurrentSample = YM2413Update();
        }

        m_iSampleCounter++;
        if (m_iSampleCounter >= m_iCyclesPerSample)
        {
            m_iSampleCounter -= m_iCyclesPerSample;

            m_pBuffer[m_iBufferIndex] = m_CurrentSample;
            m_pBuffer[m_iBufferIndex + 1] = m_CurrentSample;
            m_iBufferIndex += 2;

            if (m_iBufferIndex >= GS_AUDIO_BUFFER_SIZE)
            {
                Debug("YM2413 Audio buffer overflow");
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
    stream.write(reinterpret_cast<const char*>(&m_CurrentSample), sizeof(s16));
    stream.write(reinterpret_cast<const char*>(&m_bEnabled), sizeof(bool));
    stream.write(reinterpret_cast<const char*>(&m_iCyclesPerSample), sizeof(int));
    stream.write(reinterpret_cast<const char*>(m_pBuffer), sizeof(s16) * GS_AUDIO_BUFFER_SIZE);

    unsigned char* context = YM2413GetContextPtr();
    unsigned int contex_size = YM2413GetContextSize();
    stream.write(reinterpret_cast<const char*>(context), contex_size);
}

void YM2413::LoadState(std::istream& stream)
{
    stream.read(reinterpret_cast<char*>(&m_iCycleCounter), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iSampleCounter), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iBufferIndex), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_ElapsedCycles), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_iClockRate), sizeof(int));
    stream.read(reinterpret_cast<char*>(&m_RegisterF2), sizeof(u8));
    stream.read(reinterpret_cast<char*>(&m_CurrentSample), sizeof(s16));
    stream.read(reinterpret_cast<char*>(&m_bEnabled), sizeof(bool));
    stream.read(reinterpret_cast<char*>(&m_iCyclesPerSample), sizeof(int));
    stream.read(reinterpret_cast<char*>(m_pBuffer), sizeof(s16) * GS_AUDIO_BUFFER_SIZE);

    unsigned char* context = YM2413GetContextPtr();
    unsigned int context_size = YM2413GetContextSize();
    stream.read(reinterpret_cast<char*>(context), context_size);
 }
