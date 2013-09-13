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

#include "SmsIOPorts.h"
#include "Audio.h"
#include "Video.h"
#include "Input.h"
#include "Cartridge.h"

SmsIOPorts::SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge)
{
    m_pAudio = pAudio;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_Port3F = 0;
}

SmsIOPorts::~SmsIOPorts()
{
}

u8 SmsIOPorts::DoInput(u8 port)
{
    if (port < 0x40)
    {
        // Reads return $FF (SMS2)
        Log("--> ** Attempting to read from port $%X", port);
        return 0xFF;
    }
    else if ((port >= 0x40) && (port < 0x80))
    {
        // Reads from even addresses return the V counter
        // Reads from odd addresses return the H counter
        if ((port & 0x01) == 0x00)
            return m_pVideo->GetVCounter();
        else
            return m_pVideo->GetHCounter();
    }
    else if ((port >= 0x80) && (port < 0xC0))
    {
        // Reads from even addresses return the VDP data port contents
        // Reads from odd address return the VDP status flags
        if ((port & 0x01) == 0x00)
            return m_pVideo->GetDataPort();
        else
            return m_pVideo->GetStatusFlags();
    }
    else
    {
        // Reads from even addresses return the I/O port A/B register
        // Reads from odd address return the I/O port B/misc. register
        if ((port & 0x01) == 0x00)
            return m_pInput->GetPortDC();
        else
            return ((m_pInput->GetPortDD() & 0x3F) | (m_Port3F & 0xC0));
    }
}

void SmsIOPorts::DoOutput(u8 port, u8 value)
{
    if (port < 0x40)
    {
        // Writes to even addresses go to memory control register.
        // Writes to odd addresses go to I/O control register.
        if ((port & 0x01) == 0x00)
        {
            Log("--> ** Attempting to write to memory control port $%X: %X", port, value);
        }
        else
        {
            m_Port3F =  ((value & 0x80) | (value & 0x20) << 1) & 0xC0;
            if (m_pCartridge->GetZone() == Cartridge::CartridgeExportSMS)
                m_Port3F ^= 0xC0;
        }
    }
    else if ((port >= 0x40) && (port < 0x80))
    {
        // Writes to any address go to the SN76489 PSG
        m_pAudio->WriteAudioRegister(value);
    }
    else if ((port >= 0x80) && (port < 0xC0))
    {
        // Writes to even addresses go to the VDP data port.
        // Writes to odd addresses go to the VDP control port.
        if ((port & 0x01) == 0x00)
            m_pVideo->WriteData(value);
        else
            m_pVideo->WriteControl(value);
    }
    else
    {
        // Writes have no effect.
        if ((port == 0xDE) || (port == 0xDF))
        {
            Log("--> ** Attempting to write to keyboard port $%X: %X", port, value);
        }
        else
        {
            Log("--> ** Attempting to write to port $%X: %X", port, value);
        }      
    }
}
