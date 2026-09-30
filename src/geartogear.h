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

#include "link_cable.h"

struct GS_GearToGear_DebugState
{
    u8 parallel_data;
    u8 direction_nint;
    u8 tx_data;
    u8 rx_data;
    u8 serial_control;
    u8 serial_status;
    GS_LinkCable_WireState local_state;
    GS_LinkCable_WireState remote_state;
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

#endif /* GEARTOGEAR_H */
