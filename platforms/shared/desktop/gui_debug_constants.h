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
#include "config.h"

struct GuiDebugColor
{
    ImVec4 dark;
    ImVec4 light;

    operator ImVec4() const
    {
        return (config_emulator.theme == config_Theme_Light) ? light : dark;
    }
};

struct GuiDebugTextColor
{
    const char* dark;
    const char* light;

    const char* c_str() const
    {
        return (config_emulator.theme == config_Theme_Light) ? light : dark;
    }

    operator const char*() const
    {
        return c_str();
    }
};

static inline ImVec4 gui_debug_color(unsigned int rgb)
{
    return ImVec4(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, 1.0f);
}

static const GuiDebugColor cyan = { gui_debug_color(0x1AE6E6), gui_debug_color(0x007C91) };
static const GuiDebugColor dark_cyan = { gui_debug_color(0x004D4D), gui_debug_color(0xCBEFF3) };
static const GuiDebugColor magenta = { gui_debug_color(0xFF80F5), gui_debug_color(0xB42375) };
static const GuiDebugColor dark_magenta = { gui_debug_color(0x4D2E45), gui_debug_color(0xF3D6E8) };
static const GuiDebugColor yellow = { gui_debug_color(0xFFE60D), gui_debug_color(0x8A6000) };
static const GuiDebugColor dark_yellow = { gui_debug_color(0x4D4000), gui_debug_color(0xF7E7B2) };
static const GuiDebugColor orange = { gui_debug_color(0xFF8000), gui_debug_color(0xC44D00) };
static const GuiDebugColor dark_orange = { gui_debug_color(0x993300), gui_debug_color(0xF8D4B6) };
static const GuiDebugColor red = { gui_debug_color(0xFA2673), gui_debug_color(0xC7254E) };
static const GuiDebugColor dark_red = { gui_debug_color(0x4D0A29), gui_debug_color(0xF6CDD8) };
static const GuiDebugColor green = { gui_debug_color(0x1AE61A), gui_debug_color(0x17823B) };
static const GuiDebugColor dim_green = { gui_debug_color(0x0D660D), gui_debug_color(0x4D7438) };
static const GuiDebugColor dark_green = { gui_debug_color(0x083305), gui_debug_color(0xD5E8D6) };
static const GuiDebugColor violet = { gui_debug_color(0xAD82FF), gui_debug_color(0x7047C2) };
static const GuiDebugColor dark_violet = { gui_debug_color(0x3D264D), gui_debug_color(0xE4D9F7) };
static const GuiDebugColor blue = { gui_debug_color(0x3366FF), gui_debug_color(0x0969DA) };
static const GuiDebugColor dark_blue = { gui_debug_color(0x121A4D), gui_debug_color(0xD7E5FA) };
static const GuiDebugColor white = { gui_debug_color(0xFFFFFF), gui_debug_color(0x21201C) };
static const GuiDebugColor gray = { gui_debug_color(0x808080), gui_debug_color(0x69645D) };
static const GuiDebugColor mid_gray = { gui_debug_color(0x666666), gui_debug_color(0x756F67) };
static const GuiDebugColor dark_gray = { gui_debug_color(0x1A1A1A), gui_debug_color(0x4B4842) };
static const GuiDebugColor black = { gui_debug_color(0x000000), gui_debug_color(0x21201C) };
static const GuiDebugColor brown = { gui_debug_color(0xAD805C), gui_debug_color(0x87502C) };
static const GuiDebugColor dark_brown = { gui_debug_color(0x61330F), gui_debug_color(0xE8D6C8) };

static const GuiDebugTextColor c_cyan = { "{1AE6E6}", "{007C91}" };
static const GuiDebugTextColor c_dark_cyan = { "{004D4D}", "{CBEFF3}" };
static const GuiDebugTextColor c_magenta = { "{FF80F5}", "{B42375}" };
static const GuiDebugTextColor c_dark_magenta = { "{4D2E45}", "{F3D6E8}" };
static const GuiDebugTextColor c_yellow = { "{FFE60D}", "{8A6000}" };
static const GuiDebugTextColor c_dark_yellow = { "{4D4000}", "{F7E7B2}" };
static const GuiDebugTextColor c_orange = { "{FF8000}", "{C44D00}" };
static const GuiDebugTextColor c_dark_orange = { "{993300}", "{F8D4B6}" };
static const GuiDebugTextColor c_red = { "{FA2673}", "{C7254E}" };
static const GuiDebugTextColor c_dark_red = { "{4D0A29}", "{F6CDD8}" };
static const GuiDebugTextColor c_green = { "{1AE61A}", "{17823B}" };
static const GuiDebugTextColor c_dim_green = { "{0D660D}", "{4D7438}" };
static const GuiDebugTextColor c_dark_green = { "{083305}", "{D5E8D6}" };
static const GuiDebugTextColor c_violet = { "{AD82FF}", "{7047C2}" };
static const GuiDebugTextColor c_dark_violet = { "{3D264D}", "{E4D9F7}" };
static const GuiDebugTextColor c_blue = { "{3366FF}", "{0969DA}" };
static const GuiDebugTextColor c_dark_blue = { "{121A4D}", "{D7E5FA}" };
static const GuiDebugTextColor c_white = { "{FFFFFF}", "{21201C}" };
static const GuiDebugTextColor c_gray = { "{808080}", "{69645D}" };
static const GuiDebugTextColor c_mid_gray = { "{666666}", "{756F67}" };
static const GuiDebugTextColor c_dark_gray = { "{1A1A1A}", "{4B4842}" };
static const GuiDebugTextColor c_black = { "{000000}", "{21201C}" };
static const GuiDebugTextColor c_brown = { "{AD805C}", "{87502C}" };
static const GuiDebugTextColor c_dark_brown = { "{61330F}", "{E8D6C8}" };

static inline ImVec4 gui_debug_lerp_color(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

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
