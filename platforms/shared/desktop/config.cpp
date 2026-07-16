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

#include <SDL3/SDL.h>
#include <iomanip>
#include <string>
#include "gearsystem.h"

#define MINI_CASE_SENSITIVE
#include "ini.h"

#define CONFIG_IMPORT
#include "config.h"
#include "shader_preset.h"
#include "utils.h"

static char* get_portable_path(void);
static bool check_portable(const char* base_path);
static int read_int(const char* group, const char* key, int default_value);
static void write_int(const char* group, const char* key, int integer);
static float read_float(const char* group, const char* key, float default_value);
static void write_float(const char* group, const char* key, float value);
static bool read_bool(const char* group, const char* key, bool default_value);
static void write_bool(const char* group, const char* key, bool boolean);
static std::string read_string(const char* group, const char* key);
static void write_string(const char* group, const char* key, const std::string& value);
static config_Hotkey read_hotkey(const char* group, const char* key, config_Hotkey default_value);
static void write_hotkey(const char* group, const char* key, config_Hotkey hotkey);
static config_Hotkey make_hotkey(SDL_Scancode key, SDL_Keymod mod);
static std::string shader_preset_section_name(const char* preset_file);
static bool parse_float_string(const std::string& value, float* result);
static void sync_shader_preset_parameter_defaults(void);
static void set_defaults(void);

static void set_defaults(void)
{
    config_emulator = config_Emulator();
    config_video = config_Video();
    config_audio = config_Audio();
    config_rewind = config_Rewind();
    config_input[0] = config_Input();
    config_input[1] = config_Input();
    config_debug = config_Debug();

    config_input[0].key_left = SDL_SCANCODE_LEFT;
    config_input[0].key_right = SDL_SCANCODE_RIGHT;
    config_input[0].key_up = SDL_SCANCODE_UP;
    config_input[0].key_down = SDL_SCANCODE_DOWN;
    config_input[0].key_1 = SDL_SCANCODE_A;
    config_input[0].key_2 = SDL_SCANCODE_S;
    config_input[0].key_start = SDL_SCANCODE_RETURN;
    config_input[0].key_reset = SDL_SCANCODE_UNKNOWN;
    config_input[0].gamepad = true;
    config_input[0].gamepad_invert_x_axis = false;
    config_input[0].gamepad_invert_y_axis = false;
    config_input[0].gamepad_1 = 1;
    config_input[0].gamepad_2 = 2;
    config_input[0].gamepad_start = 9;
    config_input[0].gamepad_reset = SDL_GAMEPAD_BUTTON_INVALID;
    config_input[0].gamepad_x_axis = 0;
    config_input[0].gamepad_y_axis = 1;
    config_input[1].key_left = SDL_SCANCODE_J;
    config_input[1].key_right = SDL_SCANCODE_L;
    config_input[1].key_up = SDL_SCANCODE_I;
    config_input[1].key_down = SDL_SCANCODE_K;
    config_input[1].key_1 = SDL_SCANCODE_G;
    config_input[1].key_2 = SDL_SCANCODE_H;
    config_input[1].key_start = SDL_SCANCODE_RSHIFT;
    config_input[1].key_reset = SDL_SCANCODE_UNKNOWN;
    config_input[1].gamepad = true;
    config_input[1].gamepad_invert_x_axis = false;
    config_input[1].gamepad_invert_y_axis = false;
    config_input[1].gamepad_1 = 1;
    config_input[1].gamepad_2 = 2;
    config_input[1].gamepad_start = 9;
    config_input[1].gamepad_reset = SDL_GAMEPAD_BUTTON_INVALID;
    config_input[1].gamepad_x_axis = 0;
    config_input[1].gamepad_y_axis = 1;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < config_HotkeyIndex_COUNT; j++)
        {
            config_input_gamepad_shortcuts[i].gamepad_shortcuts[j] = SDL_GAMEPAD_BUTTON_INVALID;
        }
    }

    config_hotkeys[config_HotkeyIndex_OpenROM] = make_hotkey(SDL_SCANCODE_O, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_ReloadROM] = make_hotkey(SDL_SCANCODE_D, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Quit] = make_hotkey(SDL_SCANCODE_Q, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Reset] = make_hotkey(SDL_SCANCODE_R, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Pause] = make_hotkey(SDL_SCANCODE_P, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_FFWD] = make_hotkey(SDL_SCANCODE_F, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Rewind] = make_hotkey(SDL_SCANCODE_BACKSPACE, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_SaveState] = make_hotkey(SDL_SCANCODE_S, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_LoadState] = make_hotkey(SDL_SCANCODE_L, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Screenshot] = make_hotkey(SDL_SCANCODE_X, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Fullscreen] = make_hotkey(SDL_SCANCODE_F12, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_ShowMainMenu] = make_hotkey(SDL_SCANCODE_M, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_CaptureMouse] = make_hotkey(SDL_SCANCODE_F1, (SDL_Keymod)0);
    config_hotkeys[config_HotkeyIndex_DebugStepInto] = make_hotkey(SDL_SCANCODE_F11, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugStepOver] = make_hotkey(SDL_SCANCODE_F10, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugStepOut] = make_hotkey(SDL_SCANCODE_F11, SDL_KMOD_SHIFT);
    config_hotkeys[config_HotkeyIndex_DebugStepFrame] = make_hotkey(SDL_SCANCODE_F6, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugContinue] = make_hotkey(SDL_SCANCODE_F5, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugBreak] = make_hotkey(SDL_SCANCODE_F7, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugRunToCursor] = make_hotkey(SDL_SCANCODE_F8, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugBreakpoint] = make_hotkey(SDL_SCANCODE_F9, SDL_KMOD_NONE);
    config_hotkeys[config_HotkeyIndex_DebugGoBack] = make_hotkey(SDL_SCANCODE_BACKSPACE, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_SelectSlot1] = make_hotkey(SDL_SCANCODE_1, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_SelectSlot2] = make_hotkey(SDL_SCANCODE_2, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_SelectSlot3] = make_hotkey(SDL_SCANCODE_3, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_SelectSlot4] = make_hotkey(SDL_SCANCODE_4, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_SelectSlot5] = make_hotkey(SDL_SCANCODE_5, SDL_KMOD_CTRL);
    config_hotkeys[config_HotkeyIndex_Mute] = make_hotkey(SDL_SCANCODE_U, SDL_KMOD_CTRL);
}

void config_init(void)
{
    const char* root_path = NULL;
    char* portable_path = get_portable_path();

    if (portable_path)
        root_path = portable_path;
    else
        root_path = SDL_GetPrefPath("Geardome", GS_TITLE);

    if (root_path == NULL)
    {
        Log("Unable to determine config path. Falling back to current directory.");
        root_path = SDL_strdup("./");
    }

    config_root_path = root_path;

    strncpy_fit(config_emu_file_path, config_root_path, sizeof(config_emu_file_path));
    strncat_fit(config_emu_file_path, "config.ini", sizeof(config_emu_file_path));

    strncpy_fit(config_imgui_file_path, config_root_path, sizeof(config_imgui_file_path));
    strncat_fit(config_imgui_file_path, "imgui.ini", sizeof(config_imgui_file_path));

    set_defaults();

    config_ini_file = new mINI::INIFile(config_emu_file_path);
}

void config_destroy(void)
{
    SafeDelete(config_ini_file);
    SDL_free((void*)config_root_path);
}

void config_load_defaults(void)
{
    Log("Loading default settings");

    set_defaults();
    config_write();
}

void config_push_recent_media(const std::string& path)
{
    if (path.empty())
        return;

    int slot = 0;
    for (slot = 0; slot < config_max_recent_roms; slot++)
    {
        if (config_emulator.recent_roms[slot].compare(path) == 0)
            break;
    }

    if (slot >= config_max_recent_roms)
        slot = config_max_recent_roms - 1;

    for (int index = slot; index > 0; index--)
    {
        config_emulator.recent_roms[index] = config_emulator.recent_roms[index - 1];
    }

    config_emulator.recent_roms[0] = path;
}

void config_read(void)
{
    if (!config_ini_file->read(config_ini_data))
    {
        Log("Unable to load settings from %s", config_emu_file_path);
        return;
    }

    int file_version = read_int("General", "Version", 0);

    if (file_version < 2)
    {
        Log("Settings version %d is outdated (current: %d). Using defaults.", file_version, config_version);
        config_write();
        return;
    }

    if (file_version < config_version)
        Log("Migrating settings version %d to %d", file_version, config_version);

    Log("Loading settings from %s (version %d)", config_emu_file_path, file_version);

#if defined(GS_DISABLE_DISASSEMBLER)
        config_debug.debug = false;
#else
        config_debug.debug = read_bool("Debug", "Debug", false);
#endif
    config_debug.show_disassembler = read_bool("Debug", "Disassembler", true);
    config_debug.show_screen = read_bool("Debug", "Screen", true);
    config_debug.show_memory = read_bool("Debug", "Memory", false);
    config_debug.show_processor = read_bool("Debug", "Processor", true);
    config_debug.show_call_stack = read_bool("Debug", "CallStack", false);
    config_debug.show_breakpoints = read_bool("Debug", "Breakpoints", false);
    config_debug.show_symbols = read_bool("Debug", "Symbols", false);
    config_debug.show_video = read_bool("Debug", "Video", false);
    config_debug.show_video_nametable = read_bool("Debug", "VideoNameTable", false);
    config_debug.show_video_tiles = read_bool("Debug", "VideoTiles", false);
    config_debug.show_video_sprites = read_bool("Debug", "VideoSprites", false);
    config_debug.show_video_palettes = read_bool("Debug", "VideoPalettes", false);
    config_debug.show_video_regs = read_bool("Debug", "VideoRegs", false);
    config_debug.show_psg = read_bool("Debug", "PSG", false);
    config_debug.show_ym2413 = read_bool("Debug", "YM2413", false);
    config_debug.show_trace_logger = read_bool("Debug", "TraceLogger", false);
    config_debug.show_rewind = read_bool("Debug", "Rewind", false);
    config_debug.trace_counter = read_bool("Debug", "TraceCounter", true);
    config_debug.trace_bank = read_bool("Debug", "TraceBank", true);
    config_debug.trace_registers = read_bool("Debug", "TraceRegisters", true);
    config_debug.trace_flags = read_bool("Debug", "TraceFlags", true);
    config_debug.trace_bytes = read_bool("Debug", "TraceBytes", true);
    config_debug.trace_cpu = read_bool("Debug", "TraceCpu", true);
    config_debug.trace_cpu_irq = read_bool("Debug", "TraceCpuIrq", true);
    config_debug.trace_vdp_write = read_bool("Debug", "TraceVdpWrite", true);
    config_debug.trace_vdp_status = read_bool("Debug", "TraceVdpStatus", true);
    config_debug.trace_psg = read_bool("Debug", "TracePsg", true);
    config_debug.trace_ym2413 = read_bool("Debug", "TraceYm2413", true);
    config_debug.trace_io_port = read_bool("Debug", "TraceIoPort", true);
    config_debug.trace_bank_switch = read_bool("Debug", "TraceBankSwitch", true);
    config_debug.dis_show_mem = read_bool("Debug", "DisMem", true);
    config_debug.dis_show_symbols = read_bool("Debug", "DisSymbols", true);
    config_debug.dis_show_segment = read_bool("Debug", "DisSegment", true);
    config_debug.dis_show_bank = read_bool("Debug", "DisBank", true);
    config_debug.dis_show_auto_symbols = read_bool("Debug", "DisAutoSymbols", true);
    config_debug.dis_dim_auto_symbols = read_bool("Debug", "DisDimAutoSymbols", false);
    config_debug.dis_replace_symbols = read_bool("Debug", "DisReplaceSymbols", true);
    config_debug.dis_replace_labels = read_bool("Debug", "DisReplaceLabels", true);
    config_debug.dis_syntax = read_int("Debug", "DisSyntax", GS_Disassembler_Syntax_Gearsystem);
    config_debug.dis_syntax = CLAMP(config_debug.dis_syntax, GS_Disassembler_Syntax_Gearsystem, GS_Disassembler_Syntax_Count - 1);
    config_debug.dis_look_ahead_count = read_int("Debug", "DisLookAheadCount", 20);
    config_debug.font_size = read_int("Debug", "FontSize", 0);
    config_debug.scale = read_int("Debug", "Scale", 2);
    config_debug.multi_viewport = read_bool("Debug", "MultiViewport", false);
    config_debug.single_instance = read_bool("Debug", "SingleInstance", false);
    config_debug.auto_debug_settings = read_bool("Debug", "AutoDebugSettings", false);

    for (int i = 0; i < config_memory_editor_count; i++)
    {
        std::string section = "MemEditor_" + std::to_string(i);
        config_debug.mem_editor_bytes_per_row[i] = read_int(section.c_str(), "BytesPerRow", 16);
        config_debug.mem_editor_preview_data_type[i] = read_int(section.c_str(), "PreviewDataType", 0);
        config_debug.mem_editor_preview_endianess[i] = read_int(section.c_str(), "PreviewEndianess", 0);
        config_debug.mem_editor_uppercase_hex[i] = read_bool(section.c_str(), "UppercaseHex", true);
        config_debug.mem_editor_gray_out_zeros[i] = read_bool(section.c_str(), "GrayOutZeros", true);
    }

    config_emulator.maximized = read_bool("Emulator", "Maximized", false);
    config_emulator.fullscreen = read_bool("Emulator", "FullScreen", false);
    config_emulator.fullscreen_mode = read_int("Emulator", "FullScreenMode", 0);
    config_emulator.always_show_menu = read_bool("Emulator", "AlwaysShowMenu", false);
    config_emulator.theme = read_int("Emulator", "Theme", config_Theme_Dark);
    config_emulator.theme = CLAMP(config_emulator.theme, config_Theme_Light, config_Theme_Dark);
    config_emulator.ffwd_speed = read_int("Emulator", "FFWD", 1);
    config_emulator.runahead = read_int("Emulator", "RunAhead", 0);
    config_emulator.runahead = CLAMP(config_emulator.runahead, 0, 3);
    config_emulator.save_slot = read_int("Emulator", "SaveSlot", 0);
    config_emulator.save_slot = CLAMP(config_emulator.save_slot, 0, 4);
    config_emulator.start_paused = read_bool("Emulator", "StartPaused", false);
    config_emulator.pause_when_inactive = read_bool("Emulator", "PauseWhenInactive", true);
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
    config_emulator.screenshots_dir_option = read_int("Emulator", "ScreenshotDirOption", 0);
    config_emulator.screenshots_path = read_string("Emulator", "ScreenshotPath");
    config_emulator.last_open_path = read_string("Emulator", "LastOpenPath");
    config_emulator.window_width = read_int("Emulator", "WindowWidth", 770);
    config_emulator.window_height = read_int("Emulator", "WindowHeight", 600);
    config_emulator.status_messages = read_bool("Emulator", "StatusMessages", false);
    config_emulator.allow_screensaver = read_bool("Emulator", "AllowScreenSaver", false);
    config_emulator.mcp_tcp_port = read_int("Emulator", "MCPTCPPort", 7777);
    config_emulator.mcp_http_address = read_string("Emulator", "MCPHTTPAddress");
    if (config_emulator.mcp_http_address.empty())
        config_emulator.mcp_http_address = "127.0.0.1";
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
    if (config_emulator.screenshots_path.empty())
    {
        config_emulator.screenshots_path = config_root_path;
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
    config_video.shader_mode = read_int("Video", "ShaderMode", config_ShaderMode_PixelPerfect);
    config_video.shader_mode = CLAMP(config_video.shader_mode, config_ShaderMode_PixelPerfect, config_ShaderMode_External);
    config_video.shader_preset_path = read_string("Video", "ShaderPresetFile");
    config_video.sync_mode = read_int("Video", "SyncMode", -1);
    if ((file_version < config_version) || (config_video.sync_mode < config_VideoSync_Disabled) || (config_video.sync_mode > config_VideoSync_VRR))
    {
        bool sync = read_bool("Video", "Sync", true);
        bool vrr = read_bool("Video", "VRR", false);
        config_video.sync_mode = sync ? (vrr ? config_VideoSync_VRR : config_VideoSync_Fixed) : config_VideoSync_Disabled;
    }
    else
        config_video.sync_mode = CLAMP(config_video.sync_mode, config_VideoSync_Disabled, config_VideoSync_VRR);
#if !defined(_WIN32)
    if (config_video.sync_mode == config_VideoSync_VRR)
    config_video.sync_mode = config_VideoSync_Fixed;
#endif
    config_video.sprite_limit = read_bool("Video", "SpriteLimit", false);
    config_video.background_color[config_Theme_Dark][0] = read_float("Video", "BackgroundColorR", 0.1f);
    config_video.background_color[config_Theme_Dark][1] = read_float("Video", "BackgroundColorG", 0.1f);
    config_video.background_color[config_Theme_Dark][2] = read_float("Video", "BackgroundColorB", 0.1f);
    config_video.background_color_debugger[config_Theme_Dark][0] = read_float("Video", "BackgroundColorDebuggerR", 0.2f);
    config_video.background_color_debugger[config_Theme_Dark][1] = read_float("Video", "BackgroundColorDebuggerG", 0.2f);
    config_video.background_color_debugger[config_Theme_Dark][2] = read_float("Video", "BackgroundColorDebuggerB", 0.2f);
    config_video.background_color[config_Theme_Light][0] = read_float("Video", "BackgroundColorLightR", 128.0f / 255.0f);
    config_video.background_color[config_Theme_Light][1] = read_float("Video", "BackgroundColorLightG", 128.0f / 255.0f);
    config_video.background_color[config_Theme_Light][2] = read_float("Video", "BackgroundColorLightB", 128.0f / 255.0f);
    config_video.background_color_debugger[config_Theme_Light][0] = read_float("Video", "BackgroundColorDebuggerLightR", 160.0f / 255.0f);
    config_video.background_color_debugger[config_Theme_Light][1] = read_float("Video", "BackgroundColorDebuggerLightG", 160.0f / 255.0f);
    config_video.background_color_debugger[config_Theme_Light][2] = read_float("Video", "BackgroundColorDebuggerLightB", 160.0f / 255.0f);
    config_video.glasses = read_int("Video", "3DGlasses", 0);

    config_audio.enable = read_bool("Audio", "Enable", true);
    config_audio.sync = read_bool("Audio", "Sync", true);
    config_audio.master_volume = read_float("Audio", "MasterVolume", 1.0f);
    config_audio.master_volume = CLAMP(config_audio.master_volume, 0.0f, 2.0f);
    config_audio.psg_volume = read_float("Audio", "PSGVolume", 1.0f);
    config_audio.fm_volume = read_float("Audio", "FMVolume", 1.0f);
    config_audio.ym2413 = read_int("Audio", "YM2413", 0);
    config_audio.buffer_count = read_int("Audio", "BufferCount", 3);

    config_rewind.enabled = read_bool("Rewind", "Enabled", true);
    config_rewind.buffer_seconds = read_int("Rewind", "BufferSeconds", 10);
    config_rewind.frames_per_snapshot = read_int("Rewind", "FramesPerSnapshot", 1);
    config_rewind.speed = read_float("Rewind", "Speed", 2.0f);

    config_input[0].key_left = (SDL_Scancode)read_int("InputA", "KeyLeft", SDL_SCANCODE_LEFT);
    config_input[0].key_right = (SDL_Scancode)read_int("InputA", "KeyRight", SDL_SCANCODE_RIGHT);
    config_input[0].key_up = (SDL_Scancode)read_int("InputA", "KeyUp", SDL_SCANCODE_UP);
    config_input[0].key_down = (SDL_Scancode)read_int("InputA", "KeyDown", SDL_SCANCODE_DOWN);
    config_input[0].key_1 = (SDL_Scancode)read_int("InputA", "Key1", SDL_SCANCODE_A);
    config_input[0].key_2 = (SDL_Scancode)read_int("InputA", "Key2", SDL_SCANCODE_S);
    config_input[0].key_start = (SDL_Scancode)read_int("InputA", "KeyStart", SDL_SCANCODE_RETURN);
    config_input[0].key_reset = (SDL_Scancode)read_int("InputA", "KeyReset", SDL_SCANCODE_UNKNOWN);

    config_input[0].allow_up_down = read_bool("InputA", "AllowUpDown", false);
    config_input[0].gamepad = read_bool("InputA", "Gamepad", true);
    config_input[0].gamepad_directional = read_int("InputA", "GamepadDirectional", 0);
    config_input[0].gamepad_invert_x_axis = read_bool("InputA", "GamepadInvertX", false);
    config_input[0].gamepad_invert_y_axis = read_bool("InputA", "GamepadInvertY", false);
    config_input[0].gamepad_1 = read_int("InputA", "Gamepad1", SDL_GAMEPAD_BUTTON_SOUTH);
    config_input[0].gamepad_2 = read_int("InputA", "Gamepad2", SDL_GAMEPAD_BUTTON_EAST);
    config_input[0].gamepad_start = read_int("InputA", "GamepadStart", SDL_GAMEPAD_BUTTON_START);
    config_input[0].gamepad_reset = read_int("InputA", "GamepadReset", SDL_GAMEPAD_BUTTON_INVALID);
    config_input[0].gamepad_x_axis = read_int("InputA", "GamepadX", SDL_GAMEPAD_AXIS_LEFTX);
    config_input[0].gamepad_y_axis = read_int("InputA", "GamepadY", SDL_GAMEPAD_AXIS_LEFTY);

    config_input[1].key_left = (SDL_Scancode)read_int("InputB", "KeyLeft", SDL_SCANCODE_J);
    config_input[1].key_right = (SDL_Scancode)read_int("InputB", "KeyRight", SDL_SCANCODE_L);
    config_input[1].key_up = (SDL_Scancode)read_int("InputB", "KeyUp", SDL_SCANCODE_I);
    config_input[1].key_down = (SDL_Scancode)read_int("InputB", "KeyDown", SDL_SCANCODE_K);
    config_input[1].key_1 = (SDL_Scancode)read_int("InputB", "Key1", SDL_SCANCODE_G);
    config_input[1].key_2 = (SDL_Scancode)read_int("InputB", "Key2", SDL_SCANCODE_H);
    config_input[1].key_start = (SDL_Scancode)read_int("InputB", "KeyStart", SDL_SCANCODE_RSHIFT);
    config_input[1].key_reset = (SDL_Scancode)read_int("InputB", "KeyReset", SDL_SCANCODE_UNKNOWN);

    config_input[1].allow_up_down = read_bool("InputB", "AllowUpDown", false);
    config_input[1].gamepad = read_bool("InputB", "Gamepad", true);
    config_input[1].gamepad_directional = read_int("InputB", "GamepadDirectional", 0);
    config_input[1].gamepad_invert_x_axis = read_bool("InputB", "GamepadInvertX", false);
    config_input[1].gamepad_invert_y_axis = read_bool("InputB", "GamepadInvertY", false);
    config_input[1].gamepad_1 = read_int("InputB", "Gamepad1", SDL_GAMEPAD_BUTTON_SOUTH);
    config_input[1].gamepad_2 = read_int("InputB", "Gamepad2", SDL_GAMEPAD_BUTTON_EAST);
    config_input[1].gamepad_start = read_int("InputB", "GamepadStart", SDL_GAMEPAD_BUTTON_START);
    config_input[1].gamepad_reset = read_int("InputB", "GamepadReset", SDL_GAMEPAD_BUTTON_INVALID);
    config_input[1].gamepad_x_axis = read_int("InputB", "GamepadX", SDL_GAMEPAD_AXIS_LEFTX);
    config_input[1].gamepad_y_axis = read_int("InputB", "GamepadY", SDL_GAMEPAD_AXIS_LEFTY);

    for (int i = 0; i < 2; i++)
    {
        char input_group[32];
        snprintf(input_group, sizeof(input_group), "InputGamepadShortcuts%d", i + 1);
        for (int j = 0; j < config_HotkeyIndex_COUNT; j++)
        {
            char key_name[32];
            snprintf(key_name, sizeof(key_name), "Shortcut%d", j);
            config_input_gamepad_shortcuts[i].gamepad_shortcuts[j] = read_int(input_group, key_name, SDL_GAMEPAD_BUTTON_INVALID);
        }
    }

    // Read hotkeys
    config_hotkeys[config_HotkeyIndex_OpenROM] = read_hotkey("Hotkeys", "OpenROM", make_hotkey(SDL_SCANCODE_O, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_ReloadROM] = read_hotkey("Hotkeys", "ReloadROM", make_hotkey(SDL_SCANCODE_D, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Quit] = read_hotkey("Hotkeys", "Quit", make_hotkey(SDL_SCANCODE_Q, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Reset] = read_hotkey("Hotkeys", "Reset", make_hotkey(SDL_SCANCODE_R, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Pause] = read_hotkey("Hotkeys", "Pause", make_hotkey(SDL_SCANCODE_P, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_FFWD] = read_hotkey("Hotkeys", "FFWD", make_hotkey(SDL_SCANCODE_F, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Rewind] = read_hotkey("Hotkeys", "Rewind", make_hotkey(SDL_SCANCODE_BACKSPACE, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_SaveState] = read_hotkey("Hotkeys", "SaveState", make_hotkey(SDL_SCANCODE_S, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_LoadState] = read_hotkey("Hotkeys", "LoadState", make_hotkey(SDL_SCANCODE_L, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Screenshot] = read_hotkey("Hotkeys", "Screenshot", make_hotkey(SDL_SCANCODE_X, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Fullscreen] = read_hotkey("Hotkeys", "Fullscreen", make_hotkey(SDL_SCANCODE_F12, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_CaptureMouse] = read_hotkey("Hotkeys", "CaptureMouse", make_hotkey(SDL_SCANCODE_F1, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_ShowMainMenu] = read_hotkey("Hotkeys", "ShowMainMenu", make_hotkey(SDL_SCANCODE_M, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_DebugStepInto] = read_hotkey("Hotkeys", "DebugStepInto", make_hotkey(SDL_SCANCODE_F11, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugStepOver] = read_hotkey("Hotkeys", "DebugStepOver", make_hotkey(SDL_SCANCODE_F10, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugStepOut] = read_hotkey("Hotkeys", "DebugStepOut", make_hotkey(SDL_SCANCODE_F11, SDL_KMOD_SHIFT));
    config_hotkeys[config_HotkeyIndex_DebugStepFrame] = read_hotkey("Hotkeys", "DebugStepFrame", make_hotkey(SDL_SCANCODE_F6, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugContinue] = read_hotkey("Hotkeys", "DebugContinue", make_hotkey(SDL_SCANCODE_F5, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugBreak] = read_hotkey("Hotkeys", "DebugBreak", make_hotkey(SDL_SCANCODE_F7, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugRunToCursor] = read_hotkey("Hotkeys", "DebugRunToCursor", make_hotkey(SDL_SCANCODE_F8, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugBreakpoint] = read_hotkey("Hotkeys", "DebugBreakpoint", make_hotkey(SDL_SCANCODE_F9, SDL_KMOD_NONE));
    config_hotkeys[config_HotkeyIndex_DebugGoBack] = read_hotkey("Hotkeys", "DebugGoBack", make_hotkey(SDL_SCANCODE_BACKSPACE, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_SelectSlot1] = read_hotkey("Hotkeys", "SelectSlot1", make_hotkey(SDL_SCANCODE_1, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_SelectSlot2] = read_hotkey("Hotkeys", "SelectSlot2", make_hotkey(SDL_SCANCODE_2, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_SelectSlot3] = read_hotkey("Hotkeys", "SelectSlot3", make_hotkey(SDL_SCANCODE_3, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_SelectSlot4] = read_hotkey("Hotkeys", "SelectSlot4", make_hotkey(SDL_SCANCODE_4, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_SelectSlot5] = read_hotkey("Hotkeys", "SelectSlot5", make_hotkey(SDL_SCANCODE_5, SDL_KMOD_CTRL));
    config_hotkeys[config_HotkeyIndex_Mute] = read_hotkey("Hotkeys", "Mute", make_hotkey(SDL_SCANCODE_U, SDL_KMOD_CTRL));

    sync_shader_preset_parameter_defaults();

    Debug("Settings loaded");
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    if (config_emulator.ffwd)
        config_audio.sync = true;

    write_int("General", "Version", config_version);

    write_bool("Debug", "Debug", config_debug.debug);
    write_bool("Debug", "Disassembler", config_debug.show_disassembler);
    write_bool("Debug", "Screen", config_debug.show_screen);
    write_bool("Debug", "Memory", config_debug.show_memory);
    write_bool("Debug", "Processor", config_debug.show_processor);
    write_bool("Debug", "CallStack", config_debug.show_call_stack);
    write_bool("Debug", "Breakpoints", config_debug.show_breakpoints);
    write_bool("Debug", "Symbols", config_debug.show_symbols);
    write_bool("Debug", "Video", config_debug.show_video);
    write_bool("Debug", "VideoNameTable", config_debug.show_video_nametable);
    write_bool("Debug", "VideoTiles", config_debug.show_video_tiles);
    write_bool("Debug", "VideoSprites", config_debug.show_video_sprites);
    write_bool("Debug", "VideoPalettes", config_debug.show_video_palettes);
    write_bool("Debug", "VideoRegs", config_debug.show_video_regs);
    write_bool("Debug", "PSG", config_debug.show_psg);
    write_bool("Debug", "YM2413", config_debug.show_ym2413);
    write_bool("Debug", "TraceLogger", config_debug.show_trace_logger);
    write_bool("Debug", "Rewind", config_debug.show_rewind);
    write_bool("Debug", "TraceCounter", config_debug.trace_counter);
    write_bool("Debug", "TraceBank", config_debug.trace_bank);
    write_bool("Debug", "TraceRegisters", config_debug.trace_registers);
    write_bool("Debug", "TraceFlags", config_debug.trace_flags);
    write_bool("Debug", "TraceBytes", config_debug.trace_bytes);
    write_bool("Debug", "TraceCpu", config_debug.trace_cpu);
    write_bool("Debug", "TraceCpuIrq", config_debug.trace_cpu_irq);
    write_bool("Debug", "TraceVdpWrite", config_debug.trace_vdp_write);
    write_bool("Debug", "TraceVdpStatus", config_debug.trace_vdp_status);
    write_bool("Debug", "TracePsg", config_debug.trace_psg);
    write_bool("Debug", "TraceYm2413", config_debug.trace_ym2413);
    write_bool("Debug", "TraceIoPort", config_debug.trace_io_port);
    write_bool("Debug", "TraceBankSwitch", config_debug.trace_bank_switch);
    write_bool("Debug", "DisMem", config_debug.dis_show_mem);
    write_bool("Debug", "DisSymbols", config_debug.dis_show_symbols);
    write_bool("Debug", "DisSegment", config_debug.dis_show_segment);
    write_bool("Debug", "DisBank", config_debug.dis_show_bank);
    write_bool("Debug", "DisAutoSymbols", config_debug.dis_show_auto_symbols);
    write_bool("Debug", "DisDimAutoSymbols", config_debug.dis_dim_auto_symbols);
    write_bool("Debug", "DisReplaceSymbols", config_debug.dis_replace_symbols);
    write_bool("Debug", "DisReplaceLabels", config_debug.dis_replace_labels);
    write_int("Debug", "DisSyntax", config_debug.dis_syntax);
    write_int("Debug", "DisLookAheadCount", config_debug.dis_look_ahead_count);
    write_int("Debug", "FontSize", config_debug.font_size);
    write_int("Debug", "Scale", config_debug.scale);
    write_bool("Debug", "MultiViewport", config_debug.multi_viewport);
    write_bool("Debug", "SingleInstance", config_debug.single_instance);
    write_bool("Debug", "AutoDebugSettings", config_debug.auto_debug_settings);

    for (int i = 0; i < config_memory_editor_count; i++)
    {
        std::string section = "MemEditor_" + std::to_string(i);
        write_int(section.c_str(), "BytesPerRow", config_debug.mem_editor_bytes_per_row[i]);
        write_int(section.c_str(), "PreviewDataType", config_debug.mem_editor_preview_data_type[i]);
        write_int(section.c_str(), "PreviewEndianess", config_debug.mem_editor_preview_endianess[i]);
        write_bool(section.c_str(), "UppercaseHex", config_debug.mem_editor_uppercase_hex[i]);
        write_bool(section.c_str(), "GrayOutZeros", config_debug.mem_editor_gray_out_zeros[i]);
    }

    write_bool("Emulator", "Maximized", config_emulator.maximized);
    write_bool("Emulator", "FullScreen", config_emulator.fullscreen);
    write_int("Emulator", "FullScreenMode", config_emulator.fullscreen_mode);
    write_bool("Emulator", "AlwaysShowMenu", config_emulator.always_show_menu);
    write_int("Emulator", "Theme", config_emulator.theme);
    write_int("Emulator", "FFWD", config_emulator.ffwd_speed);
    write_int("Emulator", "RunAhead", config_emulator.runahead);
    write_int("Emulator", "SaveSlot", config_emulator.save_slot);
    write_bool("Emulator", "StartPaused", config_emulator.start_paused);
    write_bool("Emulator", "PauseWhenInactive", config_emulator.pause_when_inactive);
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
    write_int("Emulator", "ScreenshotDirOption", config_emulator.screenshots_dir_option);
    write_string("Emulator", "ScreenshotPath", config_emulator.screenshots_path);
    write_string("Emulator", "LastOpenPath", config_emulator.last_open_path);
    write_int("Emulator", "WindowWidth", config_emulator.window_width);
    write_int("Emulator", "WindowHeight", config_emulator.window_height);
    write_bool("Emulator", "StatusMessages", config_emulator.status_messages);
    write_bool("Emulator", "AllowScreenSaver", config_emulator.allow_screensaver);
    write_int("Emulator", "MCPTCPPort", config_emulator.mcp_tcp_port);
    write_string("Emulator", "MCPHTTPAddress", config_emulator.mcp_http_address);
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
    write_int("Video", "ShaderMode", config_video.shader_mode);
    write_string("Video", "ShaderPresetFile", get_filename(config_video.shader_preset_path.c_str()));
    sync_shader_preset_parameter_defaults();
    write_int("Video", "SyncMode", config_video.sync_mode);
    write_bool("Video", "SpriteLimit", config_video.sprite_limit);
    write_float("Video", "BackgroundColorR", config_video.background_color[config_Theme_Dark][0]);
    write_float("Video", "BackgroundColorG", config_video.background_color[config_Theme_Dark][1]);
    write_float("Video", "BackgroundColorB", config_video.background_color[config_Theme_Dark][2]);
    write_float("Video", "BackgroundColorDebuggerR", config_video.background_color_debugger[config_Theme_Dark][0]);
    write_float("Video", "BackgroundColorDebuggerG", config_video.background_color_debugger[config_Theme_Dark][1]);
    write_float("Video", "BackgroundColorDebuggerB", config_video.background_color_debugger[config_Theme_Dark][2]);
    write_float("Video", "BackgroundColorLightR", config_video.background_color[config_Theme_Light][0]);
    write_float("Video", "BackgroundColorLightG", config_video.background_color[config_Theme_Light][1]);
    write_float("Video", "BackgroundColorLightB", config_video.background_color[config_Theme_Light][2]);
    write_float("Video", "BackgroundColorDebuggerLightR", config_video.background_color_debugger[config_Theme_Light][0]);
    write_float("Video", "BackgroundColorDebuggerLightG", config_video.background_color_debugger[config_Theme_Light][1]);
    write_float("Video", "BackgroundColorDebuggerLightB", config_video.background_color_debugger[config_Theme_Light][2]);
    write_int("Video", "3DGlasses", config_video.glasses);

    write_bool("Audio", "Enable", config_audio.enable);
    write_bool("Audio", "Sync", config_audio.sync);
    write_float("Audio", "MasterVolume", config_audio.master_volume);
    write_float("Audio", "PSGVolume", config_audio.psg_volume);
    write_float("Audio", "FMVolume", config_audio.fm_volume);
    write_int("Audio", "YM2413", config_audio.ym2413);
    write_int("Audio", "BufferCount", config_audio.buffer_count);

    write_bool("Rewind", "Enabled", config_rewind.enabled);
    write_int("Rewind", "BufferSeconds", config_rewind.buffer_seconds);
    write_int("Rewind", "FramesPerSnapshot", config_rewind.frames_per_snapshot);
    write_float("Rewind", "Speed", config_rewind.speed);

    write_int("InputA", "KeyLeft", config_input[0].key_left);
    write_int("InputA", "KeyRight", config_input[0].key_right);
    write_int("InputA", "KeyUp", config_input[0].key_up);
    write_int("InputA", "KeyDown", config_input[0].key_down);
    write_int("InputA", "Key1", config_input[0].key_1);
    write_int("InputA", "Key2", config_input[0].key_2);
    write_int("InputA", "KeyStart", config_input[0].key_start);
    write_int("InputA", "KeyReset", config_input[0].key_reset);

    write_bool("InputA", "AllowUpDown", config_input[0].allow_up_down);
    write_bool("InputA", "Gamepad", config_input[0].gamepad);
    write_int("InputA", "GamepadDirectional", config_input[0].gamepad_directional);
    write_bool("InputA", "GamepadInvertX", config_input[0].gamepad_invert_x_axis);
    write_bool("InputA", "GamepadInvertY", config_input[0].gamepad_invert_y_axis);
    write_int("InputA", "Gamepad1", config_input[0].gamepad_1);
    write_int("InputA", "Gamepad2", config_input[0].gamepad_2);
    write_int("InputA", "GamepadStart", config_input[0].gamepad_start);
    write_int("InputA", "GamepadReset", config_input[0].gamepad_reset);
    write_int("InputA", "GamepadX", config_input[0].gamepad_x_axis);
    write_int("InputA", "GamepadY", config_input[0].gamepad_y_axis);

    write_int("InputB", "KeyLeft", config_input[1].key_left);
    write_int("InputB", "KeyRight", config_input[1].key_right);
    write_int("InputB", "KeyUp", config_input[1].key_up);
    write_int("InputB", "KeyDown", config_input[1].key_down);
    write_int("InputB", "Key1", config_input[1].key_1);
    write_int("InputB", "Key2", config_input[1].key_2);
    write_int("InputB", "KeyStart", config_input[1].key_start);
    write_int("InputB", "KeyReset", config_input[1].key_reset);

    write_bool("InputB", "AllowUpDown", config_input[1].allow_up_down);
    write_bool("InputB", "Gamepad", config_input[1].gamepad);
    write_int("InputB", "GamepadDirectional", config_input[1].gamepad_directional);
    write_bool("InputB", "GamepadInvertX", config_input[1].gamepad_invert_x_axis);
    write_bool("InputB", "GamepadInvertY", config_input[1].gamepad_invert_y_axis);
    write_int("InputB", "Gamepad1", config_input[1].gamepad_1);
    write_int("InputB", "Gamepad2", config_input[1].gamepad_2);
    write_int("InputB", "GamepadStart", config_input[1].gamepad_start);
    write_int("InputB", "GamepadReset", config_input[1].gamepad_reset);
    write_int("InputB", "GamepadX", config_input[1].gamepad_x_axis);
    write_int("InputB", "GamepadY", config_input[1].gamepad_y_axis);

    for (int i = 0; i < 2; i++)
    {
        char input_group[32];
        snprintf(input_group, sizeof(input_group), "InputGamepadShortcuts%d", i + 1);
        for (int j = 0; j < config_HotkeyIndex_COUNT; j++)
        {
            char key_name[32];
            snprintf(key_name, sizeof(key_name), "Shortcut%d", j);
            write_int(input_group, key_name, config_input_gamepad_shortcuts[i].gamepad_shortcuts[j]);
        }
    }

    // Write hotkeys
    write_hotkey("Hotkeys", "OpenROM", config_hotkeys[config_HotkeyIndex_OpenROM]);
    write_hotkey("Hotkeys", "ReloadROM", config_hotkeys[config_HotkeyIndex_ReloadROM]);
    write_hotkey("Hotkeys", "Quit", config_hotkeys[config_HotkeyIndex_Quit]);
    write_hotkey("Hotkeys", "Reset", config_hotkeys[config_HotkeyIndex_Reset]);
    write_hotkey("Hotkeys", "Pause", config_hotkeys[config_HotkeyIndex_Pause]);
    write_hotkey("Hotkeys", "FFWD", config_hotkeys[config_HotkeyIndex_FFWD]);
    write_hotkey("Hotkeys", "Rewind", config_hotkeys[config_HotkeyIndex_Rewind]);
    write_hotkey("Hotkeys", "SaveState", config_hotkeys[config_HotkeyIndex_SaveState]);
    write_hotkey("Hotkeys", "LoadState", config_hotkeys[config_HotkeyIndex_LoadState]);
    write_hotkey("Hotkeys", "Screenshot", config_hotkeys[config_HotkeyIndex_Screenshot]);
    write_hotkey("Hotkeys", "Fullscreen", config_hotkeys[config_HotkeyIndex_Fullscreen]);
    write_hotkey("Hotkeys", "CaptureMouse", config_hotkeys[config_HotkeyIndex_CaptureMouse]);
    write_hotkey("Hotkeys", "ShowMainMenu", config_hotkeys[config_HotkeyIndex_ShowMainMenu]);
    write_hotkey("Hotkeys", "DebugStepInto", config_hotkeys[config_HotkeyIndex_DebugStepInto]);
    write_hotkey("Hotkeys", "DebugStepOver", config_hotkeys[config_HotkeyIndex_DebugStepOver]);
    write_hotkey("Hotkeys", "DebugStepOut", config_hotkeys[config_HotkeyIndex_DebugStepOut]);
    write_hotkey("Hotkeys", "DebugStepFrame", config_hotkeys[config_HotkeyIndex_DebugStepFrame]);
    write_hotkey("Hotkeys", "DebugContinue", config_hotkeys[config_HotkeyIndex_DebugContinue]);
    write_hotkey("Hotkeys", "DebugBreak", config_hotkeys[config_HotkeyIndex_DebugBreak]);
    write_hotkey("Hotkeys", "DebugRunToCursor", config_hotkeys[config_HotkeyIndex_DebugRunToCursor]);
    write_hotkey("Hotkeys", "DebugBreakpoint", config_hotkeys[config_HotkeyIndex_DebugBreakpoint]);
    write_hotkey("Hotkeys", "DebugGoBack", config_hotkeys[config_HotkeyIndex_DebugGoBack]);
    write_hotkey("Hotkeys", "SelectSlot1", config_hotkeys[config_HotkeyIndex_SelectSlot1]);
    write_hotkey("Hotkeys", "SelectSlot2", config_hotkeys[config_HotkeyIndex_SelectSlot2]);
    write_hotkey("Hotkeys", "SelectSlot3", config_hotkeys[config_HotkeyIndex_SelectSlot3]);
    write_hotkey("Hotkeys", "SelectSlot4", config_hotkeys[config_HotkeyIndex_SelectSlot4]);
    write_hotkey("Hotkeys", "SelectSlot5", config_hotkeys[config_HotkeyIndex_SelectSlot5]);
    write_hotkey("Hotkeys", "Mute", config_hotkeys[config_HotkeyIndex_Mute]);

    if (config_ini_file->write(config_ini_data, true))
    {
        Debug("Settings saved");
    }
    else
    {
        Error("Unable to save settings to %s", config_emu_file_path);
    }
}

static char* get_portable_path(void)
{
    const char* base_path = SDL_GetBasePath();
    if (base_path == NULL)
        return NULL;

#if defined(__APPLE__)
    std::string app_path = base_path;
    const std::string app_contents = ".app/Contents/";
    size_t app_contents_pos = app_path.rfind(app_contents);

    if (app_contents_pos != std::string::npos)
    {
        size_t app_dir_pos = app_path.rfind('/', app_contents_pos);

        if (app_dir_pos != std::string::npos)
        {
            std::string portable_path = app_path.substr(0, app_dir_pos + 1);

            if (check_portable(portable_path.c_str()))
                return SDL_strdup(portable_path.c_str());
        }
    }
#endif

    if (check_portable(base_path))
        return SDL_strdup(base_path);

    return NULL;
}

static bool check_portable(const char* base_path)
{
    char portable_file_path[512];

    if (base_path == NULL)
        return false;

    if (snprintf(portable_file_path, sizeof(portable_file_path), "%sportable.ini", base_path) >= (int)sizeof(portable_file_path))
        return false;

    FILE* file = fopen_utf8(portable_file_path, "r");
    
    if (IsValidPointer(file))
    {
        fclose(file);
        return true;
    }

    return false;
}

static int read_int(const char* group, const char* key, int default_value)
{
    int ret = default_value;

    std::string value = config_ini_data[group][key];

    if (!value.empty())
    {
        std::istringstream iss(value);
        if (!(iss >> ret))
            ret = default_value;
    }

    Debug("Load integer setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Debug("Save integer setting: [%s][%s]=%s", group, key, value.c_str());
}

static float read_float(const char* group, const char* key, float default_value)
{
    float ret = default_value;

    std::string value = config_ini_data[group][key];

    if (!value.empty())
    {
        std::istringstream converter(value);
        converter.imbue(std::locale::classic());
        if (!(converter >> ret))
            ret = default_value;
    }

    Debug("Load float setting: [%s][%s]=%.2f", group, key, ret);
    return ret;
}

static void write_float(const char* group, const char* key, float value)
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::fixed << std::setprecision(2) << value;
    std::string value_str = oss.str();
    config_ini_data[group][key] = value_str;
    Debug("Save float setting: [%s][%s]=%s", group, key, value_str.c_str());
}

static bool read_bool(const char* group, const char* key, bool default_value)
{
    bool ret = default_value;

    std::string value = config_ini_data[group][key];

    if (!value.empty())
    {
        std::istringstream converter(value);
        if (!(converter >> std::boolalpha >> ret))
            ret = default_value;
    }

    Debug("Load bool setting: [%s][%s]=%s", group, key, ret ? "true" : "false");
    return ret;
}

static void write_bool(const char* group, const char* key, bool boolean)
{
    std::stringstream converter;
    converter << std::boolalpha << boolean;
    std::string value;
    value = converter.str();
    config_ini_data[group][key] = value;
    Debug("Save bool setting: [%s][%s]=%s", group, key, value.c_str());
}

static std::string read_string(const char* group, const char* key)
{
    std::string ret = config_ini_data[group][key];
    Debug("Load string setting: [%s][%s]=%s", group, key, ret.c_str());
    return ret;
}

static void write_string(const char* group, const char* key, const std::string& value)
{
    config_ini_data[group][key] = value;
    Debug("Save string setting: [%s][%s]=%s", group, key, value.c_str());
}

static config_Hotkey read_hotkey(const char* group, const char* key, config_Hotkey default_value)
{
    config_Hotkey ret = default_value;

    std::string scancode_key = std::string(key) + "Scancode";
    std::string mod_key = std::string(key) + "Mod";

    ret.key = (SDL_Scancode)read_int(group, scancode_key.c_str(), default_value.key);
    ret.mod = (SDL_Keymod)read_int(group, mod_key.c_str(), default_value.mod);

    config_update_hotkey_string(&ret);

    return ret;
}

static void write_hotkey(const char* group, const char* key, config_Hotkey hotkey)
{
    std::string scancode_key = std::string(key) + "Scancode";
    std::string mod_key = std::string(key) + "Mod";

    write_int(group, scancode_key.c_str(), hotkey.key);
    write_int(group, mod_key.c_str(), hotkey.mod);
}

static std::string shader_preset_section_name(const char* preset_file)
{
    return std::string("ShaderPreset.") + get_filename(preset_file);
}

static bool parse_float_string(const std::string& value, float* result)
{
    if (value.empty() || !result)
        return false;

    char* end = NULL;
    float parsed = strtof(value.c_str(), &end);
    if (end == value.c_str())
        return false;

    *result = parsed;
    return true;
}

bool config_read_shader_parameter(const char* preset_file, const char* parameter_name, float* value)
{
    if (!preset_file || preset_file[0] == '\0' || !parameter_name || parameter_name[0] == '\0' || !value)
        return false;

    std::string section = shader_preset_section_name(preset_file);
    if (!config_ini_data.has(section))
        return false;

    mINI::INIMap<std::string> parameters = config_ini_data.get(section);
    if (!parameters.has(parameter_name))
        return false;

    return parse_float_string(parameters.get(parameter_name), value);
}

void config_write_shader_parameter(const char* preset_file, const char* parameter_name, float value)
{
    if (!preset_file || preset_file[0] == '\0' || !parameter_name || parameter_name[0] == '\0')
        return;

    std::string section = shader_preset_section_name(preset_file);
    write_float(section.c_str(), parameter_name, value);
}

static void sync_shader_preset_parameter_defaults(void)
{
    ShaderPresetInfo presets[SHADER_PRESET_MAX_DISCOVERED];
    int preset_count = shader_preset_scan_bundled(presets, SHADER_PRESET_MAX_DISCOVERED);

    for (int i = 0; i < preset_count; i++)
    {
        ShaderPreset preset;
        char error[512];
        if (!shader_preset_load(presets[i].path, &preset, error, sizeof(error)))
            continue;

        char preset_file[SHADER_PRESET_MAX_PATH];
        if (!shader_preset_get_config_path(preset.preset_path, preset_file, sizeof(preset_file)))
            continue;

        std::string section = shader_preset_section_name(preset_file);
        for (int j = 0; j < preset.parameter_count; j++)
        {
            ShaderPresetParameter* parameter = &preset.parameters[j];
            if (config_ini_data[section].has(parameter->name))
                continue;

            write_float(section.c_str(), parameter->name, parameter->default_value);
        }
    }
}

static config_Hotkey make_hotkey(SDL_Scancode key, SDL_Keymod mod)
{
    config_Hotkey hotkey;
    hotkey.key = key;
    hotkey.mod = mod;
    config_update_hotkey_string(&hotkey);
    return hotkey;
}

void config_update_hotkey_string(config_Hotkey* hotkey)
{
    if (hotkey->key == SDL_SCANCODE_UNKNOWN)
    {
        strcpy(hotkey->str, "");
        return;
    }

    std::string result = "";

    if (hotkey->mod & (SDL_KMOD_CTRL | SDL_KMOD_LCTRL | SDL_KMOD_RCTRL))
        result += "Ctrl+";
    if (hotkey->mod & (SDL_KMOD_SHIFT | SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT))
        result += "Shift+";
    if (hotkey->mod & (SDL_KMOD_ALT | SDL_KMOD_LALT | SDL_KMOD_RALT))
        result += "Alt+";
    if (hotkey->mod & (SDL_KMOD_GUI | SDL_KMOD_LGUI | SDL_KMOD_RGUI))
        result += "Cmd+";

    const char* key_name = SDL_GetScancodeName(hotkey->key);
    if (key_name && strlen(key_name) > 0)
        result += key_name;
    else
        result += "Unknown";

    strncpy(hotkey->str, result.c_str(), sizeof(hotkey->str) - 1);
    hotkey->str[sizeof(hotkey->str) - 1] = '\0';
}
