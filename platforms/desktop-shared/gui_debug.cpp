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

#include <math.h>
#include "imgui/imgui.h"
#include "imgui/memory_editor.h"
#include "imgui/colors.h"
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

static MemEditor mem_edit[10];
static int mem_edit_select = -1;
static int current_mem_edit = 0;
static std::vector<DebugSymbol> symbols;
static Memory::stDisassembleRecord* selected_record = NULL;
static char brk_address_cpu[8] = "";
static char brk_address_mem[10] = "";
static bool brk_new_mem_read = true;
static bool brk_new_mem_write = true;
static char goto_address[5] = "";
static bool goto_address_requested = false;
static u16 goto_address_target = 0;
static bool goto_back_requested = false;
static int goto_back = 0;
static char set_value_buffer[5] = {0};

static void debug_window_processor(void);
static void debug_window_memory(void);
static void memory_editor_menu(void);
static void debug_window_disassembler(void);
static void debug_window_vram(void);
static void debug_window_vram_background(void);
static void debug_window_vram_tiles(void);
static void debug_window_vram_sprites(void);
static void debug_window_vram_palettes(void);
static void debug_window_vram_regs(void);
static void add_symbol(const char* line);
static void add_breakpoint_cpu(void);
static void add_breakpoint_mem(void);
static void request_goto_address(u16 addr);
static ImVec4 color_444_to_float(u16 color);
static ImVec4 color_222_to_float(u8 color);
static bool is_return_instruction(u8 opcode1, u8 opcode2);

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
    gui_debug_reset_breakpoints_cpu();
    gui_debug_reset_breakpoints_mem();
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
        bool valid_section = true;

        while (std::getline(file, line))
        {
            size_t comment = line.find_first_of(';');
            if (comment != std::string::npos)
                line = line.substr(0, comment);
            line = line.erase(0, line.find_first_not_of(" \t\r\n"));
            line = line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty())
                continue;

            if (line.find("[") != std::string::npos)
            {
                valid_section = (line.find("[labels]") != std::string::npos);
                continue;
            }

            if (valid_section)
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
        std::vector<Memory::stDisassembleRecord*>* breakpoints = emu_get_core()->GetMemory()->GetBreakpointsCPU();

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

void gui_debug_reset_breakpoints_cpu(void)
{
    emu_get_core()->GetMemory()->GetBreakpointsCPU()->clear();
    brk_address_cpu[0] = 0;
}

void gui_debug_reset_breakpoints_mem(void)
{
    emu_get_core()->GetMemory()->GetBreakpointsMem()->clear();
    brk_address_mem[0] = 0;
}

void gui_debug_go_back(void)
{
    goto_back_requested = true;
}

void gui_debug_copy_memory(void)
{
    mem_edit[current_mem_edit].Copy();
}

void gui_debug_paste_memory(void)
{
    mem_edit[current_mem_edit].Paste();
}

static void memory_editor_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save Memory As..."))
        {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = { { "Memory Dump Files", "txt" } };
            nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, NULL, NULL);
            if (result == NFD_OKAY)
            {
                mem_edit[current_mem_edit].SaveToFile(outPath);
                NFD_FreePath(outPath);
            }
            else if (result != NFD_CANCEL)
            {
                Log("Save Memory Dump Error: %s", NFD_GetError());
            }
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Copy", "Ctrl+C"))
        {
            gui_debug_copy_memory();
        }

        if (ImGui::MenuItem("Paste", "Ctrl+V"))
        {
            gui_debug_paste_memory();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Selection"))
    {
        if (ImGui::MenuItem("Select All", "Ctrl+A"))
        {
            mem_edit[current_mem_edit].SelectAll();
        }

        if (ImGui::MenuItem("Clear Selection"))
        {
            mem_edit[current_mem_edit].ClearSelection();
        }

        if (ImGui::BeginMenu("Set value"))
        {
            ImGui::SetNextItemWidth(50);
            if (ImGui::InputTextWithHint("##set_value", "XXXX", set_value_buffer, IM_ARRAYSIZE(set_value_buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
            {
                try
                {
                    mem_edit[current_mem_edit].SetValueToSelection((int)std::stoul(set_value_buffer, 0, 16));
                    set_value_buffer[0] = 0;
                }
                catch(const std::invalid_argument&)
                {
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Set!", ImVec2(40, 0)))
            {
                try
                {
                    mem_edit[current_mem_edit].SetValueToSelection((int)std::stoul(set_value_buffer, 0, 16));
                    set_value_buffer[0] = 0;
                }
                catch(const std::invalid_argument&)
                {
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Bookmarks"))
    {
        if (ImGui::MenuItem("Clear All"))
        {
            mem_edit[current_mem_edit].RemoveBookmarks();
        }

        if (ImGui::MenuItem("Add Bookmark"))
        {
            mem_edit[current_mem_edit].AddBookmark();
        }

        std::vector<MemEditor::Bookmark>* bookmarks = mem_edit[current_mem_edit].GetBookmarks();

        if (bookmarks->size() > 0)
            ImGui::Separator();

        for (long unsigned int i = 0; i < bookmarks->size(); i++)
        {
            MemEditor::Bookmark* bookmark = &(*bookmarks)[i];

            char label[80];
            snprintf(label, 80, "$%04X: %s", bookmark->address, bookmark->name);

            if (ImGui::MenuItem(label))
            {
                mem_edit[current_mem_edit].JumpToAddress(bookmark->address);
            }
        }

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

static void debug_window_memory(void)
{
    ImGui::SetNextWindowPos(ImVec2(160, 380), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(482, 308), ImGuiCond_FirstUseEver);

    ImGui::Begin("Memory Editor", &config_debug.show_memory, ImGuiWindowFlags_MenuBar);

    memory_editor_menu();

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
            if (ImGui::BeginTabItem("FIXED 1KB", NULL, mem_edit_select == 0 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
            {
                ImGui::PushFont(gui_default_font);
                if (mem_edit_select == 0)
                    mem_edit_select = -1;
                current_mem_edit = 0;
                mem_edit[current_mem_edit].Draw(memory->GetMemoryMap(), 0x400, 0);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("PAGE0", NULL, mem_edit_select == 1 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
            {
                ImGui::PushFont(gui_default_font);
                if (mem_edit_select == 1)
                    mem_edit_select = -1;
                current_mem_edit = 1;
                mem_edit[current_mem_edit].Draw(memory->GetCurrentRule()->GetPage(0) + 0x400, 0x4000 - 0x400, 0x400);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        } 
        else if (ImGui::BeginTabItem("PAGE0", NULL, mem_edit_select == 2 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == 2)
                    mem_edit_select = -1;
                current_mem_edit = 2;
            mem_edit[current_mem_edit].Draw(memory->GetCurrentRule()->GetPage(0), 0x4000, 0);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("PAGE1", NULL, mem_edit_select == 3 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == 3)
                    mem_edit_select = -1;
                current_mem_edit = 3;
            mem_edit[current_mem_edit].Draw(memory->GetCurrentRule()->GetPage(1), 0x4000, 0x4000);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if ((cart->GetType() == Cartridge::CartridgeCodemastersMapper) && IsValidPointer(memory->GetCurrentRule()->GetRamBanks()))
        {
            if (ImGui::BeginTabItem("PAGE2", NULL, mem_edit_select == 4 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
            {
                ImGui::PushFont(gui_default_font);
                if (mem_edit_select == 4)
                    mem_edit_select = -1;
                current_mem_edit = 4;
                mem_edit[current_mem_edit].Draw(memory->GetCurrentRule()->GetPage(2), 0x2000, 0x8000);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("EXT RAM", NULL, mem_edit_select == 5 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
            {
                ImGui::PushFont(gui_default_font);
                if (mem_edit_select == 5)
                    mem_edit_select = -1;
                current_mem_edit = 5;
                mem_edit[current_mem_edit].Draw(memory->GetCurrentRule()->GetRamBanks(), 0x2000, 0xA000);
                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        }
        else if (ImGui::BeginTabItem("PAGE2", NULL, mem_edit_select == 6 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == 6)
                    mem_edit_select = -1;
                current_mem_edit = 6;
            mem_edit[current_mem_edit].Draw(memory->GetCurrentRule()->GetPage(2), 0x4000, 0x8000);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("RAM", NULL, mem_edit_select == 7 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == 7)
                    mem_edit_select = -1;
                current_mem_edit = 7;
            mem_edit[current_mem_edit].Draw(memory->GetMemoryMap() + 0xC000, 0x4000, 0xC000);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("VRAM", NULL, mem_edit_select == 8 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == 8)
                    mem_edit_select = -1;
                current_mem_edit = 8;
            mem_edit[current_mem_edit].Draw(video->GetVRAM(), 0x4000, 0);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("CRAM", NULL, mem_edit_select == 9 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == 9)
                    mem_edit_select = -1;
                current_mem_edit = 9;
            mem_edit[current_mem_edit].Draw(video->GetCRAM(), 0x40, 0);
            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

static void debug_window_disassembler(void)
{
    ImGui::SetNextWindowPos(ImVec2(160, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(482, 344), ImGuiCond_FirstUseEver);

    ImGui::Begin("Disassembler", &config_debug.show_disassembler);

    GearsystemCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();
    Memory* memory = core->GetMemory();
    std::vector<Memory::stDisassembleRecord*>* breakpoints_cpu = memory->GetBreakpointsCPU();
    std::vector<Memory::stMemoryBreakpoint>* breakpoints_mem = memory->GetBreakpointsMem();
    Memory::stDisassembleRecord** memory_map = memory->GetDisassembledMemoryMap();
    Memory::stDisassembleRecord** rom_map = memory->GetDisassembledROMMemoryMap();
    Memory::stDisassembleRecord** map = NULL;

    int pc = proc_state->PC->GetValue();

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

    static bool follow_pc = true;
    static bool show_mem = true;
    static bool show_symbols = true;

    ImGui::Checkbox("Follow PC", &follow_pc); ImGui::SameLine();
    ImGui::Checkbox("Show Memory", &show_mem);  ImGui::SameLine();
    ImGui::Checkbox("Show Symbols", &show_symbols);

    ImGui::Separator();

    ImGui::Text("Go To Address: ");
    ImGui::SameLine();
    ImGui::PushItemWidth(45);
    if (ImGui::InputTextWithHint("##goto_address", "XXXX", goto_address, IM_ARRAYSIZE(goto_address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
    {
        try
        {
            request_goto_address((u16)std::stoul(goto_address, 0, 16));
            follow_pc = false;
        }
        catch(const std::invalid_argument&)
        {
        }
        goto_address[0] = 0;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Go", ImVec2(30, 0)))
    {
        try
        {
            request_goto_address((u16)std::stoul(goto_address, 0, 16));
            follow_pc = false;
        }
        catch(const std::invalid_argument&)
        {
        }
        goto_address[0] = 0;
    }

    ImGui::SameLine();
    if (ImGui::Button("Back", ImVec2(50, 0)))
    {
        goto_back_requested = true;
        follow_pc = false;
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Processor Breakpoints"))
    {
        ImGui::Checkbox("Disable All##disable_all_cpu", &emu_debug_disable_breakpoints_cpu);

        ImGui::Columns(2, "breakpoints_cpu");
        ImGui::SetColumnOffset(1, 85);

        ImGui::Separator();

        if (IsValidPointer(selected_record))
            snprintf(brk_address_cpu, sizeof(brk_address_cpu), "%02X:%04X", selected_record->bank, selected_record->address);

        ImGui::PushItemWidth(70);
        if (ImGui::InputTextWithHint("##add_breakpoint_cpu", "XX:XXXX", brk_address_cpu, IM_ARRAYSIZE(brk_address_cpu), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            add_breakpoint_cpu();
        }
        ImGui::PopItemWidth();

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Use XXXX format for addresses in bank 0 or XX:XXXX for selecting bank and address");

        if (ImGui::Button("Add##add_cpu", ImVec2(70, 0)))
        {
            add_breakpoint_cpu();
        }

        if (ImGui::Button("Clear All##clear_all_cpu", ImVec2(70, 0)))
        {
            gui_debug_reset_breakpoints_cpu();
        }

        ImGui::NextColumn();

        ImGui::BeginChild("breakpoints_cpu", ImVec2(0, 80), false);

        int remove = -1;

        for (long unsigned int b = 0; b < breakpoints_cpu->size(); b++)
        {
            if (!IsValidPointer((*breakpoints_cpu)[b]))
                continue;

            ImGui::PushID(b);
            if (ImGui::SmallButton("X"))
            {
               remove = b;
               ImGui::PopID();
               continue;
            }

            ImGui::PopID();

            ImGui::PushFont(gui_default_font);
            ImGui::SameLine();
            ImGui::TextColored(red, "%02X:%04X", (*breakpoints_cpu)[b]->bank, (*breakpoints_cpu)[b]->address);
            ImGui::SameLine();
            ImGui::TextColored(gray, "%s", (*breakpoints_cpu)[b]->name);
            ImGui::PopFont();
        }

        if (remove >= 0)
        {
            breakpoints_cpu->erase(breakpoints_cpu->begin() + remove);
        }

        ImGui::EndChild();
        ImGui::Columns(1);
        ImGui::Separator();

    }

    if (ImGui::CollapsingHeader("Memory Breakpoints"))
    {
        ImGui::Checkbox("Disable All##diable_all_mem", &emu_debug_disable_breakpoints_mem);

        ImGui::Columns(2, "breakpoints_mem");
        ImGui::SetColumnOffset(1, 100);

        ImGui::Separator();

        ImGui::PushItemWidth(85);
        if (ImGui::InputTextWithHint("##add_breakpoint_mem", "XXXX-XXXX", brk_address_mem, IM_ARRAYSIZE(brk_address_mem), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            add_breakpoint_mem();
        }
        ImGui::PopItemWidth();

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Use XXXX format for single addresses or XXXX-XXXX for address ranges");

        ImGui::Checkbox("Read", &brk_new_mem_read);
        ImGui::Checkbox("Write", &brk_new_mem_write);

        if (ImGui::Button("Add##add_mem", ImVec2(85, 0)))
        {
            add_breakpoint_mem();
        }

        if (ImGui::Button("Clear All##clear_all_mem", ImVec2(85, 0)))
        {
            gui_debug_reset_breakpoints_mem();
        }

        ImGui::NextColumn();

        ImGui::BeginChild("breakpoints_mem", ImVec2(0, 130), false);

        int remove = -1;

        for (long unsigned int b = 0; b < breakpoints_mem->size(); b++)
        {
            ImGui::PushID(10000 + b);
            if (ImGui::SmallButton("X"))
            {
               remove = b;
               ImGui::PopID();
               continue;
            }

            ImGui::PopID();

            ImGui::PushFont(gui_default_font);
            ImGui::SameLine();
            if ((*breakpoints_mem)[b].range)
                ImGui::TextColored(red, "%04X-%04X", (*breakpoints_mem)[b].address1, (*breakpoints_mem)[b].address2);
            else
                ImGui::TextColored(red, "%04X", (*breakpoints_mem)[b].address1);
            if ((*breakpoints_mem)[b].read)
            {
                ImGui::SameLine(); ImGui::TextColored(gray, "R");
            }
            if ((*breakpoints_mem)[b].write)
            {
                ImGui::SameLine(); ImGui::TextColored(gray, "W");
            }
            ImGui::PopFont();
        }

        if (remove >= 0)
        {
            breakpoints_mem->erase(breakpoints_mem->begin() + remove);
        }

        ImGui::EndChild();
        ImGui::Columns(1);
        ImGui::Separator();
    }

    ImGui::PushFont(gui_default_font);

    bool window_visible = ImGui::BeginChild("##dis", ImVec2(ImGui::GetContentRegionAvail().x, 0), true, 0);
    
    if (window_visible)
    {
        int dis_size = 0;
        int pc_pos = 0;
        int goto_address_pos = 0;
        
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
                map = rom_map;
                break;
            case 0x4000:
                bank = memory->GetCurrentRule()->GetBank(1);
                offset = (0x4000 * bank) + (i & 0x3FFF);
                map = rom_map;
                break;
            case 0x8000:
                bank = memory->GetCurrentRule()->GetBank(2);
                offset = (0x4000 * bank) + (i & 0x3FFF);
                map = rom_map;
                break;
            default:
                map = memory_map;
            }

            if (IsValidPointer(map[offset]) && (map[offset]->name[0] != 0))
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
                vec[dis_size].record = map[offset];

                if (vec[dis_size].record->address == pc)
                    pc_pos = dis_size;

                if (goto_address_requested && (vec[dis_size].record->address <= goto_address_target))
                    goto_address_pos = dis_size;

                vec[dis_size].is_breakpoint = false;

                for (long unsigned int b = 0; b < breakpoints_cpu->size(); b++)
                {
                    if ((*breakpoints_cpu)[b] == vec[dis_size].record)
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

        if (goto_address_requested)
        {
            goto_address_requested = false;
            goto_back = (int)ImGui::GetScrollY();
            ImGui::SetScrollY((goto_address_pos * ImGui::GetTextLineHeightWithSpacing()) + 2);
        }

        if (goto_back_requested)
        {
            goto_back_requested = false;
            ImGui::SetScrollY((float)goto_back);
        }

        ImGuiListClipper clipper;
        clipper.Begin(dis_size, ImGui::GetTextLineHeightWithSpacing());

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

                if (ImGui::Selectable("", is_selected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    if (ImGui::IsMouseDoubleClicked(0) && vec[item].record->jump)
                    {
                        follow_pc = false;
                        request_goto_address(vec[item].record->jump_address);
                    }
                    else if (is_selected)
                    {
                        InitPointer(selected_record);
                        brk_address_cpu[0] = 0;
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

                bool is_ret = is_return_instruction(vec[item].record->opcodes[0], vec[item].record->opcodes[1]);
                if (is_ret)
                {
                    ImGui::PushStyleColor(ImGuiCol_Separator, (vec[item].record->opcodes[0] == 0xC9) ? gray : dark_gray);
                    ImGui::Separator();
                    ImGui::PopStyleColor();
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
    ImGui::SetNextWindowPos(ImVec2(6, 30), ImGuiCond_FirstUseEver);

    ImGui::Begin("Z80 Status", &config_debug.show_processor, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::PushFont(gui_default_font);

    GearsystemCore* core = emu_get_core();
    Processor* processor = core->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();

    ImGui::Separator();

    ImGui::TextColored(orange, "  S Z Y H X P N C");
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
    ImGui::Text(" $%02X", *proc_state->I);
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(*proc_state->I));

    ImGui::NextColumn();
    ImGui::TextColored(cyan, " R"); ImGui::SameLine();
    ImGui::Text(" $%02X", *proc_state->R);
    ImGui::Text(BYTE_TO_BINARY_PATTERN_SPACED, BYTE_TO_BINARY(*proc_state->R));

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
    ImGui::SetNextWindowPos(ImVec2(648, 254), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(604, 534), ImGuiCond_FirstUseEver);

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
            debug_window_vram_sprites();
            ImGui::EndTabItem();
        }

        bool isSG1000 = emu_get_core()->GetVideo()->IsSG1000Mode();

        if (ImGui::BeginTabItem(isSG1000 ? "Registers" : "Palettes & Registers"))
        {
            if (!isSG1000)
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
    bool isMode224 = video->IsExtendedMode224();
    bool isGG = emu_get_core()->GetCartridge()->IsGameGear();
    bool isSG1000 = video->IsSG1000Mode();
    int SG1000mode = video->GetSG1000Mode();

    bool window_hovered = ImGui::IsWindowHovered();
    static bool show_grid = true;
    static bool show_screen = true;
    int lines = 28;
    if (isMode224)
        lines = 32;
    else if (isSG1000)
        lines = 24;
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

    ImGui::Image((ImTextureID)(intptr_t)renderer_emu_debug_vram_background, ImVec2(size_h, size_v), ImVec2(0.0f, 0.0f), ImVec2(1.0f, (1.0f / 32.0f) * lines));

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

        if (isGG)
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
    if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < size_h) && (mouse_y >= 0.0f) && (mouse_y < size_v))
    {
        tile_x = (int)(mouse_x / spacing);
        tile_y = (int)(mouse_y / spacing);

        draw_list->AddRect(ImVec2(p.x + (tile_x * spacing), p.y + (tile_y * spacing)), ImVec2(p.x + ((tile_x + 1) * spacing), p.y + ((tile_y + 1) * spacing)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::NextColumn();

        ImGui::Image((ImTextureID)(intptr_t)renderer_emu_debug_vram_background, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 32.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 32.0f) * (tile_y + 1)));

        ImGui::TextColored(yellow, "INFO:");

        ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_x); ImGui::SameLine();
        ImGui::TextColored(cyan, "   Y:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_y);

        if (isSG1000)
        {
            int name_table_addr = (regs[2] & 0x0F) << 10;
            int region = (regs[4] & 0x03) << 8;
            int tile_number = (tile_y * 32) + tile_x;
            int name_tile_addr = name_table_addr + tile_number;

            int name_tile = 0;

            if (SG1000mode == 0x200)
                name_tile = vram[name_tile_addr] | (region & 0x300 & tile_number);
            else
                name_tile = vram[name_tile_addr];

            int pattern_table_addr = (regs[4] & (SG1000mode == 0x200 ? 0x04 : 0x07)) << 11;
            int tile_addr = pattern_table_addr + (name_tile << 3);

            ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
            ImGui::Text(" $%04X", tile_addr);
            ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
            ImGui::Text("$%03X", name_tile);
        }
        else
        {
            int name_table_addr = (regs[2] & (isMode224 ? 0x0C : 0x0E)) << 10;
            if (isMode224)
                name_table_addr |= 0x700;
            u16 map_addr = name_table_addr + (64 * tile_y) + (tile_x * 2);

            ImGui::TextColored(cyan, " Map Addr: "); ImGui::SameLine();
            ImGui::Text(" $%04X", map_addr);

            u16 tile_info_lo = vram[map_addr];
            u16 tile_info_hi = vram[map_addr + 1];

            int tile_number = ((tile_info_hi & 1) << 8) | tile_info_lo;
            bool tile_hflip = IsSetBit((u8)tile_info_hi, 1);
            bool tile_vflip = IsSetBit((u8)tile_info_hi, 2);
            int tile_palette = IsSetBit((u8)tile_info_hi, 3) ? 16 : 0;
            bool tile_priority = IsSetBit((u8)tile_info_hi, 4);

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
    }

    ImGui::Columns(1);

    ImGui::PopFont();
}

static void debug_window_vram_tiles(void)
{
    Video* video = emu_get_core()->GetVideo();
    u8* regs = video->GetRegisters();
    int SG1000mode = video->GetSG1000Mode();
    bool isSG1000 = video->IsSG1000Mode();

    bool window_hovered = ImGui::IsWindowHovered();
    static bool show_grid = true;
    int lines = isSG1000 ? 32 : 16;
    float scale = 1.5f;
    float width = 8.0f * 32.0f * scale;
    float height = 8.0f * lines * scale;
    float spacing = 8.0f * scale;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p;

    ImGui::Checkbox("Show Grid##grid_tiles", &show_grid);

    if (!isSG1000)
    {
        ImGui::SameLine(140.0f);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("Palette##tile_palette", &emu_debug_tile_palette, "Palette 0 (BG)\0Palette 1 (BG & Sprites)\0\0");
        ImGui::PopItemWidth();
    }

    ImGui::Columns(2, "tiles", false);
    ImGui::SetColumnOffset(1, width + 10.0f);

    p = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)renderer_emu_debug_vram_tiles, ImVec2(width, height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, (1.0f / 32.0f) * lines));

    if (show_grid)
    {
        float x = p.x;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + height), ImColor(dark_gray), 1.0f);
            x += spacing;
        }

        float y = p.y;  
        for (int n = 0; n <= lines; n++)
        {
            draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + width, y), ImColor(dark_gray), 1.0f);
            y += spacing;
        }
    }

    float mouse_x = io.MousePos.x - p.x;
    float mouse_y = io.MousePos.y - p.y;

    int tile_x = -1;
    int tile_y = -1;

    if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < height))
    {
        tile_x = (int)(mouse_x / spacing);
        tile_y = (int)(mouse_y / spacing);

        draw_list->AddRect(ImVec2(p.x + (tile_x * spacing), p.y + (tile_y * spacing)), ImVec2(p.x + ((tile_x + 1) * spacing), p.y + ((tile_y + 1) * spacing)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::NextColumn();

        ImGui::Image((ImTextureID)(intptr_t)renderer_emu_debug_vram_tiles, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 32.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 32.0f) * (tile_y + 1)));

        ImGui::PushFont(gui_default_font);

        ImGui::TextColored(yellow, "DETAILS:");

        int tile = (tile_y << 5) + tile_x;

        int tile_addr = 0;

        if (isSG1000)
        {
            int pattern_table_addr = (regs[4] & (SG1000mode == 0x200 ? 0x04 : 0x07)) << 11;
            tile_addr = pattern_table_addr + (tile << 3);
        }
        else
        {
            tile_addr = tile << 5;
        }

        ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
        ImGui::Text("$%03X", tile); 
        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", tile_addr); 

        ImGui::PopFont();
    }

    ImGui::Columns(1);
}

static void debug_window_vram_sprites(void)
{
    float scale = 4.0f;
    float size_8 = 8.0f * scale;
    float size_16 = 16.0f * scale;

    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);
    bool isGG = core->GetCartridge()->IsGameGear();
    bool isSG1000 = video->IsSG1000Mode();
    bool sprites_16 = IsSetBit(regs[1], 1);

    float width = 0.0f;
    float height = 0.0f;

    if (isSG1000)
    {
        width = sprites_16 ? size_16 : size_8;
        height = sprites_16 ? size_16 : size_8;
    }
    else
    {
        width = size_8;
        height = sprites_16 ? size_16 : size_8;
    }

    ImVec2 p[64];

    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "spr", false);
    ImGui::SetColumnOffset(1, (sprites_16 && isSG1000) ? 330.0f : 200.0f);

    ImGui::BeginChild("sprites", ImVec2(0, 0.0f), true);

    bool window_hovered = ImGui::IsWindowHovered();

    for (int s = 0; s < 64; s++)
    {
        p[s] = ImGui::GetCursorScreenPos();

        ImGui::Image((ImTextureID)(intptr_t)renderer_emu_debug_vram_sprites[s], ImVec2(width, height), ImVec2(0.0f, 0.0f), ImVec2((1.0f / 16.0f) * (width / scale), (1.0f / 16.0f) * (height / scale)));

        float mouse_x = io.MousePos.x - p[s].x;
        float mouse_y = io.MousePos.y - p[s].y;

        if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < height))
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + width, p[s].y + height), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 3.0f);
        }

        if (s % 4 < 3)
            ImGui::SameLine();
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    ImVec2 p_screen = ImGui::GetCursorScreenPos();

    float screen_scale = 1.0f;

    float tex_h = (float)runtime.screen_width / (float)(GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN);
    float tex_v = (float)runtime.screen_height / (float)(GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN);

    ImGui::Image((ImTextureID)(intptr_t)renderer_emu_texture, ImVec2(runtime.screen_width * screen_scale, runtime.screen_height * screen_scale), ImVec2(0, 0), ImVec2(tex_h, tex_v));

    for (int s = 0; s < 64; s++)
    {
        if ((p[s].x == 0) && (p[s].y == 0))
            continue;

        float mouse_x = io.MousePos.x - p[s].x;
        float mouse_y = io.MousePos.y - p[s].y;

        if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < height))
        {
            int x = 0;
            int y = 0;
            int tile = 0;
            int sprite_tile_addr = 0;
            int sprite_shift = 0;
            float real_x = 0.0f;
            float real_y = 0.0f;

            if (isSG1000)
            {
                u16 sprite_attribute_addr = (regs[5] & 0x7F) << 7;
                u16 sprite_pattern_addr = (regs[6] & 0x07) << 11;
                int sprite_attribute_offset = sprite_attribute_addr + (s << 2);
                tile = vram[sprite_attribute_offset + 2];
                sprite_tile_addr = sprite_pattern_addr + (tile << 3);
                sprite_shift = (vram[sprite_attribute_offset + 3] & 0x80) ? 32 : 0;
                x = vram[sprite_attribute_offset + 1];
                y = vram[sprite_attribute_offset];

                int final_y = (y + 1) & 0xFF;

                if (final_y >= 0xE0)
                    final_y = -(0x100 - final_y);

                real_x = (float)(x - sprite_shift);
                real_y = (float)final_y;
            }
            else
            {
                sprite_shift = IsSetBit(regs[0], 3) ? 8 : 0;
                u16 sprite_table_address = (regs[5] << 7) & 0x3F00;
                u16 sprite_table_address_2 = sprite_table_address + 0x80;
                u16 sprite_info_address = sprite_table_address_2 + (s << 1);
                u16 sprite_tiles_address = (regs[6] << 11) & 0x2000;
                y = vram[sprite_table_address + s];
                x = vram[sprite_info_address];
                tile = vram[sprite_info_address + 1];
                tile &= sprites_16 ? 0xFE : 0xFF;
                sprite_tile_addr = sprite_tiles_address + (tile << 5);

                real_x = (float)(x - sprite_shift - (isGG ? GS_RESOLUTION_GG_X_OFFSET : 0));
                real_y = (float)(y + 1.0f - (isGG ? GS_RESOLUTION_GG_Y_OFFSET : 0));
            }

            float max_width = 8.0f;
            float max_height = sprites_16 ? 16.0f : 8.0f;

            if (isSG1000)
            {
                if (sprites_16)
                    max_width = 16.0f;

                if(IsSetBit(regs[1], 0))
                {
                    max_width *= 2.0f;
                    max_height *= 2.0f;
                }
            }

            float rectx_min = p_screen.x + (real_x * screen_scale);
            float rectx_max = p_screen.x + ((real_x + max_width) * screen_scale);
            float recty_min = p_screen.y + (real_y * screen_scale);
            float recty_max = p_screen.y + ((real_y + max_height) * screen_scale);

            rectx_min = fminf(fmaxf(rectx_min, p_screen.x), p_screen.x + (runtime.screen_width * screen_scale));
            rectx_max = fminf(fmaxf(rectx_max, p_screen.x), p_screen.x + (runtime.screen_width * screen_scale));
            recty_min = fminf(fmaxf(recty_min, p_screen.y), p_screen.y + (runtime.screen_height * screen_scale));
            recty_max = fminf(fmaxf(recty_max, p_screen.y), p_screen.y + (runtime.screen_height * screen_scale));
            
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

            ImGui::TextColored(yellow, "DETAILS:");
            ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
            ImGui::Text("$%02X", x); ImGui::SameLine();
            ImGui::TextColored(cyan, "  Y:"); ImGui::SameLine();
            ImGui::Text("$%02X", y); ImGui::SameLine();

            ImGui::TextColored(cyan, "  Tile:"); ImGui::SameLine();
            ImGui::Text("$%02X", tile);

            ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
            ImGui::Text("$%04X", sprite_tile_addr);

            ImGui::TextColored(cyan, " Horizontal Sprite Shift:"); ImGui::SameLine();
            sprite_shift > 0 ? ImGui::TextColored(green, "ON ") : ImGui::TextColored(gray, "OFF");
        }
    }

    ImGui::Columns(1);

    ImGui::PopFont();
}

static void debug_window_vram_palettes(void)
{
    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* palettes = video->GetCRAM();
    bool isGG = core->GetCartridge()->IsGameGear();

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(yellow, "PALETTE 0 (BG):");

    for (int i = 0; i < 2; i ++)
    {
        ImGui::Text(" "); ImGui::SameLine(31.0f);
        for (int c = 0; c < 16; c++)
        {
            ImVec4 float_color;
            if (isGG)
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
            snprintf(id, sizeof(id), "##pal_%d_%d", i, c);
            ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 15)
            {   
                ImGui::SameLine(31.0f + (29.0f * (c + 1))); 
            }
        }

        ImGui::Text("  "); ImGui::SameLine();

        for (int c = 0; c < 16; c++)
        {
            if (isGG)
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
    Debug("Loading symbol %s", line);

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

        try
        {
            if (separator != std::string::npos)
            {
                s.address = (u16)std::stoul(str.substr(separator + 1 , std::string::npos), 0, 16);

                s.bank = std::stoul(str.substr(0, separator), 0 , 16);
            }
            else
            {
                s.address = (u16)std::stoul(str, 0, 16);
                s.bank = 0;
            }

            symbols.push_back(s);
        }
        catch(const std::invalid_argument&)
        {
        }
    }
}

static void add_breakpoint_cpu(void)
{
    int input_len = (int)strlen(brk_address_cpu);
    u16 target_address = 0;
    int target_bank = 0;
    int target_offset = 0;

    try
    {
        if ((input_len == 7) && (brk_address_cpu[2] == ':'))
        {
            std::string str(brk_address_cpu);
            std::size_t separator = str.find(":");

            if (separator != std::string::npos)
            {
                target_address = (u16)std::stoul(str.substr(separator + 1 , std::string::npos), 0, 16);

                target_bank = std::stoul(str.substr(0, separator), 0 , 16);
                target_bank &= 0xFF;
            }
        } 
        else if (input_len == 4)
        {
            target_bank = 0; 
            target_address = (u16)std::stoul(brk_address_cpu, 0, 16);
        }
        else
        {
            return;
        }
    }
    catch(const std::invalid_argument&)
    {
        return;
    }

    Memory::stDisassembleRecord** memoryMap = emu_get_core()->GetMemory()->GetDisassembledMemoryMap();
    Memory::stDisassembleRecord** romMap = emu_get_core()->GetMemory()->GetDisassembledROMMemoryMap();
    Memory::stDisassembleRecord** map = NULL;

    bool rom = true;

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
        target_offset = target_address;
        map = memoryMap;
        rom = false;
    }

    brk_address_cpu[0] = 0;

    bool found = false;
    std::vector<Memory::stDisassembleRecord*>* breakpoints = emu_get_core()->GetMemory()->GetBreakpointsCPU();

    if (IsValidPointer(map[target_offset]))
    {
        for (long unsigned int b = 0; b < breakpoints->size(); b++)
        {
            if ((*breakpoints)[b] == map[target_offset])
            {
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        if (!IsValidPointer(map[target_offset]))
        {
            map[target_offset] = new Memory::stDisassembleRecord;

            if (rom)
            {
                map[target_offset]->address = target_offset & 0x3FFF;
                map[target_offset]->bank = target_offset >> 14;
            }
            else
            {
                map[target_offset]->address = 0;
                map[target_offset]->bank = 0;
            }

            map[target_offset]->name[0] = 0;
            map[target_offset]->bytes[0] = 0;
            map[target_offset]->size = 0;
            map[target_offset]->jump = false;
            map[target_offset]->jump_address = 0;
            for (int i = 0; i < 4; i++)
                map[target_offset]->opcodes[i] = 0;
        }

        breakpoints->push_back(map[target_offset]);
    }
}

static void add_breakpoint_mem(void)
{
    int input_len = (int)strlen(brk_address_mem);
    u16 address1 = 0;
    u16 address2 = 0;
    bool range = false;

    try
    {
        if ((input_len == 9) && (brk_address_mem[4] == '-'))
        {
            std::string str(brk_address_mem);
            std::size_t separator = str.find("-");

            if (separator != std::string::npos)
            {
                address1 = (u16)std::stoul(str.substr(0, separator), 0 , 16);
                address2 = (u16)std::stoul(str.substr(separator + 1 , std::string::npos), 0, 16);
                range = true;
            }
        }
        else if (input_len == 4)
        {
            address1 = (u16)std::stoul(brk_address_mem, 0, 16);
        }
        else
        {
            return;
        }
    }
    catch(const std::invalid_argument&)
    {
        return;
    }

    bool found = false;
    std::vector<Memory::stMemoryBreakpoint>* breakpoints = emu_get_core()->GetMemory()->GetBreakpointsMem();

    for (long unsigned int b = 0; b < breakpoints->size(); b++)
    {
        Memory::stMemoryBreakpoint temp = (*breakpoints)[b];
        if ((temp.address1 == address1) && (temp.address2 == address2) && (temp.range == range))
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        Memory::stMemoryBreakpoint new_breakpoint;
        new_breakpoint.address1 = address1;
        new_breakpoint.address2 = address2;
        new_breakpoint.range = range;
        new_breakpoint.read = brk_new_mem_read;
        new_breakpoint.write = brk_new_mem_write;

        breakpoints->push_back(new_breakpoint);
    }

    brk_address_mem[0] = 0;
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

static void request_goto_address(u16 address)
{
    goto_address_requested = true;
    goto_address_target = address;
}

static bool is_return_instruction(u8 opcode1, u8 opcode2)
{
    switch (opcode1)
    {
        case 0xC9: // RET
        case 0xC0: // RET NZ
        case 0xC8: // RET Z
        case 0xD0: // RET NC
        case 0xD8: // RET C
        case 0xE0: // RET PO
        case 0xE8: // RET PE
        case 0xF0: // RET P
        case 0xF8: // RET M
            return true;
        case 0xED: // Extended instructions
            if (opcode2 == 0x45 || opcode2 == 0x4D) // RETN, RETI
                return true;
            else
                return false;
        default:
            return false;
    }
}
