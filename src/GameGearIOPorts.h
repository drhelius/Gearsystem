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

#ifndef GAMEGEARIOPORTS_H
#define	GAMEGEARIOPORTS_H

#include "IOPorts.h"

class Audio;
class Video;
class Input;
class Cartridge;
class Memory;
class TraceLogger;

class GameGearIOPorts : public IOPorts
{
public:
    GameGearIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge, Memory* pMemory);
    virtual ~GameGearIOPorts();
    void Reset();
    virtual u8 DoInput(u8 port);
    virtual void DoOutput(u8 port, u8 value);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);
    void SetTraceLogger(TraceLogger* pTraceLogger);

private:
    INLINE void TraceInputReadEvent(u8 port, u8 raw, u8 effective, u8 player);
    INLINE void TraceIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous = 0, u8 auxiliary = 0);
    void LogInputReadEvent(u8 port, u8 raw, u8 effective, u8 player);
    void LogIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary);
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Memory* m_pMemory;
    Cartridge* m_pCartridge;
    TraceLogger* m_pTraceLogger;
    u8 m_Port3F;
    u8 m_Ports[6];
};

#include "Audio.h"
#include "Video.h"
#include "Input.h"
#include "Cartridge.h"
#include "Memory.h"
#include "TraceLogger.h"

INLINE void GameGearIOPorts::TraceInputReadEvent(u8 port, u8 raw, u8 effective, u8 player)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_INPUT, TRACE_INPUT_READ))
        LogInputReadEvent(port, raw, effective, player);
}

INLINE void GameGearIOPorts::TraceIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_IO, event))
        LogIOEvent(event, port, raw, effective, previous, auxiliary);
}

inline u8 GameGearIOPorts::DoInput(u8 port)
{
    if (port < 0x07)
    {
        if (port == 0x00)
        {
            u8 start_button = m_pInput->GetPort00();
            u8 raw = start_button;
            if (m_pCartridge->GetZone() != Cartridge::CartridgeJapanGG)
                start_button |= 0x40;
            TraceInputReadEvent(port, raw, start_button, 1);
            TraceIOEvent(TRACE_IO_GAMEGEAR_READ, port, raw, start_button);
            return start_button;
        }
        else if (port < 6)
        {
            u8 value = m_Ports[port];
            TraceIOEvent(TRACE_IO_GAMEGEAR_READ, port, value, value);
            return value;
        }
        else
            return 0xFF;
    }
    else if (port < 0x40)
    {
        // Reads return $FF (GG)
        Debug("--> ** Attempting to read from port $%X", port);
        return 0xFF;
    }
    else if (port < 0x80)
    {
        // Reads from even addresses return the V counter
        if ((port & 0x01) == 0x00)
        {
            u8 value = m_pVideo->GetVCounter();
            TraceIOEvent(TRACE_IO_COUNTER_READ, port, value, value, 0, 0);
            return value;
        }
        // Reads from odd addresses return the H counter
        else
        {
            u8 value = m_pVideo->GetHCounter();
            TraceIOEvent(TRACE_IO_COUNTER_READ, port, value, value, 0, 1);
            return value;
        }
    }
    else if (port < 0xC0)
    {
        // Reads from even addresses return the VDP data port contents
        if ((port & 0x01) == 0x00)
            return m_pVideo->GetDataPort();
        // Reads from odd address return the VDP status flags
        else
            return m_pVideo->GetStatusFlags();
    }
    else
    {
        switch (port)
        {
            // Reads from $C0 and $DC return the I/O port A/B register.
            case 0xC0:
            case 0xDC:
            {
                u8 value = m_pInput->GetPortDC();
                TraceInputReadEvent(port, value, value, 1);
                return value;
            }
            // Reads from $C1 and $DD return the I/O port B/misc. register.
            case 0xC1:
            case 0xDD:
            {
                u8 raw = m_pInput->GetPortDD();
                u8 value = (raw & 0x3F) | (m_Port3F & 0xC0);
                TraceInputReadEvent(port, raw, value, 2);
                return value;
            }
            // The remaining locations return $FF.
            default:
            {
                Debug("--> ** Attempting to read from port $%X", port);
                return 0xFF;
            }
        }
    }
}

inline void GameGearIOPorts::DoOutput(u8 port, u8 value)
{
    if (port < 0x07)
    {
        if (port == 0x06)
        {
            // SN76489 PSG
            m_pAudio->WriteGGStereoRegister(value);
        }
        else if (port != 0x00)
        {
            u8 previous = m_Ports[port];
            m_Ports[port] = value;
            TraceIOEvent(TRACE_IO_GAMEGEAR_WRITE, port, value, m_Ports[port], previous);
        }
    }
    else if (port < 0x40)
    {
        // Writes to even addresses go to memory control register.
        if ((port & 0x01) == 0x00)
        {
            Debug("--> ** Output to memory control port $%X: %X", port, value);
            m_pMemory->SetPort3E(value);
            TraceIOEvent(TRACE_IO_CONTROL, port, value, value);
        }
        // Writes to odd addresses go to I/O control register.
        else
        {
            u8 previous = m_Port3F;
            bool th_changed_a = (value & 0x02) && (value & 0x20) && !(m_Port3F & 0x20);
            bool th_changed_b = (value & 0x08) && (value & 0x80) && !(m_Port3F & 0x80);

            if (th_changed_a || th_changed_b)
            {
                m_pVideo->LatchHCounter();
                TraceIOEvent(TRACE_IO_COUNTER_LATCH, port, value, 0, previous, (th_changed_a ? 1 : 0) | (th_changed_b ? 2 : 0));
            }

            m_Port3F = value;
            TraceIOEvent(TRACE_IO_CONTROL, port, value, m_Port3F, previous);
        }
    }
    else if (port < 0x80)
    {
        // Writes to any address go to the SN76489 PSG
        m_pAudio->WriteAudioRegister(value);
    }
    else if (port < 0xC0)
    {
        // Writes to even addresses go to the VDP data port.
        if ((port & 0x01) == 0x00)
            m_pVideo->WriteData(value);
        // Writes to odd addresses go to the VDP control port.
        else
            m_pVideo->WriteControl(value);
    }
#ifdef GS_DEBUG
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

#endif	/* GAMEGEARIOPORTS_H */
