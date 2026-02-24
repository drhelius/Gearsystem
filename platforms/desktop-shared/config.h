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

#ifndef CONFIG_H
#define	CONFIG_H

#include <SDL.h>
#include "../../src/gearsystem.h"
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "imgui/imgui.h"

#ifdef CONFIG_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

static const int config_max_recent_roms = 10;

struct config_Emulator
{
    bool maximized = false;
    bool fullscreen = false;
    int fullscreen_mode = 1;
    bool always_show_menu = false;
    bool paused = false;
    int save_slot = 0;
    bool start_paused = false;
    bool pause_when_inactive = true;
    bool ffwd = false;
    int ffwd_speed = 1;
    int system = 0;
    int zone = 0;
    int mapper = 0;
    int region = 0;
    bool show_info = false;
    std::string recent_roms[config_max_recent_roms];
    bool sms_bootrom;
    std::string sms_bootrom_path;
    bool gg_bootrom;
    std::string gg_bootrom_path;
    int media = 0;
    int savefiles_dir_option = 0;
    std::string savefiles_path;
    int savestates_dir_option = 0;
    std::string savestates_path;
    std::string last_open_path;
    int window_width = 640;
    int window_height = 503;
    bool status_messages = false;
    bool light_phaser = false;
    bool light_phaser_crosshair = false;
    int light_phaser_crosshair_shape = 0;
    int light_phaser_crosshair_color = 0;
    int light_phaser_x_offset = 0;
    int light_phaser_y_offset = 0;
    bool paddle_control = false;
    int paddle_sensitivity = 5;
    bool capture_mouse = false;
};

struct config_Video
{
    int scale = 0;
    int scale_manual = 1;
    int ratio = 1;
    int overscan = 1;
    int hide_left_bar = 0;
    bool fps = false;
    bool bilinear = false;
    bool mix_frames = true;
    float mix_frames_intensity = 0.50f;
    bool scanlines = true;
    bool  scanlines_filter = true;
    float scanlines_intensity = 0.10f;
    bool sync = true;
    float background_color[3] = {0.1f, 0.1f, 0.1f};
    float background_color_debugger[3] = {0.2f, 0.2f, 0.2f};
    int glasses = 0;
};

struct config_Audio
{
    bool enable = true;
    bool sync = true;
    int ym2413 = 0;
};

struct config_Input
{
    SDL_Scancode key_left;
    SDL_Scancode key_right;
    SDL_Scancode key_up;
    SDL_Scancode key_down;
    SDL_Scancode key_1;
    SDL_Scancode key_2;
    SDL_Scancode key_start;
    SDL_Scancode key_reset;

    bool gamepad;
    int gamepad_directional;
    bool gamepad_invert_x_axis;
    bool gamepad_invert_y_axis;
    int gamepad_1;
    int gamepad_2;
    int gamepad_start;
    int gamepad_reset;
    int gamepad_x_axis;
    int gamepad_y_axis;
};

enum config_HotkeyIndex
{
    config_HotkeyIndex_OpenROM = 0,
    config_HotkeyIndex_Quit,
    config_HotkeyIndex_Reset,
    config_HotkeyIndex_Pause,
    config_HotkeyIndex_FFWD,
    config_HotkeyIndex_SaveState,
    config_HotkeyIndex_LoadState,
    config_HotkeyIndex_Screenshot,
    config_HotkeyIndex_Fullscreen,
    config_HotkeyIndex_CaptureMouse,
    config_HotkeyIndex_ShowMainMenu,
    config_HotkeyIndex_DebugStep,
    config_HotkeyIndex_DebugContinue,
    config_HotkeyIndex_DebugNextFrame,
    config_HotkeyIndex_DebugRunToCursor,
    config_HotkeyIndex_DebugBreakpoint,
    config_HotkeyIndex_DebugGoBack,
    config_HotkeyIndex_SelectSlot1,
    config_HotkeyIndex_SelectSlot2,
    config_HotkeyIndex_SelectSlot3,
    config_HotkeyIndex_SelectSlot4,
    config_HotkeyIndex_SelectSlot5,
    config_HotkeyIndex_COUNT
};

struct config_Input_Gamepad_Shortcuts
{
    int gamepad_shortcuts[config_HotkeyIndex_COUNT];
};

struct config_Hotkey
{
    SDL_Scancode key;
    SDL_Keymod mod;
    char str[64];
};

struct config_Debug
{
    bool debug = false;
    bool show_screen = true;
    bool show_disassembler = true;
    bool show_processor = true;
    bool show_memory = true;
    bool show_video = false;
    int font_size = 0;
    bool multi_viewport = false;
};

EXTERN mINI::INIFile* config_ini_file;
EXTERN mINI::INIStructure config_ini_data;
EXTERN char* config_root_path;
EXTERN char config_emu_file_path[260];
EXTERN char config_imgui_file_path[260];
EXTERN config_Emulator config_emulator;
EXTERN config_Video config_video;
EXTERN config_Audio config_audio;
EXTERN config_Input config_input[2];
EXTERN config_Input_Gamepad_Shortcuts config_input_gamepad_shortcuts[2];
EXTERN config_Hotkey config_hotkeys[config_HotkeyIndex_COUNT];
EXTERN config_Debug config_debug;

EXTERN void config_init(void);
EXTERN void config_destroy(void);
EXTERN void config_read(void);
EXTERN void config_write(void);
EXTERN void config_update_hotkey_string(config_Hotkey* hotkey);

#undef CONFIG_IMPORT
#undef EXTERN
#endif	/* CONFIG_H */
