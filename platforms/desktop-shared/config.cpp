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
#include "../../src/gearsystem.h"
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"

#define CONFIG_IMPORT
#include "config.h"

static int read_int(const char* group, const char* key, int default_value);
static void write_int(const char* group, const char* key, int integer);
static bool read_bool(const char* group, const char* key, bool default_value);
static void write_bool(const char* group, const char* key, bool boolean);
static std::string read_string(const char* group, const char* key);
static void write_string(const char* group, const char* key, std::string value);

void config_init(void)
{
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

    config_emulator.save_slot = read_int("Emulator", "SaveSlot", 0);
    config_emulator.start_paused = read_bool("Emulator", "StartPaused", false);
    config_emulator.save_in_rom_folder = read_bool("Emulator", "SaveInROMFolder", false);
    config_emulator.system = read_int("Emulator", "System", 0);
    config_emulator.zone = read_int("Emulator", "Zone", 0);
    config_emulator.mapper = read_int("Emulator", "Mapper", 0);
    config_emulator.region = read_int("Emulator", "Region", 0);

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        config_emulator.recent_roms[i] = read_string("Emulator", item.c_str());
    }

    config_video.scale = read_int("Video", "Scale", 0);
    config_video.ratio = read_int("Video", "AspectRatio", 0);
    config_video.fps = read_bool("Video", "FPS", false);
    config_video.bilinear = read_bool("Video", "Bilinear", false);
    config_video.mix_frames = read_bool("Video", "MixFrames", true);
    config_video.scanlines = read_bool("Video", "Scanlines", true);
    config_video.sync = read_bool("Video", "Sync", true);
    
    config_audio.enable = read_bool("Audio", "Enable", true);
    config_audio.sync = read_bool("Audio", "Sync", true);

    config_input[0].key_left = (SDL_Scancode)read_int("InputA", "KeyLeft", SDL_SCANCODE_LEFT);
    config_input[0].key_right = (SDL_Scancode)read_int("InputA", "KeyRight", SDL_SCANCODE_RIGHT);
    config_input[0].key_up = (SDL_Scancode)read_int("InputA", "KeyUp", SDL_SCANCODE_UP);
    config_input[0].key_down = (SDL_Scancode)read_int("InputA", "KeyDown", SDL_SCANCODE_DOWN);
    config_input[0].key_1 = (SDL_Scancode)read_int("InputA", "Key1", SDL_SCANCODE_A);
    config_input[0].key_2 = (SDL_Scancode)read_int("InputA", "Key2", SDL_SCANCODE_S);
    config_input[0].key_start = (SDL_Scancode)read_int("InputA", "KeyStart", SDL_SCANCODE_RETURN);

    config_input[0].gamepad = read_bool("InputA", "Gamepad", true);
    config_input[0].gamepad_invert_x_axis = read_bool("InputA", "GamepadInvertX", false);
    config_input[0].gamepad_invert_y_axis = read_bool("InputA", "GamepadInvertY", false);
    config_input[0].gamepad_1 = read_int("InputA", "Gamepad1", 1);
    config_input[0].gamepad_2 = read_int("InputA", "Gamepad2", 2);
    config_input[0].gamepad_start = read_int("InputA", "GamepadStart", 9);
    config_input[0].gamepad_x_axis = read_int("InputA", "GamepadX", 0);
    config_input[0].gamepad_y_axis = read_int("InputA", "GamepadY", 1);

    config_input[1].key_left = (SDL_Scancode)read_int("InputB", "KeyLeft", SDL_SCANCODE_J);
    config_input[1].key_right = (SDL_Scancode)read_int("InputB", "KeyRight", SDL_SCANCODE_L);
    config_input[1].key_up = (SDL_Scancode)read_int("InputB", "KeyUp", SDL_SCANCODE_I);
    config_input[1].key_down = (SDL_Scancode)read_int("InputB", "KeyDown", SDL_SCANCODE_K);
    config_input[1].key_1 = (SDL_Scancode)read_int("InputB", "Key1", SDL_SCANCODE_G);
    config_input[1].key_2 = (SDL_Scancode)read_int("InputB", "Key2", SDL_SCANCODE_H);
    config_input[1].key_start = (SDL_Scancode)read_int("InputB", "KeyStart", SDL_SCANCODE_RSHIFT);

    config_input[1].gamepad = read_bool("InputB", "Gamepad", true);
    config_input[1].gamepad_invert_x_axis = read_bool("InputB", "GamepadInvertX", false);
    config_input[1].gamepad_invert_y_axis = read_bool("InputB", "GamepadInvertY", false);
    config_input[1].gamepad_1 = read_int("InputB", "Gamepad1", 1);
    config_input[1].gamepad_2 = read_int("InputB", "Gamepad2", 2);
    config_input[1].gamepad_start = read_int("InputB", "GamepadStart", 9);
    config_input[1].gamepad_x_axis = read_int("InputB", "GamepadX", 0);
    config_input[1].gamepad_y_axis = read_int("InputB", "GamepadY", 1);

    Log("Settings loaded");
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    write_int("Emulator", "SaveSlot", config_emulator.save_slot);
    write_bool("Emulator", "StartPaused", config_emulator.start_paused);
    write_bool("Emulator", "SaveInROMFolder", config_emulator.save_in_rom_folder);
    write_int("Emulator", "System", config_emulator.system);
    write_int("Emulator", "Zone", config_emulator.zone);
    write_int("Emulator", "Mapper", config_emulator.mapper);
    write_int("Emulator", "Region", config_emulator.region);

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        write_string("Emulator", item.c_str(), config_emulator.recent_roms[i]);
    }

    write_int("Video", "Scale", config_video.scale);
    write_int("Video", "AspectRatio", config_video.ratio);
    write_bool("Video", "FPS", config_video.fps);
    write_bool("Video", "Bilinear", config_video.bilinear);
    write_bool("Video", "MixFrames", config_video.mix_frames);
    write_bool("Video", "Scanlines", config_video.scanlines);
    write_bool("Video", "Sync", config_video.sync);

    write_bool("Audio", "Enable", config_audio.enable);
    write_bool("Audio", "Sync", config_audio.sync);

    write_int("InputA", "KeyLeft", config_input[0].key_left);
    write_int("InputA", "KeyRight", config_input[0].key_right);
    write_int("InputA", "KeyUp", config_input[0].key_up);
    write_int("InputA", "KeyDown", config_input[0].key_down);
    write_int("InputA", "Key1", config_input[0].key_1);
    write_int("InputA", "Key2", config_input[0].key_2);
    write_int("InputA", "KeyStart", config_input[0].key_start);

    write_bool("InputA", "Gamepad", config_input[0].gamepad);
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
    write_bool("InputB", "GamepadInvertX", config_input[1].gamepad_invert_x_axis);
    write_bool("InputB", "GamepadInvertY", config_input[1].gamepad_invert_y_axis);
    write_int("InputB", "Gamepad1", config_input[1].gamepad_1);
    write_int("InputB", "Gamepad2", config_input[1].gamepad_2);
    write_int("InputB", "GamepadStart", config_input[1].gamepad_start);
    write_int("InputB", "GamepadX", config_input[1].gamepad_x_axis);
    write_int("InputB", "GamepadY", config_input[1].gamepad_y_axis);

    if (config_ini_file->write(config_ini_data, true))
    {
        Log("Settings saved");
    }
}

static int read_int(const char* group, const char* key, int default_value)
{
    int ret = 0;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = std::stoi(value);

    Log("Load setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static bool read_bool(const char* group, const char* key, bool default_value)
{
    bool ret;
    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        std::istringstream(value) >> std::boolalpha >> ret;

    Log("Load setting: [%s][%s]=%s", group, key, ret ? "true" : "false");
    return ret;
}

static void write_bool(const char* group, const char* key, bool boolean)
{
    std::stringstream converter;
    converter << std::boolalpha << boolean;
    std::string value;
    value = converter.str();
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static std::string read_string(const char* group, const char* key)
{
    std::string ret = config_ini_data[group][key];
    Log("Load setting: [%s][%s]=%s", group, key, ret.c_str());
    return ret;
}

static void write_string(const char* group, const char* key, std::string value)
{
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}