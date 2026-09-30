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

#define GUI_DEBUG_MARKIII_LINK_IMPORT
#include "gui_debug_markiii_link.h"

#include "imgui.h"
#include "MarkIIILink.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "gui_debug_widgets.h"
#include "config.h"
#include "emu.h"

static void ppi_write_callback(u16 address, u8 value, void* user_data)
{
    UNUSED(user_data);

    if (emu_markiii_link_get_debug_state().peripheral_attached)
        emu_get_core()->GetMarkIIILink()->DoOutput((u8)address, value);
}

static void draw_register(const char* label, u16 address, u8 value, u8 latch)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextColored(cyan, "$%02X %s", address, label);
    ImGui::TableNextColumn();
    ImGui::Text("$%02X", value);
    ImGui::SameLine();
    ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED ")", BYTE_TO_BINARY(value));
    ImGui::TableNextColumn();
    EditableRegister8(NULL, NULL, address, latch, ppi_write_callback, NULL, EditableRegisterFlags_ShowBinary);
}

static void draw_registers(const GS_MarkIII_LinkDebugState& hardware)
{
    ImGui::TextColored(magenta, "PPI REGISTERS:");

    ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX;

    if (ImGui::BeginTable("ppi_registers", 3, flags))
    {
        ImGui::TableSetupColumn("I/O PORT");
        ImGui::TableSetupColumn("PPI READ");
        ImGui::TableSetupColumn("WRITE LATCH");
        ImGui::TableHeadersRow();

        draw_register("PA", 0xDC, hardware.port_a, hardware.port_a_latch);
        draw_register("PB", 0xDD, hardware.port_b, hardware.port_b_latch);
        draw_register("PC", 0xDE, hardware.port_c, hardware.port_c_latch);

        ImGui::EndTable();
    }

    EditableRegister8("MODE WORD", "$DF", 0xDF, hardware.control, ppi_write_callback, NULL);

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Last mode-set word; the control register is write-only.\n"
            "D7 = 1: set mode/directions and clear all output latches.\n"
            "D7 = 0: set/reset port C bit D3-D1 to the value in D0.");
    }

    u8 group_a_mode = (hardware.control >> 5) & 0x03;

    ImGui::TextColored(violet, " GROUP A / B    "); ImGui::SameLine();
    ImGui::Text("MODE %u / %u", group_a_mode >= 2 ? 2 : group_a_mode, (hardware.control >> 2) & 0x01);
    ImGui::TextColored(violet, " PORT A / B     "); ImGui::SameLine();
    ImGui::Text("%s / %s", (hardware.control & 0x10) ? "INPUT" : "OUTPUT",
        (hardware.control & 0x02) ? "INPUT" : "OUTPUT");
    ImGui::TextColored(violet, " PC7-4 / PC3-0  "); ImGui::SameLine();
    ImGui::Text("%s / %s", (hardware.control & 0x08) ? "INPUT" : "OUTPUT",
        (hardware.control & 0x01) ? "INPUT" : "OUTPUT");
}

static void draw_keyboard(const GS_MarkIII_LinkDebugState& hardware)
{
    static const char* key_names[4] = { "1", "2", "SPACE", "RETURN" };
    static const u8 key_rows[4] = { 0, 1, 1, 5 };
    static const u8 key_bits[4] = { 0, 0, 4, 6 };

    ImGui::Separator();
    ImGui::TextColored(magenta, "KEYBOARD:");

    ImGui::TextColored(violet, " ROW (PC0-2)    "); ImGui::SameLine();
    ImGui::Text("%u", hardware.selected_row);
    ImGui::TextColored(violet, " CPU READS      "); ImGui::SameLine();
    ImGui::Text("%s", hardware.selected_row == 7 ? "CONTROLLERS" : "PPI");

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Row 7 routes CPU I/O reads to controllers.");

    ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX;

    if (ImGui::BeginTable("keyboard_keys", 4, flags))
    {
        ImGui::TableSetupColumn("LINK KEY");
        ImGui::TableSetupColumn("ROW");
        ImGui::TableSetupColumn("INPUT");
        ImGui::TableSetupColumn("STATE (ACTIVE LOW)");
        ImGui::TableHeadersRow();

        for (int key = 0; key < 4; key++)
        {
            bool pressed = (hardware.keyboard_a[key_rows[key]] & (1 << key_bits[key])) == 0;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(white, "%s", key_names[key]);
            ImGui::TableNextColumn();
            ImGui::TextColored(hardware.selected_row == key_rows[key] ? yellow : gray, "%u", key_rows[key]);
            ImGui::TableNextColumn();
            ImGui::Text("PA%u", key_bits[key]);
            ImGui::TableNextColumn();
            ImGui::TextColored(pressed ? green : gray, "%s", pressed ? "0  PRESSED" : "1  RELEASED");
        }

        ImGui::EndTable();
    }
}

static void draw_signals(const GS_MarkIII_LinkDebugState& hardware)
{
    ImGui::Separator();
    ImGui::TextColored(magenta, "LINK SIGNALS:");

    ImGui::TextColored(violet, " CABLE          "); ImGui::SameLine();
    ImGui::TextColored(hardware.cable_connected ? green : gray, "%s",
        hardware.cable_connected ? "CONNECTED" : "DISCONNECTED");

    ImGui::TextColored(gray, "PC5: DATA    PC6: /RESET");

    for (int bit = 5; bit <= 6; bit++)
    {
        u8 mask = (u8)(1 << bit);
        bool driven = (hardware.local_state.drive_mask & mask) != 0;

        ImGui::TextColored(violet, " LOCAL PC%d -> PEER PB%d ", bit, bit); ImGui::SameLine();

        if (driven)
        {
            ImGui::TextColored(white, "%d  OUTPUT", (hardware.local_state.levels & mask) ? 1 : 0);
        }
        else
            ImGui::TextColored(gray, "HIGH-Z");
    }

    for (int bit = 5; bit <= 6; bit++)
    {
        u8 mask = (u8)(1 << bit);
        bool driven = hardware.cable_connected && (hardware.remote_state.drive_mask & mask) != 0;
        bool level = !driven || (hardware.remote_state.levels & mask) != 0;

        ImGui::TextColored(violet, " PEER PC%d -> LOCAL PB%d ", bit, bit); ImGui::SameLine();
        ImGui::TextColored(driven ? white : gray, "%d  %s", level ? 1 : 0, driven ? "INPUT" : "PULL-UP");
    }
}

void gui_debug_window_markiii_link(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(180, 120), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(306, 460), ImGuiCond_FirstUseEver);
    ImGui::Begin("Mark III Link", &config_debug.show_markiii_link);
    ImGui::PushFont(gui_default_font);

    GS_MarkIII_LinkDebugState hardware = emu_markiii_link_get_debug_state();

    draw_registers(hardware);
    draw_keyboard(hardware);
    draw_signals(hardware);

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
}
