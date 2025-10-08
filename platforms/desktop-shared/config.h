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
#include "gui_events.h"

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

    bool gamepad;
    int gamepad_directional;
    bool gamepad_invert_x_axis;
    bool gamepad_invert_y_axis;
    int gamepad_1;
    int gamepad_2;
    int gamepad_start;
    int gamepad_x_axis;
    int gamepad_y_axis;
};

struct config_Key
{
    SDL_Scancode scancode;
    SDL_Keymod modifier;
};

struct config_GuiShortCuts
{
    config_Key shortcuts[gui_ShortCutEventMax];
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
EXTERN config_Debug config_debug;
EXTERN config_GuiShortCuts config_shortcuts;

EXTERN void config_init(void);
EXTERN void config_destroy(void);
EXTERN void config_read(void);
EXTERN void config_write(void);

#undef CONFIG_IMPORT
#undef EXTERN
#endif	/* CONFIG_H */
