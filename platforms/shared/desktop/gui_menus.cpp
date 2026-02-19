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

#define GUI_MENUS_IMPORT
#include "gui_menus.h"
#include "gui.h"
#include "gui_filedialogs.h"
#include "gui_popups.h"
#include "gui_actions.h"
#include "gui_debug_disassembler.h"
#include "gui_debug_memory.h"
#include "config.h"
#include "application.h"
#include "display.h"
#include "gamepad.h"
#include "emu.h"
#include "ogl_renderer.h"
#include "utils.h"
#include "gearsystem.h"

static bool open_rom = false;
static bool open_ram = false;
static bool save_ram = false;
static bool open_state = false;
static bool save_state = false;
static bool open_about = false;
static bool open_load_defaults = false;
static bool save_screenshot = false;
static bool save_vgm = false;
static bool choose_savestates_path = false;
static bool choose_screenshots_path = false;
static bool choose_backup_ram_path = false;
static bool open_sms_bootrom = false;
static bool open_gg_bootrom = false;
static bool save_debug_settings = false;
static bool load_debug_settings = false;

static void menu_gearsystem(void);
static void menu_emulator(void);
static void menu_video(void);
static void menu_input(void);
static void menu_audio(void);
static void menu_debug(void);
static void menu_about(void);
static void file_dialogs(void);
static void keyboard_configuration_item(const char* text, SDL_Scancode* key, int player);
static void gamepad_configuration_item(const char* text, int* button, int player);
static void hotkey_configuration_item(const char* text, config_Hotkey* hotkey);
static void gamepad_device_selector(int player);
static void draw_savestate_slot_info(int slot);

void gui_init_menus(void)
{
    gui_shortcut_open_rom = false;
}

void gui_main_menu(void)
{
    open_rom = false;
    open_ram = false;
    save_ram = false;
    open_state = false;
    save_state = false;
    open_about = false;
    open_load_defaults = false;
    save_screenshot = false;
    save_vgm = false;
    choose_savestates_path = false;
    choose_screenshots_path = false;
    gui_main_menu_hovered = false;
    choose_backup_ram_path = false;
    open_sms_bootrom = false;
    open_gg_bootrom = false;
    save_debug_settings = false;
    load_debug_settings = false;

    if (application_show_menu && ImGui::BeginMainMenuBar())
    {
        gui_main_menu_hovered = ImGui::IsWindowHovered();

        menu_gearsystem();
        menu_emulator();
        menu_video();
        menu_input();
        menu_audio();
        menu_debug();
        menu_about();

        gui_main_menu_height = (int)ImGui::GetWindowSize().y;

        ImGui::EndMainMenuBar();
    }

    file_dialogs();
}

static void menu_gearsystem(void)
{
    if (ImGui::BeginMenu(GS_TITLE))
    {
        gui_in_use = true;

        if (ImGui::MenuItem("Open ROM/CD...", config_hotkeys[config_HotkeyIndex_OpenROM].str))
        {
            open_rom = true;
        }

        if (ImGui::BeginMenu("Open Recent"))
        {
            for (int i = 0; i < config_max_recent_roms; i++)
            {
                if (config_emulator.recent_roms[i].length() > 0)
                {
                    if (ImGui::MenuItem(config_emulator.recent_roms[i].c_str()))
                    {
                        char rom_path[4096];
                        strcpy(rom_path, config_emulator.recent_roms[i].c_str());
                        gui_load_rom(rom_path);
                    }
                }
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();
        
        if (ImGui::MenuItem("Reset", config_hotkeys[config_HotkeyIndex_Reset].str))
        {
            gui_action_reset();
        }

        if (ImGui::MenuItem("Pause", config_hotkeys[config_HotkeyIndex_Pause].str, &config_emulator.paused))
        {
            gui_action_pause();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Fast Forward", config_hotkeys[config_HotkeyIndex_FFWD].str, &config_emulator.ffwd))
        {
            gui_action_ffwd();
        }

        if (ImGui::BeginMenu("Fast Forward Speed"))
        {
            ImGui::PushItemWidth(100.0f);
            ImGui::Combo("##fwd", &config_emulator.ffwd_speed, "X 1.5\0X 2\0X 2.5\0X 3\0Unlimited\0\0");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save BRAM As..."))
        {
            save_ram = true;
        }

        if (ImGui::MenuItem("Load BRAM From..."))
        {
            open_ram = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save State As...")) 
        {
            save_state = true;
        }

        if (ImGui::MenuItem("Load State From..."))
        {
            open_state = true;
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Save State Slot"))
        {
            ImGui::PushItemWidth(100.0f);
            ImGui::Combo("##slot", &config_emulator.save_slot, "Slot 1\0Slot 2\0Slot 3\0Slot 4\0Slot 5\0\0");
            ImGui::PopItemWidth();

            ImGui::Separator();
            draw_savestate_slot_info(config_emulator.save_slot);

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Save State", config_hotkeys[config_HotkeyIndex_SaveState].str))
        {
            std::string message("Saving state to slot ");
            message += std::to_string(config_emulator.save_slot + 1);
            gui_set_status_message(message.c_str(), 3000);
            emu_save_state_slot(config_emulator.save_slot + 1);
        }

        if (ImGui::MenuItem("Load State", config_hotkeys[config_HotkeyIndex_LoadState].str))
        {
            std::string message("Loading state from slot ");
            message += std::to_string(config_emulator.save_slot + 1);
            gui_set_status_message(message.c_str(), 3000);
            emu_load_state_slot(config_emulator.save_slot + 1);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Slot: %d", config_emulator.save_slot + 1);
            ImGui::Separator();
            draw_savestate_slot_info(config_emulator.save_slot);
            ImGui::EndTooltip();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Screenshot As..."))
        {
            save_screenshot = true;
        }

        if (ImGui::MenuItem("Save Screenshot", config_hotkeys[config_HotkeyIndex_Screenshot].str))
        {
            gui_action_save_screenshot(NULL);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Load Default Settings"))
        {
            open_load_defaults = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit", config_hotkeys[config_HotkeyIndex_Quit].str))
        {
            application_trigger_quit();
        }

        ImGui::EndMenu();
    }
}

static void menu_emulator(void)
{
    if (ImGui::BeginMenu("Emulator"))
    {
        gui_in_use = true;

        if (ImGui::BeginMenu("Save States Dir"))
        {
            ImGui::PushItemWidth(220.0f);
            if (ImGui::Combo("##savestate_option", &config_emulator.savestates_dir_option, "Default Location\0Same as ROM\0Custom Location\0\0"))
            {
                update_savestates_data();
            }

            switch ((Directory_Location)config_emulator.savestates_dir_option)
            {
                case Directory_Location_Default:
                {
                    ImGui::Text("%s", config_root_path);
                    break;
                }
                case Directory_Location_ROM:
                {
                    if (!emu_is_empty())
                        ImGui::Text("%s", emu_get_core()->GetCartridge()->GetFileDirectory());
                    break;
                }
                case Directory_Location_Custom:
                {
                    if (ImGui::MenuItem("Choose..."))
                    {
                        choose_savestates_path = true;
                    }

                    ImGui::PushItemWidth(450);
                    if (ImGui::InputText("##savestate_path", gui_savestates_path, IM_ARRAYSIZE(gui_savestates_path), ImGuiInputTextFlags_AutoSelectAll))
                    {
                        config_emulator.savestates_path.assign(gui_savestates_path);
                        update_savestates_data();
                    }
                    ImGui::PopItemWidth();
                    break;
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Save Files Dir"))
        {
            ImGui::PushItemWidth(220.0f);
            ImGui::Combo("##savefiles_option", &config_emulator.savefiles_dir_option, "Default Location\0Same as ROM\0Custom Location\0\0");

            switch ((Directory_Location)config_emulator.savefiles_dir_option)
            {
                case Directory_Location_Default:
                {
                    ImGui::Text("%s", config_root_path);
                    break;
                }
                case Directory_Location_ROM:
                {
                    if (!emu_is_empty())
                        ImGui::Text("%s", emu_get_core()->GetCartridge()->GetFileDirectory());
                    break;
                }
                case Directory_Location_Custom:
                {
                    if (ImGui::MenuItem("Choose..."))
                    {
                        choose_backup_ram_path = true;
                    }

                    ImGui::PushItemWidth(450);
                    if (ImGui::InputText("##savefiles_path", gui_savefiles_path, IM_ARRAYSIZE(gui_savefiles_path), ImGuiInputTextFlags_AutoSelectAll))
                    {
                        config_emulator.savefiles_path.assign(gui_savefiles_path);
                    }
                    ImGui::PopItemWidth();
                    break;
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Screenshots Dir"))
        {
            ImGui::PushItemWidth(220.0f);
            ImGui::Combo("##screenshots_option", &config_emulator.screenshots_dir_option, "Default Location\0Same as ROM\0Custom Location\0\0");

            switch ((Directory_Location)config_emulator.screenshots_dir_option)
            {
                case Directory_Location_Default:
                {
                    ImGui::Text("%s", config_root_path);
                    break;
                }
                case Directory_Location_ROM:
                {
                    if (!emu_is_empty())
                        ImGui::Text("%s", emu_get_core()->GetCartridge()->GetFileDirectory());
                    break;
                }
                case Directory_Location_Custom:
                {
                    if (ImGui::MenuItem("Choose..."))
                    {
                        choose_screenshots_path = true;
                    }

                    ImGui::PushItemWidth(450);
                    if (ImGui::InputText("##screenshots_path", gui_screenshots_path, IM_ARRAYSIZE(gui_screenshots_path), ImGuiInputTextFlags_AutoSelectAll))
                    {
                        config_emulator.screenshots_path.assign(gui_screenshots_path);
                    }
                    ImGui::PopItemWidth();
                    break;
                }
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("BIOS"))
        {
            if (ImGui::BeginMenu("Master System BIOS"))
            {
                if (ImGui::MenuItem("Enable", "", &config_emulator.sms_bootrom))
                {
                    emu_enable_bootrom_sms(config_emulator.sms_bootrom);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("When the BIOS is enabled it will execute as in original hardware,\ncausing invalid roms to lock or preventing some others to boot.\n\nSet 'Media Slot' to 'None' in order to boot the games included in BIOS.");
                if (ImGui::MenuItem("Load BIOS..."))
                {
                    open_sms_bootrom = true;
                }
                ImGui::PushItemWidth(350);
                if (ImGui::InputText("##sms_bootrom_path", gui_sms_bootrom_path, IM_ARRAYSIZE(gui_sms_bootrom_path), ImGuiInputTextFlags_AutoSelectAll))
                {
                    config_emulator.sms_bootrom_path.assign(gui_sms_bootrom_path);
                    emu_load_bootrom_sms(gui_sms_bootrom_path);
                }
                ImGui::PopItemWidth();

                ImGui::Separator();
                if (strlen(gui_sms_bootrom_path) > 0)
                {
                    ImGui::TextColored(ImVec4(0.10f, 0.90f, 0.10f, 1.0f), "SMS Bootrom loaded");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.50f, 0.50f, 0.50f, 1.0f), "No SMS Bootrom loaded");
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("GG Bootrom"))
            {
                if (ImGui::MenuItem("Enable", "", &config_emulator.gg_bootrom))
                {
                    emu_enable_bootrom_gg(config_emulator.gg_bootrom);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("When the BIOS is enabled it will execute as in original hardware,\ncausing invalid roms to lock or preventing some others to boot.\n\nSet 'Media Slot' to 'None' in order to boot the games included in BIOS.");
                if (ImGui::MenuItem("Load BIOS..."))
                {
                    open_gg_bootrom = true;
                }
                ImGui::PushItemWidth(350);
                if (ImGui::InputText("##gg_bootrom_path", gui_gg_bootrom_path, IM_ARRAYSIZE(gui_gg_bootrom_path), ImGuiInputTextFlags_AutoSelectAll))
                {
                    config_emulator.gg_bootrom_path.assign(gui_gg_bootrom_path);
                    emu_load_bootrom_gg(gui_gg_bootrom_path);
                }
                ImGui::PopItemWidth();

                ImGui::Separator();
                if (strlen(gui_gg_bootrom_path) > 0)
                {
                    ImGui::TextColored(ImVec4(0.10f, 0.90f, 0.10f, 1.0f), "GG Bootrom loaded");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.50f, 0.50f, 0.50f, 1.0f), "No GG Bootrom loaded");
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("System"))
        {
            ImGui::PushItemWidth(200.0f);
            ImGui::Combo("##emu_system", &config_emulator.system, "Auto\0Master System / Mark III\0Game Gear\0SG-1000 / Multivision\0\0");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Region"))
        {
            ImGui::PushItemWidth(200.0f);
            ImGui::Combo("##emu_region", &config_emulator.zone, "Auto\0Master System Japan\0Master System Export\0Game Gear Japan\0Game Gear Export\0Game Gear International\0\0");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Mapper"))
        {
            ImGui::PushItemWidth(220.0f);
            ImGui::Combo("##emu_mapper", &config_emulator.mapper,
                "Auto\0"
                "ROM Only\0"
                "SEGA\0"
                "Codemasters\0"
                "Korean\0"
                "SG-1000\0"
                "MSX\0"
                "Janggun\0"
                "Korean Multi 2000 XOR F1\0"
                "Korean Multi MSX 32KB 2000\0"
                "Korean Multi MSX SMS 8000\0"
                "Korean Multi SMS 32KB 2000\0"
                "Korean Multi MSX 8KB 0300\0"
                "Korean 0000 XOR FF\0"
                "Korean FFFF HiCom\0"
                "Korean FFFE\0"
                "Korean BFFC\0"
                "Korean FFF3 FFFC\0"
                "Korean MD FFF5\0"
                "Korean MD FFF0\0"
                "Jumbo Dahjee\0"
                "EEPROM 93C46\0\0");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Refresh Rate"))
        {
            ImGui::PushItemWidth(130.0f);
            if (ImGui::Combo("##emu_rate", &config_emulator.region, "Auto\0NTSC (60 Hz)\0PAL (50 Hz)\0\0"))
            {
                if (config_emulator.region > 0)
                {
                    config_emulator.ffwd = false;
                    config_audio.sync = true;
                }
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Media Slot"))
        {
            ImGui::PushItemWidth(130.0f);
            if (ImGui::Combo("##emu_media", &config_emulator.media, "Cartridge\0Card\0Expansion\0None\0\0"))
            {
                emu_set_media_slot(config_emulator.media);
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::SetNextWindowSizeConstraints({300.0f, 200.0f}, {300.0f, 500.0f});
        if (ImGui::BeginMenu("Cheats"))
        {
            ImGui::Text("Game Genie or Pro Action Replay codes\n(one code per line):");

            ImGui::Columns(2, "cheats", false);

            static char cheat_buffer[20*50] = "";
            ImGui::PushItemWidth(150);
            ImGui::InputTextMultiline("##cheats_input", cheat_buffer, IM_ARRAYSIZE(cheat_buffer));
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            if (ImGui::Button("Add Cheat Code"))
            {
                std::string cheats = cheat_buffer;
                std::istringstream ss(cheats);
                std::string cheat;

                while (getline(ss, cheat))
                {
                    if ((gui_cheat_list.size() < 50) && ((cheat.length() == 7) || (cheat.length() == 11) || (cheat.length() == 8) || (cheat.length() == 9)))
                    {
                        gui_cheat_list.push_back(cheat);
                        emu_add_cheat(cheat.c_str());
                        cheat_buffer[0] = 0;
                    }
                }
            }

            if (gui_cheat_list.size() > 0)
            {
                if (ImGui::Button("Clear All"))
                {
                    gui_cheat_list.clear();
                    emu_clear_cheats();
                }
            }

            ImGui::Columns(1);

            std::list<std::string>::iterator it;

            for (it = gui_cheat_list.begin(); it != gui_cheat_list.end(); it++)
            {
                if ((it->length() == 7) || (it->length() == 11))
                    ImGui::Text("Game Genie: %s", it->c_str());
                else
                    ImGui::Text("Pro Action Replay: %s", it->c_str());
            }

            ImGui::EndMenu();
        }

        ImGui::MenuItem("Show ROM info", "", &config_emulator.show_info);
        ImGui::MenuItem("Status Messages", "", &config_emulator.status_messages);

        ImGui::Separator();

        ImGui::MenuItem("Start Paused", "", &config_emulator.start_paused);
        ImGui::MenuItem("Pause When Inactive", "", &config_emulator.pause_when_inactive);

        ImGui::Separator();

        if (ImGui::BeginMenu("Hotkeys"))
        {
            hotkey_configuration_item("Open ROM:", &config_hotkeys[config_HotkeyIndex_OpenROM]);
            hotkey_configuration_item("Quit:", &config_hotkeys[config_HotkeyIndex_Quit]);
            hotkey_configuration_item("Reset:", &config_hotkeys[config_HotkeyIndex_Reset]);
            hotkey_configuration_item("Pause:", &config_hotkeys[config_HotkeyIndex_Pause]);
            hotkey_configuration_item("Fast Forward:", &config_hotkeys[config_HotkeyIndex_FFWD]);
            hotkey_configuration_item("Save State:", &config_hotkeys[config_HotkeyIndex_SaveState]);
            hotkey_configuration_item("Load State:", &config_hotkeys[config_HotkeyIndex_LoadState]);
            hotkey_configuration_item("Save State Slot 1:", &config_hotkeys[config_HotkeyIndex_SelectSlot1]);
            hotkey_configuration_item("Save State Slot 2:", &config_hotkeys[config_HotkeyIndex_SelectSlot2]);
            hotkey_configuration_item("Save State Slot 3:", &config_hotkeys[config_HotkeyIndex_SelectSlot3]);
            hotkey_configuration_item("Save State Slot 4:", &config_hotkeys[config_HotkeyIndex_SelectSlot4]);
            hotkey_configuration_item("Save State Slot 5:", &config_hotkeys[config_HotkeyIndex_SelectSlot5]);
            hotkey_configuration_item("Screenshot:", &config_hotkeys[config_HotkeyIndex_Screenshot]);
            hotkey_configuration_item("Fullscreen:", &config_hotkeys[config_HotkeyIndex_Fullscreen]);
            hotkey_configuration_item("Capture Mouse:", &config_hotkeys[config_HotkeyIndex_CaptureMouse]);
            hotkey_configuration_item("Show Main Menu:", &config_hotkeys[config_HotkeyIndex_ShowMainMenu]);

            gui_popup_modal_hotkey();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug Hotkeys"))
        {
            hotkey_configuration_item("Reload ROM:", &config_hotkeys[config_HotkeyIndex_ReloadROM]);
            hotkey_configuration_item("Step Into:", &config_hotkeys[config_HotkeyIndex_DebugStepInto]);
            hotkey_configuration_item("Step Over:", &config_hotkeys[config_HotkeyIndex_DebugStepOver]);
            hotkey_configuration_item("Step Out:", &config_hotkeys[config_HotkeyIndex_DebugStepOut]);
            hotkey_configuration_item("Step Frame:", &config_hotkeys[config_HotkeyIndex_DebugStepFrame]);
            hotkey_configuration_item("Continue:", &config_hotkeys[config_HotkeyIndex_DebugContinue]);
            hotkey_configuration_item("Break:", &config_hotkeys[config_HotkeyIndex_DebugBreak]);
            hotkey_configuration_item("Run to Cursor:", &config_hotkeys[config_HotkeyIndex_DebugRunToCursor]);
            hotkey_configuration_item("Toggle Breakpoint:", &config_hotkeys[config_HotkeyIndex_DebugBreakpoint]);
            hotkey_configuration_item("Go Back:", &config_hotkeys[config_HotkeyIndex_DebugGoBack]);

            gui_popup_modal_hotkey();

            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::MenuItem("Single Instance", "", &config_debug.single_instance);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("RESTART REQUIRED");
            ImGui::NewLine();
            ImGui::Text("When enabled, opening a ROM while another instance is running");
            ImGui::Text("will send the ROM to the running instance instead of");
            ImGui::Text("starting a new one.");
            ImGui::EndTooltip();
        }

        ImGui::EndMenu();
    }
}

static void menu_video(void)
{
    if (ImGui::BeginMenu("Video"))
    {
        gui_in_use = true;

        if (ImGui::MenuItem("Full Screen", config_hotkeys[config_HotkeyIndex_Fullscreen].str, &config_emulator.fullscreen))
        {
            application_trigger_fullscreen(config_emulator.fullscreen);
        }

#if !defined(__APPLE__)
        if (ImGui::BeginMenu("Fullscreen Mode"))
        {
            ImGui::PushItemWidth(130.0f);
            ImGui::Combo("##fullscreen_mode", &config_emulator.fullscreen_mode, "Full Screen Desktop\0Full Screen\0\0");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }
#endif

        ImGui::Separator();

        ImGui::MenuItem("Always Show Menu", config_hotkeys[config_HotkeyIndex_ShowMainMenu].str, &config_emulator.always_show_menu);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("This option will enable menu even in fullscreen.");
            ImGui::Text("Menu always shows in debug mode.");
            ImGui::EndTooltip();
        }

        if (ImGui::MenuItem("Resize Window to Content"))
        {
            if (!config_debug.debug)
            {
                application_trigger_fit_to_content(gui_main_window_width, gui_main_window_height + gui_main_menu_height);
            }
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Scale"))
        {
            ImGui::PushItemWidth(250.0f);
            ImGui::Combo("##scale", &config_video.scale, "Integer Scale (Auto)\0Integer Scale (Manual)\0Scale to Window Height\0Scale to Window Width & Height\0\0");
            if (config_video.scale == 1)
                ImGui::SliderInt("##scale_manual", &config_video.scale_manual, 1, 10);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Aspect Ratio"))
        {
            ImGui::PushItemWidth(200.0f);
            ImGui::Combo("##ratio", &config_video.ratio, "Square Pixels (1:1 PAR)\0Standard (4:3 DAR)\0Wide (16:9 DAR)\0Wide (16:10 DAR)\0\0");
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Overscan"))
        {
            ImGui::PushItemWidth(150.0f);
            if (ImGui::Combo("##overscan", &config_video.overscan, "Disabled\0Top+Bottom\0Full (284 width)\0Full (320 width)\0\0"))
            {
                emu_set_overscan(config_debug.debug ? 0 : config_video.overscan);
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Hide Left Bar"))
        {
            ImGui::PushItemWidth(80.0f);
            if (ImGui::Combo("##hide_left_bar", &config_video.hide_left_bar, "No\0Auto\0Always\0\0"))
            {
                emu_set_hide_left_bar(config_debug.debug ? 0 : config_video.hide_left_bar);
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Vertical Sync", "", &config_video.sync))
        {
            display_set_vsync(config_video.sync);

            if (config_video.sync)
            {
                config_audio.sync = true;
                config_emulator.ffwd = false;
                emu_audio_reset();
            }
        }

        ImGui::MenuItem("Show FPS", "", &config_video.fps);

        ImGui::Separator();

        ImGui::MenuItem("Bilinear Filtering", "", &config_video.bilinear);

        if (ImGui::BeginMenu("Screen Ghosting"))
        {
            ImGui::MenuItem("Enable Screen Ghosting", "", &config_video.mix_frames);
            ImGui::SliderFloat("##screen_ghosting", &config_video.mix_frames_intensity, 0.0f, 1.0f, "Intensity = %.2f");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scanlines"))
        {
            ImGui::MenuItem("Enable Scanlines", "", &config_video.scanlines);
            ImGui::MenuItem("Enable Scanlines Filter", "", &config_video.scanlines_filter);
            ImGui::SliderFloat("##scanlines", &config_video.scanlines_intensity, 0.0f, 1.0f, "Intensity = %.2f");
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("3D Glasses"))
        {
            ImGui::PushItemWidth(160.0f);
            if (ImGui::Combo("##glasses", &config_video.glasses, "Both Eyes / OFF\0Only Left Eye\0Only Right Eye\0\0"))
            {
                emu_set_3d_glasses_config(config_video.glasses);
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Background Color"))
        {
            ImGui::ColorEdit3("##normal_bg", config_video.background_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
            ImGui::SameLine();
            ImGui::Text("Normal Background");

            ImGui::Separator();

            if (ImGui::ColorEdit3("##debugger_bg", config_video.background_color_debugger, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float))
            {
                ImGuiStyle& style = ImGui::GetStyle();
                style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(config_video.background_color_debugger[0], config_video.background_color_debugger[1], config_video.background_color_debugger[2], 1.0f);
            }
            ImGui::SameLine();
            ImGui::Text("Debugger Background");

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

static void menu_input(void)
{
    if (ImGui::BeginMenu("Input"))
    {
        gui_in_use = true;

        if (ImGui::BeginMenu("Keyboard Configuration"))
        {
            if (ImGui::BeginMenu("Player 1"))
            {
                ImGui::TextDisabled("Keyboard Player 1");
                ImGui::Separator();
                keyboard_configuration_item("Left:", &config_input[0].key_left, 0);
                keyboard_configuration_item("Right:", &config_input[0].key_right, 0);
                keyboard_configuration_item("Up:", &config_input[0].key_up, 0);
                keyboard_configuration_item("Down:", &config_input[0].key_down, 0);
                keyboard_configuration_item("1:", &config_input[0].key_1, 0);
                keyboard_configuration_item("2:", &config_input[0].key_2, 0);
                keyboard_configuration_item("Start:", &config_input[0].key_start, 0);

                gui_popup_modal_keyboard();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Player 2"))
            {
                ImGui::TextDisabled("Keyboard Player 2");
                ImGui::Separator();
                keyboard_configuration_item("Left:", &config_input[1].key_left, 1);
                keyboard_configuration_item("Right:", &config_input[1].key_right, 1);
                keyboard_configuration_item("Up:", &config_input[1].key_up, 1);
                keyboard_configuration_item("Down:", &config_input[1].key_down, 1);
                keyboard_configuration_item("1:", &config_input[1].key_1, 1);
                keyboard_configuration_item("2:", &config_input[1].key_2, 1);
                keyboard_configuration_item("Start:", &config_input[1].key_start, 1);

                gui_popup_modal_keyboard();

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

                    ImGui::Separator();

        if (ImGui::BeginMenu("Gamepads"))
        {
            if (ImGui::BeginMenu("Player 1"))
            {
                ImGui::MenuItem("Enable Gamepad P1", "", &config_input[0].gamepad);

                if (ImGui::BeginMenu("Device"))
                {
                    gamepad_device_selector(0);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Directional Controls"))
                {
                    ImGui::PushItemWidth(150.0f);
                    ImGui::Combo("##directional", &config_input[0].gamepad_directional, "D-pad\0Left Analog Stick\0\0");
                    ImGui::PopItemWidth();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Button Configuration"))
                {
                    ImGui::TextDisabled("Gamepad Player 1");
                    ImGui::Separator();
                    gamepad_configuration_item("1:", &config_input[0].gamepad_1, 0);
                    gamepad_configuration_item("2:", &config_input[0].gamepad_2, 0);
                    gamepad_configuration_item("START:", &config_input[0].gamepad_start, 0);

                    gui_popup_modal_gamepad(0);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Shortcut Configuration"))
                {
                    ImGui::TextDisabled("Gamepad Player 1 - Shortcuts");
                    ImGui::Separator();

                    gamepad_configuration_item("Save State:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_SaveState], 0);
                    gamepad_configuration_item("Load State:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_LoadState], 0);
                    gamepad_configuration_item("Save State Slot 1:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_SelectSlot1], 0);
                    gamepad_configuration_item("Save State Slot 2:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_SelectSlot2], 0);
                    gamepad_configuration_item("Save State Slot 3:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_SelectSlot3], 0);
                    gamepad_configuration_item("Save State Slot 4:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_SelectSlot4], 0);
                    gamepad_configuration_item("Save State Slot 5:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_SelectSlot5], 0);

                    ImGui::Separator();

                    gamepad_configuration_item("Reset:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_Reset], 0);
                    gamepad_configuration_item("Pause:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_Pause], 0);
                    gamepad_configuration_item("Fast Forward:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_FFWD], 0);
                    gamepad_configuration_item("Screenshot:", &config_input_gamepad_shortcuts[0].gamepad_shortcuts[config_HotkeyIndex_Screenshot], 0);

                    gui_popup_modal_gamepad(0);

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Player 2"))
            {
                ImGui::MenuItem("Enable Gamepad P2", "", &config_input[1].gamepad);

                if (ImGui::BeginMenu("Device"))
                {
                    gamepad_device_selector(1);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Directional Controls"))
                {
                    ImGui::PushItemWidth(150.0f);
                    ImGui::Combo("##directional", &config_input[1].gamepad_directional, "D-pad\0Left Analog Stick\0\0");
                    ImGui::PopItemWidth();
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Button Configuration"))
                {
                    ImGui::TextDisabled("Gamepad Player 2");
                    ImGui::Separator();
                    gamepad_configuration_item("1:", &config_input[1].gamepad_1, 1);
                    gamepad_configuration_item("2:", &config_input[1].gamepad_2, 1);
                    gamepad_configuration_item("START:", &config_input[1].gamepad_start, 1);

                    gui_popup_modal_gamepad(1);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Shortcut Configuration"))
                {
                    ImGui::TextDisabled("Gamepad Player 2 - Shortcuts");
                    ImGui::Separator();

                    gamepad_configuration_item("Save State:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_SaveState], 1);
                    gamepad_configuration_item("Load State:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_LoadState], 1);
                    gamepad_configuration_item("Save State Slot 1:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_SelectSlot1], 1);
                    gamepad_configuration_item("Save State Slot 2:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_SelectSlot2], 1);
                    gamepad_configuration_item("Save State Slot 3:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_SelectSlot3], 1);
                    gamepad_configuration_item("Save State Slot 4:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_SelectSlot4], 1);
                    gamepad_configuration_item("Save State Slot 5:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_SelectSlot5], 1);

                    ImGui::Separator();

                    gamepad_configuration_item("Reset:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_Reset], 1);
                    gamepad_configuration_item("Pause:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_Pause], 1);
                    gamepad_configuration_item("Fast Forward:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_FFWD], 1);
                    gamepad_configuration_item("Screenshot:", &config_input_gamepad_shortcuts[1].gamepad_shortcuts[config_HotkeyIndex_Screenshot], 1);

                    gui_popup_modal_gamepad(1);

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Light Phaser"))
        {
            if (ImGui::MenuItem("Enable Light Phaser", "", &config_emulator.light_phaser))
            {
                emu_enable_phaser(config_emulator.light_phaser);

                if (config_emulator.light_phaser && config_emulator.paddle_control)
                {
                    config_emulator.paddle_control = false;
                    emu_enable_paddle(false);
                }
            }

            if (ImGui::MenuItem("Enable Crosshair", "", &config_emulator.light_phaser_crosshair))
            {
                emu_enable_phaser_crosshair(config_emulator.light_phaser_crosshair, config_emulator.light_phaser_crosshair_shape, config_emulator.light_phaser_crosshair_color);
            }

            if (ImGui::BeginMenu("Crosshair Shape"))
            {
                ImGui::PushItemWidth(100.0f);
                if (ImGui::Combo("##crosshair_shape", &config_emulator.light_phaser_crosshair_shape, "Cross\0Square\0\0"))
                {
                    emu_enable_phaser_crosshair(config_emulator.light_phaser_crosshair, config_emulator.light_phaser_crosshair_shape, config_emulator.light_phaser_crosshair_color);
                }
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Crosshair Color"))
            {
                ImGui::PushItemWidth(100.0f);
                if (ImGui::Combo("##crosshair_color", &config_emulator.light_phaser_crosshair_color, "White\0Black\0Red\0Green\0Blue\0Yellow\0Magenta\0Cyan\0\0"))
                {
                    emu_enable_phaser_crosshair(config_emulator.light_phaser_crosshair, config_emulator.light_phaser_crosshair_shape, config_emulator.light_phaser_crosshair_color);
                }
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Crosshair Calibration"))
            {
                ImGui::SliderInt("##crosshair_offset_x", &config_emulator.light_phaser_x_offset, -10, 10, "X = %d");
                ImGui::SliderInt("##crosshair_offset_y", &config_emulator.light_phaser_y_offset, -10, 10, "Y = %d");
                emu_set_phaser_offset(config_emulator.light_phaser_x_offset, config_emulator.light_phaser_y_offset);
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Paddle Control"))
        {
            if (ImGui::MenuItem("Enable Paddle Control", "", &config_emulator.paddle_control))
            {
                emu_enable_paddle(config_emulator.paddle_control);

                if (config_emulator.paddle_control && config_emulator.light_phaser)
                {
                    config_emulator.light_phaser = false;
                    emu_enable_phaser(false);
                }
            }

            ImGui::MenuItem("Capture Mouse", config_hotkeys[config_HotkeyIndex_CaptureMouse].str, &config_emulator.capture_mouse);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("When enabled, the mouse will be captured inside\nthe emulator window to use the paddle freely.\nPress %s to release the mouse.", config_hotkeys[config_HotkeyIndex_CaptureMouse].str);
            }

            ImGui::SliderInt("##paddle_sensitivity", &config_emulator.paddle_sensitivity, 1, 15, "Sensitivity = %d");

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

static void menu_audio(void)
{
    if (ImGui::BeginMenu("Audio"))
    {
        gui_in_use = true;

        if (ImGui::MenuItem("Enable Audio", "", &config_audio.enable))
        {
            emu_audio_mute(!config_audio.enable);
        }

        // if (ImGui::MenuItem("Sync With Emulator", "", &config_audio.sync))
        // {
        //     config_emulator.ffwd = false;

        //     if (!config_audio.sync)
        //     {
        //         config_video.sync = false;
        //         display_set_vsync(false);
        //     }
        // }

        if (ImGui::BeginMenu("YM2413 FM Sound"))
        {
            ImGui::PushItemWidth(130.0f);
            if (ImGui::Combo("##emu_ym2413", &config_audio.ym2413, "Auto\0Disabled\0\0"))
            {
                emu_disable_ym2413(config_audio.ym2413 == 1);
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Buffer Size", config_audio.enable))
        {
            ImGui::PushItemWidth(150.0f);
            if (ImGui::SliderInt("##buffer_count", &config_audio.buffer_count, 2, 5, "Buffers = %d"))
            {
                emu_audio_reset();
            }
            ImGui::PopItemWidth();
            if (ImGui::IsItemHovered())
            {
                float latency_ms = (config_audio.buffer_count * GS_AUDIO_QUEUE_SIZE) / (float)(GS_AUDIO_SAMPLE_RATE * 2) * 1000.0f;
                ImGui::BeginTooltip();
                ImGui::Text("Lower values reduce audio latency.");
                ImGui::Text("Higher values prevent audio underruns.");
                ImGui::Text("Enabling VSync may force higher buffer counts.");
                ImGui::Text("Current audio latency: %.0f ms", latency_ms);
                ImGui::EndTooltip();
            }
            ImGui::EndMenu();
        }

#ifndef GS_DISABLE_VGMRECORDER
        ImGui::Separator();

        bool is_recording = emu_is_vgm_recording();

        if (ImGui::MenuItem("Start VGM Recording...", "", false, !is_recording && !emu_is_empty()))
        {
            save_vgm = true;
        }

        if (ImGui::MenuItem("Stop VGM Recording", "", false, is_recording))
        {
            emu_stop_vgm_recording();
            gui_set_status_message("VGM recording stopped", 3000);
        }
#endif

        ImGui::EndMenu();
    }
}

static void menu_debug(void)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    if (ImGui::BeginMenu("Debug"))
    {
        gui_in_use = true;

        if (ImGui::MenuItem("Enable", "", &config_debug.debug))
        {
            emu_set_overscan(config_debug.debug ? 0 : config_video.overscan);
            emu_set_hide_left_bar(config_debug.debug ? 0 : config_video.hide_left_bar);

            if (config_debug.debug)
                emu_debug_step_over();
            else
                emu_debug_continue();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Debug Settings...", "", false, config_debug.debug))
        {
            save_debug_settings = true;
        }

        if (ImGui::MenuItem("Load Debug Settings...", "", false, config_debug.debug))
        {
            load_debug_settings = true;
        }

        ImGui::MenuItem("Auto Save/Load Debug Settings", "", &config_debug.auto_debug_settings, config_debug.debug);

        ImGui::Separator();

        if (ImGui::MenuItem("Reload ROM", config_hotkeys[config_HotkeyIndex_ReloadROM].str, false, config_debug.debug && !emu_is_empty()))
        {
            gui_action_reload_rom();
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("MCP Server", config_debug.debug))
        {
            bool mcp_running = emu_mcp_is_running();
            int transport_mode = emu_mcp_get_transport_mode();
            bool http_running = mcp_running && (transport_mode == 1);
            bool stdio_running = mcp_running && (transport_mode == 0);

            if (ImGui::MenuItem("Start HTTP Server", "", false, !mcp_running))
            {
                emu_mcp_set_transport(1, config_emulator.mcp_tcp_port);
                emu_mcp_start();
            }

            if (ImGui::MenuItem("Stop HTTP Server", "", false, http_running))
            {
                emu_mcp_stop();
            }

            ImGui::Separator();

            if (stdio_running)
                ImGui::TextColored(ImVec4(0.90f, 0.70f, 0.10f, 1.0f), "STDIO mode active");
            else if (http_running)
                ImGui::TextColored(ImVec4(0.10f, 0.90f, 0.10f, 1.0f), "Listening on %d", config_emulator.mcp_tcp_port);
            else
                ImGui::TextColored(ImVec4(0.98f, 0.15f, 0.45f, 1.0f), "Stopped");

            ImGui::Separator();

            ImGui::Text("HTTP Port:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50);
            if (ImGui::InputInt("##mcp_port", &config_emulator.mcp_tcp_port, 0, 0))
            {
                if (config_emulator.mcp_tcp_port < 1)
                    config_emulator.mcp_tcp_port = 1;
                if (config_emulator.mcp_tcp_port > 65535)
                    config_emulator.mcp_tcp_port = 65535;
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Step Over", config_hotkeys[config_HotkeyIndex_DebugStepOver].str, (void*)0, config_debug.debug))
        {
            emu_debug_step_over();
        }

        if (ImGui::MenuItem("Step Frame", config_hotkeys[config_HotkeyIndex_DebugStepFrame].str, (void*)0, config_debug.debug))
        {
            emu_debug_step_frame();
            gui_debug_memory_step_frame();
        }

        if (ImGui::MenuItem("Continue", config_hotkeys[config_HotkeyIndex_DebugContinue].str, (void*)0, config_debug.debug))
        {
            emu_debug_continue();
        }

        if (ImGui::MenuItem("Run To Cursor", config_hotkeys[config_HotkeyIndex_DebugRunToCursor].str, (void*)0, config_debug.debug))
        {
            gui_debug_runtocursor();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Go Back", config_hotkeys[config_HotkeyIndex_DebugGoBack].str, (void*)0, config_debug.debug))
        {
            gui_debug_go_back();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Toggle Breakpoint", config_hotkeys[config_HotkeyIndex_DebugBreakpoint].str, (void*)0, config_debug.debug))
        {
            gui_debug_toggle_breakpoint();
        }

        if (ImGui::MenuItem("Clear All Breakpoints", 0, (void*)0, config_debug.debug))
        {
            gui_debug_reset_breakpoints();
        }

        ImGui::MenuItem("Disable All Breakpoints", 0, &emu_debug_disable_breakpoints, config_debug.debug);

        ImGui::Separator();

        ImGui::MenuItem("Show Output Screen", "", &config_debug.show_screen, config_debug.debug);

        if (ImGui::BeginMenu("Output Scale", config_debug.debug))
        {
            ImGui::PushItemWidth(200.0f);
            ImGui::SliderInt("##debug_scale", &config_debug.scale, 1, 10);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::MenuItem("Show Disassembler", "", &config_debug.show_disassembler, config_debug.debug);

        ImGui::MenuItem("Show Z80 Status", "", &config_debug.show_processor, config_debug.debug);

        ImGui::MenuItem("Show Memory Editor", "", &config_debug.show_memory, config_debug.debug);

        ImGui::MenuItem("Show VRAM Viewer", "", &config_debug.show_video, config_debug.debug);

        ImGui::MenuItem("Show PSG", "", &config_debug.show_psg, config_debug.debug);

        ImGui::MenuItem("Show Call Stack", "", &config_debug.show_call_stack, config_debug.debug);

        ImGui::MenuItem("Show Breakpoints", "", &config_debug.show_breakpoints, config_debug.debug);

        ImGui::MenuItem("Show Symbols", "", &config_debug.show_symbols, config_debug.debug);

        ImGui::MenuItem("Show Trace Logger", "", &config_debug.show_trace_logger, config_debug.debug);


#if defined(__APPLE__) || defined(_WIN32)
        ImGui::Separator();
        ImGui::MenuItem("Multi-Viewport", "", &config_debug.multi_viewport, config_debug.debug);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("RESTART REQUIRED");
            ImGui::NewLine();
            ImGui::Text("Enables docking of debug windows outside of main window.");
            ImGui::EndTooltip();
        }
#endif

        ImGui::Separator();

        if (ImGui::BeginMenu("Font Size", config_debug.debug))
        {
            ImGui::PushItemWidth(110.0f);
            if (ImGui::Combo("##font", &config_debug.font_size, "Very Small\0Small\0Medium\0Large\0\0"))
            {
                gui_default_font = gui_default_fonts[config_debug.font_size];
            }
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
#endif
}

static void menu_about(void)
{
    if (ImGui::BeginMenu("About"))
    {
        gui_in_use = true;

        if (ImGui::MenuItem("About " GS_TITLE " " GS_VERSION " ..."))
        {
            open_about = true;
        }
        ImGui::EndMenu();
    }
}

static void file_dialogs(void)
{
    gui_file_dialog_process_results();

    if (open_rom || gui_shortcut_open_rom)
    {
        gui_shortcut_open_rom = false;
        gui_file_dialog_open_rom();
    }
    if (open_ram)
        gui_file_dialog_load_ram();
    if (save_ram)
        gui_file_dialog_save_ram();
    if (open_state)
        gui_file_dialog_load_state();
    if (save_state)
        gui_file_dialog_save_state();
    if (save_screenshot)
        gui_file_dialog_save_screenshot();
    if (save_vgm)
        gui_file_dialog_save_vgm();
    if (choose_savestates_path)
        gui_file_dialog_choose_savestate_path();
    if (choose_screenshots_path)
        gui_file_dialog_choose_screenshot_path();
    if (choose_backup_ram_path)
        gui_file_dialog_choose_saves_path();
    if (open_sms_bootrom)
        gui_file_dialog_load_sms_bootrom();
    if (open_gg_bootrom)
        gui_file_dialog_load_gg_bootrom();
    if (save_debug_settings)
        gui_file_dialog_save_debug_settings();
    if (load_debug_settings)
        gui_file_dialog_load_debug_settings();
    if (open_about)
    {
        gui_dialog_in_use = true;
        ImGui::OpenPopup("About " GS_TITLE);
    }

    if (open_load_defaults)
    {
        gui_dialog_in_use = true;
        ImGui::OpenPopup("Load Default Settings");
    }

    gui_popup_modal_about();
    gui_popup_modal_load_defaults();
}

static void keyboard_configuration_item(const char* text, SDL_Scancode* key, int player)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(120);

    char button_label[256];
    snprintf(button_label, 256, "%s##%s%d", SDL_GetKeyName(SDL_GetKeyFromScancode(*key, SDL_KMOD_NONE, false)), text, player);

    if (ImGui::Button(button_label, ImVec2(90,0)))
    {
        gui_configured_key = key;
        ImGui::OpenPopup("Keyboard Configuration");
    }

    ImGui::SameLine();

    char remove_label[256];
    snprintf(remove_label, sizeof(remove_label), "X##rk%s%d", text, player);

    if (ImGui::Button(remove_label))
    {
        *key = SDL_SCANCODE_UNKNOWN;
    }
}

static void gamepad_configuration_item(const char* text, int* button, int player)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(130);

    const char* button_name = "";

    if (*button == SDL_GAMEPAD_BUTTON_INVALID)
    {
        button_name = "";
    }
    else if (*button >= 0 && *button < SDL_GAMEPAD_BUTTON_COUNT)
    {
        static const char* gamepad_names[21] = {"A", "B", "X" ,"Y", "BACK", "GUIDE", "START", "L3", "R3", "L1", "R1", "UP", "DOWN", "LEFT", "RIGHT", "MISC", "PAD1", "PAD2", "PAD3", "PAD4", "TOUCH"};
        button_name = gamepad_names[*button];
    }
    else if (*button >= GAMEPAD_VBTN_AXIS_BASE)
    {
        int axis = *button - GAMEPAD_VBTN_AXIS_BASE;
        if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER)
            button_name = "L2";
        else if (axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)
            button_name = "R2";
        else
            button_name = "??";
    }

    char button_label[256];
    snprintf(button_label, sizeof(button_label), "%s##%s%d", button_name, text, player);

    if (ImGui::Button(button_label, ImVec2(70,0)))
    {
        gui_configured_button = button;
        ImGui::OpenPopup("Gamepad Configuration");
    }

    ImGui::SameLine();

    char remove_label[256];
    snprintf(remove_label, sizeof(remove_label), "X##rg%s%d", text, player);

    if (ImGui::Button(remove_label))
    {
        *button = SDL_GAMEPAD_BUTTON_INVALID;
    }
}

static void hotkey_configuration_item(const char* text, config_Hotkey* hotkey)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(150);

    char button_label[256];
    snprintf(button_label, sizeof(button_label), "%s##%s", hotkey->str[0] != '\0' ? hotkey->str : "<None>", text);

    if (ImGui::Button(button_label, ImVec2(150,0)))
    {
        gui_configured_hotkey = hotkey;
        ImGui::OpenPopup("Hotkey Configuration");
    }

    ImGui::SameLine();

    char remove_label[256];
    snprintf(remove_label, sizeof(remove_label), "X##rh%s", text);

    if (ImGui::Button(remove_label))
    {
        hotkey->key = SDL_SCANCODE_UNKNOWN;
        hotkey->mod = SDL_KMOD_NONE;
        config_update_hotkey_string(hotkey);
    }
}

static void gamepad_device_selector(int player)
{
    if (player < 0 || player >= GS_MAX_GAMEPADS)
        return;

    const int max_detected_gamepads = 32;
    SDL_JoystickID id_map[max_detected_gamepads];
    id_map[0] = 0;
    int count = 1;

    std::string items;
    items.reserve(4096);
    items.append("<None>");
    items.push_back('\0');

    Gamepad_Detected_Info detected[max_detected_gamepads];
    int num_detected = gamepad_get_detected(detected, max_detected_gamepads);

    SDL_JoystickID current_id = 0;
    if (IsValidPointer(gamepad_controller[player]))
        current_id = SDL_GetJoystickID(SDL_GetGamepadJoystick(gamepad_controller[player]));

    int selected = 0;

    for (int i = 0; i < num_detected && count < max_detected_gamepads; i++)
    {
        const char* name = detected[i].name;
        if (!IsValidPointer(name))
            name = "Unknown Gamepad";

        id_map[count] = detected[i].id;

        if (current_id == detected[i].id)
            selected = count;

        size_t len = strlen(detected[i].guid_str);
        const char* id_8 = detected[i].guid_str + (len > 8 ? len - 8 : 0);

        char label[192];
        snprintf(label, sizeof(label), "%s (ID: %s)", name, id_8);

        items.append(label);
        items.push_back('\0');
        count++;
    }

    items.push_back('\0');

    char label[32];
    snprintf(label, sizeof(label), "##device_player%d", player + 1);

    if (ImGui::Combo(label, &selected, items.c_str()))
    {
        SDL_JoystickID instance_id = id_map[selected];
        gamepad_assign(player, instance_id);
    }
}

static void draw_savestate_slot_info(int slot)
{
    if (emu_savestates[slot].rom_name[0] != 0)
    {
        if (emu_savestates[slot].version != GS_SAVESTATE_VERSION)
        {
            if (emu_savestates[slot].version == GS_SAVESTATE_VERSION_V1)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.80f, 0.0f, 1.0f), "This save state is from an older version");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.98f, 0.15f, 0.45f, 1.0f), "This save state is from an older version and will not work");
                if (emu_savestates[slot].emu_build[0] != 0)
                    ImGui::TextColored(ImVec4(0.98f, 0.15f, 0.45f, 1.0f), "Use %s - %s", GS_TITLE, emu_savestates[slot].emu_build);
            }
            ImGui::Separator();
        }

        ImGui::Text("%s", emu_savestates[slot].rom_name);
        char date[64];
        get_date_time_string(emu_savestates[slot].timestamp, date, sizeof(date));
        ImGui::Text("%s", date);

        if (IsValidPointer(emu_savestates_screenshots[slot].data))
        {
            float width = (float)emu_savestates_screenshots[slot].width;
            float height = (float)emu_savestates_screenshots[slot].height;
            ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_savestates, ImVec2((height / 3.0f) * 4.0f, height), ImVec2(0, 0), ImVec2(width / 2048.0f, height / 256.0f));
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.50f, 0.50f, 0.50f, 1.0f), "Slot %d is empty", slot + 1);
    }
}
