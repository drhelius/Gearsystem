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
    m_bEnabled = false;
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
    m_iCycleCounter = 0;
    m_iSampleCounter = 0;
    m_iBufferIndex = 0;
    m_RegisterF2 = 0;
    m_CurrentSample = 0;
    m_bEnabled = false;

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
    else if (m_bEnabled)
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
    stream.write(reinterpret_cast<const char*>(&m_CurrentSample), sizeof(m_CurrentSample));
    stream.write(reinterpret_cast<const char*>(&m_bEnabled), sizeof(m_bEnabled));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->chip_type), sizeof(m_pOPLL->chip_type));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->adr), sizeof(m_pOPLL->adr));
    stream.write(reinterpret_cast<const char*>(m_pOPLL->reg), sizeof(m_pOPLL->reg));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->test_flag), sizeof(m_pOPLL->test_flag));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot_key_status), sizeof(m_pOPLL->slot_key_status));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->rhythm_mode), sizeof(m_pOPLL->rhythm_mode));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->eg_counter), sizeof(m_pOPLL->eg_counter));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->pm_phase), sizeof(m_pOPLL->pm_phase));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->am_phase), sizeof(m_pOPLL->am_phase));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->lfo_am), sizeof(m_pOPLL->lfo_am));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->noise), sizeof(m_pOPLL->noise));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->short_noise), sizeof(m_pOPLL->short_noise));
    stream.write(reinterpret_cast<const char*>(m_pOPLL->patch_number), sizeof(m_pOPLL->patch_number));
    stream.write(reinterpret_cast<const char*>(m_pOPLL->patch), sizeof(m_pOPLL->patch));
    stream.write(reinterpret_cast<const char*>(&m_pOPLL->mask), sizeof(m_pOPLL->mask));
    stream.write(reinterpret_cast<const char*>(m_pOPLL->ch_out), sizeof(m_pOPLL->ch_out));
    stream.write(reinterpret_cast<const char*>(m_pOPLL->mix_out), sizeof(m_pOPLL->mix_out));
    for (int i = 0; i < 18; i++)
    {
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].number), sizeof(m_pOPLL->slot[i].number));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].type), sizeof(m_pOPLL->slot[i].type));
        stream.write(reinterpret_cast<const char*>(m_pOPLL->slot[i].output), sizeof(m_pOPLL->slot[i].output));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].pg_phase), sizeof(m_pOPLL->slot[i].pg_phase));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].pg_out), sizeof(m_pOPLL->slot[i].pg_out));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].pg_keep), sizeof(m_pOPLL->slot[i].pg_keep));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].blk_fnum), sizeof(m_pOPLL->slot[i].blk_fnum));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].fnum), sizeof(m_pOPLL->slot[i].fnum));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].blk), sizeof(m_pOPLL->slot[i].blk));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].eg_state), sizeof(m_pOPLL->slot[i].eg_state));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].volume), sizeof(m_pOPLL->slot[i].volume));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].key_flag), sizeof(m_pOPLL->slot[i].key_flag));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].sus_flag), sizeof(m_pOPLL->slot[i].sus_flag));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].tll), sizeof(m_pOPLL->slot[i].tll));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].rks), sizeof(m_pOPLL->slot[i].rks));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].eg_rate_h), sizeof(m_pOPLL->slot[i].eg_rate_h));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].eg_rate_l), sizeof(m_pOPLL->slot[i].eg_rate_l));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].eg_shift), sizeof(m_pOPLL->slot[i].eg_shift));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].eg_out), sizeof(m_pOPLL->slot[i].eg_out));
        stream.write(reinterpret_cast<const char*>(&m_pOPLL->slot[i].update_requests), sizeof(m_pOPLL->slot[i].update_requests));
    }
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
    stream.read(reinterpret_cast<char*>(&m_CurrentSample), sizeof(m_CurrentSample));
    stream.read(reinterpret_cast<char*>(&m_bEnabled), sizeof(m_bEnabled));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->chip_type), sizeof(m_pOPLL->chip_type));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->adr), sizeof(m_pOPLL->adr));
    stream.read(reinterpret_cast<char*>(m_pOPLL->reg), sizeof(m_pOPLL->reg));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->test_flag), sizeof(m_pOPLL->test_flag));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->slot_key_status), sizeof(m_pOPLL->slot_key_status));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->rhythm_mode), sizeof(m_pOPLL->rhythm_mode));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->eg_counter), sizeof(m_pOPLL->eg_counter));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->pm_phase), sizeof(m_pOPLL->pm_phase));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->am_phase), sizeof(m_pOPLL->am_phase));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->lfo_am), sizeof(m_pOPLL->lfo_am));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->noise), sizeof(m_pOPLL->noise));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->short_noise), sizeof(m_pOPLL->short_noise));
    stream.read(reinterpret_cast<char*>(m_pOPLL->patch_number), sizeof(m_pOPLL->patch_number));
    stream.read(reinterpret_cast<char*>(m_pOPLL->patch), sizeof(m_pOPLL->patch));
    stream.read(reinterpret_cast<char*>(&m_pOPLL->mask), sizeof(m_pOPLL->mask));
    stream.read(reinterpret_cast<char*>(m_pOPLL->ch_out), sizeof(m_pOPLL->ch_out));
    stream.read(reinterpret_cast<char*>(m_pOPLL->mix_out), sizeof(m_pOPLL->mix_out));
    for (int i = 0; i < 18; i++)
    {
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].number), sizeof(m_pOPLL->slot[i].number));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].type), sizeof(m_pOPLL->slot[i].type));
        stream.read(reinterpret_cast<char*>(m_pOPLL->slot[i].output), sizeof(m_pOPLL->slot[i].output));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].pg_phase), sizeof(m_pOPLL->slot[i].pg_phase));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].pg_out), sizeof(m_pOPLL->slot[i].pg_out));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].pg_keep), sizeof(m_pOPLL->slot[i].pg_keep));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].blk_fnum), sizeof(m_pOPLL->slot[i].blk_fnum));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].fnum), sizeof(m_pOPLL->slot[i].fnum));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].blk), sizeof(m_pOPLL->slot[i].blk));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].eg_state), sizeof(m_pOPLL->slot[i].eg_state));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].volume), sizeof(m_pOPLL->slot[i].volume));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].key_flag), sizeof(m_pOPLL->slot[i].key_flag));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].sus_flag), sizeof(m_pOPLL->slot[i].sus_flag));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].tll), sizeof(m_pOPLL->slot[i].tll));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].rks), sizeof(m_pOPLL->slot[i].rks));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].eg_rate_h), sizeof(m_pOPLL->slot[i].eg_rate_h));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].eg_rate_l), sizeof(m_pOPLL->slot[i].eg_rate_l));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].eg_shift), sizeof(m_pOPLL->slot[i].eg_shift));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].eg_out), sizeof(m_pOPLL->slot[i].eg_out));
        stream.read(reinterpret_cast<char*>(&m_pOPLL->slot[i].update_requests), sizeof(m_pOPLL->slot[i].update_requests));
    }
}
