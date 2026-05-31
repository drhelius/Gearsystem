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

static const GuiDebugColor cyan =          { ImVec4(0.10f, 0.90f, 0.90f, 1.0f), ImVec4(0.00f, 0.49f, 0.60f, 1.0f) };
static const GuiDebugColor dark_cyan =     { ImVec4(0.00f, 0.30f, 0.30f, 1.0f), ImVec4(0.64f, 0.91f, 0.95f, 1.0f) };
static const GuiDebugColor magenta =       { ImVec4(1.00f, 0.50f, 0.96f, 1.0f), ImVec4(0.82f, 0.08f, 0.76f, 1.0f) };
static const GuiDebugColor dark_magenta =  { ImVec4(0.30f, 0.18f, 0.27f, 1.0f), ImVec4(0.93f, 0.74f, 0.92f, 1.0f) };
static const GuiDebugColor yellow =        { ImVec4(1.00f, 0.90f, 0.05f, 1.0f), ImVec4(0.64f, 0.48f, 0.00f, 1.0f) };
static const GuiDebugColor dark_yellow =   { ImVec4(0.30f, 0.25f, 0.00f, 1.0f), ImVec4(0.96f, 0.88f, 0.50f, 1.0f) };
static const GuiDebugColor orange =        { ImVec4(1.00f, 0.50f, 0.00f, 1.0f), ImVec4(0.84f, 0.28f, 0.00f, 1.0f) };
static const GuiDebugColor dark_orange =   { ImVec4(0.60f, 0.20f, 0.00f, 1.0f), ImVec4(0.98f, 0.76f, 0.58f, 1.0f) };
static const GuiDebugColor red =           { ImVec4(0.98f, 0.15f, 0.45f, 1.0f), ImVec4(0.86f, 0.00f, 0.26f, 1.0f) };
static const GuiDebugColor dark_red =      { ImVec4(0.30f, 0.04f, 0.16f, 1.0f), ImVec4(0.97f, 0.68f, 0.78f, 1.0f) };
static const GuiDebugColor green =         { ImVec4(0.10f, 0.90f, 0.10f, 1.0f), ImVec4(0.00f, 0.55f, 0.10f, 1.0f) };
static const GuiDebugColor dim_green =     { ImVec4(0.05f, 0.40f, 0.05f, 1.0f), ImVec4(0.32f, 0.60f, 0.28f, 1.0f) };
static const GuiDebugColor dark_green =    { ImVec4(0.03f, 0.20f, 0.02f, 1.0f), ImVec4(0.68f, 0.91f, 0.64f, 1.0f) };
static const GuiDebugColor violet =        { ImVec4(0.68f, 0.51f, 1.00f, 1.0f), ImVec4(0.46f, 0.24f, 0.82f, 1.0f) };
static const GuiDebugColor dark_violet =   { ImVec4(0.24f, 0.15f, 0.30f, 1.0f), ImVec4(0.80f, 0.70f, 0.94f, 1.0f) };
static const GuiDebugColor blue =          { ImVec4(0.20f, 0.40f, 1.00f, 1.0f), ImVec4(0.05f, 0.24f, 0.88f, 1.0f) };
static const GuiDebugColor dark_blue =     { ImVec4(0.07f, 0.10f, 0.30f, 1.0f), ImVec4(0.68f, 0.76f, 0.96f, 1.0f) };
static const GuiDebugColor white =         { ImVec4(1.00f, 1.00f, 1.00f, 1.0f), ImVec4(0.12f, 0.11f, 0.15f, 1.0f) };
static const GuiDebugColor gray =          { ImVec4(0.50f, 0.50f, 0.50f, 1.0f), ImVec4(0.45f, 0.43f, 0.50f, 1.0f) };
static const GuiDebugColor mid_gray =      { ImVec4(0.40f, 0.40f, 0.40f, 1.0f), ImVec4(0.62f, 0.59f, 0.67f, 1.0f) };
static const GuiDebugColor dark_gray =     { ImVec4(0.10f, 0.10f, 0.10f, 1.0f), ImVec4(0.64f, 0.61f, 0.69f, 1.0f) };
static const GuiDebugColor black =         { ImVec4(0.00f, 0.00f, 0.00f, 1.0f), ImVec4(1.00f, 1.00f, 1.00f, 1.0f) };
static const GuiDebugColor brown =         { ImVec4(0.68f, 0.50f, 0.36f, 1.0f), ImVec4(0.56f, 0.30f, 0.10f, 1.0f) };
static const GuiDebugColor dark_brown =    { ImVec4(0.38f, 0.20f, 0.06f, 1.0f), ImVec4(0.90f, 0.72f, 0.55f, 1.0f) };

static const GuiDebugTextColor c_cyan = { "{19E6E6}", "{007D99}" };
static const GuiDebugTextColor c_dark_cyan = { "{004C4C}", "{A3E8F2}" };
static const GuiDebugTextColor c_magenta = { "{FF80F5}", "{D114C2}" };
static const GuiDebugTextColor c_dark_magenta = { "{4C2E45}", "{EDBDEB}" };
static const GuiDebugTextColor c_yellow = { "{FFE60D}", "{A37A00}" };
static const GuiDebugTextColor c_dark_yellow = { "{4C4000}", "{F5E080}" };
static const GuiDebugTextColor c_orange = { "{FF8000}", "{D64700}" };
static const GuiDebugTextColor c_dark_orange = { "{993300}", "{FAC294}" };
static const GuiDebugTextColor c_red = { "{FA2673}", "{DB0042}" };
static const GuiDebugTextColor c_dark_red = { "{4C0A29}", "{F7ADC7}" };
static const GuiDebugTextColor c_green = { "{19E619}", "{008C1A}" };
static const GuiDebugTextColor c_dim_green = { "{0D660D}", "{529947}" };
static const GuiDebugTextColor c_dark_green = { "{083305}", "{ADE8A3}" };
static const GuiDebugTextColor c_violet = { "{AD82FF}", "{753DD1}" };
static const GuiDebugTextColor c_dark_violet = { "{3D274D}", "{CCB3F0}" };
static const GuiDebugTextColor c_blue = { "{3366FF}", "{0D3DE0}" };
static const GuiDebugTextColor c_dark_blue = { "{12194D}", "{ADC2F5}" };
static const GuiDebugTextColor c_white = { "{FFFFFF}", "{1F1D26}" };
static const GuiDebugTextColor c_gray = { "{808080}", "{736E80}" };
static const GuiDebugTextColor c_mid_gray = { "{666666}", "{9E96AB}" };
static const GuiDebugTextColor c_dark_gray = { "{1A1A1A}", "{A39CB0}" };
static const GuiDebugTextColor c_black = { "{000000}", "{000000}" };
static const GuiDebugTextColor c_brown = { "{AD805C}", "{8F4D1A}" };
static const GuiDebugTextColor c_dark_brown = { "{61330F}", "{E6B88C}" };

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