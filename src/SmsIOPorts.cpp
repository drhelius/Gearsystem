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

SmsIOPorts::SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge, Memory* pMemory, Processor* pProcessor)
{
    m_pAudio = pAudio;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    InitPointer(m_pTraceLogger);
    Reset();
}

SmsIOPorts::~SmsIOPorts()
{
}

void SmsIOPorts::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void SmsIOPorts::LogInputReadEvent(u8 port, u8 raw, u8 effective, u8 player)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    u8 device = m_pInput->IsPaddleEnabled() ? 2 : (m_pInput->IsPhaserEnabled() ? 1 : 0);
    if ((player > 0) && m_pInput->IsSportsPadEnabled((GS_Joypads)(player - 1)))
        device = 3;
    GS_Trace_Entry e = {};
    e.type = TRACE_INPUT;
    e.input.event = TRACE_INPUT_READ;
    e.input.port = port;
    e.input.raw = raw;
    e.input.effective = effective;
    e.input.control = m_Port3F;
    e.input.player = player;
    e.input.device = device;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(port);
    UNUSED(raw);
    UNUSED(effective);
    UNUSED(player);
#endif
}

void SmsIOPorts::LogIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    if (event == TRACE_IO_COUNTER_LATCH)
        effective = m_pVideo->GetHCounter();
    GS_Trace_Entry e = {};
    e.type = TRACE_IO;
    e.io.event = event;
    e.io.port = port;
    e.io.raw = raw;
    e.io.effective = effective;
    e.io.previous = previous;
    e.io.auxiliary = auxiliary;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(port);
    UNUSED(raw);
    UNUSED(effective);
    UNUSED(previous);
    UNUSED(auxiliary);
#endif
}

void SmsIOPorts::Reset()
{
    m_Port3F = 0xFF;
}

void SmsIOPorts::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_Port3F), sizeof(m_Port3F));
}

void SmsIOPorts::LoadState(std::istream& stream, int version)
{
    using namespace std;
    UNUSED(version);

    stream.read(reinterpret_cast<char*> (&m_Port3F), sizeof(m_Port3F));
}
