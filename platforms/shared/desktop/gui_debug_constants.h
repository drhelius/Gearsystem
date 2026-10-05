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

#ifndef GUI_DEBUG_CONSTANTS_H
#define GUI_DEBUG_CONSTANTS_H

#include "imgui.h"
#include "gearsystem.h"
#include "gui_colors.h"

struct stDebugLabel
{
    u16 address;
    const char* label;
};

enum eDebugIODirection
{
    IO_IN   = 1,
    IO_OUT  = 2,
    IO_BOTH = 3,
};

struct stDebugIOLabel
{
    u16 address;
    const char* label;
    int direction;
};

static const int k_debug_io_label_count = 25;
static const stDebugIOLabel k_debug_io_labels[k_debug_io_label_count] = 
{
    // VDP Ports (0x80-0xBF range, even/odd decoding)
    { 0xBE, "VDP_DATA", IO_BOTH },
    { 0xBF, "VDP_STATUS", IO_IN },
    { 0xBF, "VDP_CTRL", IO_OUT },
    // V/H Counters (0x40-0x7F range, even/odd decoding, read only)
    { 0x7E, "VDP_VCOUNTER", IO_IN },
    { 0x7F, "VDP_HCOUNTER", IO_IN },
    // PSG (0x40-0x7F range, write only)
    { 0x7E, "PSG", IO_OUT },
    { 0x7F, "PSG", IO_OUT },
    // YM2413 FM Synth (0xF0-0xFF range, SMS only)
    { 0xF0, "FM_STATUS", IO_IN },
    { 0xF0, "FM_ADDR", IO_OUT },
    { 0xF1, "FM_STATUS", IO_IN },
    { 0xF1, "FM_DATA", IO_OUT },
    { 0xF2, "FM_DETECT", IO_BOTH },
    // I/O Control (0x00-0x3F odd, write only)
    { 0x3F, "IO_CTRL", IO_OUT },
    // Joypad Ports (0xC0-0xFF range, even/odd decoding, read only)
    { 0xDC, "JOYPAD_1", IO_IN },
    { 0xDD, "JOYPAD_2", IO_IN },
    { 0xC0, "JOYPAD_1", IO_IN },
    { 0xC1, "JOYPAD_2", IO_IN },
    // Memory Control (0x00-0x3F even, write only)
    { 0x3E, "MEM_CTRL", IO_OUT },
    // Game Gear specific (0x00-0x06)
    { 0x00, "GG_START", IO_IN },
    { 0x01, "GG_SERIAL_DATA", IO_BOTH },
    { 0x02, "GG_SERIAL_DIR", IO_BOTH },
    { 0x03, "GG_SERIAL_TX", IO_BOTH },
    { 0x04, "GG_SERIAL_RX", IO_BOTH },
    { 0x05, "GG_SERIAL_STATUS", IO_BOTH },
    { 0x06, "GG_STEREO", IO_OUT },
};

static const int k_debug_symbol_count = 9;

static const stDebugLabel k_debug_symbols[k_debug_symbol_count] = 
{
    { 0x0000, "RST_00" },
    { 0x0008, "RST_08" },
    { 0x0010, "RST_10" },
    { 0x0018, "RST_18" },
    { 0x0020, "RST_20" },
    { 0x0028, "RST_28" },
    { 0x0030, "RST_30" },
    { 0x0038, "RST_38" },
    { 0x0066, "NMI_Interrupt" },
};

#endif /* GUI_DEBUG_CONSTANTS_H */
