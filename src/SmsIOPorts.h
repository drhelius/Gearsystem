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

#ifndef SMSIOPORTS_H
#define	SMSIOPORTS_H

#include "IOPorts.h"

class Audio;
class Video;
class Input;
class Cartridge;
class Memory;
class Processor;

class SmsIOPorts : public IOPorts
{
public:
    SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge, Memory* pMemory, Processor* pProcessor);
    ~SmsIOPorts();
    void Reset();
    u8 DoInput(u8 port);
    void DoOutput(u8 port, u8 value);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);
private:
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    Memory* m_pMemory;
    Processor* m_pProcessor;

    u8 m_Port3F;
    u8 m_Port3F_HC;
};

#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "Memory.h"
#include "Processor.h"
#include "YM2413.h"

inline u8 SmsIOPorts::DoInput(u8 port)
{
    if (port < 0x40)
    {
        // Reads return $FF (SMS2)
        Log("--> ** Attempting to read from port $%02X", port);
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
        if (m_pMemory->IsIOEnabled())
        {
            // Reads from even addresses return the I/O port A/B register
            // Reads from odd address return the I/O port B/misc. register
            if ((port & 0x01) == 0x00)
                return m_pInput->GetPortDC();
            else
                return ((m_pInput->GetPortDD() & 0x3F) | (m_Port3F & 0xC0));
        }
        else
        {
            return m_pAudio->YM2413Read(port);
        }
    }
}

inline void SmsIOPorts::DoOutput(u8 port, u8 value)
{
    if (port < 0x40)
    {
        // Writes to even addresses go to memory control register.
        // Writes to odd addresses go to I/O control register.
        if ((port & 0x01) == 0x00)
        {
            Log("--> ** Output to memory control port $%02X: %02X", port, value);
            m_pMemory->SetPort3E(value);
        }
        else
        {
            if (((value  & 0x01) && !(m_Port3F_HC & 0x01)) || ((value  & 0x08) && !(m_Port3F_HC & 0x08)))
                m_pVideo->LatchHCounter();
            m_Port3F_HC = value & 0x05;

            m_Port3F =  ((value & 0x80) | (value & 0x20) << 1) & 0xC0;
            if (m_pCartridge->GetZone() == Cartridge::CartridgeJapanSMS)
                m_Port3F ^= 0xC0;
        }
    }
    else if ((port >= 0x40) && (port < 0x80))
    {
        // Writes to any address go to the SN76489 PSG
        m_pAudio->WriteAudioRegister(value);
        if (m_pCartridge->IsSG1000())
            m_pProcessor->InjectTStates(32);
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
        if ((port == 0xF0) || (port == 0xF1) || (port == 0xF2))
        {
            m_pAudio->YM2413Write(port, value);
        }
#ifdef DEBUG_GEARSYSTEM
        else if ((port == 0xDE) || (port == 0xDF))
        {
            Log("--> ** Output to keyboard port $%02X: %02X", port, value);
        }
        else
        {
            Log("--> ** Output to port $%02X: %02X", port, value);
        }
#endif
    }
}

#endif	/* SMSIOPORTS_H */
