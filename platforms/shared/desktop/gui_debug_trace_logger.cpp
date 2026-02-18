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

#define GUI_DEBUG_TRACE_LOGGER_IMPORT
#include "gui_debug_trace_logger.h"

#include <deque>
#include "imgui.h"
#include "gui.h"
#include "gui_filedialogs.h"
#include "config.h"
#include "emu.h"
#include "gui_debug.h"
#include "utils.h"

static bool trace_logger_enabled = false;
static int trace_logger_count = 0;
static unsigned int trace_logger_instruction_count = 0;
static std::deque<std::string> trace_logger_lines;

static void trace_logger_menu(void);

static const int k_line_count[] = { 1000, 5000, 10000, 50000, 100000, 500000, 1000000 };

void gui_debug_window_trace_logger(void)
{
/*
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(340, 168), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(342, 262), ImGuiCond_FirstUseEver);

    ImGui::Begin("Trace Logger", &config_debug.show_trace_logger, ImGuiWindowFlags_MenuBar);

    trace_logger_menu();

    ImGui::Text("Log last: ");

    ImGui::SameLine();

    ImGui::PushItemWidth(100);
    if (ImGui::Combo("lines  ", &trace_logger_count, "1000\0 5000\0 10000\0 50000\0 100000\0 500000\0 1000000\0\0"))
    {
        if ((int)trace_logger_lines.size() > k_line_count[trace_logger_count])
        {
            int diff = (int)trace_logger_lines.size() - k_line_count[trace_logger_count];
            trace_logger_lines.erase(trace_logger_lines.begin(), trace_logger_lines.begin() + diff);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(trace_logger_enabled ? "Stop" : "Start"))
    {
        trace_logger_enabled = !trace_logger_enabled;
        emu_debug_set_callback(trace_logger_enabled ? gui_debug_callback : NULL);
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear"))
    {
        gui_debug_trace_logger_clear();
    }

    if (ImGui::BeginChild("##logger", ImVec2(ImGui::GetContentRegionAvail().x, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::PushFont(gui_default_font);

        ImGuiListClipper clipper;
        clipper.Begin((int)trace_logger_lines.size(), ImGui::GetTextLineHeightWithSpacing());

        while (clipper.Step())
        {
            for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; item++)
            {
                ImGui::Text("%s", trace_logger_lines[item].c_str());
            }
        }

        ImGui::PopFont();
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
*/
}

void gui_debug_trace_logger_update(void)
{
/*
    if (trace_logger_enabled)
    {
        if ((int)trace_logger_lines.size() >= k_line_count[trace_logger_count])
        {
            trace_logger_lines.pop_front();
        }

        Memory* memory = emu_get_core()->GetMemory();
        HuC6280* huc6280 = emu_get_core()->GetHuC6280();
        HuC6280::HuC6280_State* state = huc6280->GetState();
        Memory::stDisassembleRecord* record = memory->GetDisassemblerRecord(state->PC->GetValue());

        if (!IsValidPointer(record))
            return;

        char bank[8];
        snprintf(bank, sizeof(bank), "%02X:", record->bank);

        char registers[40];
        snprintf(registers, sizeof(registers), "A: %02X  X: %02X  Y: %02X  S: %02X   ",
            state->A->GetValue(), state->X->GetValue(),
            state->Y->GetValue(), state->S->GetValue());

        char flags[32];
        u8 flags_state = state->P->GetValue();
        snprintf(flags, sizeof(flags), "P: %c%c%c%c%c%c%c%c   ",
            (flags_state & FLAG_NEGATIVE) ? 'N' : 'n',
            (flags_state & FLAG_OVERFLOW) ? 'V' : 'v',
            (flags_state & FLAG_TRANSFER) ? 'T' : 't',
            (flags_state & FLAG_BREAK) ? 'B' : 'b',
            (flags_state & FLAG_DECIMAL) ? 'D' : 'd',
            (flags_state & FLAG_INTERRUPT) ? 'I' : 'i',
            (flags_state & FLAG_ZERO) ? 'Z' : 'z',
            (flags_state & FLAG_CARRY) ? 'C' : 'c');

        char counter[16];
        snprintf(counter, sizeof(counter), "%d  ", trace_logger_instruction_count);

        std::string instr = record->name;
        strip_color_tags(instr);

        char line[256];
        snprintf(line, sizeof(line), "%s%s%04X   %s%s%s   %s",
            config_debug.trace_counter ? counter : "",
            config_debug.trace_bank ? bank : "", 
            state->PC->GetValue(), 
            config_debug.trace_registers ? registers : "", 
            config_debug.trace_flags ? flags : "",
            instr.c_str(),
            config_debug.trace_bytes ? record->bytes : "");

        trace_logger_lines.push_back(line);
        trace_logger_instruction_count++;
    }
*/
}

void gui_debug_trace_logger_clear(void)
{
    trace_logger_lines.clear();
    trace_logger_instruction_count = 0;
}

void gui_debug_save_log(const char* file_path)
{
    FILE* file = fopen_utf8(file_path, "w");

    if (file != NULL)
    {
        for (long unsigned int i = 0; i < trace_logger_lines.size(); i++)
        {
            fprintf(file, "%s\n", trace_logger_lines[i].c_str());
        }

        fclose(file);
    }
}

static void trace_logger_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save Log As..."))
        {
            gui_file_dialog_save_log();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Log"))
    {
        ImGui::MenuItem("Instruction Counter", "", &config_debug.trace_counter);
        ImGui::MenuItem("Bank Number", "", &config_debug.trace_bank);
        ImGui::MenuItem("Registers", "", &config_debug.trace_registers);
        ImGui::MenuItem("Flags", "", &config_debug.trace_flags);
        ImGui::MenuItem("Bytes", "", &config_debug.trace_bytes);

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}
