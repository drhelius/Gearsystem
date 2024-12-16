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

#include "GameGearIOPorts.h"
#include "Audio.h"
#include "Video.h"
#include "Input.h"
#include "Cartridge.h"
#include "Memory.h"

GameGearIOPorts::GameGearIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge, Memory* pMemory)
{
    m_pAudio = pAudio;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_pMemory = pMemory;
    m_Port3F = 0;
    m_Port3F_HC = 0;
}

GameGearIOPorts::~GameGearIOPorts()
{
}

void GameGearIOPorts::Reset()
{
    m_Port3F = 0;
    m_Port3F_HC = 0;
    m_Port2 = 0;
}

u8 GameGearIOPorts::DoInput(u8 port)
{
    if (port < 0x07)
    {
        switch (port)
        {
            case 0x00:
            {
                u8 port00 = m_pInput->GetPort00();
                if (m_pCartridge->GetZone() != Cartridge::CartridgeJapanGG)
                    port00 |= 0x40;
                return port00;
            }
            case 0x01:
                return 0x7F;
            case 0x02:
                return m_Port2;
            case 0x03:
            case 0x05:
                return 0x00;
            default:
                return 0xFF;
        }
    }
    else if (port < 0x40)
    {
        // Reads return $FF (GG)
        Debug("--> ** Attempting to read from port $%X", port);
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
        // Reads from $C0 and $DC return the I/O port A/B register.
        // Reads from $C1 and $DD return the I/O port B/misc. register.
        // The remaining locations return $FF.
        switch (port)
        {
            case 0xC0:
            case 0xDC:
            {
                return m_pInput->GetPortDC();
            }
            case 0xC1:
            case 0xDD:
            {
                return ((m_pInput->GetPortDD() & 0x3F) | (m_Port3F & 0xC0));
            }
            default:
            {
                Debug("--> ** Attempting to read from port $%X", port);
                return 0xFF;
            }
        }
    }
}

void GameGearIOPorts::DoOutput(u8 port, u8 value)
{
    if (port < 0x07)
    {
        if (port == 0x06)
        {
            // SN76489 PSG
            m_pAudio->WriteGGStereoRegister(value);
        }
        else if (port == 0x02)
        {
            m_Port2 = value;
        }
    }
    else if (port < 0x40)
    {
        // Writes to even addresses go to memory control register.
        // Writes to odd addresses go to I/O control register.
        if ((port & 0x01) == 0x00)
        {
            Debug("--> ** Output to memory control port $%X: %X", port, value);
            m_pMemory->SetPort3E(value);
        }
        else
        {
            if (((value  & 0x01) && !(m_Port3F_HC & 0x01)) || ((value  & 0x08) && !(m_Port3F_HC & 0x08)))
                m_pVideo->LatchHCounter();
            m_Port3F_HC = value & 0x05;

            m_Port3F =  ((value & 0x80) | (value & 0x20) << 1) & 0xC0;
            if (m_pCartridge->GetZone() == Cartridge::CartridgeJapanGG)
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
#ifdef DEBUG_GEARSYSTEM
    else
    {
        // Writes have no effect.
        if ((port == 0xDE) || (port == 0xDF))
        {
            Debug("--> ** Output to keyboard port $%X: %X", port, value);
        }
        else if ((port == 0xF0) || (port == 0xF1) || (port == 0xF2))
        {
            Debug("--> ** Output to YM2413 port $%X: %X", port, value);
        }
        else
        {
            Debug("--> ** Output to port $%X: %X", port, value);
        }
    }
#endif
}

void GameGearIOPorts::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_Port3F), sizeof(m_Port3F));
    stream.write(reinterpret_cast<const char*> (&m_Port3F_HC), sizeof(m_Port3F_HC));
}

void GameGearIOPorts::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_Port3F), sizeof(m_Port3F));
    stream.read(reinterpret_cast<char*> (&m_Port3F_HC), sizeof(m_Port3F_HC));
}
