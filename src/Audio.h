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
#include "VgmRecorder.h"

class Cartridge;

class Audio
{
public:
    Audio(Cartridge* pCartridge);
    ~Audio();
    void Init();
    void Reset(bool bPAL);
    void Mute(bool bMute);
    void SetPSGVolume(float volume);
    void SetFMVolume(float volume);
    void WriteAudioRegister(u8 value);
    void WriteGGStereoRegister(u8 value);
    void YM2413Write(u8 port, u8 value);
    u8 YM2413Read();
    void Tick(unsigned int clockCycles);
    void EndFrame(s16* pSampleBuffer, int* pSampleCount);
    void DisableYM2413(bool bDisable);
    Sms_Apu* GetPSG();
    void EnablePSGDebug(bool enable);
    bool IsPSGDebugEnabled();
    blip_sample_t* GetDebugChannelBuffer(int channel);
    int GetDebugChannelSamples(int channel);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);
    void LoadStateV1(std::istream& stream);
    bool StartVgmRecording(const char* file_path, int clock_rate, bool is_pal, bool has_ym2413);
    void StopVgmRecording();
    bool IsVgmRecording() const;

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
    float m_psg_volume;
    float m_fm_volume;
    VgmRecorder m_VgmRecorder;
    bool m_bVgmRecordingEnabled;
    blip_sample_t* m_pDebugChannelBuffer[4];
    long m_iDebugChannelSamples[4];
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
#ifndef GS_DISABLE_VGMRECORDER
    if (m_bVgmRecordingEnabled)
        m_VgmRecorder.WritePSG(value);
#endif
}

inline void Audio::WriteGGStereoRegister(u8 value)
{
    m_pApu->write_ggstereo(m_ElapsedCycles, value);
#ifndef GS_DISABLE_VGMRECORDER
    if (m_bVgmRecordingEnabled)
        m_VgmRecorder.WriteGGStereo(value);
#endif
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

        if (m_bYM2413Enabled && m_bPSGEnabled)
            m_pApu->volume(0.8);
        else
            m_pApu->volume(1.0);

        m_pYM2413->Enable(m_bYM2413Enabled);
    }

    m_pYM2413->Write(port, value);

#ifndef GS_DISABLE_VGMRECORDER
    if (m_bVgmRecordingEnabled && (port == 0xF0 || port == 0xF1))
        m_VgmRecorder.WriteYM2413(port, value);
#endif
}

inline u8 Audio::YM2413Read()
{
    if (m_bYM2413ForceDisabled || m_bYM2413CartridgeNotSupported)
        return 0xFF;
    else
        return m_pYM2413->Read();
}

inline Sms_Apu* Audio::GetPSG()
{
    return m_pApu;
}

inline void Audio::EnablePSGDebug(bool enable)
{
    if (enable && !m_pApu->is_debug_enabled())
    {
        long clock = m_bPAL ? (m_pCartridge->IsSG1000() ? GS_MASTER_CLOCK_PAL_SG1000 : GS_MASTER_CLOCK_PAL) : GS_MASTER_CLOCK_NTSC;
        m_pApu->init_debug_buffers(m_iSampleRate, clock);
    }
    else if (!enable && m_pApu->is_debug_enabled())
    {
        m_pApu->disable_debug_buffers();
    }
}

inline bool Audio::IsPSGDebugEnabled()
{
    return m_pApu->is_debug_enabled();
}

inline blip_sample_t* Audio::GetDebugChannelBuffer(int channel)
{
    if (channel < 0 || channel >= 4)
        return NULL;
    return m_pDebugChannelBuffer[channel];
}

inline int Audio::GetDebugChannelSamples(int channel)
{
    if (channel < 0 || channel >= 4)
        return 0;
    return (int)m_iDebugChannelSamples[channel];
}

inline void Audio::SetPSGVolume(float volume)
{
    m_psg_volume = CLAMP(volume, 0.0f, 2.0f);
}

inline void Audio::SetFMVolume(float volume)
{
    m_fm_volume = CLAMP(volume, 0.0f, 2.0f);
}

#endif	/* AUDIO_H */
