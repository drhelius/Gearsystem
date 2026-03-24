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

#ifndef TRACE_LOGGER_H
#define TRACE_LOGGER_H

#include "definitions.h"

#define TRACE_BUFFER_SIZE 100000

enum GS_Trace_Type : u8
{
    TRACE_CPU = 0,
    TRACE_CPU_IRQ,
    TRACE_VDP_WRITE,
    TRACE_VDP_STATUS,
    TRACE_PSG,
    TRACE_YM2413,
    TRACE_IO_PORT,
    TRACE_BANK_SWITCH,
    TRACE_TYPE_COUNT,
};

#define TRACE_FLAG_CPU          (1 << TRACE_CPU)
#define TRACE_FLAG_CPU_IRQ      (1 << TRACE_CPU_IRQ)
#define TRACE_FLAG_VDP_WRITE    (1 << TRACE_VDP_WRITE)
#define TRACE_FLAG_VDP_STATUS   (1 << TRACE_VDP_STATUS)
#define TRACE_FLAG_PSG          (1 << TRACE_PSG)
#define TRACE_FLAG_YM2413       (1 << TRACE_YM2413)
#define TRACE_FLAG_IO_PORT      (1 << TRACE_IO_PORT)
#define TRACE_FLAG_BANK_SWITCH  (1 << TRACE_BANK_SWITCH)
#define TRACE_FLAG_ALL          0xFF

#define GS_VDP_EVENT_VINT        0
#define GS_VDP_EVENT_HINT        1
#define GS_VDP_EVENT_VINT_FLAG   2
#define GS_VDP_EVENT_DISPLAY     3
#define GS_VDP_EVENT_SCROLL_X    4
#define GS_VDP_EVENT_SCROLL_Y    5
#define GS_VDP_EVENT_SPRITE_OVR  6
#define GS_VDP_EVENT_SPRITE_COL  7

struct GS_Trace_Entry
{
    GS_Trace_Type type;
    u64 cycle;
    union
    {
        struct
        {
            u16 pc;
            u8 bank;
            u16 af;
            u16 bc;
            u16 de;
            u16 hl;
            u16 sp;
        } cpu;

        struct
        {
            u16 pc;
            u16 vector;
            u8 type;
        } irq;

        struct
        {
            u8 reg;
            u8 value;
        } vdp_write;

        struct
        {
            u8 event;
            u8 value;
            u16 line;
        } vdp_status;

        struct
        {
            u8 value;
        } psg;

        struct
        {
            u8 port;
            u8 value;
        } ym2413;

        struct
        {
            u8 port;
            u8 value;
            bool is_write;
        } io_port;

        struct
        {
            u16 address;
            u8 value;
        } bank_switch;
    };
};

class TraceLogger
{
public:
    TraceLogger();
    ~TraceLogger();
    void Reset();
    INLINE bool IsEnabled(GS_Trace_Type type) const;
    INLINE void TraceLog(const GS_Trace_Entry& entry);
    void SetEnabledFlags(u32 flags);
    u32 GetEnabledFlags() const;
    const GS_Trace_Entry* GetBuffer() const;
    u32 GetCount() const;
    u32 GetPosition() const;
    u64 GetTotalLogged() const;
    const GS_Trace_Entry& GetEntry(u32 index) const;

private:
    GS_Trace_Entry* m_buffer;
    u32 m_position;
    u32 m_count;
    u32 m_enabled_flags;
    u64 m_total_logged;
};

INLINE bool TraceLogger::IsEnabled(GS_Trace_Type type) const
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    return (m_enabled_flags & (1 << type)) != 0;
#else
    UNUSED(type);
    return false;
#endif
}

INLINE void TraceLogger::TraceLog(const GS_Trace_Entry& entry)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    m_buffer[m_position] = entry;
    m_position = (m_position + 1) % TRACE_BUFFER_SIZE;
    if (m_count < TRACE_BUFFER_SIZE)
        m_count++;
    m_total_logged++;
#else
    UNUSED(entry);
#endif
}

#endif /* TRACE_LOGGER_H */
