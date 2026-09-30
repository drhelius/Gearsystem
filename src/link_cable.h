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

#ifndef LINK_CABLE_H
#define LINK_CABLE_H

#include "definitions.h"

#define LINK_CABLE_MAX_PEERS 2
#define LINK_CABLE_MAX_SYNC_CYCLES 32
#define LINK_CABLE_MAX_LEAD_CYCLES 64

enum GS_LinkCable_Protocol
{
    LinkCableProtocolNone, LinkCableProtocolGearToGear, LinkCableProtocolMarkIII
};

struct GS_LinkCable_WireState
{
    u8 drive_mask;
    u8 levels;
};

struct GS_LinkCable_WireEvent
{
    u64 cycle;
    GS_LinkCable_WireState state;
};

typedef void (*GS_LinkCable_Publish_Callback)(u64 cycle, const GS_LinkCable_WireState* state, void* user_data);
typedef bool (*GS_LinkCable_Sample_Callback)(u64 cycle, GS_LinkCable_WireState* state, void* user_data);
typedef bool (*GS_LinkCable_Poll_Callback)(u64 through_cycle, GS_LinkCable_WireEvent* event, void* user_data);
typedef void (*GS_LinkCable_Fence_Callback)(u64 cycle, void* user_data);
typedef void (*GS_LinkCable_Sync_Callback)(u64 cycle, u32 lead_cycles, void* user_data);

#endif /* LINK_CABLE_H */
