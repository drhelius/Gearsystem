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

#include "imgui.h"
#include "gui.h"
#include "gui_filedialogs.h"
#include "gui_debug_constants.h"
#include "gui_debug_text.h"
#include "config.h"
#include "emu.h"
#include "gui_debug.h"

static bool trace_logger_enabled = false;
static u64 trace_logger_start_total = 0;

static void trace_logger_menu(void);
static void trace_logger_sync_flags(void);
static void format_entry_text(const GS_Trace_Entry& entry, char* buf, int buf_size);
static void format_cpu_entry(const GS_Trace_Entry& entry, char* buf, int buf_size);
static void render_entry_colored(const GS_Trace_Entry& entry, u32 index);
static void render_cpu_entry_colored(const GS_Trace_Entry& entry);

void gui_debug_window_trace_logger(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(340, 168), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(544, 362), ImGuiCond_FirstUseEver);

    ImGui::Begin("Trace Logger", &config_debug.show_trace_logger, ImGuiWindowFlags_MenuBar);

    trace_logger_menu();

    TraceLogger* tl = emu_get_core()->GetTraceLogger();

    if (ImGui::Button(trace_logger_enabled ? "Stop" : "Start"))
    {
        trace_logger_enabled = !trace_logger_enabled;
        if (trace_logger_enabled)
        {
            trace_logger_start_total = tl->GetTotalLogged();
            trace_logger_sync_flags();
        }
        else
            tl->SetEnabledFlags(0);
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear"))
    {
        gui_debug_trace_logger_clear();
    }

    ImGui::SameLine();
    ImGui::Text("Entries: %u / %d", tl->GetCount(), TRACE_BUFFER_SIZE);

    if (trace_logger_enabled)
        trace_logger_sync_flags();

    if (ImGui::BeginChild("##logger", ImVec2(ImGui::GetContentRegionAvail().x, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::PushFont(gui_default_font);

        u32 count = tl->GetCount();

        ImGuiListClipper clipper;
        clipper.Begin((int)count, ImGui::GetTextLineHeightWithSpacing());

        while (clipper.Step())
        {
            for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; item++)
            {
                const GS_Trace_Entry& entry = tl->GetEntry((u32)item);
                u64 entry_number = tl->GetTotalLogged() - (u64)count + (u64)item - trace_logger_start_total;
                render_entry_colored(entry, (u32)entry_number);
            }
        }

        ImGui::PopFont();
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_trace_logger_clear(void)
{
    emu_get_core()->GetTraceLogger()->Reset();
    trace_logger_start_total = 0;
}

void gui_debug_save_log(const char* file_path)
{
    FILE* file = fopen_utf8(file_path, "w");

    if (file != NULL)
    {
        TraceLogger* tl = emu_get_core()->GetTraceLogger();
        u32 count = tl->GetCount();
        char buf[256];

        for (u32 i = 0; i < count; i++)
        {
            const GS_Trace_Entry& entry = tl->GetEntry(i);
            format_entry_text(entry, buf, sizeof(buf));
            if (config_debug.trace_counter)
                fprintf(file, "%06u %s\n", i, buf);
            else
                fprintf(file, "%s\n", buf);
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

    if (ImGui::BeginMenu("CPU"))
    {
        ImGui::MenuItem("Instruction Counter", "", &config_debug.trace_counter);
        ImGui::MenuItem("Bank Number", "", &config_debug.trace_bank);
        ImGui::MenuItem("Registers", "", &config_debug.trace_registers);
        ImGui::MenuItem("Flags", "", &config_debug.trace_flags);
        ImGui::MenuItem("Bytes", "", &config_debug.trace_bytes);

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Filter"))
    {
        ImGui::MenuItem("IRQs", "", &config_debug.trace_cpu_irq);
        ImGui::MenuItem("VDP Writes", "", &config_debug.trace_vdp_write);
        ImGui::MenuItem("VDP Status", "", &config_debug.trace_vdp_status);
        ImGui::MenuItem("PSG", "", &config_debug.trace_psg);
        ImGui::MenuItem("YM2413", "", &config_debug.trace_ym2413);
        ImGui::MenuItem("IO Ports", "", &config_debug.trace_io_port);
        ImGui::MenuItem("Bank Switch", "", &config_debug.trace_bank_switch);

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

static void trace_logger_sync_flags(void)
{
    u32 flags = TRACE_FLAG_CPU;
    if (config_debug.trace_cpu_irq)       flags |= TRACE_FLAG_CPU_IRQ;
    if (config_debug.trace_vdp_write)     flags |= TRACE_FLAG_VDP_WRITE;
    if (config_debug.trace_vdp_status)    flags |= TRACE_FLAG_VDP_STATUS;
    if (config_debug.trace_psg)           flags |= TRACE_FLAG_PSG;
    if (config_debug.trace_ym2413)        flags |= TRACE_FLAG_YM2413;
    if (config_debug.trace_io_port)       flags |= TRACE_FLAG_IO_PORT;
    if (config_debug.trace_bank_switch)   flags |= TRACE_FLAG_BANK_SWITCH;
    emu_get_core()->GetTraceLogger()->SetEnabledFlags(flags);
}

static void format_cpu_entry(const GS_Trace_Entry& entry, char* buf, int buf_size)
{
    Memory* memory = emu_get_core()->GetMemory();
    GS_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.cpu.pc, entry.cpu.bank);

    char instr[64] = "???";
    char bytes[25] = "";
    char bank[8] = "";
    if (IsValidPointer(record))
    {
        strncpy(instr, record->name, sizeof(instr) - 1);
        instr[sizeof(instr) - 1] = '\0';

        char* p = instr;
        while (*p)
        {
            if (*p == '{')
            {
                char* end = strchr(p, '}');
                if (end)
                    memmove(p, end + 1, strlen(end + 1) + 1);
                else
                    break;
            }
            else
                p++;
        }
        strncpy(bytes, record->bytes, sizeof(bytes) - 1);
        bytes[sizeof(bytes) - 1] = '\0';
        snprintf(bank, sizeof(bank), "%02X:", record->bank);
    }

    u8 a = (entry.cpu.af >> 8) & 0xFF;
    u8 f = entry.cpu.af & 0xFF;

    char registers[80] = "";
    if (config_debug.trace_registers)
        snprintf(registers, sizeof(registers), "A:%02X  BC:%04X  DE:%04X  HL:%04X  SP:%04X  ",
                 a, entry.cpu.bc, entry.cpu.de, entry.cpu.hl, entry.cpu.sp);

    char flags[20] = "";
    if (config_debug.trace_flags)
    {
        snprintf(flags, sizeof(flags), "%c%c%c%c%c%c%c%c  ",
                 (f & FLAG_SIGN) ? 'S' : 's',
                 (f & FLAG_ZERO) ? 'Z' : 'z',
                 (f & FLAG_Y) ? 'Y' : 'y',
                 (f & FLAG_HALF) ? 'H' : 'h',
                 (f & FLAG_X) ? 'X' : 'x',
                 (f & FLAG_PARITY) ? 'P' : 'p',
                 (f & FLAG_NEGATIVE) ? 'N' : 'n',
                 (f & FLAG_CARRY) ? 'C' : 'c');
    }

    snprintf(buf, buf_size, "%s%04X  %s%s%-24s %s",
             config_debug.trace_bank ? bank : "",
             entry.cpu.pc,
             registers, flags, instr,
             config_debug.trace_bytes ? bytes : "");
}

static void format_entry_text(const GS_Trace_Entry& entry, char* buf, int buf_size)
{
    switch (entry.type)
    {
        case TRACE_CPU:
            format_cpu_entry(entry, buf, buf_size);
            break;
        case TRACE_CPU_IRQ:
            snprintf(buf, buf_size, "  [CPU]  %s       PC:$%04X  Vector:$%04X",
                     entry.irq.type == 2 ? "NMI" : "IRQ",
                     entry.irq.pc, entry.irq.vector);
            break;
        case TRACE_VDP_WRITE:
            snprintf(buf, buf_size, "  [VDP]  REG %02d   Value:$%02X",
                     entry.vdp_write.reg, entry.vdp_write.value);
            break;
        case TRACE_VDP_STATUS:
        {
            static const char* k_vdp_events[] = {"VINT", "HINT", "VFLAG", "DISPLAY", "SCROLL X", "SCROLL Y", "SPR OVR", "SPR COL"};
            const char* event_name = (entry.vdp_status.event < 8) ? k_vdp_events[entry.vdp_status.event] : "???";
            switch (entry.vdp_status.event)
            {
                case GS_VDP_EVENT_VINT:
                case GS_VDP_EVENT_HINT:
                case GS_VDP_EVENT_VINT_FLAG:
                case GS_VDP_EVENT_SPRITE_OVR:
                    snprintf(buf, buf_size, "  [VDP]  %-9s Line:%d",
                             event_name, entry.vdp_status.line);
                    break;
                case GS_VDP_EVENT_DISPLAY:
                    snprintf(buf, buf_size, "  [VDP]  %-9s Line:%d  %s",
                             event_name, entry.vdp_status.line,
                             entry.vdp_status.value ? "ON" : "OFF");
                    break;
                case GS_VDP_EVENT_SCROLL_X:
                case GS_VDP_EVENT_SCROLL_Y:
                    snprintf(buf, buf_size, "  [VDP]  %-9s Line:%d  Value:$%02X",
                             event_name, entry.vdp_status.line, entry.vdp_status.value);
                    break;
                case GS_VDP_EVENT_SPRITE_COL:
                    snprintf(buf, buf_size, "  [VDP]  %-9s Line:%d  X:%d",
                             event_name, entry.vdp_status.line, entry.vdp_status.value);
                    break;
                default:
                    snprintf(buf, buf_size, "  [VDP]  %-9s Line:%d",
                             event_name, entry.vdp_status.line);
                    break;
            }
            break;
        }
        case TRACE_PSG:
            snprintf(buf, buf_size, "  [PSG]  WRITE    Value:$%02X",
                     entry.psg.value);
            break;
        case TRACE_YM2413:
            snprintf(buf, buf_size, "  [FM]   WRITE    Port:$%02X  Value:$%02X",
                     entry.ym2413.port, entry.ym2413.value);
            break;
        case TRACE_IO_PORT:
            snprintf(buf, buf_size, "  [IO]   %s     Port:$%02X  Value:$%02X",
                     entry.io_port.is_write ? "OUT" : "IN ",
                     entry.io_port.port, entry.io_port.value);
            break;
        case TRACE_BANK_SWITCH:
            snprintf(buf, buf_size, "  [MAP]  BANK     Addr:$%04X  Value:$%02X",
                     entry.bank_switch.address, entry.bank_switch.value);
            break;
        default:
            snprintf(buf, buf_size, "  [???]");
            break;
    }
}

static void render_cpu_entry_colored(const GS_Trace_Entry& entry)
{
    Memory* memory = emu_get_core()->GetMemory();
    GS_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.cpu.pc, entry.cpu.bank);

    if (config_debug.trace_bank && IsValidPointer(record))
    {
        ImGui::TextColored(violet, "%02X ", record->bank);
        ImGui::SameLine(0, 0);
    }

    ImGui::TextColored(cyan, "%04X", entry.cpu.pc);

    u8 a = (entry.cpu.af >> 8) & 0xFF;
    u8 f = entry.cpu.af & 0xFF;

    if (config_debug.trace_registers)
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  A:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%02X", a);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  BC:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.bc);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  DE:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.de);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  HL:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.hl);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  SP:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.sp);
    }

    if (config_debug.trace_flags)
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(yellow, "  %c%c%c%c%c%c%c%c",
                 (f & FLAG_SIGN) ? 'S' : 's',
                 (f & FLAG_ZERO) ? 'Z' : 'z',
                 (f & FLAG_Y) ? 'Y' : 'y',
                 (f & FLAG_HALF) ? 'H' : 'h',
                 (f & FLAG_X) ? 'X' : 'x',
                 (f & FLAG_PARITY) ? 'P' : 'p',
                 (f & FLAG_NEGATIVE) ? 'N' : 'n',
                 (f & FLAG_CARRY) ? 'C' : 'c');
    }

    if (IsValidPointer(record))
    {
        std::string instr = record->name;
        size_t pos;
        pos = instr.find("{n}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_white);
        pos = instr.find("{o}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_brown);
        pos = instr.find("{e}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_blue);

        ImGui::SameLine(0, 0);
        TextColoredEx("  %s%s", c_white, instr.c_str());

        if (config_debug.trace_bytes)
        {
            float char_width = ImGui::CalcTextSize("A").x;
            float bytes_column = char_width * 37;
            if (config_debug.trace_registers) bytes_column += char_width * 34;
            if (config_debug.trace_flags)     bytes_column += char_width * 10;
            if (config_debug.trace_counter)   bytes_column += char_width * 7;
            if (config_debug.trace_bank)      bytes_column += char_width * 3;
            ImGui::SameLine(bytes_column);
            ImGui::TextColored(gray, "%s", record->bytes);
        }
    }
    else
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(gray, "  ???");
    }
}

static void render_entry_colored(const GS_Trace_Entry& entry, u32 index)
{
    char buf[256];

    if (config_debug.trace_counter)
    {
        ImGui::TextColored(gray, "%06u ", index);
        ImGui::SameLine(0, 0);
    }

    switch (entry.type)
    {
        case TRACE_CPU:
            render_cpu_entry_colored(entry);
            break;
        case TRACE_CPU_IRQ:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(red, "%s", buf);
            break;
        case TRACE_VDP_WRITE:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(green, "%s", buf);
            break;
        case TRACE_VDP_STATUS:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(orange, "%s", buf);
            break;
        case TRACE_PSG:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(blue, "%s", buf);
            break;
        case TRACE_YM2413:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(violet, "%s", buf);
            break;
        case TRACE_IO_PORT:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(yellow, "%s", buf);
            break;
        case TRACE_BANK_SWITCH:
            format_entry_text(entry, buf, sizeof(buf));
            ImGui::TextColored(magenta, "%s", buf);
            break;
        default:
            break;
    }
}
