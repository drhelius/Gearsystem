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
#include "audio/Stereo_Buffer.h"
#include "audio/Sms_Apu.h"
#include "YM2413.h"

class Cartridge;

class Audio
{
public:
    Audio(Cartridge* pCartridge);
    ~Audio();
    void Init();
    void Reset(bool bPAL);
    void Mute(bool bMute);
    void WriteAudioRegister(u8 value);
    void WriteGGStereoRegister(u8 value);
    void YM2413Write(u8 port, u8 value);
    u8 YM2413Read(u8 port);
    void Tick(unsigned int clockCycles);
    void EndFrame(s16* pSampleBuffer, int* pSampleCount);
    void DisableYM2413(bool bDisable);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);

private:
    YM2413* m_pYM2413;
    Sms_Apu* m_pApu;
    Stereo_Buffer* m_pBuffer;
    int m_ElapsedCycles;
    int m_iSampleRate;
    blip_sample_t* m_pSampleBuffer;
    bool m_bPAL;
    bool m_bYM2413Enabled;
    bool m_bPSGEnabled;
    bool m_bYM2413ForceDisabled;
    bool m_bYM2413CartridgeNotSupported;
    Cartridge* m_pCartridge;
    s16* m_pYM2413Buffer;
    bool m_bMute;
};

#include "Cartridge.h"

inline void Audio::Tick(unsigned int clockCycles)
{
    m_ElapsedCycles += clockCycles;
    m_pYM2413->Tick(clockCycles);
}

inline void Audio::WriteAudioRegister(u8 value)
{
    m_pApu->write_data(m_ElapsedCycles, value);
}

inline void Audio::WriteGGStereoRegister(u8 value)
{
    m_pApu->write_ggstereo(m_ElapsedCycles, value);
}

inline void Audio::YM2413Write(u8 port, u8 value)
{
    if (m_bYM2413ForceDisabled || m_bYM2413CartridgeNotSupported)
        return;

    if (port == 0xF2)
    {
        if (m_pCartridge->GetZone() == Cartridge::CartridgeJapanSMS)
        {
            u8 mixer = value & 0x03;
            m_bPSGEnabled = (mixer == 0 || mixer == 3);
            m_bYM2413Enabled = (mixer == 1 || mixer == 3);
        }
        else
        {
            m_bPSGEnabled = true;
            m_bYM2413Enabled = (value & 0x01) == 0x01;
        }

        m_pYM2413->Enable(m_bYM2413Enabled);
    }

    m_pYM2413->Write(port, value);
}

inline u8 Audio::YM2413Read(u8 port)
{
    if (m_bYM2413ForceDisabled || m_bYM2413CartridgeNotSupported)
        return 0xFF;
    else
        return m_pYM2413->Read(port);
}

#endif	/* AUDIO_H */
