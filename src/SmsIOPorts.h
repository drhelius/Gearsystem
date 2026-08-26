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
class TraceLogger;

class SmsIOPorts : public IOPorts
{
public:
    SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge, Memory* pMemory, Processor* pProcessor);
    ~SmsIOPorts();
    void Reset();
    u8 DoInput(u8 port);
    void DoOutput(u8 port, u8 value);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream, int version);
    void SetTraceLogger(TraceLogger* pTraceLogger);

private:
    INLINE void TraceInputReadEvent(u8 port, u8 raw, u8 effective, u8 player);
    INLINE void TraceIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous = 0, u8 auxiliary = 0);
    void LogInputReadEvent(u8 port, u8 raw, u8 effective, u8 player);
    void LogIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary);
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    Memory* m_pMemory;
    Processor* m_pProcessor;
    TraceLogger* m_pTraceLogger;
    u8 m_Port3F;
};

#include "Video.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "Memory.h"
#include "Processor.h"
#include "YM2413.h"
#include "TraceLogger.h"

INLINE void SmsIOPorts::TraceInputReadEvent(u8 port, u8 raw, u8 effective, u8 player)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_INPUT, TRACE_INPUT_READ))
        LogInputReadEvent(port, raw, effective, player);
}

INLINE void SmsIOPorts::TraceIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_IO, event))
        LogIOEvent(event, port, raw, effective, previous, auxiliary);
}

inline u8 SmsIOPorts::DoInput(u8 port)
{
    if (port < 0x40)
    {
        Debug("--> ** Attempting to read from port $%02X", port);
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
        if (port >= 0xF0)
        {
            return m_pAudio->YM2413Read();
        }
        else
        {
            // Reads from even addresses return the I/O port A/B register
            if ((port & 0x01) == 0x00)
            {
                u8 ret_dc = m_pInput->GetPortDC();
                u8 raw = ret_dc;
                if (!(m_Port3F & 0x01))
                {
                    ret_dc &= 0xDF;
                    ret_dc |= (m_Port3F & 0x10) << 1;
                }
                TraceInputReadEvent(port, raw, ret_dc, 1);
                return ret_dc;
            }
            // Reads from odd address return the I/O port B/misc. register
            else
            {
                u8 ret_dd = m_pInput->GetPortDD();
                u8 raw = ret_dd;

                if (m_pCartridge->GetZone() != Cartridge::CartridgeJapanSMS)
                {
                    if (!(m_Port3F & 0x02))
                    {
                        ret_dd &= 0xbf;
                        ret_dd |= (m_Port3F & 0x20) << 1;
                    }
                    if (!(m_Port3F & 0x08))
                    {
                        ret_dd &= 0x7F;
                        ret_dd |= (m_Port3F & 0x80);
                    }
                }

                if (!(m_Port3F & 0x04))
                {
                    ret_dd &= 0xF7;
                    ret_dd |= (m_Port3F & 0x40) >> 3;
                }

                TraceInputReadEvent(port, raw, ret_dd, 2);
                return ret_dd;
            }
        }
    }
}

inline void SmsIOPorts::DoOutput(u8 port, u8 value)
{
    if (port < 0x40)
    {
        // Writes to even addresses go to memory control register.
        if ((port & 0x01) == 0x00)
        {
            Debug("--> ** Output to memory control port $%02X: %02X", port, value);
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
        if (m_pCartridge->IsSG1000())
            m_pProcessor->InjectTStates(32);
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
    else if (port >= 0xF0)
    {
        m_pAudio->YM2413Write(port, value);
    }
#if 0
    else if ((port == 0xDE) || (port == 0xDF))
    {
        Debug("--> ** Output to keyboard port $%02X: %02X", port, value);
    }
#endif
}

#endif	/* SMSIOPORTS_H */
