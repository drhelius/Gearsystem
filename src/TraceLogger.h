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
    TRACE_VDP,
    TRACE_INPUT,
    TRACE_IO,
    TRACE_PSG,
    TRACE_YM2413,
    TRACE_MAPPER,
    TRACE_TYPE_COUNT,
};

static_assert(TRACE_TYPE_COUNT < 32, "Trace category flags exceed 32 bits");

#define TRACE_VDP_WRITE TRACE_VDP
#define TRACE_VDP_STATUS TRACE_VDP
#define TRACE_IO_PORT TRACE_IO
#define TRACE_BANK_SWITCH TRACE_MAPPER

#define TRACE_FLAG_CPU          (1U << TRACE_CPU)
#define TRACE_FLAG_CPU_IRQ      (1U << TRACE_CPU_IRQ)
#define TRACE_FLAG_VDP          (1U << TRACE_VDP)
#define TRACE_FLAG_INPUT        (1U << TRACE_INPUT)
#define TRACE_FLAG_IO           (1U << TRACE_IO)
#define TRACE_FLAG_PSG          (1U << TRACE_PSG)
#define TRACE_FLAG_YM2413       (1U << TRACE_YM2413)
#define TRACE_FLAG_MAPPER       (1U << TRACE_MAPPER)
#define TRACE_FLAG_ALL          ((1U << TRACE_TYPE_COUNT) - 1U)

#define TRACE_FLAG_VDP_WRITE TRACE_FLAG_VDP
#define TRACE_FLAG_VDP_STATUS TRACE_FLAG_VDP
#define TRACE_FLAG_IO_PORT TRACE_FLAG_IO
#define TRACE_FLAG_BANK_SWITCH TRACE_FLAG_MAPPER

enum GS_Trace_VDP_Event : u8
{
    TRACE_VDP_REG_WRITE = 0,
    TRACE_VDP_VINT_REQUEST,
    TRACE_VDP_HINT_REQUEST,
    TRACE_VDP_VINT_FLAG,
    TRACE_VDP_STATUS_READ,
    TRACE_VDP_SPRITE_OVERFLOW,
    TRACE_VDP_SPRITE_COLLISION,
    TRACE_VDP_DISPLAY_LATCH,
    TRACE_VDP_SCROLL_X_LATCH,
    TRACE_VDP_SCROLL_Y_LATCH,
    TRACE_VDP_MODE_CHANGE,
    TRACE_VDP_TIMING,
    TRACE_VDP_CONTROL,
    TRACE_VDP_DATA_READ,
    TRACE_VDP_DATA_WRITE,
    TRACE_VDP_CRAM_WRITE,
};

#define TRACE_VDP_EVENT_REGISTERS  (1U << TRACE_VDP_REG_WRITE)
#define TRACE_VDP_EVENT_INTERRUPTS ((1U << TRACE_VDP_VINT_REQUEST) | (1U << TRACE_VDP_HINT_REQUEST))
#define TRACE_VDP_EVENT_STATUS     ((1U << TRACE_VDP_VINT_FLAG) | (1U << TRACE_VDP_STATUS_READ))
#define TRACE_VDP_EVENT_SPRITES    ((1U << TRACE_VDP_SPRITE_OVERFLOW) | (1U << TRACE_VDP_SPRITE_COLLISION))
#define TRACE_VDP_EVENT_STATE      ((1U << TRACE_VDP_DISPLAY_LATCH) | (1U << TRACE_VDP_SCROLL_X_LATCH) | (1U << TRACE_VDP_SCROLL_Y_LATCH) | (1U << TRACE_VDP_MODE_CHANGE) | (1U << TRACE_VDP_TIMING))
#define TRACE_VDP_EVENT_DATA       ((1U << TRACE_VDP_CONTROL) | (1U << TRACE_VDP_DATA_READ) | (1U << TRACE_VDP_DATA_WRITE))
#define TRACE_VDP_EVENT_CRAM       (1U << TRACE_VDP_CRAM_WRITE)
#define TRACE_VDP_EVENT_ALL        (TRACE_VDP_EVENT_REGISTERS | TRACE_VDP_EVENT_INTERRUPTS | TRACE_VDP_EVENT_STATUS | TRACE_VDP_EVENT_SPRITES | TRACE_VDP_EVENT_STATE | TRACE_VDP_EVENT_DATA | TRACE_VDP_EVENT_CRAM)

enum GS_Trace_Input_Event : u8
{
    TRACE_INPUT_READ = 0,
    TRACE_INPUT_CHANGE,
};

#define TRACE_INPUT_EVENT_READS   (1U << TRACE_INPUT_READ)
#define TRACE_INPUT_EVENT_CHANGES (1U << TRACE_INPUT_CHANGE)
#define TRACE_INPUT_EVENT_ALL     (TRACE_INPUT_EVENT_READS | TRACE_INPUT_EVENT_CHANGES)

enum GS_Trace_IO_Event : u8
{
    TRACE_IO_CONTROL = 0,
    TRACE_IO_COUNTER_READ,
    TRACE_IO_COUNTER_LATCH,
    TRACE_IO_GAMEGEAR_READ,
    TRACE_IO_GAMEGEAR_WRITE,
};

#define TRACE_IO_EVENT_CONTROL  (1U << TRACE_IO_CONTROL)
#define TRACE_IO_EVENT_COUNTERS ((1U << TRACE_IO_COUNTER_READ) | (1U << TRACE_IO_COUNTER_LATCH))
#define TRACE_IO_EVENT_GAMEGEAR ((1U << TRACE_IO_GAMEGEAR_READ) | (1U << TRACE_IO_GAMEGEAR_WRITE))
#define TRACE_IO_EVENT_ALL      (TRACE_IO_EVENT_CONTROL | TRACE_IO_EVENT_COUNTERS | TRACE_IO_EVENT_GAMEGEAR)

enum GS_Trace_PSG_Event : u8
{
    TRACE_PSG_TONE = 0,
    TRACE_PSG_VOLUME,
    TRACE_PSG_NOISE,
    TRACE_PSG_STEREO,
};

#define TRACE_PSG_EVENT_TONE   (1U << TRACE_PSG_TONE)
#define TRACE_PSG_EVENT_VOLUME (1U << TRACE_PSG_VOLUME)
#define TRACE_PSG_EVENT_NOISE  (1U << TRACE_PSG_NOISE)
#define TRACE_PSG_EVENT_STEREO (1U << TRACE_PSG_STEREO)
#define TRACE_PSG_EVENT_ALL    (TRACE_PSG_EVENT_TONE | TRACE_PSG_EVENT_VOLUME | TRACE_PSG_EVENT_NOISE | TRACE_PSG_EVENT_STEREO)

enum GS_Trace_YM2413_Event : u8
{
    TRACE_YM2413_REGISTER = 0,
    TRACE_YM2413_MIXER,
};

#define TRACE_YM2413_EVENT_REGISTERS (1U << TRACE_YM2413_REGISTER)
#define TRACE_YM2413_EVENT_MIXER     (1U << TRACE_YM2413_MIXER)
#define TRACE_YM2413_EVENT_ALL       (TRACE_YM2413_EVENT_REGISTERS | TRACE_YM2413_EVENT_MIXER)

enum GS_Trace_Mapper_Event : u8
{
    TRACE_MAPPER_ROM = 0,
    TRACE_MAPPER_RAM,
    TRACE_MAPPER_CONTROL,
    TRACE_MAPPER_EEPROM,
    TRACE_MAPPER_FLASH,
};

#define TRACE_MAPPER_EVENT_ROM     (1U << TRACE_MAPPER_ROM)
#define TRACE_MAPPER_EVENT_RAM     (1U << TRACE_MAPPER_RAM)
#define TRACE_MAPPER_EVENT_CONTROL (1U << TRACE_MAPPER_CONTROL)
#define TRACE_MAPPER_EVENT_EEPROM  (1U << TRACE_MAPPER_EEPROM)
#define TRACE_MAPPER_EVENT_FLASH   (1U << TRACE_MAPPER_FLASH)
#define TRACE_MAPPER_EVENT_ALL     (TRACE_MAPPER_EVENT_ROM | TRACE_MAPPER_EVENT_RAM | TRACE_MAPPER_EVENT_CONTROL | TRACE_MAPPER_EVENT_EEPROM | TRACE_MAPPER_EVENT_FLASH)

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
            u16 bank;
            u16 af;
            u16 bc;
            u16 de;
            u16 hl;
            u16 ix;
            u16 iy;
            u16 sp;
            u8 i;
            u8 r;
            u8 im;
            u8 size;
            u8 opcodes[7];
            bool iff1;
            bool iff2;
            bool halt;
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
            u8 event;
            u8 reg;
            u8 raw;
            u8 effective;
            u8 code;
            u8 status_before;
            u8 status_after;
            u16 address;
            u16 line;
            u16 hpos;
            u16 auxiliary;
        } vdp;

        struct
        {
            u8 event;
            u8 port;
            u8 raw;
            u8 effective;
            u8 control;
            u8 player;
            u8 device;
        } input;

        struct
        {
            u8 event;
            u8 port;
            u8 raw;
            u8 effective;
            u8 previous;
            u8 auxiliary;
        } io;

        struct
        {
            u8 value;
            u8 event;
            u8 channel;
            u8 latch;
            u8 attenuation;
            u16 period;
        } psg;

        struct
        {
            u8 port;
            u8 value;
            u8 event;
            u8 reg;
            u8 effective;
            bool accepted;
            bool psg_enabled;
            bool fm_enabled;
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

        struct
        {
            u8 event;
            u8 mapper;
            u8 value;
            u8 flags;
            u8 flags_valid;
            u16 address;
            u16 banks[6];
            s16 ram_bank;
            u16 auxiliary;
        } mapper;
    };
};

static_assert(sizeof(GS_Trace_Entry) <= 48, "GS_Trace_Entry exceeds its memory budget");

class TraceLogger
{
public:
    TraceLogger(const u64* master_clock_cycles = NULL);
    ~TraceLogger();
    void Reset();
    bool SetCapacity(u32 capacity);
    INLINE bool IsEnabled(GS_Trace_Type type) const;
    INLINE bool IsEventEnabled(GS_Trace_Type type, u8 event) const;
    INLINE void TraceLog(const GS_Trace_Entry& entry);
    void SetEnabledFlags(u32 flags);
    void SetEventFilter(GS_Trace_Type type, u32 filter);
    u32 GetEnabledFlags() const;
    u32 GetEventFilter(GS_Trace_Type type) const;
    const GS_Trace_Entry* GetBuffer() const;
    u32 GetCount() const;
    u32 GetCapacity() const;
    u32 GetPosition() const;
    u64 GetTotalLogged() const;
    u64 GetSequence() const;
    const GS_Trace_Entry& GetEntry(u32 index) const;

private:
#if !defined(GS_DISABLE_DISASSEMBLER)
    void UpdateEnabled();
#endif
    GS_Trace_Entry* m_buffer;
    u32 m_position;
    u32 m_count;
    u32 m_capacity;
    u32 m_enabled_flags;
#if !defined(GS_DISABLE_DISASSEMBLER)
    bool m_enabled;
#endif
    u32 m_event_filters[TRACE_TYPE_COUNT];
    u64 m_total_logged;
    u64 m_sequence;
    const u64* m_master_clock_cycles;
};

INLINE bool TraceLogger::IsEnabled(GS_Trace_Type type) const
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    if (likely(!m_enabled))
        return false;

    return type < TRACE_TYPE_COUNT && (m_enabled_flags & (1U << type)) != 0;
#else
    UNUSED(type);
    return false;
#endif
}

INLINE bool TraceLogger::IsEventEnabled(GS_Trace_Type type, u8 event) const
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    if (likely(!m_enabled))
        return false;

    return type < TRACE_TYPE_COUNT && (m_enabled_flags & (1U << type)) != 0 &&
        event < 32 && (m_event_filters[type] & (1U << event)) != 0;
#else
    UNUSED(type);
    UNUSED(event);
    return false;
#endif
}

INLINE void TraceLogger::TraceLog(const GS_Trace_Entry& entry)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    m_buffer[m_position] = entry;
    if (IsValidPointer(m_master_clock_cycles))
        m_buffer[m_position].cycle = *m_master_clock_cycles;
    m_position++;
    if (m_position == m_capacity)
        m_position = 0;
    if (m_count < m_capacity)
        m_count++;
    m_total_logged++;
    m_sequence++;
#else
    UNUSED(entry);
#endif
}

#endif /* TRACE_LOGGER_H */
