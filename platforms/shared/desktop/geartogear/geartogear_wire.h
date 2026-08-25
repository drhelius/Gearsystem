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

#ifndef GEARTOGEAR_WIRE_H
#define GEARTOGEAR_WIRE_H

#include <atomic>
#include "geartogear.h"

struct GearToGearLocalWireEvent
{
    u64 cycle;
    u8 drive_mask;
    u8 levels;
};

struct GearToGearSharedWireEvent
{
    std::atomic<u32> sequence;
    std::atomic<u32> generation;
    std::atomic<u64> cycle;
    std::atomic<u32> wire_state;

    GearToGearSharedWireEvent() : sequence(0), generation(0), cycle(0), wire_state(0) {}
};

static_assert(alignof(GearToGearSharedWireEvent) >= alignof(std::atomic<u64>),
    "Gear-to-Gear shared events require aligned 64-bit atomics");

inline u32 geartogear_pack_wire_state(u8 drive_mask, u8 levels)
{
    return (u32)(drive_mask & 0x7F) | ((u32)(levels & 0x7F) << 8);
}

inline void geartogear_unpack_wire_state(u32 packed, u8& drive_mask, u8& levels)
{
    drive_mask = (u8)(packed & 0x7F);
    levels = (u8)((packed >> 8) & 0x7F);
}

inline bool geartogear_shared_event_atomics_lock_free(const GearToGearSharedWireEvent& event)
{
    return event.sequence.is_lock_free() && event.generation.is_lock_free() &&
        event.cycle.is_lock_free() && event.wire_state.is_lock_free();
}

inline void geartogear_publish_shared_event(GearToGearSharedWireEvent& event, u32 generation, u64 cycle, u8 drive_mask, u8 levels)
{
    u32 sequence = event.sequence.load(std::memory_order_relaxed);
    u32 busy_sequence = (sequence + 1) | 1u;

    event.sequence.exchange(busy_sequence, std::memory_order_acq_rel);
    event.generation.store(generation, std::memory_order_relaxed);
    event.cycle.store(cycle, std::memory_order_relaxed);
    event.wire_state.store(geartogear_pack_wire_state(drive_mask, levels), std::memory_order_relaxed);
    event.sequence.store(busy_sequence + 1, std::memory_order_release);
}

inline bool geartogear_read_shared_event(const GearToGearSharedWireEvent& source, u32 expected_generation, GearToGearLocalWireEvent& event)
{
    u32 before = source.sequence.load(std::memory_order_acquire);

    if ((before & 1) != 0)
        return false;

    u32 generation = source.generation.load(std::memory_order_relaxed);
    event.cycle = source.cycle.load(std::memory_order_relaxed);
    u32 packed = source.wire_state.load(std::memory_order_relaxed);

    std::atomic_thread_fence(std::memory_order_acquire);
    u32 after = source.sequence.load(std::memory_order_acquire);

    if (before != after || (after & 1) != 0 || generation != expected_generation)
    {
        return false;
    }

    geartogear_unpack_wire_state(packed, event.drive_mask, event.levels);
    return true;
}

inline u8 geartogear_map_remote_bits_to_local(u8 value)
{
    static const u8 remote_to_local[7] = { 2, 3, 0, 1, 5, 4, 6 };
    u8 mapped = 0;

    for (int remote_bit = 0; remote_bit < 7; remote_bit++)
    {
        if (value & (1 << remote_bit))
            mapped |= (u8)(1 << remote_to_local[remote_bit]);
    }

    return mapped & 0x7F;
}

#endif /* GEARTOGEAR_WIRE_H */
