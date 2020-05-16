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

#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "config.h"
#include "emu.h"
#include "renderer.h"
#include "../../src/gearsystem.h"
#include "gui.h"
#include "gui_debug_constants.h"

#define GUI_DEBUG_IMPORT
#include "gui_debug.h"

struct DebugSymbol
{
    int bank;
    u16 address;
    std::string text;
};

struct DisassmeblerLine
{
    bool is_symbol;
    bool is_breakpoint;
    Memory::stDisassembleRecord* record;
    std::string symbol;
};

static MemoryEditor mem_edit;
static ImVec4 cyan = ImVec4(0.0f,1.0f,1.0f,1.0f);
static ImVec4 magenta = ImVec4(1.0f,0.502f,0.957f,1.0f);
static ImVec4 yellow = ImVec4(1.0f,1.0f,0.0f,1.0f);
static ImVec4 red = ImVec4(1.0f,0.149f,0.447f,1.0f);
static ImVec4 green = ImVec4(0.0f,1.0f,0.0f,1.0f);
static ImVec4 white = ImVec4(1.0f,1.0f,1.0f,1.0f);
static ImVec4 gray = ImVec4(0.5f,0.5f,0.5f,1.0f);
static ImVec4 dark_gray = ImVec4(0.1f,0.1f,0.1f,1.0f);
static std::vector<DebugSymbol> symbols;
static Memory::stDisassembleRecord* selected_record = NULL;
static char brk_address[8] = "";

static void debug_window_processor(void);
static void debug_window_memory(void);
static void debug_window_disassembler(void);
static void debug_window_vram(void);
static void debug_window_vram_background(void);
static void debug_window_vram_tiles(void);
static void debug_window_vram_oam(void);
static void debug_window_vram_palettes(void);
static void debug_window_vram_regs(void);
static void add_symbol(const char* line);
static void add_breakpoint(void);
static ImVec4 color_444_to_float(u16 color);
static ImVec4 color_222_to_float(u8 color);

void gui_debug_windows(void)
{
    if (config_debug.debug)
    {
        if (config_debug.show_processor)
            debug_window_processor();
        if (config_debug.show_memory)
            debug_window_memory();
        if (config_debug.show_disassembler)
            debug_window_disassembler();
        if (config_debug.show_video)
            debug_window_vram();
    }
}

void gui_debug_reset(void)
{
    gui_debug_reset_breakpoints();
    gui_debug_reset_symbols();
    selected_record = NULL;
}

void gui_debug_reset_symbols(void)
{
    symbols.clear();
    
    for (int i = 0; i < gui_debug_symbols_count; i++)
        add_symbol(gui_debug_symbols[i]);
}

void gui_debug_load_symbols_file(const char* path)
{
    Log("Loading symbol file %s", path);

    std::ifstream file(path);

    if (file.is_open())
    {
        std::string line;

        while (std::getline(file, line))
        {
            add_symbol(line.c_str());
        }

        file.close();
    }
}

void gui_debug_toggle_breakpoint(void)
{
    if (IsValidPointer(selected_record))
    {
        bool found = false;
        std::vector<Memory::stDisassembleRecord*>* breakpoints = emu_get_core()->GetMemory()->GetBreakpoints();

        for (long unsigned int b = 0; b < breakpoints->size(); b++)
        {
            if ((*breakpoints)[b] == selected_record)
            {
                found = true;
                 InitPointer((*breakpoints)[b]);
                break;
            }
        }

        if (!found)
        {
            breakpoints->push_back(selected_record);
        }
    }
}

void gui_debug_runtocursor(void)
{
    if (IsValidPointer(selected_record))
    {
        emu_get_core()->GetMemory()->SetRunToBreakpoint(selected_record);
        emu_debug_continue();
    }
}

void gui_debug_reset_breakpoints(void)
{
    emu_get_core()->GetMemory()->GetBreakpoints()->clear();
    brk_address[0] = 0;
}

static void debug_window_memory(void)
{
    ImGui::SetNextWindowPos(ImVec2(180, 382), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(482, 308), ImGuiCond_FirstUseEver);

    ImGui::Begin("Memory Editor", &config_debug.show_memory);

    GearsystemCore* core = emu_get_core();
    Memory* memory = core->GetMemory();
    Cartridge* cart = core->GetCartridge();
    Video* video = core->GetVideo();

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(cyan, "  BANKS: ");ImGui::SameLine();

    ImGui::TextColored(magenta, "ROM0");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentRule()->GetBank(0)); ImGui::SameLine();
    ImGui::TextColored(magenta, "  ROM1");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentRule()->GetBank(1)); ImGui::SameLine();
    ImGui::TextColored(magenta, "  ROM2");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentRule()->GetBank(2)); 

    if (cart->GetType() == Cartridge::CartridgeSegaMapper)
    {
        ImGui::SameLine();
        ImGui::TextColored(magenta, "  RAM");ImGui::SameLine();
        ImGui::Text("$%02X", memory->GetCurrentRule()->GetRamBank());
    }

    ImGui::PopFont();

    if (ImGui::BeginTabBar("##memory_tabs", ImGuiTabBarFlags_None))
    {
        if (cart->GetType() == Cartridge::CartridgeSegaMapper)
        {
            if (ImGui::BeginTabItem("FIXED 1KB"))
            {
                ImGui::PushFont(gui_default_font);
                mem_edit.DrawContents(memory->GetMemoryMap(), 0x400, 0);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("PAGE0"))
            {
                ImGui::PushFont(gui_default_font);
                mem_edit.DrawContents(memory->GetCurrentRule()->GetPage(0) + 0x400, 0x4000 - 0x400, 0x400);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        } 
        else if (ImGui::BeginTabItem("PAGE0"))
        {
            ImGui::PushFont(gui_default_font);
            mem_edit.DrawContents(memory->GetCurrentRule()->GetPage(0), 0x4000, 0);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("PAGE1"))
        {
            ImGui::PushFont(gui_default_font);
            mem_edit.DrawContents(memory->GetCurrentRule()->GetPage(1), 0x4000, 0x4000);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if ((cart->GetType() == Cartridge::CartridgeCodemastersMapper) && IsValidPointer(memory->GetCurrentRule()->GetRamBanks()))
        {
            if (ImGui::BeginTabItem("PAGE2"))
            {
                ImGui::PushFont(gui_default_font);
                mem_edit.DrawContents(memory->GetCurrentRule()->GetPage(2), 0x2000, 0x8000);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("EXT RAM"))
            {
                ImGui::PushFont(gui_default_font);
                mem_edit.DrawContents(memory->GetCurrentRule()->GetRamBanks(), 0x2000, 0xA000);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        }
        else if (ImGui::BeginTabItem("PAGE2"))
        {
            ImGui::PushFont(gui_default_font);
            mem_edit.DrawContents(memory->GetCurrentRule()->GetPage(2), 0x4000, 0x8000);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("RAM"))
        {
            ImGui::PushFont(gui_default_font);
            mem_edit.DrawContents(memory->GetMemoryMap() + 0xC000, 0x4000, 0xC000);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("VRAM"))
        {
            ImGui::PushFont(gui_default_font);
            mem_edit.DrawContents(video->GetVRAM(), 0x4000, 0);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("CRAM"))
        {
            ImGui::PushFont(gui_default_font);
            mem_edit.DrawContents(video->GetCRAM(), 0x40, 0);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

static void debug_window_disassembler(void)
{
    ImGui::SetNextWindowPos(ImVec2(180, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(482, 344), ImGuiCond_FirstUseEver);

    ImGui::Begin("Disassembler", &config_debug.show_disassembler);

    GearsystemCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();
    Memory* memory = core->GetMemory();
    std::vector<Memory::stDisassembleRecord*>* breakpoints = memory->GetBreakpoints();
    Memory::stDisassembleRecord* memoryMap = memory->GetDisassembledMemoryMap();
    Memory::stDisassembleRecord* romMap = memory->GetDisassembledROMMemoryMap();
    Memory::stDisassembleRecord* map = NULL;

    int pc = proc_state->PC->GetValue();

    static bool follow_pc = true;
    static bool show_mem = true;
    static bool show_symbols = true;

    ImGui::Checkbox("Follow PC", &follow_pc); ImGui::SameLine();
    ImGui::Checkbox("Show Memory", &show_mem);  ImGui::SameLine();
    ImGui::Checkbox("Show Symbols", &show_symbols);

    if (ImGui::Button("Step Over"))
        emu_debug_step();
    ImGui::SameLine();
    if (ImGui::Button("Step Frame"))
        emu_debug_next_frame();
    ImGui::SameLine();
    if (ImGui::Button("Continue"))
        emu_debug_continue(); 
    ImGui::SameLine();
    if (ImGui::Button("Run To Cursor"))
        gui_debug_runtocursor();

    if (ImGui::CollapsingHeader("Breakpoints"))
    {
        ImGui::Checkbox("Disable All", &emu_debug_disable_breakpoints);

        ImGui::Columns(2, "breakpoints");
        ImGui::SetColumnOffset(1, 85);

        ImGui::Separator();

        if (IsValidPointer(selected_record))
            sprintf(brk_address, "%02X:%04X", selected_record->bank, selected_record->address);
        
        ImGui::PushItemWidth(70);
        if (ImGui::InputTextWithHint("##add_breakpoint", "XX:XXXX", brk_address, IM_ARRAYSIZE(brk_address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            add_breakpoint();
        }
        ImGui::PopItemWidth();
        
        if (ImGui::Button("Add", ImVec2(70, 0)))
        {
            add_breakpoint();
        }
        
        if (ImGui::Button("Clear All", ImVec2(70, 0)))
        {
            gui_debug_reset_breakpoints();
        }

        ImGui::NextColumn();

        ImGui::BeginChild("breakpoints", ImVec2(0, 74), false);

        for (long unsigned int b = 0; b < breakpoints->size(); b++)
        {
            if (!IsValidPointer((*breakpoints)[b]))
                continue;

            ImGui::PushID(b);
            if (ImGui::SmallButton("X"))
            {
               InitPointer((*breakpoints)[b]);
               ImGui::PopID();
               continue;
            }

            ImGui::PopID();

            ImGui::PushFont(gui_default_font);
            ImGui::SameLine();
            ImGui::TextColored(red, "%02X:%04X", (*breakpoints)[b]->bank, (*breakpoints)[b]->address);
            ImGui::SameLine();
            ImGui::TextColored(gray, "%s", (*breakpoints)[b]->name);
            ImGui::PopFont();
        }

        ImGui::EndChild();
        ImGui::Columns(1);
        ImGui::Separator();
    }

    ImGui::PushFont(gui_default_font);

    bool window_visible = ImGui::BeginChild("##dis", ImVec2(ImGui::GetWindowContentRegionWidth(), 0), true, 0);
    
    if (window_visible)
    {
        int dis_size = 0;
        int pc_pos = 0;
        
        std::vector<DisassmeblerLine> vec(0x10000);
        
        for (int i = 0; i < 0x10000; i++)
        {
            int offset = i;
            int bank = 0;

            switch (i & 0xC000)
            {
            case 0x0000:
                bank = memory->GetCurrentRule()->GetBank(0);
                offset = (0x4000 * bank) + i;
                map = romMap;
                break;
            case 0x4000:
                bank = memory->GetCurrentRule()->GetBank(1);
                offset = (0x4000 * bank) + (i & 0x3FFF);
                map = romMap;
                break;
            case 0x8000:
                bank = memory->GetCurrentRule()->GetBank(2);
                offset = (0x4000 * bank) + (i & 0x3FFF);
                map = romMap;
                break;
            default:
                map = memoryMap;
            }

            if (map[offset].name[0] != 0)
            {
                for (long unsigned int s = 0; s < symbols.size(); s++)
                {
                    if ((symbols[s].bank == bank) && (symbols[s].address == offset) && show_symbols)
                    {
                        vec[dis_size].is_symbol = true;
                        vec[dis_size].symbol = symbols[s].text;
                        dis_size ++;
                    }
                }

                vec[dis_size].is_symbol = false;
                vec[dis_size].record = &map[offset];

                if (vec[dis_size].record->address == pc)
                    pc_pos = dis_size;

                vec[dis_size].is_breakpoint = false;

                for (long unsigned int b = 0; b < breakpoints->size(); b++)
                {
                    if ((*breakpoints)[b] == vec[dis_size].record)
                    {
                        vec[dis_size].is_breakpoint = true;
                        break;
                    }
                }

                dis_size++;
            }
        }

        if (follow_pc)
        {
            float window_offset = ImGui::GetWindowHeight() / 2.0f;
            float offset = window_offset - (ImGui::GetTextLineHeightWithSpacing() - 2.0f);
            ImGui::SetScrollY((pc_pos * ImGui::GetTextLineHeightWithSpacing()) - offset);
        }

        ImGuiListClipper clipper(dis_size, ImGui::GetTextLineHeightWithSpacing());

        while (clipper.Step())
        {
            for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; item++)
            {
                if (vec[item].is_symbol)
                {
                    ImGui::TextColored(green, "%s:", vec[item].symbol.c_str());
                    continue;
                }

                ImGui::PushID(item);

                bool is_selected = (selected_record == vec[item].record);

                if (ImGui::Selectable("", is_selected))
                {
                    if (is_selected)
                    {
                        InitPointer(selected_record);
                        brk_address[0] = 0;
                    }
                    else
                        selected_record = vec[item].record;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                if (vec[item].is_breakpoint)
                {
                    ImGui::SameLine();
                    if (vec[item].record->address == pc)
                    {
                        if (show_mem)
                            ImGui::TextColored(red, " %02X:%04X  %s", vec[item].record->bank, vec[item].record->address, vec[item].record->bytes);
                        else
                            ImGui::TextColored(red, " %02X:%04X ", vec[item].record->bank, vec[item].record->address);
                        ImGui::SameLine();
                        ImGui::TextColored(yellow, "->");
                        ImGui::SameLine();
                        ImGui::TextColored(red, "%s", vec[item].record->name);
                    }
                    else
                    {
                        if (show_mem)
                            ImGui::TextColored(red, " %02X:%04X  %s    %s", vec[item].record->bank, vec[item].record->address, vec[item].record->bytes, vec[item].record->name);
                        else
                            ImGui::TextColored(red, " %02X:%04X:    %s", vec[item].record->bank, vec[item].record->address, vec[item].record->name);
                    }
                } 
                else if (vec[item].record->address == pc)
                {
                    ImGui::SameLine();
                    if (show_mem)
                        ImGui::TextColored(yellow, " %02X:%04X  %s -> %s", vec[item].record->bank, vec[item].record->address, vec[item].record->bytes, vec[item].record->name);
                    else
                        ImGui::TextColored(yellow, " %02X:%04X  -> %s", vec[item].record->bank, vec[item].record->address, vec[item].record->name);
                }
                else
                {
                    ImGui::SameLine();
                    ImGui::TextColored(cyan, " %02X:%04X ", vec[item].record->bank, vec[item].record->address);
                    ImGui::SameLine();
                    if (show_mem)
                        ImGui::TextColored(gray, "%s   ", vec[item].record->bytes);
                    else
                        ImGui::TextColored(gray, "  ");
                    
                    ImGui::SameLine();
                    ImGui::TextColored(white, "%s", vec[item].record->name);
                }

                ImGui::PopID();
            }
        }
    }

    ImGui::EndChild();
    
    ImGui::PopFont();

    ImGui::End();
}

static void debug_window_processor(void)
{
    ImGui::SetNextWindowPos(ImVec2(14, 210), ImGuiCond_FirstUseEver);

    ImGui::Begin("Z80 Status", &config_debug.show_processor, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::PushFont(gui_default_font);

    GearsystemCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();

    ImGui::Separator();

    ImGui::TextColored(magenta, "  S Z Y H X P N C");
    ImGui::Text("  " BYTE_TO_BINARY_PATTERN_ALL_SPACED, BYTE_TO_BINARY(proc_state->AF->GetLow()));

    ImGui::Columns(2, "registers");
    ImGui::Separator();
    ImGui::TextColored(cyan, " A"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->AF->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " F"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->AF->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " A'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->AF2->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF2->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " F'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->AF2->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->AF2->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " B"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->BC->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " C"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->BC->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " B'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->BC2->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC2->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " C'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->BC2->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->BC2->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " D"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->DE->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " E"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->DE->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " D'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->DE2->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE2->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " E'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->DE2->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->DE2->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " H"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->HL->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " L"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->HL->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " H'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->HL2->GetHigh());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL2->GetHigh()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " L'"); ImGui::SameLine();
    ImGui::Text("$%02X", proc_state->HL2->GetLow());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->HL2->GetLow()));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::TextColored(cyan, " I"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->I->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->I->GetValue()));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " R"); ImGui::SameLine();
    ImGui::Text(" $%02X", proc_state->R->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->R->GetValue()));

    ImGui::NextColumn();
    ImGui::Columns(1);

    ImGui::Separator();
    ImGui::TextColored(yellow, "    IX"); ImGui::SameLine();
    ImGui::Text("= $%04X", proc_state->IX->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->IX->GetHigh()), BYTE_TO_BINARY(proc_state->IX->GetLow()));

    ImGui::Separator();
    ImGui::TextColored(yellow, "    IY"); ImGui::SameLine();
    ImGui::Text("= $%04X", proc_state->IY->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->IY->GetHigh()), BYTE_TO_BINARY(proc_state->IY->GetLow()));

    ImGui::Separator();
    ImGui::TextColored(yellow, "    WZ"); ImGui::SameLine();
    ImGui::Text("= $%04X", proc_state->WZ->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->WZ->GetHigh()), BYTE_TO_BINARY(proc_state->WZ->GetLow()));

    ImGui::Separator();
    ImGui::TextColored(yellow, "    SP"); ImGui::SameLine();
    ImGui::Text("= $%04X", proc_state->SP->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->SP->GetHigh()), BYTE_TO_BINARY(proc_state->SP->GetLow()));

    ImGui::Separator();
    ImGui::TextColored(yellow, "    PC"); ImGui::SameLine();
    ImGui::Text("= $%04X", proc_state->PC->GetValue());
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(proc_state->PC->GetHigh()), BYTE_TO_BINARY(proc_state->PC->GetLow()));

    ImGui::Separator();

    ImGui::TextColored(*proc_state->IFF1 ? green : gray, " IFF1"); ImGui::SameLine();
    ImGui::TextColored(*proc_state->IFF2 ? green : gray, " IFF2"); ImGui::SameLine();
    ImGui::TextColored(*proc_state->Halt ? green : gray, " HALT");
    
    ImGui::TextColored(*proc_state->INT ? green : gray, "    INT"); ImGui::SameLine();
    ImGui::TextColored(*proc_state->NMI ? green : gray, "  NMI");

    ImGui::PopFont();

    ImGui::End();
}

static void debug_window_vram(void)
{
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(544, 534), ImGuiCond_FirstUseEver);

    ImGui::Begin("VDP Viewer", &config_debug.show_video);

    if (ImGui::BeginTabBar("##vram_tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Name Table"))
        {
            debug_window_vram_background();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Pattern Table"))
        {
            debug_window_vram_tiles();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Sprites"))
        {
            debug_window_vram_oam();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Palettes & Registers"))
        {
            debug_window_vram_palettes();
            debug_window_vram_regs();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

static void debug_window_vram_background(void)
{
    Video* video = emu_get_core()->GetVideo();
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();

    static bool show_grid = true;
    static bool show_screen = true;
    int lines = video->IsExtendedMode224() ? 32 : 28;
    float scale = 1.5f;
    float size_h = 256.0f * scale;
    float size_v = 8.0f * lines * scale;
    float spacing = 8.0f * scale;

    ImGui::Checkbox("Show Grid##grid_bg", &show_grid); ImGui::SameLine();
    ImGui::Checkbox("Show Screen Rect", &show_screen);

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "bg", false);
    ImGui::SetColumnOffset(1, size_h + 10.0f);

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Image((void*)(intptr_t)renderer_emu_debug_vram_background, ImVec2(size_h, size_v), ImVec2(0.0f, 0.0f), ImVec2(1.0f, (1.0f / 32.0f) * lines));

    if (show_grid)
    {
        float x = p.x;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + size_v), ImColor(dark_gray), 1.0f);
            x += spacing;
        }

        float y = p.y;  
        for (int n = 0; n <= lines; n++)
        {
            draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + size_h, y), ImColor(dark_gray), 1.0f);
            y += spacing;
        }
    }

    if (show_screen)
    {
        int scroll_x = 256 - regs[8];
        int scroll_y = regs[9];

        if (emu_get_core()->GetCartridge()->IsGameGear())
        {
            scroll_x += GS_RESOLUTION_GG_X_OFFSET;
            scroll_y += GS_RESOLUTION_GG_Y_OFFSET;
        }

        scroll_x &= 0xFF;
        scroll_y &= 0xFF;

        float grid_x_max = p.x + size_h;
        float grid_y_max = p.y + size_v;

        float rect_x_min = p.x + (scroll_x * scale);
        float rect_y_min = p.y + (scroll_y * scale);
        float rect_x_max = p.x + ((scroll_x + runtime.screen_width) * scale);
        float rect_y_max = p.y + ((scroll_y + runtime.screen_height) * scale);

        float x_overflow = 0.0f;
        float y_overflow = 0.0f;

        if (rect_x_max > grid_x_max)
            x_overflow = rect_x_max - grid_x_max;
        if (rect_y_max > grid_y_max)
            y_overflow = rect_y_max - grid_y_max;

        draw_list->AddLine(ImVec2(rect_x_min, rect_y_min), ImVec2(fminf(rect_x_max, grid_x_max), rect_y_min), ImColor(green), 2.0f);
        if (x_overflow > 0.0f)
            draw_list->AddLine(ImVec2(p.x, rect_y_min), ImVec2(p.x + x_overflow, rect_y_min), ImColor(green), 2.0f);

        draw_list->AddLine(ImVec2(rect_x_min, rect_y_min), ImVec2(rect_x_min, fminf(rect_y_max, grid_y_max)), ImColor(green), 2.0f);
        if (y_overflow > 0.0f)
            draw_list->AddLine(ImVec2(rect_x_min, p.y), ImVec2(rect_x_min, p.y + y_overflow), ImColor(green), 2.0f);

        draw_list->AddLine(ImVec2(rect_x_min, (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImVec2(fminf(rect_x_max, grid_x_max), (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImColor(green), 2.0f);
        if (x_overflow > 0.0f)
            draw_list->AddLine(ImVec2(p.x, (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImVec2(p.x + x_overflow, (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImColor(green), 2.0f);

        draw_list->AddLine(ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, rect_y_min), ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, fminf(rect_y_max, grid_y_max)), ImColor(green), 2.0f);
        if (y_overflow > 0.0f)
            draw_list->AddLine(ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, p.y), ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, p.y + y_overflow), ImColor(green), 2.0f);
    }

    float mouse_x = io.MousePos.x - p.x;
    float mouse_y = io.MousePos.y - p.y;

    int tile_x = -1;
    int tile_y = -1;
    if ((mouse_x >= 0.0f) && (mouse_x < size_h) && (mouse_y >= 0.0f) && (mouse_y < size_v))
    {
        tile_x = mouse_x / spacing;
        tile_y = mouse_y / spacing;

        draw_list->AddRect(ImVec2(p.x + (tile_x * spacing), p.y + (tile_y * spacing)), ImVec2(p.x + ((tile_x + 1) * spacing), p.y + ((tile_y + 1) * spacing)), ImColor(cyan), 2.0f, 15, 2.0f);

        ImGui::NextColumn();

        ImGui::Image((void*)(intptr_t)renderer_emu_debug_vram_background, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 32.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 32.0f) * (tile_y + 1)));

        ImGui::TextColored(yellow, "INFO:");

        ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_x); ImGui::SameLine();
        ImGui::TextColored(cyan, "   Y:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_y);

        int name_table_addr = (regs[2] & (video->IsExtendedMode224() ? 0x0C : 0x0E)) << 10;
        if (video->IsExtendedMode224())
                name_table_addr |= 0x700;
        u16 map_addr = name_table_addr + (64 * tile_y) + (tile_x * 2);

        ImGui::TextColored(cyan, " Map Addr: "); ImGui::SameLine();
        ImGui::Text(" $%04X", map_addr);

        u16 tile_info_lo = vram[map_addr];
        u16 tile_info_hi = vram[map_addr + 1];

        int tile_number = ((tile_info_hi & 1) << 8) | tile_info_lo;
        bool tile_hflip = IsSetBit(tile_info_hi, 1);
        bool tile_vflip = IsSetBit(tile_info_hi, 2);
        int tile_palette = IsSetBit(tile_info_hi, 3) ? 16 : 0;
        bool tile_priority = IsSetBit(tile_info_hi, 4);       

        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text(" $%04X", tile_number << 5);
        ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
        ImGui::Text("$%03X", tile_number);

        ImGui::TextColored(cyan, " Value:"); ImGui::SameLine();
        ImGui::Text("$%04X", tile_info_hi << 8 | tile_info_lo);
        ImGui::TextColored(cyan, " Palette:"); ImGui::SameLine();
        ImGui::Text("%d", tile_palette);

        ImGui::TextColored(cyan, " H-Flip:"); ImGui::SameLine();
        tile_hflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

        ImGui::TextColored(cyan, " V-Flip:"); ImGui::SameLine();
        tile_vflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

        ImGui::TextColored(cyan, " Priority:"); ImGui::SameLine();
        tile_priority ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");
    }

    ImGui::Columns(1);

    ImGui::PopFont();
}

static void debug_window_vram_tiles(void)
{
    static bool show_grid = true;
    float scale = 1.5f;
    float width = 8.0f * 32.0f * scale;
    float height = 8.0f * 16.0f * scale;
    float spacing = 8.0f * scale;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p;

    ImGui::Checkbox("Show Grid##grid_tiles", &show_grid);
    ImGui::SameLine(140.0f);

    ImGui::PushItemWidth(200.0f);

    ImGui::Combo("Palette##tile_palette", &emu_debug_tile_palette, "Palette 0 (BG)\0Palette 1 (BG & Sprites)\0\0");

    ImGui::PopItemWidth();

    ImGui::Columns(2, "tiles", false);
    ImGui::SetColumnOffset(1, width + 10.0f);

    p = ImGui::GetCursorScreenPos();
    
    ImGui::Image((void*)(intptr_t)renderer_emu_debug_vram_tiles, ImVec2(width, height));

    if (show_grid)
    {
        float x = p.x;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + height), ImColor(dark_gray), 1.0f);
            x += spacing;
        }

        float y = p.y;  
        for (int n = 0; n <= 16; n++)
        {
            draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + width, y), ImColor(dark_gray), 1.0f);
            y += spacing;
        }
    }

    float mouse_x = io.MousePos.x - p.x;
    float mouse_y = io.MousePos.y - p.y;

    int tile_x = -1;
    int tile_y = -1;

    if ((mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < height))
    {
        tile_x = mouse_x / spacing;
        tile_y = mouse_y / spacing;

        draw_list->AddRect(ImVec2(p.x + (tile_x * spacing), p.y + (tile_y * spacing)), ImVec2(p.x + ((tile_x + 1) * spacing), p.y + ((tile_y + 1) * spacing)), ImColor(cyan), 2.0f, 15, 2.0f);

        ImGui::NextColumn();

        ImGui::Image((void*)(intptr_t)renderer_emu_debug_vram_tiles, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 16.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 16.0f) * (tile_y + 1)));

        ImGui::PushFont(gui_default_font);

        ImGui::TextColored(yellow, "DETAILS:");

        int tile = (tile_y << 5) + tile_x;

        ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
        ImGui::Text("$%03X", tile); 
        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", tile << 5); 

        ImGui::PopFont();
    }

    ImGui::Columns(1);
}

static void debug_window_vram_oam(void)
{
    /*
    float scale = 5.0f;
    float width = 8.0f * scale;
    float height_8 = 8.0f * scale;
    float height_16 = 16.0f * scale;

    GearsystemCore* core = emu_get_core();
    Memory* memory = core->GetMemory();

    ImVec2 p[40];

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();

    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_16 = IsSetBit(lcdc, 2);

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "oam", true);

    for (int s = 0; s < 40; s++)
    {
        p[s] = ImGui::GetCursorScreenPos();

        ImGui::Image((void*)(intptr_t)renderer_emu_debug_vram_oam[s], ImVec2(width, sprites_16 ? height_16 : height_8), ImVec2(0.0f, 0.0f), ImVec2(1.0f, sprites_16 ? 1.0f : 0.5f));

        float mouse_x = io.MousePos.x - p[s].x;
        float mouse_y = io.MousePos.y - p[s].y;

        if ((mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < (sprites_16 ? height_16 : height_8)))
        {
            draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + width, p[s].y + (sprites_16 ? height_16 : height_8)), ImColor(cyan), 2.0f, 15, 3.0f);
        }

        if (s % 5 < 4)
            ImGui::SameLine();
    }

    ImGui::NextColumn();

    ImVec2 p_screen = ImGui::GetCursorScreenPos();

    float screen_scale = 1.5f;

    ImGui::Image((void*)(intptr_t)renderer_emu_texture, ImVec2(GAMEBOY_WIDTH * screen_scale, GAMEBOY_HEIGHT * screen_scale));

    for (int s = 0; s < 40; s++)
    {
        float mouse_x = io.MousePos.x - p[s].x;
        float mouse_y = io.MousePos.y - p[s].y;

        if ((mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < (sprites_16 ? height_16 : height_8)))
        {
            u16 address = 0xFE00 + (4 * s);

            u8 y = memory->Retrieve(address);
            u8 x = memory->Retrieve(address + 1);
            u8 tile = memory->Retrieve(address + 2);
            u8 flags = memory->Retrieve(address + 3);
            int palette = IsSetBit(flags, 4) ? 1 : 0;
            bool xflip = IsSetBit(flags, 5);
            bool yflip = IsSetBit(flags, 6);
            bool priority = !IsSetBit(flags, 7);
            bool cgb_bank = IsSetBit(flags, 3);
            int cgb_pal = flags & 0x07;

            float real_x = x - 8.0f;
            float real_y = y - 16.0f;
            float rectx_min = p_screen.x + (real_x * screen_scale);
            float rectx_max = p_screen.x + ((real_x + 8.0f) * screen_scale);
            float recty_min = p_screen.y + (real_y * screen_scale);
            float recty_max = p_screen.y + ((real_y + (sprites_16 ? 16.0f : 8.0f)) * screen_scale);

            rectx_min = fminf(fmaxf(rectx_min, p_screen.x), p_screen.x + (GAMEBOY_WIDTH * screen_scale));
            rectx_max = fminf(fmaxf(rectx_max, p_screen.x), p_screen.x + (GAMEBOY_WIDTH * screen_scale));
            recty_min = fminf(fmaxf(recty_min, p_screen.y), p_screen.y + (GAMEBOY_HEIGHT * screen_scale));
            recty_max = fminf(fmaxf(recty_max, p_screen.y), p_screen.y + (GAMEBOY_HEIGHT * screen_scale));
            

            draw_list->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), ImColor(cyan), 2.0f, 15, 2.0f);

            ImGui::TextColored(yellow, "DETAILS:");
            ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
            ImGui::Text("$%02X", x); ImGui::SameLine();
            ImGui::TextColored(cyan, "  Y:"); ImGui::SameLine();
            ImGui::Text("$%02X", y); ImGui::SameLine();

            ImGui::TextColored(cyan, "   Tile:"); ImGui::SameLine();
            ImGui::Text("$%02X", tile);

            ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
            ImGui::Text("$%04X", 0x8000 + (tile * 16)); ImGui::SameLine();

            ImGui::TextColored(cyan, "  Bank:"); ImGui::SameLine();
            ImGui::Text("%d", cgb_bank);

            ImGui::TextColored(cyan, " OAM Addr:"); ImGui::SameLine();
            ImGui::Text("$%04X", address); ImGui::SameLine();

            
            ImGui::TextColored(cyan, "  Flags:"); ImGui::SameLine();
            ImGui::Text("$%02X", flags); 

            ImGui::TextColored(cyan, " Priority:"); ImGui::SameLine();
            priority ? ImGui::TextColored(green, "ON ") : ImGui::TextColored(gray, "OFF"); ImGui::SameLine();

            ImGui::TextColored(cyan, "  Palette:"); ImGui::SameLine();
            ImGui::Text("%d", emu_is_cgb() ? cgb_pal : palette);

            ImGui::TextColored(cyan, " X-Flip:"); ImGui::SameLine();
            xflip ? ImGui::TextColored(green, "ON ") : ImGui::TextColored(gray, "OFF"); ImGui::SameLine();

            ImGui::TextColored(cyan, "  Y-Flip:"); ImGui::SameLine();
            yflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");
        }
    }

    ImGui::Columns(1);

    ImGui::PopFont();
    */
}

static void debug_window_vram_palettes(void)
{
    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* palettes = video->GetCRAM();

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(yellow, "PALETTE 0 (BG):");

    for (int i = 0; i < 2; i ++)
    {
        ImGui::Text(" "); ImGui::SameLine(31.0f);
        for (int c = 0; c < 16; c++)
        {
            ImVec4 float_color;
            if (core->GetCartridge()->IsGameGear())
            {
                u8 color_lo = palettes[(i << 5) + (c << 1)];
                u8 color_hi = palettes[(i << 5) + (c << 1) + 1];
                u16 color = color_hi << 8 | color_lo;
                float_color = color_444_to_float(color);
            }
            else
            {
                u8 color = palettes[(i << 4) + c];
                float_color = color_222_to_float(color);
            }

            char id[16];
            sprintf(id, "##pal_%d_%d", i, c);
            ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 15)
            {   
                ImGui::SameLine(31.0f + (29.0f * (c + 1))); 
            }
        }

        ImGui::Text("  "); ImGui::SameLine();

        for (int c = 0; c < 16; c++)
        {
            if (core->GetCartridge()->IsGameGear())
            {
                u8 color_lo = palettes[(i << 5) + (c << 1)];
                u8 color_hi = palettes[(i << 5) + (c << 1) + 1];
                u16 color = color_hi << 8 | color_lo;
                ImGui::Text("%03X", color);
            }
            else
            {
                u8 color = palettes[(i << 4) + c];
                ImGui::Text("$%02X", color);
            }
            
            if (c < 15)
                ImGui::SameLine();
        }

        if (i == 0)
        {
            ImGui::TextColored(yellow, " ");
            ImGui::TextColored(yellow, "PALETTE 1 (BG & SPRITES):");
        }
    }
   
    ImGui::PopFont();
}

static void debug_window_vram_regs(void)
{
    ImGui::PushFont(gui_default_font);

    Video* video = emu_get_core()->GetVideo();
    u8* regs = video->GetRegisters();

    const char* reg_desc[] = {"CONTROL 1     ", "CONTROL 2     ", "NAME TABLE    ", "COLOR TABLE   ", "PATTERN TABLE ", "SPRITE ATTR   ", "SPRITE PATTERN", "BACKDROP COLOR", "H SCROLL      ", "V SCROLL      ", "V INTERRUPT   "};

    ImGui::TextColored(yellow, " ");
    ImGui::TextColored(yellow, "VDP REGISTERS:");

    for (int i = 0; i < 11; i++)
    {
        ImGui::TextColored(cyan, " REG $%01X ", i); ImGui::SameLine();
        ImGui::TextColored(magenta, "%s ", reg_desc[i]); ImGui::SameLine();
        ImGui::Text("$%02X  (" BYTE_TO_BINARY_PATTERN_SPACED ")", regs[i], BYTE_TO_BINARY(regs[i]));
    }

    ImGui::PopFont();
}

static void add_symbol(const char* line)
{
    Log("Loading symbol %s", line);

    DebugSymbol s;

    std::string str(line);

    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        str = "";
    }
    else
    {
        size_t last = str.find_last_not_of(' ');
        str = str.substr(first, (last - first + 1));
    }

    std::size_t comment = str.find(";");

    if (comment != std::string::npos)
        str = str.substr(0 , comment);

    std::size_t space = str.find(" ");

    if (space != std::string::npos)
    {
        s.text = str.substr(space + 1 , std::string::npos);
        str = str.substr(0, space);

        std::size_t separator = str.find(":");

        if (separator != std::string::npos)
        {
            s.address = std::stoul(str.substr(separator + 1 , std::string::npos), 0, 16);

            s.bank = std::stoul(str.substr(0, separator), 0 , 16);
        }
        else
        {
            s.address = std::stoul(str, 0, 16);
            s.bank = 0;
        }

        symbols.push_back(s);
    }
}

static void add_breakpoint(void)
{
    int input_len = strlen(brk_address);
    u16 target_address = 0;
    int target_bank = 0;
    int target_offset = 0;

    if ((input_len == 7) && (brk_address[2] == ':'))
    {
        std::string str(brk_address);   
        std::size_t separator = str.find(":");

        if (separator != std::string::npos)
        {
            target_address = std::stoul(str.substr(separator + 1 , std::string::npos), 0, 16);

            target_bank = std::stoul(str.substr(0, separator), 0 , 16);
            target_bank &= 0xFF;
        }
    } 
    else if (input_len == 4)
    {
        target_bank = 0; 
        target_address = std::stoul(brk_address, 0, 16);
    }
    else
    {
        return;
    }

    Memory::stDisassembleRecord* memoryMap = emu_get_core()->GetMemory()->GetDisassembledMemoryMap();
    Memory::stDisassembleRecord* romMap = emu_get_core()->GetMemory()->GetDisassembledROMMemoryMap();
    Memory::stDisassembleRecord* map = NULL;

    if ((target_address & 0xC000) == 0x0000)
    {
        target_offset = (0x4000 * target_bank) + target_address;
        map = romMap;
    }
    else if ((target_address & 0xC000) == 0x4000)
    {
        target_offset = (0x4000 * target_bank) + (target_address & 0x3FFF);
        map = romMap;
    }
    else
    {
        map = memoryMap;
    }

    brk_address[0] = 0;

    bool found = false;
    std::vector<Memory::stDisassembleRecord*>* breakpoints = emu_get_core()->GetMemory()->GetBreakpoints();

    for (long unsigned int b = 0; b < breakpoints->size(); b++)
    {
        if ((*breakpoints)[b] == &map[target_offset])
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        breakpoints->push_back(&map[target_offset]);
    }
}

static ImVec4 color_444_to_float(u16 color)
{
    ImVec4 ret;
    ret.w = 0;
    ret.x = (1.0f / 15.0f) * (color & 0xF);
    ret.y = (1.0f / 15.0f) * ((color >> 4) & 0xF);
    ret.z = (1.0f / 15.0f) * ((color >> 8) & 0xF);
    return ret;
}

static ImVec4 color_222_to_float(u8 color)
{
    ImVec4 ret;
    ret.w = 0;
    ret.x = (1.0f / 3.0f) * (color & 0x3);
    ret.y = (1.0f / 3.0f) * ((color >> 2) & 0x3);
    ret.z = (1.0f / 3.0f) * ((color >> 4) & 0x3);
    return ret;
}
