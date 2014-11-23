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

#ifndef AUDIO_H
#define	AUDIO_H

#include "definitions.h"
#include "audio/Multi_Buffer.h"
#include "audio/Sound_Queue.h"
#include "audio/Sms_Apu.h"

class Audio
{
public:
    Audio();
    ~Audio();
    void Init();
    void Reset(bool soft = false);
    void Enable(bool enabled);
    bool IsEnabled() const;
    void SetSampleRate(int rate);
    void WriteAudioRegister(u8 value);
    void WriteGGStereoRegister(u8 value);
    void EndFrame();
    void Tick(unsigned int clockCycles);

private:
    bool m_bEnabled;
    Sms_Apu* m_pApu;
    Stereo_Buffer* m_pBuffer;
    long m_Time;
    Sound_Queue* m_pSound;
    int m_iSampleRate;
    blip_sample_t* m_pSampleBuffer;
};

const long kSampleBufferSize = 4096;

#endif	/* AUDIO_H */
