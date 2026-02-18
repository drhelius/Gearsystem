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

#define GUI_DEBUG_MEMORY_IMPORT
#include "gui_debug_memory.h"

#include "gearsystem.h"
#include "imgui.h"
#include "gui_debug_memeditor.h"
#include "gui_filedialogs.h"
#include "config.h"
#include "gui.h"
#include "emu.h"

static MemEditor mem_edit[MEMORY_EDITOR_MAX];
static int mem_edit_select = -1;
static int current_mem_edit = 0;
static char set_value_buffer[5] = { };

static void memory_editor_menu(void);
static void draw_tabs(void);

void gui_debug_memory_reset(void)
{ /*
    GeargrafxCore* core = emu_get_core();
    Memory* memory = core->GetMemory();
    Media* media = core->GetCartridge();
    HuC6260* huc6260 = core->GetHuC6260();
    HuC6270* huc6270_1 = core->GetHuC6270_1();
    HuC6270* huc6270_2 = core->GetHuC6270_2();
    Adpcm* adpcm = core->GetAdpcm();
    bool is_sgx = media->IsSGX();

    mem_edit[MEMORY_EDITOR_RAM].Reset("WRAM", memory->GetWorkingRAM(), 0x2000 * (is_sgx ? 4 : 1));
    mem_edit[MEMORY_EDITOR_ZERO_PAGE].Reset("ZP", memory->GetWorkingRAM(), 0x100);
    mem_edit[MEMORY_EDITOR_ROM].Reset("ROM", media->GetROM(), media->GetROMSize());
    mem_edit[MEMORY_EDITOR_CARD_RAM].Reset("CARD RAM", memory->GetCardRAM(), memory->GetCardRAMSize());
    mem_edit[MEMORY_EDITOR_BACKUP_RAM].Reset("BRAM", memory->GetBackupRAM(), 0x800);
    mem_edit[MEMORY_EDITOR_PALETTES].Reset("PALETTES", (u8*)huc6260->GetColorTable(), 512, 0, 2);
    mem_edit[MEMORY_EDITOR_VRAM_1].Reset(is_sgx ? "VRAM 1" : "VRAM", (u8*)huc6270_1->GetVRAM(), HUC6270_VRAM_SIZE, 0, 2);
    mem_edit[MEMORY_EDITOR_VRAM_2].Reset("VRAM 2", (u8*)huc6270_2->GetVRAM(), HUC6270_VRAM_SIZE, 0, 2);
    mem_edit[MEMORY_EDITOR_SAT_1].Reset(is_sgx ? "SAT 1" : "SAT", (u8*)huc6270_1->GetSAT(), HUC6270_SAT_SIZE, 0, 2);
    mem_edit[MEMORY_EDITOR_SAT_2].Reset("SAT 2", (u8*)huc6270_2->GetSAT(), HUC6270_SAT_SIZE, 0, 2);
    mem_edit[MEMORY_EDITOR_CDROM_RAM].Reset("CDROM RAM", memory->GetCDROMRAM(), memory->GetCDROMRAMSize());
    mem_edit[MEMORY_EDITOR_ADPCM_RAM].Reset("ADPCM", adpcm->GetRAM(), 0x10000);
    mem_edit[MEMORY_EDITOR_ARCADE_RAM].Reset("ARCADE", memory->GetArcadeRAM(), memory->GetArcadeCardRAMSize());
    mem_edit[MEMORY_EDITOR_MB128].Reset("MB128", core->GetInput()->GetMB128()->GetRAM(), 0x20000);
    */
}

void gui_debug_window_memory(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        mem_edit[i].SetGuiFont(gui_roboto_font);
        mem_edit[i].WatchPopup();
        mem_edit[i].BookMarkPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(670, 330), ImGuiCond_FirstUseEver);

    ImGui::Begin("Memory Editor", &config_debug.show_memory, ImGuiWindowFlags_MenuBar);

    memory_editor_menu();

    if (ImGui::BeginTabBar("##memory_tabs", ImGuiTabBarFlags_None))
    {
        draw_tabs();
        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_memory_search_window(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        ImGui::PushFont(gui_default_font);
        mem_edit[i].DrawSearchWindow();
        ImGui::PopFont();
    }
}

void gui_debug_memory_watches_window(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        ImGui::PushFont(gui_default_font);
        mem_edit[i].DrawWatchWindow();
        ImGui::PopFont();
    }
}

void gui_debug_memory_step_frame(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        mem_edit[i].StepFrame();
    }
}

void gui_debug_memory_copy(void)
{
    mem_edit[current_mem_edit].Copy();
}

void gui_debug_memory_paste(void)
{
    mem_edit[current_mem_edit].Paste();
}

void gui_debug_memory_select_all(void)
{
    mem_edit[current_mem_edit].SelectAll();
}

void gui_debug_memory_goto(int editor, int address)
{
    mem_edit_select = editor;
    mem_edit[mem_edit_select].JumpToAddress(address);
}

void gui_debug_memory_save_dump(const char* file_path, bool binary)
{
    if (binary)
        mem_edit[current_mem_edit].SaveToBinaryFile(file_path);
    else
        mem_edit[current_mem_edit].SaveToTextFile(file_path);
}

static void draw_tabs(void)
{
    /*
    GeargrafxCore* core = emu_get_core();
    Media* media = core->GetCartridge();
    bool is_sgx = media->IsSGX();
    bool is_cdrom = media->IsCDROM();
    bool is_arcade_card = core->GetCartridge()->IsArcadeCard();

    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        if (!is_sgx && (i == MEMORY_EDITOR_VRAM_2 || i == MEMORY_EDITOR_SAT_2))
            continue;
        if (i == MEMORY_EDITOR_ROM && !IsValidPointer(media->GetROM()))
            continue;
        if (i == MEMORY_EDITOR_CARD_RAM && core->GetMemory()->GetCardRAMSize() == 0)
            continue;
        if (i == MEMORY_EDITOR_BACKUP_RAM && !core->GetMemory()->IsBackupRamEnabled())
            continue;
        if (i == MEMORY_EDITOR_CDROM_RAM && !is_cdrom)
            continue;
        if (i == MEMORY_EDITOR_ADPCM_RAM && !is_cdrom)
            continue;
        if (i == MEMORY_EDITOR_ARCADE_RAM && !is_arcade_card)
            continue;
        if (i == MEMORY_EDITOR_MB128 && !core->GetInput()->GetMB128()->IsConnected())
            continue;

        if (ImGui::BeginTabItem(mem_edit[i].GetTitle(), NULL, mem_edit_select == i ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
                if (mem_edit_select == i)
                mem_edit_select = -1;
            current_mem_edit = i;
            mem_edit[i].Draw();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }
    }
        */
}

static void memory_editor_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save Memory As Text..."))
        {
            gui_file_dialog_save_memory_dump(false);
        }

        if (ImGui::MenuItem("Save Memory As Binary..."))
        {
            gui_file_dialog_save_memory_dump(true);
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Copy", "Ctrl+C"))
        {
            gui_debug_memory_copy();
        }

        if (ImGui::MenuItem("Paste", "Ctrl+V"))
        {
            gui_debug_memory_paste();
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
            ImVec2 character_size = ImGui::CalcTextSize("X");
            int word_bytes = mem_edit[current_mem_edit].GetWordBytes();
            ImGui::SetNextItemWidth(((word_bytes * 2) + 1) * character_size.x);
            if (ImGui::InputTextWithHint("##set_value", word_bytes == 1 ? "XX" : "XXXX", set_value_buffer, IM_ARRAYSIZE(set_value_buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
            {
                u16 value = 0;
                if (parse_hex_string(set_value_buffer, strlen(set_value_buffer), &value))
                {
                    mem_edit[current_mem_edit].SetValueToSelection((int)value);
                    set_value_buffer[0] = 0;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Set!", ImVec2(40, 0)))
            {
                u16 value = 0;
                if (parse_hex_string(set_value_buffer, strlen(set_value_buffer), &value))
                {
                    mem_edit[current_mem_edit].SetValueToSelection((int)value);
                    set_value_buffer[0] = 0;
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Bookmarks"))
    {
        if (ImGui::MenuItem("Add Bookmark"))
        {
            mem_edit[current_mem_edit].AddBookmark();
        }

        if (ImGui::MenuItem("Clear All"))
        {
            mem_edit[current_mem_edit].RemoveBookmarks();
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

    char label[64];

    if (ImGui::BeginMenu("Watches"))
    {
        snprintf(label, 64, "Open %s Watch Window", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].OpenWatchWindow();
        }

        snprintf(label, 64, "Add %s Watch", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].AddWatch();
        }

        snprintf(label, 64, "Clear All %s Watches", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].RemoveWatches();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Search"))
    {
        snprintf(label, 64, "Open %s Search Window", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].OpenSearchWindow();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

void gui_debug_memory_select_range(int editor, int start_address, int end_address)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    mem_edit_select = editor;
    mem_edit[editor].SetSelection(start_address, end_address);
    mem_edit[editor].ScrollToAddress(start_address);
}

void gui_debug_memory_set_selection_value(int editor, u8 value)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    mem_edit[editor].SetValueToSelection(value);
}

void gui_debug_memory_add_bookmark(int editor, int address, const char* name)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    std::vector<MemEditor::Bookmark>* bookmarks = mem_edit[editor].GetBookmarks();
    MemEditor::Bookmark bookmark;
    bookmark.address = address;

    if (name && strlen(name) > 0)
    {
        snprintf(bookmark.name, sizeof(bookmark.name), "%s", name);
    }
    else
    {
        snprintf(bookmark.name, sizeof(bookmark.name), "Bookmark_%04X", address);
    }

    bookmarks->push_back(bookmark);
}

void gui_debug_memory_remove_bookmark(int editor, int address)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    std::vector<MemEditor::Bookmark>* bookmarks = mem_edit[editor].GetBookmarks();

    for (std::vector<MemEditor::Bookmark>::iterator it = bookmarks->begin(); it != bookmarks->end(); ++it)
    {
        if (it->address == address)
        {
            bookmarks->erase(it);
            break;
        }
    }
}

void gui_debug_memory_add_watch(int editor, int address, const char* notes, int size)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    int size_index = 0;
    switch (size)
    {
        case 8: size_index = 0; break;
        case 16: size_index = 1; break;
        case 24: size_index = 2; break;
        case 32: size_index = 3; break;
        default: size_index = 0; break;
    }

    mem_edit[editor].AddWatchDirect(address, notes, size_index);
}

void gui_debug_memory_open_watch_popup(int editor, int address, const char* notes)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    mem_edit[editor].PrepareAddWatch(address, notes);
}

void gui_debug_memory_remove_watch(int editor, int address)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    std::vector<MemEditor::Watch>* watches = mem_edit[editor].GetWatches();

    for (std::vector<MemEditor::Watch>::iterator it = watches->begin(); it != watches->end(); ++it)
    {
        if (it->address == address)
        {
            watches->erase(it);
            break;
        }
    }
}

int gui_debug_memory_get_bookmarks(int editor, void** bookmarks_ptr)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        *bookmarks_ptr = NULL;
        return 0;
    }

    std::vector<MemEditor::Bookmark>* bookmarks = mem_edit[editor].GetBookmarks();
    *bookmarks_ptr = (void*)bookmarks;
    return (int)bookmarks->size();
}

int gui_debug_memory_get_watches(int editor, void** watches_ptr)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        *watches_ptr = NULL;
        return 0;
    }

    std::vector<MemEditor::Watch>* watches = mem_edit[editor].GetWatches();
    *watches_ptr = (void*)watches;
    return (int)watches->size();
}

void gui_debug_memory_get_selection(int editor, int* start, int* end)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        *start = -1;
        *end = -1;
        return;
    }

    mem_edit[editor].GetSelection(start, end);
}

void gui_debug_memory_search_capture(int editor)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
        return;

    mem_edit[editor].SearchCapture();
}

int gui_debug_memory_search(int editor, int op, int compare_type, int compare_value, int data_type, void** results_ptr)
{
    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        *results_ptr = NULL;
        return 0;
    }

    int count = mem_edit[editor].PerformSearch(op, compare_type, compare_value, data_type);
    std::vector<MemEditor::Search>* results = mem_edit[editor].GetSearchResults();
    *results_ptr = (void*)results;
    return count;
}

void gui_debug_memory_save_settings(std::ostream& stream)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        mem_edit[i].SaveSettings(stream);
    }
}

void gui_debug_memory_load_settings(std::istream& stream)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        mem_edit[i].LoadSettings(stream);
    }
}
