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
#include "Cartridge.h"

Audio::Audio(Cartridge * pCartridge)
{
    m_pCartridge = pCartridge;
    m_ElapsedCycles = 0;
    m_iSampleRate = GS_AUDIO_SAMPLE_RATE;
    InitPointer(m_pYM2413);
    InitPointer(m_pApu);
    InitPointer(m_pBuffer);
    InitPointer(m_pSampleBuffer);
    InitPointer(m_pYM2413Buffer);
    m_bPAL = false;
    m_bYM2413Enabled = false;
    m_bPSGEnabled = true;
}

Audio::~Audio()
{
    SafeDelete(m_pYM2413);
    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDeleteArray(m_pSampleBuffer);
    SafeDeleteArray(m_pYM2413Buffer);
}

void Audio::Init()
{
    m_pSampleBuffer = new blip_sample_t[GS_AUDIO_BUFFER_SIZE];

    m_pApu = new Sms_Apu();
    m_pBuffer = new Stereo_Buffer();

    m_pBuffer->clock_rate(m_bPAL ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC);
    m_pBuffer->set_sample_rate(m_iSampleRate);
    //m_pBuffer->bass_freq(100);
    m_pApu->output(m_pBuffer->center(), m_pBuffer->left(), m_pBuffer->right());
    //m_pApu->treble_eq(-15.0);

    m_pYM2413Buffer = new s16[GS_AUDIO_BUFFER_SIZE];

    m_pYM2413 = new YM2413();
    m_pYM2413->Init(m_bPAL ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC);
}

void Audio::Reset(bool bPAL)
{
    m_bPAL = bPAL;
    m_bYM2413Enabled = false;
    m_bPSGEnabled = true;
    m_pApu->reset();
    m_pApu->volume(0.5);
    m_pBuffer->clear();
    m_pBuffer->clock_rate(m_bPAL ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC);
    m_pYM2413->Reset(m_bPAL ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC);
    m_ElapsedCycles = 0;
}

void Audio::SetSampleRate(int rate)
{
    if (rate != m_iSampleRate)
    {
        m_iSampleRate = rate;
        m_pBuffer->set_sample_rate(m_iSampleRate);
    }
}

void Audio::SetVolume(float volume)
{
    m_pApu->volume(volume);
}

void Audio::EndFrame(s16* pSampleBuffer, int* pSampleCount)
{
    m_pApu->end_frame(m_ElapsedCycles);
    m_pBuffer->end_frame(m_ElapsedCycles);

    int count = static_cast<int>(m_pBuffer->read_samples(m_pSampleBuffer, GS_AUDIO_BUFFER_SIZE));

    m_pYM2413->EndFrame(m_pYM2413Buffer);

    if (IsValidPointer(pSampleBuffer) && IsValidPointer(pSampleCount))
    {
        *pSampleCount = count;

        for (int i=0; i<count; i++)
        {
            pSampleBuffer[i] = 0;
            pSampleBuffer[i] += m_bPSGEnabled ? m_pSampleBuffer[i] : 0;
            pSampleBuffer[i] += m_bYM2413Enabled ? m_pYM2413Buffer[i] : 0;
        }
    }

    m_ElapsedCycles = 0;
}

void Audio::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_ElapsedCycles), sizeof(m_ElapsedCycles));
    stream.write(reinterpret_cast<const char*> (m_pSampleBuffer), sizeof(blip_sample_t) * GS_AUDIO_BUFFER_SIZE);
    stream.write(reinterpret_cast<const char*> (&m_bYM2413Enabled), sizeof(m_bYM2413Enabled));
    stream.write(reinterpret_cast<const char*> (&m_bPSGEnabled), sizeof(m_bPSGEnabled));
    stream.write(reinterpret_cast<const char*> (m_pYM2413Buffer), sizeof(s16) * GS_AUDIO_BUFFER_SIZE);
    m_pYM2413->SaveState(stream);
}

void Audio::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_ElapsedCycles), sizeof(m_ElapsedCycles));
    stream.read(reinterpret_cast<char*> (m_pSampleBuffer), sizeof(blip_sample_t) * GS_AUDIO_BUFFER_SIZE);
    stream.read(reinterpret_cast<char*> (&m_bYM2413Enabled), sizeof(m_bYM2413Enabled));
    stream.read(reinterpret_cast<char*> (&m_bPSGEnabled), sizeof(m_bPSGEnabled));
    stream.read(reinterpret_cast<char*> (m_pYM2413Buffer), sizeof(s16) * GS_AUDIO_BUFFER_SIZE);
    m_pYM2413->LoadState(stream);

    m_pApu->reset();
    m_pApu->volume(0.5);
    m_pBuffer->clear();
}
