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

#ifndef GEARTOGEAR_H
#define GEARTOGEAR_H

#include "definitions.h"

#define GEARTOGEAR_MAX_PEERS 2
#define GEARTOGEAR_MAX_SYNC_CYCLES 32
#define GEARTOGEAR_MAX_LEAD_CYCLES 64

struct GS_GearToGear_WireState
{
    u8 drive_mask;
    u8 levels;
};

struct GS_GearToGear_WireEvent
{
    u64 cycle;
    GS_GearToGear_WireState state;
};

struct GS_GearToGear_DebugState
{
    u8 parallel_data;
    u8 direction_nint;
    u8 tx_data;
    u8 rx_data;
    u8 serial_control;
    u8 serial_status;
    GS_GearToGear_WireState local_state;
    GS_GearToGear_WireState remote_state;
    u8 resolved_pins;
    u8 contention_mask;
    bool tx_busy;
    bool tx_line;
    u8 tx_frame_data;
    u8 tx_phase;
    u32 tx_bit_cycles;
    u64 tx_next_cycle;
    u8 rx_state;
    u8 rx_shift;
    u8 rx_bit;
    u32 rx_bit_cycles;
    u64 rx_next_cycle;
    bool rx_ready;
    bool frame_error;
    bool parallel_nmi;
    bool serial_nmi;
    bool nmi_asserted;
    bool nint_armed;
    u8 nint_arm_delay;
    u64 cycle;
};

typedef void (*GS_GearToGear_Publish_Callback)(u64 cycle, const GS_GearToGear_WireState* state, void* user_data);
typedef bool (*GS_GearToGear_Sample_Callback)(u64 cycle, GS_GearToGear_WireState* state, void* user_data);
typedef bool (*GS_GearToGear_Poll_Callback)(u64 through_cycle, GS_GearToGear_WireEvent* event, void* user_data);
typedef void (*GS_GearToGear_Fence_Callback)(u64 cycle, void* user_data);
typedef void (*GS_GearToGear_Sync_Callback)(u64 cycle, u32 lead_cycles, void* user_data);

#endif /* GEARTOGEAR_H */
