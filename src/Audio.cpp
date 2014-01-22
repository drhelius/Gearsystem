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

#include "Audio.h"
#include "Memory.h"
#include "audio/Sound_Queue.h"
#include "audio/Sms_Apu.h"

Audio::Audio()
{
    m_bEnabled = true;
    m_Time = 0;
    m_iSampleRate = 44100;
    InitPointer(m_pApu);
    InitPointer(m_pBuffer);
    InitPointer(m_pSound);
    InitPointer(m_pSampleBuffer);
}

Audio::~Audio()
{
    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDelete(m_pSound);
    SafeDeleteArray(m_pSampleBuffer);
}

void Audio::Init()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        Log("--> ** SDL Audio not initialized");
    }

    atexit(SDL_Quit);

    m_pSampleBuffer = new blip_sample_t[kSampleBufferSize];

    m_pApu = new Sms_Apu();
    m_pBuffer = new Stereo_Buffer();
    m_pSound = new Sound_Queue();

    // Clock rate for NTSC is 3579545, 3559545 to avoid sttutering at 60hz
    // Clock rate for PAL is 3546893
    m_pBuffer->clock_rate(3559545);
    m_pBuffer->set_sample_rate(m_iSampleRate);

    m_pApu->treble_eq(-15.0);
    m_pBuffer->bass_freq(100);

    m_pApu->output(m_pBuffer->center(), m_pBuffer->left(), m_pBuffer->right());
    
    m_pSound->start(m_iSampleRate, 2);
}

void Audio::Reset()
{
    m_bEnabled = true;
    m_pApu->reset();
    m_pBuffer->clear();
    m_Time = 0;
}

void Audio::Enable(bool enabled)
{
    m_bEnabled = enabled;
}

bool Audio::IsEnabled() const
{
    return m_bEnabled;
}

void Audio::SetSampleRate(int rate)
{
    if (rate != m_iSampleRate)
    {
        m_iSampleRate = rate;
        m_pBuffer->set_sample_rate(m_iSampleRate);
        m_pSound->stop();
        m_pSound->start(m_iSampleRate, 2);
    }
}

void Audio::WriteAudioRegister(u8 value)
{
    m_pApu->write_data(m_Time, value);
}

void Audio::WriteGGStereoRegister(u8 value)
{
    m_pApu->write_ggstereo(m_Time, value);
}

void Audio::EndFrame()
{
    m_pApu->end_frame(m_Time);
    m_pBuffer->end_frame(m_Time);
    m_Time = 0;

    if (m_pBuffer->samples_avail() >= kSampleBufferSize)
    {
        long count = m_pBuffer->read_samples(m_pSampleBuffer, kSampleBufferSize);
        if (m_bEnabled)
        {
            m_pSound->write(m_pSampleBuffer, count);
        }
    }
}

void Audio::Tick(unsigned int clockCycles)
{
    m_Time += clockCycles;
}
