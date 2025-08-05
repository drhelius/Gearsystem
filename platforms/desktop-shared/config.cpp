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

#include <SDL.h>
#include <iomanip>
#include "../../src/gearsystem.h"
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"

#define CONFIG_IMPORT
#include "config.h"

static bool check_portable(void);
static config_Key read_shortcut_key(const char* group, const char* key, config_Key default_value);
static int read_int(const char* group, const char* key, int default_value);
static void write_int(const char* group, const char* key, int integer);
static float read_float(const char* group, const char* key, float default_value);
static void write_float(const char* group, const char* key, float value);
static bool read_bool(const char* group, const char* key, bool default_value);
static void write_bool(const char* group, const char* key, bool boolean);
static std::string read_string(const char* group, const char* key);
static void write_string(const char* group, const char* key, std::string value);

struct KModMap
{
    const char* keymodName;
    SDL_Keymod keymod;
};

static const KModMap modifiers_map[] =
{
    { "NONE", KMOD_NONE },
    { "LSHIFT", KMOD_LSHIFT },
    { "RSHIFT", KMOD_RSHIFT },
    { "SHIFT", static_cast<SDL_Keymod>(KMOD_LSHIFT | KMOD_RSHIFT) },
    { "LCTRL", KMOD_LCTRL },
    { "RCTRL", KMOD_RCTRL },
    { "CTRL", static_cast<SDL_Keymod>(KMOD_LCTRL | KMOD_RCTRL) },
    { "LALT", KMOD_LALT },
    { "RALT", KMOD_RALT },
    { "ALT", static_cast<SDL_Keymod>(KMOD_LALT | KMOD_RALT) },
    { "L" GUI_EVENT_PLATFORM_KEY_UPPERCASE, KMOD_LGUI },
    { "R" GUI_EVENT_PLATFORM_KEY_UPPERCASE, KMOD_RGUI },
    { GUI_EVENT_PLATFORM_KEY_UPPERCASE, static_cast<SDL_Keymod>(KMOD_LGUI | KMOD_RGUI) }
};

static const char* config_modifiers[] =
{
    "LShift",
    "RShift",
    "Shift",
    "LCtrl",
    "RCtrl",
    "Ctrl",
    "LAlt",
    "RAlt",
    "Alt",
    "L" GUI_EVENT_PLATFORM_KEY_CAMELCASE,
    "R" GUI_EVENT_PLATFORM_KEY_CAMELCASE,
    GUI_EVENT_PLATFORM_KEY_CAMELCASE
};

static void config_setShortcut(gui_ShortCutEvent event, uint32_t modifier, SDL_Scancode scancode)
{
    assert(event >= 0 && event < gui_ShortCutEventMax);
    config_shortcuts.shortcuts[event].modifier = static_cast<SDL_Keymod>(modifier);
    config_shortcuts.shortcuts[event].scancode = scancode;
}

void config_init(void)
{
    if (check_portable())
        config_root_path = SDL_GetBasePath();
    else
        config_root_path = SDL_GetPrefPath("Geardome", GEARSYSTEM_TITLE);

    strcpy(config_emu_file_path, config_root_path);
    strcat(config_emu_file_path, "config.ini");

    strcpy(config_imgui_file_path, config_root_path);
    strcat(config_imgui_file_path, "imgui.ini");

    config_input[0].key_left = SDL_SCANCODE_LEFT;
    config_input[0].key_right = SDL_SCANCODE_RIGHT;
    config_input[0].key_up = SDL_SCANCODE_UP;
    config_input[0].key_down = SDL_SCANCODE_DOWN;
    config_input[0].key_1 = SDL_SCANCODE_A;
    config_input[0].key_2 = SDL_SCANCODE_S;
    config_input[0].key_start = SDL_SCANCODE_RETURN;
    config_input[0].gamepad = true;
    config_input[0].gamepad_invert_x_axis = false;
    config_input[0].gamepad_invert_y_axis = false;
    config_input[0].gamepad_1 = 1;
    config_input[0].gamepad_2 = 2;
    config_input[0].gamepad_start = 9;
    config_input[0].gamepad_x_axis = 0;
    config_input[0].gamepad_y_axis = 1;
    config_input[1].key_left = SDL_SCANCODE_J;
    config_input[1].key_right = SDL_SCANCODE_L;
    config_input[1].key_up = SDL_SCANCODE_I;
    config_input[1].key_down = SDL_SCANCODE_K;
    config_input[1].key_1 = SDL_SCANCODE_G;
    config_input[1].key_2 = SDL_SCANCODE_H;
    config_input[1].key_start = SDL_SCANCODE_RSHIFT;
    config_input[1].gamepad = true;
    config_input[1].gamepad_invert_x_axis = false;
    config_input[1].gamepad_invert_y_axis = false;
    config_input[1].gamepad_1 = 1;
    config_input[1].gamepad_2 = 2;
    config_input[1].gamepad_start = 9;
    config_input[1].gamepad_x_axis = 0;
    config_input[1].gamepad_y_axis = 1;

    // Initialise default shortcuts.
    config_setShortcut(gui_ShortcutOpenROM, KMOD_CTRL, SDL_SCANCODE_O);
    config_setShortcut(gui_ShortcutReset, KMOD_CTRL, SDL_SCANCODE_R);
    config_setShortcut(gui_ShortcutPause, KMOD_CTRL, SDL_SCANCODE_P);
    config_setShortcut(gui_ShortcutFFWD, KMOD_CTRL, SDL_SCANCODE_F);
    config_setShortcut(gui_ShortcutSaveState, KMOD_CTRL, SDL_SCANCODE_S);
    config_setShortcut(gui_ShortcutLoadState, KMOD_CTRL, SDL_SCANCODE_L);
    config_setShortcut(gui_ShortcutScreenshot, KMOD_CTRL, SDL_SCANCODE_X);
    config_setShortcut(gui_ShortcutDebugStep, KMOD_CTRL, SDL_SCANCODE_F10);
    config_setShortcut(gui_ShortcutDebugContinue, KMOD_CTRL, SDL_SCANCODE_F5);
    config_setShortcut(gui_ShortcutDebugNextFrame, KMOD_CTRL, SDL_SCANCODE_F6);
    config_setShortcut(gui_ShortcutDebugBreakpoint, KMOD_CTRL, SDL_SCANCODE_F9);
    config_setShortcut(gui_ShortcutDebugRuntocursor, KMOD_CTRL, SDL_SCANCODE_F8);
    config_setShortcut(gui_ShortcutDebugGoBack, KMOD_CTRL, SDL_SCANCODE_BACKSPACE);
    config_setShortcut(gui_ShortcutDebugCopy, KMOD_CTRL, SDL_SCANCODE_C);
    config_setShortcut(gui_ShortcutDebugPaste, KMOD_CTRL, SDL_SCANCODE_V);
    config_setShortcut(gui_ShortcutShowMainMenu, KMOD_CTRL, SDL_SCANCODE_M);
    config_setShortcut(gui_ShortcutQuit, KMOD_CTRL, SDL_SCANCODE_Q);

    config_ini_file = new mINI::INIFile(config_emu_file_path);
}

void config_destroy(void)
{
    SafeDelete(config_ini_file)
    SDL_free(config_root_path);
}

void config_read(void)
{
    if (!config_ini_file->read(config_ini_data))
    {
        Log("Unable to load settings from %s", config_emu_file_path);
        return;
    }

    Log("Loading settings from %s", config_emu_file_path);

    config_debug.debug = read_bool("Debug", "Debug", false);
    config_debug.show_disassembler = read_bool("Debug", "Disassembler", true);
    config_debug.show_screen = read_bool("Debug", "Screen", true);
    config_debug.show_memory = read_bool("Debug", "Memory", true);
    config_debug.show_processor = read_bool("Debug", "Processor", true);
    config_debug.show_video = read_bool("Debug", "Video", false);
    config_debug.font_size = read_int("Debug", "FontSize", 0);
    config_debug.multi_viewport = read_bool("Debug", "MultiViewport", false);

    for (int i = 0; i < gui_ShortCutEventMax; i++)
    {
        // The shortcut was configured with a default setting at startup, so pass that in as the default in case the setting doesn't exist in the .ini
        gui_ShortCutEvent event = static_cast<gui_ShortCutEvent>(i);
        const char* name = gui_event_get_name(event);
        config_shortcuts.shortcuts[i] = read_shortcut_key("Shortcuts", name, config_shortcuts.shortcuts[i]);
    }

    config_emulator.maximized = read_bool("Emulator", "Maximized", false);
    config_emulator.fullscreen = read_bool("Emulator", "FullScreen", false);
    config_emulator.always_show_menu = read_bool("Emulator", "AlwaysShowMenu", false);
    config_emulator.ffwd_speed = read_int("Emulator", "FFWD", 1);
    config_emulator.save_slot = read_int("Emulator", "SaveSlot", 0);
    config_emulator.start_paused = read_bool("Emulator", "StartPaused", false);
    config_emulator.system = read_int("Emulator", "System", 0);
    config_emulator.zone = read_int("Emulator", "Zone", 0);
    config_emulator.mapper = read_int("Emulator", "Mapper", 0);
    config_emulator.region = read_int("Emulator", "Region", 0);
    config_emulator.sms_bootrom = read_bool("Emulator", "SMSBootrom", false);
    config_emulator.sms_bootrom_path = read_string("Emulator", "SMSBootromPath");
    config_emulator.gg_bootrom = read_bool("Emulator", "GGBootrom", false);
    config_emulator.gg_bootrom_path = read_string("Emulator", "GGBootromPath");
    config_emulator.media = read_int("Emulator", "Media", 0);
    config_emulator.savefiles_dir_option = read_int("Emulator", "SaveFilesDirOption", 0);
    config_emulator.savefiles_path = read_string("Emulator", "SaveFilesPath");
    config_emulator.savestates_dir_option = read_int("Emulator", "SaveStatesDirOption", 0);
    config_emulator.savestates_path = read_string("Emulator", "SaveStatesPath");
    config_emulator.last_open_path = read_string("Emulator", "LastOpenPath");
    config_emulator.window_width = read_int("Emulator", "WindowWidth", 640);
    config_emulator.window_height = read_int("Emulator", "WindowHeight", 503);
    config_emulator.status_messages = read_bool("Emulator", "StatusMessages", false);
    config_emulator.light_phaser = read_bool("Emulator", "LightPhaser", false);
    config_emulator.light_phaser_crosshair = read_bool("Emulator", "LightPhaserCrosshair", false);
    config_emulator.light_phaser_crosshair_shape = read_int("Emulator", "LightPhaserCrosshairShape", 0);
    config_emulator.light_phaser_crosshair_color = read_int("Emulator", "LightPhaserCrosshairColor", 0);
    config_emulator.light_phaser_x_offset = read_int("Emulator", "LightPhaserXOffset", 0);
    config_emulator.light_phaser_y_offset = read_int("Emulator", "LightPhaserYOffset", 0);
    config_emulator.paddle_control = read_bool("Emulator", "PaddleControl", false);
    config_emulator.paddle_sensitivity = read_int("Emulator", "PaddleSensitivity", 5);

    if (config_emulator.light_phaser)
        config_emulator.paddle_control = false;

    if (config_emulator.savefiles_path.empty())
    {
        config_emulator.savefiles_path = config_root_path;
    }
    if (config_emulator.savestates_path.empty())
    {
        config_emulator.savestates_path = config_root_path;
    }

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        config_emulator.recent_roms[i] = read_string("Emulator", item.c_str());
    }

    config_video.scale = read_int("Video", "Scale", 0);
    if (config_video.scale > 3)
        config_video.scale -= 2;
    config_video.scale_manual = read_int("Video", "ScaleManual", 1);
    config_video.ratio = read_int("Video", "AspectRatio", 1);
    config_video.overscan = read_int("Video", "Overscan", 1);
    config_video.hide_left_bar = read_int("Video", "HideLeftBar", 0);
    config_video.fps = read_bool("Video", "FPS", false);
    config_video.bilinear = read_bool("Video", "Bilinear", false);
    config_video.mix_frames = read_bool("Video", "MixFrames", true);
    config_video.mix_frames_intensity = read_float("Video", "MixFramesIntensity", 0.50f);
    config_video.scanlines = read_bool("Video", "Scanlines", true);
    config_video.scanlines_filter = read_bool("Video", "ScanlinesFilter", true);
    config_video.scanlines_intensity = read_float("Video", "ScanlinesIntensity", 0.10f);
    config_video.sync = read_bool("Video", "Sync", true);
    config_video.glasses = read_int("Video", "3DGlasses", 0);
    
    config_audio.enable = read_bool("Audio", "Enable", true);
    config_audio.sync = read_bool("Audio", "Sync", true);
    config_audio.ym2413 = read_int("Audio", "YM2413", 0);

    config_input[0].key_left = (SDL_Scancode)read_int("InputA", "KeyLeft", SDL_SCANCODE_LEFT);
    config_input[0].key_right = (SDL_Scancode)read_int("InputA", "KeyRight", SDL_SCANCODE_RIGHT);
    config_input[0].key_up = (SDL_Scancode)read_int("InputA", "KeyUp", SDL_SCANCODE_UP);
    config_input[0].key_down = (SDL_Scancode)read_int("InputA", "KeyDown", SDL_SCANCODE_DOWN);
    config_input[0].key_1 = (SDL_Scancode)read_int("InputA", "Key1", SDL_SCANCODE_A);
    config_input[0].key_2 = (SDL_Scancode)read_int("InputA", "Key2", SDL_SCANCODE_S);
    config_input[0].key_start = (SDL_Scancode)read_int("InputA", "KeyStart", SDL_SCANCODE_RETURN);

    config_input[0].gamepad = read_bool("InputA", "Gamepad", true);
    config_input[0].gamepad_directional = read_int("InputA", "GamepadDirectional", 0);
    config_input[0].gamepad_invert_x_axis = read_bool("InputA", "GamepadInvertX", false);
    config_input[0].gamepad_invert_y_axis = read_bool("InputA", "GamepadInvertY", false);
    config_input[0].gamepad_1 = read_int("InputA", "Gamepad1", SDL_CONTROLLER_BUTTON_A);
    config_input[0].gamepad_2 = read_int("InputA", "Gamepad2", SDL_CONTROLLER_BUTTON_B);
    config_input[0].gamepad_start = read_int("InputA", "GamepadStart", SDL_CONTROLLER_BUTTON_START);
    config_input[0].gamepad_x_axis = read_int("InputA", "GamepadX", SDL_CONTROLLER_AXIS_LEFTX);
    config_input[0].gamepad_y_axis = read_int("InputA", "GamepadY", SDL_CONTROLLER_AXIS_LEFTY);

    config_input[1].key_left = (SDL_Scancode)read_int("InputB", "KeyLeft", SDL_SCANCODE_J);
    config_input[1].key_right = (SDL_Scancode)read_int("InputB", "KeyRight", SDL_SCANCODE_L);
    config_input[1].key_up = (SDL_Scancode)read_int("InputB", "KeyUp", SDL_SCANCODE_I);
    config_input[1].key_down = (SDL_Scancode)read_int("InputB", "KeyDown", SDL_SCANCODE_K);
    config_input[1].key_1 = (SDL_Scancode)read_int("InputB", "Key1", SDL_SCANCODE_G);
    config_input[1].key_2 = (SDL_Scancode)read_int("InputB", "Key2", SDL_SCANCODE_H);
    config_input[1].key_start = (SDL_Scancode)read_int("InputB", "KeyStart", SDL_SCANCODE_RSHIFT);

    config_input[1].gamepad = read_bool("InputB", "Gamepad", true);
    config_input[1].gamepad_directional = read_int("InputB", "GamepadDirectional", 0);
    config_input[1].gamepad_invert_x_axis = read_bool("InputB", "GamepadInvertX", false);
    config_input[1].gamepad_invert_y_axis = read_bool("InputB", "GamepadInvertY", false);
    config_input[1].gamepad_1 = read_int("InputB", "Gamepad1", SDL_CONTROLLER_BUTTON_A);
    config_input[1].gamepad_2 = read_int("InputB", "Gamepad2", SDL_CONTROLLER_BUTTON_B);
    config_input[1].gamepad_start = read_int("InputB", "GamepadStart", SDL_CONTROLLER_BUTTON_START);
    config_input[1].gamepad_x_axis = read_int("InputB", "GamepadX", SDL_CONTROLLER_AXIS_LEFTX);
    config_input[1].gamepad_y_axis = read_int("InputB", "GamepadY", SDL_CONTROLLER_AXIS_LEFTY);

    Debug("Settings loaded");
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    if (config_emulator.ffwd)
        config_audio.sync = true;

    write_bool("Debug", "Debug", config_debug.debug);
    write_bool("Debug", "Disassembler", config_debug.show_disassembler);
    write_bool("Debug", "Screen", config_debug.show_screen);
    write_bool("Debug", "Memory", config_debug.show_memory);
    write_bool("Debug", "Processor", config_debug.show_processor);
    write_bool("Debug", "Video", config_debug.show_video);
    write_int("Debug", "FontSize", config_debug.font_size);
    write_bool("Debug", "MultiViewport", config_debug.multi_viewport);

    write_bool("Emulator", "Maximized", config_emulator.maximized);
    write_bool("Emulator", "FullScreen", config_emulator.fullscreen);
    write_bool("Emulator", "AlwaysShowMenu", config_emulator.always_show_menu);
    write_int("Emulator", "FFWD", config_emulator.ffwd_speed);
    write_int("Emulator", "SaveSlot", config_emulator.save_slot);
    write_bool("Emulator", "StartPaused", config_emulator.start_paused);
    write_int("Emulator", "System", config_emulator.system);
    write_int("Emulator", "Zone", config_emulator.zone);
    write_int("Emulator", "Mapper", config_emulator.mapper);
    write_int("Emulator", "Region", config_emulator.region);
    write_bool("Emulator", "SMSBootrom", config_emulator.sms_bootrom);
    write_string("Emulator", "SMSBootromPath", config_emulator.sms_bootrom_path);
    write_bool("Emulator", "GGBootrom", config_emulator.gg_bootrom);
    write_string("Emulator", "GGBootromPath", config_emulator.gg_bootrom_path);
    write_int("Emulator", "Media", config_emulator.media);
    write_int("Emulator", "SaveFilesDirOption", config_emulator.savefiles_dir_option);
    write_string("Emulator", "SaveFilesPath", config_emulator.savefiles_path);
    write_int("Emulator", "SaveStatesDirOption", config_emulator.savestates_dir_option);
    write_string("Emulator", "SaveStatesPath", config_emulator.savestates_path);
    write_string("Emulator", "LastOpenPath", config_emulator.last_open_path);
    write_int("Emulator", "WindowWidth", config_emulator.window_width);
    write_int("Emulator", "WindowHeight", config_emulator.window_height);
    write_bool("Emulator", "StatusMessages", config_emulator.status_messages);
    write_bool("Emulator", "LightPhaser", config_emulator.light_phaser);
    write_bool("Emulator", "LightPhaserCrosshair", config_emulator.light_phaser_crosshair);
    write_int("Emulator", "LightPhaserCrosshairShape", config_emulator.light_phaser_crosshair_shape);
    write_int("Emulator", "LightPhaserCrosshairColor", config_emulator.light_phaser_crosshair_color);
    write_int("Emulator", "LightPhaserXOffset", config_emulator.light_phaser_x_offset);
    write_int("Emulator", "LightPhaserYOffset", config_emulator.light_phaser_y_offset);
    write_bool("Emulator", "PaddleControl", config_emulator.paddle_control);
    write_int("Emulator", "PaddleSensitivity", config_emulator.paddle_sensitivity);

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        write_string("Emulator", item.c_str(), config_emulator.recent_roms[i]);
    }

    write_int("Video", "Scale", config_video.scale);
    write_int("Video", "ScaleManual", config_video.scale_manual);
    write_int("Video", "AspectRatio", config_video.ratio);
    write_int("Video", "Overscan", config_video.overscan);
    write_int("Video", "HideLeftBar", config_video.hide_left_bar);
    write_bool("Video", "FPS", config_video.fps);
    write_bool("Video", "Bilinear", config_video.bilinear);
    write_bool("Video", "MixFrames", config_video.mix_frames);
    write_float("Video", "MixFramesIntensity", config_video.mix_frames_intensity);
    write_bool("Video", "Scanlines", config_video.scanlines);
    write_bool("Video", "ScanlinesFilter", config_video.scanlines_filter);
    write_float("Video", "ScanlinesIntensity", config_video.scanlines_intensity);
    write_bool("Video", "Sync", config_video.sync);
    write_int("Video", "3DGlasses", config_video.glasses);

    write_bool("Audio", "Enable", config_audio.enable);
    write_bool("Audio", "Sync", config_audio.sync);
    write_int("Audio", "YM2413", config_audio.ym2413);

    write_int("InputA", "KeyLeft", config_input[0].key_left);
    write_int("InputA", "KeyRight", config_input[0].key_right);
    write_int("InputA", "KeyUp", config_input[0].key_up);
    write_int("InputA", "KeyDown", config_input[0].key_down);
    write_int("InputA", "Key1", config_input[0].key_1);
    write_int("InputA", "Key2", config_input[0].key_2);
    write_int("InputA", "KeyStart", config_input[0].key_start);

    write_bool("InputA", "Gamepad", config_input[0].gamepad);
    write_int("InputA", "GamepadDirectional", config_input[0].gamepad_directional);
    write_bool("InputA", "GamepadInvertX", config_input[0].gamepad_invert_x_axis);
    write_bool("InputA", "GamepadInvertY", config_input[0].gamepad_invert_y_axis);
    write_int("InputA", "Gamepad1", config_input[0].gamepad_1);
    write_int("InputA", "Gamepad2", config_input[0].gamepad_2);
    write_int("InputA", "GamepadStart", config_input[0].gamepad_start);
    write_int("InputA", "GamepadX", config_input[0].gamepad_x_axis);
    write_int("InputA", "GamepadY", config_input[0].gamepad_y_axis);

    write_int("InputB", "KeyLeft", config_input[1].key_left);
    write_int("InputB", "KeyRight", config_input[1].key_right);
    write_int("InputB", "KeyUp", config_input[1].key_up);
    write_int("InputB", "KeyDown", config_input[1].key_down);
    write_int("InputB", "Key1", config_input[1].key_1);
    write_int("InputB", "Key2", config_input[1].key_2);
    write_int("InputB", "KeyStart", config_input[1].key_start);

    write_bool("InputB", "Gamepad", config_input[1].gamepad);
    write_int("InputB", "GamepadDirectional", config_input[1].gamepad_directional);
    write_bool("InputB", "GamepadInvertX", config_input[1].gamepad_invert_x_axis);
    write_bool("InputB", "GamepadInvertY", config_input[1].gamepad_invert_y_axis);
    write_int("InputB", "Gamepad1", config_input[1].gamepad_1);
    write_int("InputB", "Gamepad2", config_input[1].gamepad_2);
    write_int("InputB", "GamepadStart", config_input[1].gamepad_start);
    write_int("InputB", "GamepadX", config_input[1].gamepad_x_axis);
    write_int("InputB", "GamepadY", config_input[1].gamepad_y_axis);

    for (int i = 0; i < gui_ShortCutEventMax; i++)
    {
        gui_ShortCutEvent event = static_cast<gui_ShortCutEvent>(i);
        const char* name = gui_event_get_name(event);

        constexpr int MAX_SHORTCUT_NAME = 32;
        char shortcut[MAX_SHORTCUT_NAME];
        gui_event_get_shortcut_string(shortcut, sizeof(shortcut), event);

        write_string("Shortcuts", name, shortcut);
    }

    if (config_ini_file->write(config_ini_data, true))
    {
        Debug("Settings saved");
    }
}

static bool check_portable(void)
{
    char* base_path;
    char portable_file_path[260];
    
    base_path = SDL_GetBasePath();
    
    strcpy(portable_file_path, base_path);
    strcat(portable_file_path, "portable.ini");

    FILE* file = fopen(portable_file_path, "r");
    
    if (IsValidPointer(file))
    {
        fclose(file);
        return true;
    }

    return false;
}

static SDL_Keymod string_to_kmod(const std::string& value)
{
    for (uint32_t i = 0; i < SDL_arraysize(modifiers_map); i++)
    {
        const KModMap& map = modifiers_map[i];
        if (!SDL_strcasecmp(map.keymodName, value.c_str()))
        {
            return map.keymod;
        }
    }

    return KMOD_NONE;
}

static config_Key read_shortcut_key(const char* group, const char* key, config_Key default_value)
{
    config_Key ret = { SDL_SCANCODE_UNKNOWN, KMOD_NONE };

    std::string value = config_ini_data[group][key];
    if(value.empty())
    {
        ret = default_value;
    }
    else
    {
        // Parse any modifiers.
        const char* modifier = nullptr;
        const char* key = value.c_str();

        for (long unsigned int i = 0; i < SDL_arraysize(config_modifiers); i++)
        {
            const char* mod = config_modifiers[i];
            size_t modLen = strlen(mod);

            // If shortcut starts with "<Mod>+" then it is a valid modifier.
            if (!strncmp(key, mod, modLen) && key[modLen] == '+')
            {
                modifier = mod;
                break;
            }
        }

        if (modifier != nullptr)
        {
            key += strlen(modifier) + 1;
        }

        ret.scancode = SDL_GetScancodeFromName(key);
        if(modifier)
            ret.modifier = string_to_kmod(modifier);
    }

    return ret;
}

static int read_int(const char* group, const char* key, int default_value)
{
    int ret = 0;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = std::stoi(value);

    Debug("Load setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Debug("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static float read_float(const char* group, const char* key, float default_value)
{
    float ret = 0.0f;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = strtof(value.c_str(), NULL);

    Debug("Load setting: [%s][%s]=%.2f", group, key, ret);
    return ret;
}

static void write_float(const char* group, const char* key, float value)
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::fixed << std::setprecision(2) << value;
    std::string value_str = oss.str();
    config_ini_data[group][key] = oss.str();
    Debug("Save float setting: [%s][%s]=%s", group, key, value_str.c_str());
}

static bool read_bool(const char* group, const char* key, bool default_value)
{
    bool ret;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        std::istringstream(value) >> std::boolalpha >> ret;

    Debug("Load setting: [%s][%s]=%s", group, key, ret ? "true" : "false");
    return ret;
}

static void write_bool(const char* group, const char* key, bool boolean)
{
    std::stringstream converter;
    converter << std::boolalpha << boolean;
    std::string value;
    value = converter.str();
    config_ini_data[group][key] = value;
    Debug("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static std::string read_string(const char* group, const char* key)
{
    std::string ret = config_ini_data[group][key];
    Debug("Load setting: [%s][%s]=%s", group, key, ret.c_str());
    return ret;
}

static void write_string(const char* group, const char* key, std::string value)
{
    config_ini_data[group][key] = value;
    Debug("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

