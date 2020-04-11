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
#include "imgui/fonts/RobotoMedium.h"
#include "FileBrowser/ImGuiFileBrowser.h"
#include "config.h"
#include "emu.h"
#include "../../src/gearsystem.h"
#include "renderer.h"
#include "application.h"

#define GUI_IMPORT
#include "gui.h"

static imgui_addons::ImGuiFileBrowser file_dialog;
static int main_menu_height;
static int main_window_width;
static int main_window_height;
static bool dialog_in_use = false;
static SDL_Scancode* configured_key;
static int* configured_button;
static ImVec4 custom_palette[4];
static std::list<std::string> cheat_list;
static bool shortcut_open_rom = false;

static void main_menu(void);
static void main_window(void);
static void file_dialog_open_rom(void);
static void file_dialog_load_ram(void);
static void file_dialog_save_ram(void);
static void file_dialog_load_state(void);
static void file_dialog_save_state(void);
static void keyboard_configuration_item(const char* text, SDL_Scancode* key, int player);
static void gamepad_configuration_item(const char* text, int* button, int player);
static void popup_modal_keyboard();
static void popup_modal_gamepad(int pad);
static void popup_modal_about(void);
static void load_rom(const char* path);
static void push_recent_rom(std::string path);
static void menu_reset(void);
static void menu_pause(void);
static void menu_ffwd(void);
static void show_info(void);
static void show_fps(void);
static Cartridge::CartridgeTypes get_mapper(int index);
static Cartridge::CartridgeZones get_zone(int index);
static Cartridge::CartridgeSystem get_system(int index);
static Cartridge::CartridgeRegions get_region(int index);

void gui_init(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = config_imgui_file_path;

    float font_scaling_factor = application_display_scale;
    float font_size = 17.0f;

    io.FontGlobalScale /= font_scaling_factor;

    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, font_size * font_scaling_factor, NULL, io.Fonts->GetGlyphRangesCyrillic());

    emu_audio_volume(config_audio.enable ? 1.0f: 0.0f);
}

void gui_destroy(void)
{
    ImGui::DestroyContext();
}

void gui_render(void)
{
    ImGui::NewFrame();

    gui_in_use = dialog_in_use;

    if (!emu_is_empty())
        main_window();
    
    main_menu();

    ImGui::Render();
}

void gui_shortcut(gui_ShortCutEvent event)
{
    switch (event)
    {  
    case gui_ShortcutOpenROM:
        shortcut_open_rom = true;
        break;
    case gui_ShortcutReset:
        menu_reset();
        break;
    case gui_ShortcutPause:
        menu_pause();
        break;
    case gui_ShortcutFFWD:
        config_emulator.ffwd = !config_emulator.ffwd;
        menu_ffwd();
        break;
    case gui_ShortcutSaveState:
        emu_save_state_slot(config_emulator.save_slot + 1);
        break;
    case gui_ShortcutLoadState:
        emu_load_state_slot(config_emulator.save_slot + 1);
        break;
    default:
        break;
    }
}

static void main_menu(void)
{
    bool open_rom = false;
    bool open_ram = false;
    bool save_ram = false;
    bool open_state = false;
    bool save_state = false;
    bool open_about = false;
    
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(GEARSYSTEM_TITLE))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Open ROM...", "Ctrl+O"))
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
                            load_rom(config_emulator.recent_roms[i].c_str());
                        }
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();
            
            if (ImGui::MenuItem("Reset", "Ctrl+R"))
            {
                menu_reset();
            }

            if (ImGui::MenuItem("Pause", "Ctrl+P", &config_emulator.paused))
            {
                menu_pause();
            }

            if (ImGui::MenuItem("Fast Forward", "Ctrl+F", &config_emulator.ffwd))
            {
                menu_ffwd();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save RAM As...")) 
            {
                save_ram = true;
            }

            if (ImGui::MenuItem("Load RAM From..."))
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
           
            if (ImGui::BeginMenu("Select State Slot"))
            {
                ImGui::Combo("", &config_emulator.save_slot, "Slot 1\0Slot 2\0Slot 3\0Slot 4\0Slot 5\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Save State", "Ctrl+S")) 
            {
                emu_save_state_slot(config_emulator.save_slot + 1);
            }

            if (ImGui::MenuItem("Load State", "Ctrl+L"))
            {
                emu_load_state_slot(config_emulator.save_slot + 1);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit", "ESC"))
            {
                application_trigger_quit();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulator"))
        {
            gui_in_use = true;

            if (ImGui::BeginMenu("System"))
            {
                ImGui::Combo("", &config_emulator.system, "Auto\0Master System / Mark III\0Game Gear\0SG-1000 / Multivision\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Region"))
            {
                ImGui::Combo("", &config_emulator.zone, "Auto\0Master System Japan\0Master System Export\0Game Gear Japan\0Game Gear Export\0Game Gear International\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Mapper"))
            {
                ImGui::Combo("", &config_emulator.mapper, "Auto\0ROM Only\0SEGA\0Codemasters\0Korean\0SG-1000\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Refresh Rate"))
            {
                if (ImGui::Combo("", &config_emulator.region, "Auto\0NTSC (60 Hz)\0PAL (50 Hz)\0\0"))
                {
                    if (config_emulator.region > 0)
                    {
                        config_emulator.ffwd = false;
                        config_audio.sync = true;
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            ImGui::MenuItem("Start Paused", "", &config_emulator.start_paused);
            
            ImGui::MenuItem("Save Files In ROM Folder", "", &config_emulator.save_in_rom_folder);
            
            ImGui::Separator();

            ImGui::MenuItem("Show ROM Info", "", &config_emulator.show_info);

            ImGui::Separator();
            
            if (ImGui::BeginMenu("Cheats"))
            {
                ImGui::Text("Game Genie or Pro Action Replay codes:");

                static char cheat_buffer[12] = "";
                ImGui::PushItemWidth(150);
                ImGui::InputText("", cheat_buffer, 12);
                ImGui::PopItemWidth();
                ImGui::SameLine();

                if (ImGui::Button("Add Cheat Code"))
                {
                    std::string cheat = cheat_buffer;

                    if ((cheat_list.size() < 10) && ((cheat.length() == 7) || (cheat.length() == 8) || (cheat.length() == 11)))
                    {
                        cheat_list.push_back(cheat_buffer);
                        emu_add_cheat(cheat_buffer);
                        cheat_buffer[0] = 0;
                    }
                }

                std::list<std::string>::iterator it;

                for (it = cheat_list.begin(); it != cheat_list.end(); it++)
                {
                    if ((it->length() == 7) || (it->length() == 11))
                        ImGui::Text("Game Genie: %s", it->c_str());
                    else
                        ImGui::Text("Pro Action Replay: %s", it->c_str());
                }

                if (cheat_list.size() > 0)
                {
                    if (ImGui::Button("Clear All"))
                    {
                        cheat_list.clear();
                        emu_clear_cheats();
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Video"))
        {
            gui_in_use = true;

            if (ImGui::BeginMenu("Scale"))
            {
                ImGui::Combo("", &config_video.scale, "Auto\0Zoom X1\0Zoom X2\0Zoom X3\0\0");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Aspect Ratio"))
            {
                ImGui::Combo("", &config_video.ratio, "Square Pixels\0Standard (4:3)\0Wide (16:9)\0Fit Window\0\0");
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Vertical Sync", "", &config_video.sync))
            {
                SDL_GL_SetSwapInterval(config_video.sync ? 1 : 0);

                if (config_video.sync)
                {
                    config_audio.sync = true;
                    emu_audio_reset();
                }
            }

            ImGui::MenuItem("Show FPS", "", &config_video.fps);
            ImGui::MenuItem("Bilinear Filtering", "", &config_video.bilinear);
            ImGui::MenuItem("Screen Ghosting", "", &config_video.mix_frames);
            ImGui::MenuItem("Scanlines", "", &config_video.scanlines);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Input"))
        {
            gui_in_use = true;

            if (ImGui::BeginMenu("Keyboard Configuration"))
            {
                if (ImGui::BeginMenu("Player 1"))
                {
                    keyboard_configuration_item("Left:", &config_input[0].key_left, 0);
                    keyboard_configuration_item("Right:", &config_input[0].key_right, 0);
                    keyboard_configuration_item("Up:", &config_input[0].key_up, 0);
                    keyboard_configuration_item("Down:", &config_input[0].key_down, 0);
                    keyboard_configuration_item("1:", &config_input[0].key_1, 0);
                    keyboard_configuration_item("2:", &config_input[0].key_2, 0);
                    keyboard_configuration_item("Start:", &config_input[0].key_start, 0);

                    popup_modal_keyboard();

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Player 2"))
                {
                    keyboard_configuration_item("Left:", &config_input[1].key_left, 1);
                    keyboard_configuration_item("Right:", &config_input[1].key_right, 1);
                    keyboard_configuration_item("Up:", &config_input[1].key_up, 1);
                    keyboard_configuration_item("Down:", &config_input[1].key_down, 1);
                    keyboard_configuration_item("1:", &config_input[1].key_1, 1);
                    keyboard_configuration_item("2:", &config_input[1].key_2, 1);
                    keyboard_configuration_item("Start:", &config_input[1].key_start, 1);

                    popup_modal_keyboard();

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            ImGui::MenuItem("Enable Gamepad P1", "", &config_input[0].gamepad);
            ImGui::MenuItem("Enable Gamepad P2", "", &config_input[1].gamepad);
            
            if (ImGui::BeginMenu("Gamepad Configuration"))
            {
                if (ImGui::BeginMenu("Player 1"))
                {
                    gamepad_configuration_item("1:", &config_input[0].gamepad_1, 0);
                    gamepad_configuration_item("2:", &config_input[0].gamepad_2, 0);
                    gamepad_configuration_item("START:", &config_input[0].gamepad_start, 0);

                    popup_modal_gamepad(0);                 

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Player 2"))
                {
                    gamepad_configuration_item("1:", &config_input[1].gamepad_1, 1);
                    gamepad_configuration_item("2:", &config_input[1].gamepad_2, 1);
                    gamepad_configuration_item("START:", &config_input[1].gamepad_start, 1);

                    popup_modal_gamepad(1);                 

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("Enable", "", &config_audio.enable))
            {
                emu_audio_volume(config_audio.enable ? 1.0f: 0.0f);
            }

            if (ImGui::MenuItem("Sync With Emulator", "", &config_audio.sync))
            {
                config_emulator.ffwd = false;

                if (!config_audio.sync)
                {
                    config_video.sync = false;
                    SDL_GL_SetSwapInterval(0);

                }
            }

            ImGui::EndMenu();
        }

        // if (ImGui::BeginMenu("Debug"))
        // {
        //     gui_in_use = true;

        //     ImGui::MenuItem("Enabled", "", &show_debug, false);
        //     ImGui::EndMenu();
        // }

        if (ImGui::BeginMenu("About"))
        {
            gui_in_use = true;

            if (ImGui::MenuItem("About " GEARSYSTEM_TITLE " " GEARSYSTEM_VERSION " ..."))
            {
               open_about = true;
            }
            ImGui::EndMenu();
        }

        main_menu_height = ImGui::GetWindowSize().y;

        ImGui::EndMainMenuBar();       
    }

    if (open_rom || shortcut_open_rom)
    {
        shortcut_open_rom = false;
        ImGui::OpenPopup("Open ROM...");
    }

    if (open_ram)
        ImGui::OpenPopup("Load RAM From...");

    if (save_ram)
        ImGui::OpenPopup("Save RAM As...");

    if (open_state)
        ImGui::OpenPopup("Load State From...");
    
    if (save_state)
        ImGui::OpenPopup("Save State As...");

    if (open_about)
    {
        dialog_in_use = true;
        ImGui::OpenPopup("About " GEARSYSTEM_TITLE);
    }
    
    popup_modal_about();
    file_dialog_open_rom();
    file_dialog_load_ram();
    file_dialog_save_ram();
    file_dialog_load_state();
    file_dialog_save_state();
}

static void main_window(void)
{
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);

    int w = ImGui::GetIO().DisplaySize.x;
    int h = ImGui::GetIO().DisplaySize.y - main_menu_height;

    float ratio;

    switch (config_video.ratio)
    {
        case 0:
            ratio = (float)runtime.screen_width / (float)runtime.screen_height;
            break;
        case 1:
            ratio = 4.0f / 3.0f;
            break;
        case 2:
            ratio = 16.0f / 9.0f;
            break;
        case 3:
            ratio = (float)w / (float)h;
            break;
        default:
            ratio = 1.0f;
    }

    int runtime_w_corrected = config_video.ratio == 3 ? w : runtime.screen_height * ratio;
    int runtime_h_corrected = config_video.ratio == 3 ? h : runtime.screen_height;

    int factor = 0;

    if (config_video.scale > 0)
    {
        factor = config_video.scale;
    }
    else
    {
        int factor_w = w / runtime_w_corrected;
        int factor_h = h / runtime_h_corrected;
        factor = (factor_w < factor_h) ? factor_w : factor_h;
    }

    main_window_width = runtime_w_corrected * factor;
    main_window_height = runtime_h_corrected * factor;

    int window_x = (w - (runtime_w_corrected * factor)) / 2;
    int window_y = ((h - (runtime_h_corrected * factor)) / 2) + main_menu_height;
    
    ImGui::SetNextWindowPos(ImVec2(window_x, window_y));
    ImGui::SetNextWindowSize(ImVec2(main_window_width, main_window_height));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin(GEARSYSTEM_TITLE, 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);

    ImGui::Image((void*)(intptr_t)renderer_emu_texture, ImVec2(main_window_width,main_window_height));

    if (config_video.fps)
        show_fps();

    if (config_emulator.show_info)
        show_info();

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

static void file_dialog_open_rom(void)
{
    if(file_dialog.showFileDialog("Open ROM...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 400), "*.*,.sms,.gg,.sg,.mv,.rom,.bin,.zip", &dialog_in_use))
    {
        push_recent_rom(file_dialog.selected_path.c_str());
        load_rom(file_dialog.selected_path.c_str());
    }
}

static void file_dialog_load_ram(void)
{
    if(file_dialog.showFileDialog("Load RAM From...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".sav,*.*", &dialog_in_use))
    {
        Cartridge::ForceConfiguration config;

        config.system = get_system(config_emulator.system);
        config.region = get_region(config_emulator.region);
        config.type = get_mapper(config_emulator.mapper);
        config.zone = get_zone(config_emulator.zone);
        
        emu_load_ram(file_dialog.selected_path.c_str(), config_emulator.save_in_rom_folder, config);
    }
}

static void file_dialog_save_ram(void)
{
    if(file_dialog.showFileDialog("Save RAM As...", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".sav", &dialog_in_use))
    {
        std::string state_path = file_dialog.selected_path;

        if (state_path.rfind(file_dialog.ext) != (state_path.size()-file_dialog.ext.size()))
        {
            state_path += file_dialog.ext;
        }

        emu_save_ram(state_path.c_str());
    }
}

static void file_dialog_load_state(void)
{
    if(file_dialog.showFileDialog("Load State From...", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".state,*.*", &dialog_in_use))
    {
        emu_load_state_file(file_dialog.selected_path.c_str());
    }
}

static void file_dialog_save_state(void)
{
    if(file_dialog.showFileDialog("Save State As...", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".state", &dialog_in_use))
    {
        std::string state_path = file_dialog.selected_path;

        if (state_path.rfind(file_dialog.ext) != (state_path.size()-file_dialog.ext.size()))
        {
            state_path += file_dialog.ext;
        }

        emu_save_state_file(state_path.c_str());
    }
}

static void keyboard_configuration_item(const char* text, SDL_Scancode* key, int player)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(70);

    char button_label[256];
    sprintf(button_label, "%s##%s%d", SDL_GetScancodeName(*key), text, player);

    if (ImGui::Button(button_label, ImVec2(90,0)))
    {
        configured_key = key;
        ImGui::OpenPopup("Keyboard Configuration");
    }
}

static void gamepad_configuration_item(const char* text, int* button, int player)
{
    ImGui::Text("%s", text);
    ImGui::SameLine(70);

    static const char* gamepad_names[16] = {"0", "A", "B" ,"3", "L", "R", "6", "7", "SELECT", "START", "10", "11", "12", "13", "14", "15"};

    char button_label[256];
    sprintf(button_label, "%s##%s%d", gamepad_names[*button], text, player);

    if (ImGui::Button(button_label, ImVec2(70,0)))
    {
        configured_button = button;
        ImGui::OpenPopup("Gamepad Configuration");
    }
}

static void popup_modal_keyboard()
{
    if (ImGui::BeginPopupModal("Keyboard Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Press any key...\n\n");
        ImGui::Separator();

        for (int i = 0; i < IM_ARRAYSIZE(ImGui::GetIO().KeysDown); i++)
        {
            if (ImGui::IsKeyPressed(i))
            {
                SDL_Scancode key = (SDL_Scancode)i;

                if ((key != SDL_SCANCODE_LCTRL) && (key != SDL_SCANCODE_RCTRL) && (key != SDL_SCANCODE_CAPSLOCK))
                {
                    *configured_key = key;
                    ImGui::CloseCurrentPopup();
                    break;
                }
            }
        }

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void popup_modal_gamepad(int pad)
{
    if (ImGui::BeginPopupModal("Gamepad Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Press any button in your gamepad...\n\n");
        ImGui::Separator();

        for (int i = 0; i < 16; i++)
        {
            if (SDL_JoystickGetButton(application_gamepad[pad], i))
            {
                *configured_button = i;
                ImGui::CloseCurrentPopup();
                break;
            }
        }

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void popup_modal_about(void)
{
    if (ImGui::BeginPopupModal("About " GEARSYSTEM_TITLE, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s %s", GEARSYSTEM_TITLE, GEARSYSTEM_VERSION);
        ImGui::Text("Build: %s", EMULATOR_BUILD);
        ImGui::Separator();
        ImGui::Text("By Ignacio Sánchez (twitter.com/drhelius)");
        ImGui::Text("%s is licensed under the GPL-3.0 License, see LICENSE for more information.", GEARSYSTEM_TITLE);
        ImGui::Separator();        
        
        #ifdef _WIN64
        ImGui::Text("Windows 64 bit detected.");
        #elif defined(_WIN32)
        ImGui::Text("Windows 32 bit detected.");
        #endif
        #ifdef __linux__
        ImGui::Text("Linux detected.");
        #endif
        #ifdef __APPLE__
        ImGui::Text("macOS detected.");
        #endif
        #ifdef _MSC_VER
        ImGui::Text("Built with Microsoft C++ %d.", _MSC_VER);
        #endif
        #ifdef __MINGW32__
        ImGui::Text("Built with MinGW 32 bit.");
        #endif
        #ifdef __MINGW64__
        ImGui::Text("Built with MinGW 64 bit.");
        #endif
        #if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
        ImGui::Text("Built with GCC %d.%d.%d", (int)__GNUC__, (int)__GNUC_MINOR__, (int)__GNUC_PATCHLEVEL__);
        #endif
        #ifdef __clang_version__
        ImGui::Text("Built with Clang %s.", __clang_version__);
        #endif
        #ifdef DEBUG
        ImGui::Text("define: DEBUG");
        #endif
        #ifdef DEBUG_GEARSYSTEM
        ImGui::Text("define: DEBUG_GEARSYSTEM");
        #endif
        ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
        ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) 
        {
            ImGui::CloseCurrentPopup();
            dialog_in_use = false;
        }
        ImGui::SetItemDefaultFocus();

        ImGui::EndPopup();
    }
}

static void load_rom(const char* path)
{
    Cartridge::ForceConfiguration config;

    config.system = get_system(config_emulator.system);
    config.region = get_region(config_emulator.region);
    config.type = get_mapper(config_emulator.mapper);
    config.zone = get_zone(config_emulator.zone);

    emu_resume();
    emu_load_rom(path, config_emulator.save_in_rom_folder, config);
    cheat_list.clear();
    emu_clear_cheats();

    if (config_emulator.start_paused)
    {
        emu_pause();
        
        for (int i=0; i < (GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT); i++)
        {
            emu_frame_buffer[i].red = 0;
            emu_frame_buffer[i].green = 0;
            emu_frame_buffer[i].blue = 0;
        }
    }
}

static void push_recent_rom(std::string path)
{
    for (int i = (config_max_recent_roms - 1); i >= 0; i--)
    {
        config_emulator.recent_roms[i] = config_emulator.recent_roms[i - 1];
    }

    config_emulator.recent_roms[0] = path;
}

static void menu_reset(void)
{
    emu_resume();

    Cartridge::ForceConfiguration config;

    config.system = get_system(config_emulator.system);
    config.region = get_region(config_emulator.region);
    config.type = get_mapper(config_emulator.mapper);
    config.zone = get_zone(config_emulator.zone);

    emu_reset(config_emulator.save_in_rom_folder, config);

    if (config_emulator.start_paused)
    {
        emu_pause();
        
        for (int i=0; i < (GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT); i++)
        {
            emu_frame_buffer[i].red = 0;
            emu_frame_buffer[i].green = 0;
            emu_frame_buffer[i].blue = 0;
        }
    }
}

static void menu_pause(void)
{
    if (emu_is_paused())
        emu_resume();
    else
        emu_pause();
}

static void menu_ffwd(void)
{
    config_audio.sync = !config_emulator.ffwd;

    if (config_emulator.ffwd)
        SDL_GL_SetSwapInterval(0);
    else
    {
        SDL_GL_SetSwapInterval(config_video.sync ? 1 : 0);
        emu_audio_reset();
    }
}

static void show_info(void)
{
    if (config_video.fps)
        ImGui::SetCursorPosX(5.0f);
    else
        ImGui::SetCursorPos(ImVec2(5.0f, 5.0f));

    static char info[512];

    emu_get_info(info);
    ImGui::Text("%s", info);
}

static void show_fps(void)
{
    ImGui::SetCursorPos(ImVec2(5.0f, 5.0f));
    ImGui::Text("Frame Rate: %.2f FPS", ImGui::GetIO().Framerate);
    ImGui::SetCursorPosX(5.0f);
    ImGui::Text("Frame Time: %.2f ms", 1000.0f / ImGui::GetIO().Framerate);
}

static Cartridge::CartridgeTypes get_mapper(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeNotSupported;
        case 1:
            return Cartridge::CartridgeRomOnlyMapper;
        case 2:
            return Cartridge::CartridgeSegaMapper;
        case 3:
            return Cartridge::CartridgeCodemastersMapper;
        case 4:
            return Cartridge::CartridgeKoreanMapper;
        case 5:
            return Cartridge::CartridgeSG1000Mapper;
        default:
            return Cartridge::CartridgeNotSupported;
    }
}

static Cartridge::CartridgeZones get_zone(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeUnknownZone;
        case 1:
            return Cartridge::CartridgeJapanSMS;
        case 2:
            return Cartridge::CartridgeExportSMS;
        case 3:
            return Cartridge::CartridgeJapanGG;
        case 4:
            return Cartridge::CartridgeExportGG;
        case 5:
            return Cartridge::CartridgeInternationalGG;
        default:
            return Cartridge::CartridgeUnknownZone;
    }
}

static Cartridge::CartridgeSystem get_system(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeUnknownSystem;
        case 1:
            return Cartridge::CartridgeSMS;
        case 2:
            return Cartridge::CartridgeGG;
        case 3:
            return Cartridge::CartridgeSG1000;
        default:
            return Cartridge::CartridgeUnknownSystem;
    }
}

static Cartridge::CartridgeRegions get_region(int index)
{
    //"Auto\0NTSC (60 Hz)\0PAL (50 Hz)\0\0");
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeUnknownRegion;
        case 1:
            return Cartridge::CartridgeNTSC;
        case 2:
            return Cartridge::CartridgePAL;
        default:
            return Cartridge::CartridgeUnknownRegion;
    }
}
