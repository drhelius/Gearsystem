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

#include "VgmRecorder.h"
#include <cstring>

VgmRecorder::VgmRecorder()
{
    m_bRecording = false;
    m_PendingWait = 0;
    m_TotalSamples = 0;
    m_ClockRate = 0;
    m_bPAL = false;
    m_bHasYM2413 = false;
    m_YM2413Register = 0;
}

VgmRecorder::~VgmRecorder()
{
    if (m_bRecording)
    {
        Stop();
    }
}

void VgmRecorder::Start(const char* file_path, int clock_rate, bool is_pal, bool has_ym2413)
{
    if (m_bRecording)
    {
        Stop();
    }

    m_FilePath = file_path;
    m_ClockRate = clock_rate;
    m_bPAL = is_pal;
    m_bHasYM2413 = has_ym2413;
    m_bRecording = true;
    m_PendingWait = 0;
    m_TotalSamples = 0;
    m_YM2413Register = 0;
    m_CommandBuffer.clear();
    m_CommandBuffer.reserve(1024 * 1024); // Reserve 1MB
}

void VgmRecorder::Stop()
{
    if (!m_bRecording)
        return;

    FlushPendingWait();

    // Write end of sound data command
    WriteCommand(0x66);

    std::ofstream file(m_FilePath.c_str(), std::ios::binary);
    if (file.is_open())
    {
        u8 header[256];
        memset(header, 0, 256);

        // File identification "Vgm " (0x56 0x67 0x6d 0x20)
        header[0x00] = 0x56;
        header[0x01] = 0x67;
        header[0x02] = 0x6d;
        header[0x03] = 0x20;

        // EOF offset (file length - 4)
        u32 eof_offset = (256 + m_CommandBuffer.size()) - 4;
        header[0x04] = (eof_offset >> 0) & 0xFF;
        header[0x05] = (eof_offset >> 8) & 0xFF;
        header[0x06] = (eof_offset >> 16) & 0xFF;
        header[0x07] = (eof_offset >> 24) & 0xFF;

        // Version number (1.70 = 0x00000170)
        header[0x08] = 0x70;
        header[0x09] = 0x01;
        header[0x0A] = 0x00;
        header[0x0B] = 0x00;

        // SN76489 clock
        u32 psg_clock = m_ClockRate;
        header[0x0C] = (psg_clock >> 0) & 0xFF;
        header[0x0D] = (psg_clock >> 8) & 0xFF;
        header[0x0E] = (psg_clock >> 16) & 0xFF;
        header[0x0F] = (psg_clock >> 24) & 0xFF;

        // YM2413 clock
        if (m_bHasYM2413)
        {
            u32 ym2413_clock = m_ClockRate;
            header[0x10] = (ym2413_clock >> 0) & 0xFF;
            header[0x11] = (ym2413_clock >> 8) & 0xFF;
            header[0x12] = (ym2413_clock >> 16) & 0xFF;
            header[0x13] = (ym2413_clock >> 24) & 0xFF;
        }

        // GD3 offset (0 = no GD3 tag)
        header[0x14] = 0x00;
        header[0x15] = 0x00;
        header[0x16] = 0x00;
        header[0x17] = 0x00;

        // Total # samples
        header[0x18] = (m_TotalSamples >> 0) & 0xFF;
        header[0x19] = (m_TotalSamples >> 8) & 0xFF;
        header[0x1A] = (m_TotalSamples >> 16) & 0xFF;
        header[0x1B] = (m_TotalSamples >> 24) & 0xFF;

        // Loop offset (0 = no loop)
        header[0x1C] = 0x00;
        header[0x1D] = 0x00;
        header[0x1E] = 0x00;
        header[0x1F] = 0x00;

        // Loop # samples (0 = no loop)
        header[0x20] = 0x00;
        header[0x21] = 0x00;
        header[0x22] = 0x00;
        header[0x23] = 0x00;

        u32 rate = m_bPAL ? 50 : 60;
        header[0x24] = (rate >> 0) & 0xFF;
        header[0x25] = (rate >> 8) & 0xFF;
        header[0x26] = (rate >> 16) & 0xFF;
        header[0x27] = (rate >> 24) & 0xFF;

        // SN76489 feedback (0x0009 for SMS/GG)
        header[0x28] = 0x09;
        header[0x29] = 0x00;

        // SN76489 shift register width (16 for SMS/GG)
        header[0x2A] = 0x10;

        // SN76489 Flags (bit 2 = GameGear stereo)
        header[0x2B] = 0x00;

        // VGM data offset (0x0000000C for version 1.50+)
        // Data starts at 0x40, so offset from 0x34 is 0x0C
        header[0x34] = 0x0C;
        header[0x35] = 0x00;
        header[0x36] = 0x00;
        header[0x37] = 0x00;

        // Write header
        file.write(reinterpret_cast<const char*>(header), 256);

        // Write command buffer
        file.write(reinterpret_cast<const char*>(&m_CommandBuffer[0]), m_CommandBuffer.size());

        file.close();
    }

    m_bRecording = false;
    m_CommandBuffer.clear();
}

void VgmRecorder::WritePSG(u8 data)
{
    if (!m_bRecording)
        return;

    FlushPendingWait();
    
    // 0x50 dd - PSG (SN76489/SN76496) write value dd
    WriteCommand(0x50, data);
}

void VgmRecorder::WriteGGStereo(u8 data)
{
    if (!m_bRecording)
        return;

    FlushPendingWait();
    
    // 0x4F dd - Game Gear PSG stereo, write dd to port 0x06
    WriteCommand(0x4F, data);
}

void VgmRecorder::WriteYM2413(u8 port, u8 data)
{
    if (!m_bRecording || !m_bHasYM2413)
        return;

    FlushPendingWait();
    
    // YM2413 uses two ports:
    // 0xF0 = register address select
    // 0xF1 = data write to selected register
    if (port == 0xF0)
    {
        // Store register address for next data write
        m_YM2413Register = data;
    }
    else if (port == 0xF1)
    {
        // 0x51 aa dd - YM2413, write value dd to register aa
        WriteCommand(0x51, m_YM2413Register, data);
    }
}

void VgmRecorder::UpdateTiming(int elapsed_samples)
{
    if (!m_bRecording)
        return;

    m_PendingWait += elapsed_samples;
    m_TotalSamples += elapsed_samples;
}

void VgmRecorder::WriteCommand(u8 command)
{
    m_CommandBuffer.push_back(command);
}

void VgmRecorder::WriteCommand(u8 command, u8 data)
{
    m_CommandBuffer.push_back(command);
    m_CommandBuffer.push_back(data);
}

void VgmRecorder::WriteCommand(u8 command, u8 data1, u8 data2)
{
    m_CommandBuffer.push_back(command);
    m_CommandBuffer.push_back(data1);
    m_CommandBuffer.push_back(data2);
}

void VgmRecorder::WriteWait(int samples)
{
    if (samples <= 0)
        return;

    while (samples > 0)
    {
        if (samples == 735)
        {
            // 0x62 - wait 735 samples (60th of a second)
            WriteCommand(0x62);
            samples -= 735;
        }
        else if (samples == 882)
        {
            // 0x63 - wait 882 samples (50th of a second)
            WriteCommand(0x63);
            samples -= 882;
        }
        else if (samples <= 16)
        {
            // 0x7n - wait n+1 samples, n can range from 0 to 15
            WriteCommand(0x70 + (samples - 1));
            samples = 0;
        }
        else if (samples <= 65535)
        {
            // 0x61 nn nn - Wait n samples
            WriteCommand(0x61);
            m_CommandBuffer.push_back(samples & 0xFF);
            m_CommandBuffer.push_back((samples >> 8) & 0xFF);
            samples = 0;
        }
        else
        {
            // Write maximum wait and continue
            WriteCommand(0x61);
            m_CommandBuffer.push_back(0xFF);
            m_CommandBuffer.push_back(0xFF);
            samples -= 65535;
        }
    }
}

void VgmRecorder::FlushPendingWait()
{
    if (m_PendingWait > 0)
    {
        WriteWait(m_PendingWait);
        m_PendingWait = 0;
    }
}
