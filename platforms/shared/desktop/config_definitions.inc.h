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

#include "config_macros.h"
#include "shader_preset.h"

#if defined(_WIN32)
static const int config_geartogear_stall_default_us = 5000;
static const int config_geartogear_stall_min_us = 1000;
static const int config_geartogear_stall_max_us = 10000;
#elif defined(__APPLE__)
static const int config_geartogear_stall_default_us = 100;
static const int config_geartogear_stall_min_us = 50;
static const int config_geartogear_stall_max_us = 1000;
#else
static const int config_geartogear_stall_default_us = 250;
static const int config_geartogear_stall_min_us = 50;
static const int config_geartogear_stall_max_us = 2000;
#endif

static inline void process(config_Operation operation)
{
    //**************************************
    // Debug
    //**************************************

    // Debugger windows
    CONFIG_BOOL("Debug", "Debug", config_debug.debug, false);
    CONFIG_BOOL("Debug", "Disassembler", config_debug.show_disassembler, true);
    CONFIG_BOOL("Debug", "Screen", config_debug.show_screen, true);
    CONFIG_BOOL("Debug", "Memory", config_debug.show_memory, false);
    CONFIG_BOOL("Debug", "Processor", config_debug.show_processor, true);
    CONFIG_BOOL("Debug", "CallStack", config_debug.show_call_stack, false);
    CONFIG_BOOL("Debug", "Breakpoints", config_debug.show_breakpoints, false);
    CONFIG_BOOL("Debug", "Symbols", config_debug.show_symbols, false);
    CONFIG_BOOL("Debug", "Video", config_debug.show_video, false);
    CONFIG_BOOL("Debug", "VideoNameTable", config_debug.show_video_nametable, false);
    CONFIG_BOOL("Debug", "VideoTiles", config_debug.show_video_tiles, false);
    CONFIG_BOOL("Debug", "VideoSprites", config_debug.show_video_sprites, false);
    CONFIG_BOOL("Debug", "VideoPalettes", config_debug.show_video_palettes, false);
    CONFIG_BOOL("Debug", "VideoRegs", config_debug.show_video_regs, false);
    CONFIG_BOOL("Debug", "PSG", config_debug.show_psg, false);
    CONFIG_BOOL("Debug", "YM2413", config_debug.show_ym2413, false);
    CONFIG_BOOL("Debug", "TraceLogger", config_debug.show_trace_logger, false);
    CONFIG_BOOL("Debug", "GameGearSerialRegisters", config_debug.show_geartogear_serial_registers, false);
    CONFIG_BOOL("Debug", "GameGearSerialStatus", config_debug.show_geartogear_serial_status, false);
    CONFIG_BOOL("Debug", "GearToGearTransport", config_debug.show_geartogear_transport, false);
    CONFIG_BOOL("Debug", "Rewind", config_debug.show_rewind, false);

    // Trace logger
    CONFIG_BOOL("Debug", "TraceCounter", config_debug.trace_counter, true);
    CONFIG_BOOL("Debug", "TraceCycles", config_debug.trace_cycles, false);
    CONFIG_BOOL("Debug", "TraceBank", config_debug.trace_bank, true);
    CONFIG_BOOL("Debug", "TraceRegisters", config_debug.trace_registers, true);
    CONFIG_BOOL("Debug", "TraceFlags", config_debug.trace_flags, true);
    CONFIG_BOOL("Debug", "TraceBytes", config_debug.trace_bytes, true);
    CONFIG_BOOL("Debug", "TraceCpuEnabled", config_debug.trace_cpu_enabled, true);
    CONFIG_BOOL("Debug", "TraceCpu", config_debug.trace_cpu, true);
    CONFIG_BOOL("Debug", "TraceCpuIrq", config_debug.trace_cpu_irq, true);
    CONFIG_BOOL("Debug", "TraceVdp", config_debug.trace_vdp, false);
    CONFIG_BOOL("Debug", "TraceInput", config_debug.trace_input, false);
    CONFIG_BOOL("Debug", "TraceIo", config_debug.trace_io, false);
    CONFIG_BOOL("Debug", "TraceGearToGear", config_debug.trace_geartogear, false);
    CONFIG_BOOL("Debug", "TracePsg", config_debug.trace_psg, false);
    CONFIG_BOOL("Debug", "TraceYm2413", config_debug.trace_ym2413, false);
    CONFIG_BOOL("Debug", "TraceMapper", config_debug.trace_mapper, false);
    CONFIG_INT_RANGE("Debug", "TraceVdpEvents", config_debug.trace_vdp_events, TRACE_VDP_EVENT_ALL, 0, TRACE_VDP_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TraceInputEvents", config_debug.trace_input_events, TRACE_INPUT_EVENT_ALL, 0, TRACE_INPUT_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TraceIoEvents", config_debug.trace_io_events, TRACE_IO_EVENT_ALL, 0, TRACE_IO_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TraceGearToGearEvents", config_debug.trace_geartogear_events, TRACE_GEARTOGEAR_EVENT_ALL, 0, TRACE_GEARTOGEAR_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TracePsgEvents", config_debug.trace_psg_events, TRACE_PSG_EVENT_ALL, 0, TRACE_PSG_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TraceYm2413Events", config_debug.trace_ym2413_events, TRACE_YM2413_EVENT_ALL, 0, TRACE_YM2413_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TraceMapperEvents", config_debug.trace_mapper_events, TRACE_MAPPER_EVENT_ALL, 0, TRACE_MAPPER_EVENT_ALL);
    CONFIG_INT_RANGE("Debug", "TraceOutput", config_debug.trace_output, 0, 0, 1);
    CONFIG_INT_RANGE("Debug", "TraceCapacity", config_debug.trace_capacity, 0, 0, 4);
    CONFIG_INT_RANGE("Debug", "TraceDiskDirOption", config_debug.trace_disk_dir_option, 0, 0, 2);
    CONFIG_INT_RANGE("Debug", "TraceDiskSize", config_debug.trace_disk_size, 2, 0, 6);
    CONFIG_STRING_NOT_EMPTY("Debug", "TraceDiskPath", config_debug.trace_disk_path, config_root_path);

    // Disassembler
    CONFIG_BOOL("Debug", "DisMem", config_debug.dis_show_mem, true);
    CONFIG_BOOL("Debug", "DisSymbols", config_debug.dis_show_symbols, true);
    CONFIG_BOOL("Debug", "DisSegment", config_debug.dis_show_segment, true);
    CONFIG_BOOL("Debug", "DisBank", config_debug.dis_show_bank, true);
    CONFIG_BOOL("Debug", "DisAutoSymbols", config_debug.dis_show_auto_symbols, true);
    CONFIG_BOOL("Debug", "DisDimAutoSymbols", config_debug.dis_dim_auto_symbols, false);
    CONFIG_BOOL("Debug", "DisReplaceSymbols", config_debug.dis_replace_symbols, true);
    CONFIG_BOOL("Debug", "DisReplaceLabels", config_debug.dis_replace_labels, true);
    CONFIG_INT_RANGE("Debug", "DisSyntax", config_debug.dis_syntax, GS_Disassembler_Syntax_Gearsystem, GS_Disassembler_Syntax_Gearsystem, GS_Disassembler_Syntax_Count - 1);
    CONFIG_INT("Debug", "DisLookAheadCount", config_debug.dis_look_ahead_count, 20);

    // Interface
    CONFIG_INT_RANGE("Debug", "FontSize", config_debug.font_size, 0, 0, 3);
    CONFIG_INT("Debug", "Scale", config_debug.scale, 2);
    CONFIG_BOOL("Debug", "MultiViewport", config_debug.multi_viewport, false);
    CONFIG_BOOL("Debug", "SingleInstance", config_debug.single_instance, false);
    CONFIG_BOOL("Debug", "AutoDebugSettings", config_debug.auto_debug_settings, false);

    // Memory editors
    for (int i = 0; i < config_memory_editor_count; i++)
    {
        char section[32];
        snprintf(section, sizeof(section), "MemEditor_%d", i);
        CONFIG_INT(section, "BytesPerRow", config_debug.mem_editor_bytes_per_row[i], 16);
        CONFIG_INT(section, "PreviewDataType", config_debug.mem_editor_preview_data_type[i], 0);
        CONFIG_INT(section, "PreviewEndianess", config_debug.mem_editor_preview_endianess[i], 0);
        CONFIG_BOOL(section, "UppercaseHex", config_debug.mem_editor_uppercase_hex[i], true);
        CONFIG_BOOL(section, "GrayOutZeros", config_debug.mem_editor_gray_out_zeros[i], true);
    }

    //**************************************
    // Emulator
    //**************************************

    // Window and interface
    CONFIG_BOOL("Emulator", "Maximized", config_emulator.maximized, false);
    CONFIG_BOOL("Emulator", "FullScreen", config_emulator.fullscreen, false);
    CONFIG_INT("Emulator", "FullScreenMode", config_emulator.fullscreen_mode, 0);
    CONFIG_BOOL("Emulator", "AlwaysShowMenu", config_emulator.always_show_menu, false);
    CONFIG_INT_RANGE("Emulator", "Theme", config_emulator.theme, config_Theme_Dark, config_Theme_Light, config_Theme_Dark);
    CONFIG_INT("Emulator", "WindowWidth", config_emulator.window_width, 770);
    CONFIG_INT("Emulator", "WindowHeight", config_emulator.window_height, 600);
    CONFIG_BOOL("Emulator", "StatusMessages", config_emulator.status_messages, false);
    CONFIG_BOOL("Emulator", "AllowScreenSaver", config_emulator.allow_screensaver, false);

    // Emulation
    CONFIG_INT("Emulator", "FFWD", config_emulator.ffwd_speed, 1);
    CONFIG_INT_RANGE("Emulator", "RunAhead", config_emulator.runahead, 0, 0, 3);
    CONFIG_INT_RANGE("Emulator", "SaveSlot", config_emulator.save_slot, 0, 0, 4);
    CONFIG_BOOL("Emulator", "StartPaused", config_emulator.start_paused, false);
    CONFIG_BOOL("Emulator", "PauseWhenInactive", config_emulator.pause_when_inactive, true);
    CONFIG_BOOL("Emulator", "SoftPatching", config_emulator.softpatching, true);
    CONFIG_INT("Emulator", "System", config_emulator.system, 0);
    CONFIG_INT("Emulator", "Zone", config_emulator.zone, 0);
    CONFIG_INT("Emulator", "Mapper", config_emulator.mapper, 0);
    CONFIG_INT("Emulator", "Region", config_emulator.region, 0);
    CONFIG_BOOL("Emulator", "SMSBootrom", config_emulator.sms_bootrom, false);
    CONFIG_STRING("Emulator", "SMSBootromPath", config_emulator.sms_bootrom_path, "");
    CONFIG_BOOL("Emulator", "GGBootrom", config_emulator.gg_bootrom, false);
    CONFIG_STRING("Emulator", "GGBootromPath", config_emulator.gg_bootrom_path, "");
    CONFIG_INT("Emulator", "Media", config_emulator.media, 0);

    // Peripherals
    CONFIG_BOOL("Emulator", "LightPhaser", config_emulator.light_phaser, false);
    CONFIG_BOOL("Emulator", "LightPhaserCrosshair", config_emulator.light_phaser_crosshair, false);
    CONFIG_INT("Emulator", "LightPhaserCrosshairShape", config_emulator.light_phaser_crosshair_shape, 0);
    CONFIG_INT("Emulator", "LightPhaserCrosshairColor", config_emulator.light_phaser_crosshair_color, 0);
    CONFIG_INT("Emulator", "LightPhaserXOffset", config_emulator.light_phaser_x_offset, 0);
    CONFIG_INT("Emulator", "LightPhaserYOffset", config_emulator.light_phaser_y_offset, 0);
    CONFIG_BOOL("Emulator", "PaddleControl", config_emulator.paddle_control, false);
    CONFIG_INT("Emulator", "PaddleSensitivity", config_emulator.paddle_sensitivity, 5);

    // Files and paths
    CONFIG_INT("Emulator", "SaveFilesDirOption", config_emulator.savefiles_dir_option, 0);
    CONFIG_STRING_NOT_EMPTY("Emulator", "SaveFilesPath", config_emulator.savefiles_path, config_root_path);
    CONFIG_INT("Emulator", "SaveStatesDirOption", config_emulator.savestates_dir_option, 0);
    CONFIG_STRING_NOT_EMPTY("Emulator", "SaveStatesPath", config_emulator.savestates_path, config_root_path);
    CONFIG_INT("Emulator", "ScreenshotDirOption", config_emulator.screenshots_dir_option, 0);
    CONFIG_STRING_NOT_EMPTY("Emulator", "ScreenshotPath", config_emulator.screenshots_path, config_root_path);
    CONFIG_STRING("Emulator", "LastOpenPath", config_emulator.last_open_path, "");
    CONFIG_STRING_ARRAY("Emulator", "RecentROM%d", config_emulator.recent_roms, config_max_recent_roms, "");

    // Services
    CONFIG_INT("Emulator", "MCPTCPPort", config_emulator.mcp_tcp_port, 7777);
    CONFIG_STRING_NOT_EMPTY("Emulator", "MCPHTTPAddress", config_emulator.mcp_http_address, "127.0.0.1");
    CONFIG_INT_RANGE("Emulator", "GearToGearSession", config_emulator.geartogear_session, 1, 1, 255);
    CONFIG_INT_RANGE("Emulator", "GearToGearStallUs", config_emulator.geartogear_stall_us,
        config_geartogear_stall_default_us, config_geartogear_stall_min_us, config_geartogear_stall_max_us);

    //**************************************
    // Video
    //**************************************

    // Display
    CONFIG_INT("Video", "Scale", config_video.scale, 0);
    CONFIG_INT_RANGE("Video", "ScaleManual", config_video.scale_manual, 1, 1, 20);
    CONFIG_INT("Video", "AspectRatio", config_video.ratio, 1);
    CONFIG_INT("Video", "Overscan", config_video.overscan, 1);
    CONFIG_INT("Video", "HideLeftBar", config_video.hide_left_bar, 0);
    CONFIG_BOOL("Video", "FPS", config_video.fps, false);
    CONFIG_BOOL("Video", "SpriteLimit", config_video.sprite_limit, false);
    CONFIG_INT("Video", "3DGlasses", config_video.glasses, 0);
    CONFIG_INT_RANGE("Video", "ShaderMode", config_video.shader_mode, config_ShaderMode_PixelPerfect, config_ShaderMode_PixelPerfect, config_ShaderMode_External);

    if (operation == config_Operation_Write)
    {
        std::string preset_file = get_filename(config_video.shader_preset_path.c_str());
        CONFIG_STRING("Video", "ShaderPresetFile", preset_file, "");
    }
    else
    {
        CONFIG_STRING("Video", "ShaderPresetFile", config_video.shader_preset_path, "");
    }

    CONFIG_INT_RANGE("Video", "SyncMode", config_video.sync_mode, config_VideoSync_Disabled, config_VideoSync_Disabled, config_VideoSync_VRR);

    // Background colors
    CONFIG_FLOAT("Video", "BackgroundColorR", config_video.background_color[config_Theme_Dark][0], 0.1f);
    CONFIG_FLOAT("Video", "BackgroundColorG", config_video.background_color[config_Theme_Dark][1], 0.1f);
    CONFIG_FLOAT("Video", "BackgroundColorB", config_video.background_color[config_Theme_Dark][2], 0.1f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerR", config_video.background_color_debugger[config_Theme_Dark][0], 0.2f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerG", config_video.background_color_debugger[config_Theme_Dark][1], 0.2f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerB", config_video.background_color_debugger[config_Theme_Dark][2], 0.2f);
    CONFIG_FLOAT("Video", "BackgroundColorLightR", config_video.background_color[config_Theme_Light][0], 128.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorLightG", config_video.background_color[config_Theme_Light][1], 128.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorLightB", config_video.background_color[config_Theme_Light][2], 128.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerLightR",
                 config_video.background_color_debugger[config_Theme_Light][0],
                 233.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerLightG",
                 config_video.background_color_debugger[config_Theme_Light][1],
                 232.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerLightB",
                 config_video.background_color_debugger[config_Theme_Light][2],
                 230.0f / 255.0f);

    //**************************************
    // Audio
    //**************************************

    CONFIG_BOOL("Audio", "Enable", config_audio.enable, true);
    CONFIG_BOOL("Audio", "Sync", config_audio.sync, true);
    CONFIG_FLOAT_RANGE("Audio", "MasterVolume", config_audio.master_volume, 1.0f, 0.0f, 2.0f);
    CONFIG_FLOAT("Audio", "PSGVolume", config_audio.psg_volume, 1.0f);
    CONFIG_FLOAT("Audio", "FMVolume", config_audio.fm_volume, 1.0f);
    CONFIG_INT("Audio", "YM2413", config_audio.ym2413, 0);
    CONFIG_INT("Audio", "BufferCount", config_audio.buffer_count, 3);

    //**************************************
    // Rewind
    //**************************************

    CONFIG_BOOL("Rewind", "Enabled", config_rewind.enabled, true);
    CONFIG_INT("Rewind", "BufferSeconds", config_rewind.buffer_seconds, 10);
    CONFIG_INT("Rewind", "FramesPerSnapshot", config_rewind.frames_per_snapshot, 1);
    CONFIG_FLOAT("Rewind", "Speed", config_rewind.speed, 2.0f);

    //**************************************
    // Input
    //**************************************

    const char* input_sections[2] = { "InputA", "InputB" };
    const SDL_Scancode keyboard_defaults[2][8] = {
        { SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
          SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_RETURN, SDL_SCANCODE_UNKNOWN },
        { SDL_SCANCODE_J, SDL_SCANCODE_L, SDL_SCANCODE_I, SDL_SCANCODE_K,
          SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_RSHIFT, SDL_SCANCODE_UNKNOWN }
    };

    const int gamepad_1_default = operation == config_Operation_Defaults ? 1 : SDL_GAMEPAD_BUTTON_SOUTH;
    const int gamepad_2_default = operation == config_Operation_Defaults ? 2 : SDL_GAMEPAD_BUTTON_EAST;
    const int gamepad_start_default = operation == config_Operation_Defaults ? 9 : SDL_GAMEPAD_BUTTON_START;

    for (int i = 0; i < 2; i++)
    {
        const char* section = input_sections[i];
        const SDL_Scancode* defaults = keyboard_defaults[i];

        // Keyboard
        CONFIG_SCANCODE(section, "KeyLeft", config_input[i].key_left, defaults[0]);
        CONFIG_SCANCODE(section, "KeyRight", config_input[i].key_right, defaults[1]);
        CONFIG_SCANCODE(section, "KeyUp", config_input[i].key_up, defaults[2]);
        CONFIG_SCANCODE(section, "KeyDown", config_input[i].key_down, defaults[3]);
        CONFIG_SCANCODE(section, "Key1", config_input[i].key_1, defaults[4]);
        CONFIG_SCANCODE(section, "Key2", config_input[i].key_2, defaults[5]);
        CONFIG_SCANCODE(section, "KeyStart", config_input[i].key_start, defaults[6]);
        CONFIG_SCANCODE(section, "KeyReset", config_input[i].key_reset, defaults[7]);

        // Gamepad
        CONFIG_BOOL(section, "AllowUpDown", config_input[i].allow_up_down, false);
        CONFIG_BOOL(section, "Gamepad", config_input[i].gamepad, true);
        CONFIG_INT(section, "GamepadDirectional", config_input[i].gamepad_directional, 0);
        CONFIG_BOOL(section, "GamepadInvertX", config_input[i].gamepad_invert_x_axis, false);
        CONFIG_BOOL(section, "GamepadInvertY", config_input[i].gamepad_invert_y_axis, false);
        CONFIG_INT(section, "Gamepad1", config_input[i].gamepad_1, gamepad_1_default);
        CONFIG_INT(section, "Gamepad2", config_input[i].gamepad_2, gamepad_2_default);
        CONFIG_INT(section, "GamepadStart", config_input[i].gamepad_start, gamepad_start_default);
        CONFIG_INT(section, "GamepadReset", config_input[i].gamepad_reset, SDL_GAMEPAD_BUTTON_INVALID);
        CONFIG_INT(section, "GamepadX", config_input[i].gamepad_x_axis, SDL_GAMEPAD_AXIS_LEFTX);
        CONFIG_INT(section, "GamepadY", config_input[i].gamepad_y_axis, SDL_GAMEPAD_AXIS_LEFTY);
    }

    // Gamepad shortcuts
    for (int i = 0; i < 2; i++)
    {
        char section[32];
        snprintf(section, sizeof(section), "InputGamepadShortcuts%d", i + 1);
        CONFIG_INT_ARRAY(section, "Shortcut%d", config_input_gamepad_shortcuts[i].gamepad_shortcuts, config_HotkeyIndex_COUNT, SDL_GAMEPAD_BUTTON_INVALID);
    }

    // Hotkeys
    CONFIG_HOTKEY("OpenROM", config_hotkeys[config_HotkeyIndex_OpenROM], SDL_SCANCODE_O, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("ReloadROM", config_hotkeys[config_HotkeyIndex_ReloadROM], SDL_SCANCODE_D, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Quit", config_hotkeys[config_HotkeyIndex_Quit], SDL_SCANCODE_Q, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Reset", config_hotkeys[config_HotkeyIndex_Reset], SDL_SCANCODE_R, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Pause", config_hotkeys[config_HotkeyIndex_Pause], SDL_SCANCODE_P, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("FFWD", config_hotkeys[config_HotkeyIndex_FFWD], SDL_SCANCODE_F, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Rewind", config_hotkeys[config_HotkeyIndex_Rewind], SDL_SCANCODE_BACKSPACE, SDL_KMOD_NONE);
    CONFIG_HOTKEY("SaveState", config_hotkeys[config_HotkeyIndex_SaveState], SDL_SCANCODE_S, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("LoadState", config_hotkeys[config_HotkeyIndex_LoadState], SDL_SCANCODE_L, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Screenshot", config_hotkeys[config_HotkeyIndex_Screenshot], SDL_SCANCODE_X, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Fullscreen", config_hotkeys[config_HotkeyIndex_Fullscreen], SDL_SCANCODE_F12, SDL_KMOD_NONE);
    CONFIG_HOTKEY("CaptureMouse", config_hotkeys[config_HotkeyIndex_CaptureMouse], SDL_SCANCODE_F1, SDL_KMOD_NONE);
    CONFIG_HOTKEY("ShowMainMenu", config_hotkeys[config_HotkeyIndex_ShowMainMenu], SDL_SCANCODE_M, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("DebugStepInto", config_hotkeys[config_HotkeyIndex_DebugStepInto], SDL_SCANCODE_F11, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugStepOver", config_hotkeys[config_HotkeyIndex_DebugStepOver], SDL_SCANCODE_F10, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugStepOut", config_hotkeys[config_HotkeyIndex_DebugStepOut], SDL_SCANCODE_F11, SDL_KMOD_SHIFT);
    CONFIG_HOTKEY("DebugStepFrame", config_hotkeys[config_HotkeyIndex_DebugStepFrame], SDL_SCANCODE_F6, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugContinue", config_hotkeys[config_HotkeyIndex_DebugContinue], SDL_SCANCODE_F5, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugBreak", config_hotkeys[config_HotkeyIndex_DebugBreak], SDL_SCANCODE_F7, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugRunToCursor", config_hotkeys[config_HotkeyIndex_DebugRunToCursor], SDL_SCANCODE_F8, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugBreakpoint", config_hotkeys[config_HotkeyIndex_DebugBreakpoint], SDL_SCANCODE_F9, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugGoBack", config_hotkeys[config_HotkeyIndex_DebugGoBack], SDL_SCANCODE_BACKSPACE, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot1", config_hotkeys[config_HotkeyIndex_SelectSlot1], SDL_SCANCODE_1, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot2", config_hotkeys[config_HotkeyIndex_SelectSlot2], SDL_SCANCODE_2, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot3", config_hotkeys[config_HotkeyIndex_SelectSlot3], SDL_SCANCODE_3, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot4", config_hotkeys[config_HotkeyIndex_SelectSlot4], SDL_SCANCODE_4, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot5", config_hotkeys[config_HotkeyIndex_SelectSlot5], SDL_SCANCODE_5, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Mute", config_hotkeys[config_HotkeyIndex_Mute], SDL_SCANCODE_U, SDL_KMOD_CTRL);
}

//**************************************
// Emulator-specific behavior
//**************************************

static void before_read(int file_version);
static void after_read(int file_version);
static void before_write(void);
static void after_write(void);
static void before_defaults(void);
static void after_defaults(void);
static void normalize(void);
static void migrate(int file_version);
static void sync_shader_preset_parameter_defaults(void);

static void before_read(int file_version)
{
    migrate(file_version);
}

static void after_read(int file_version)
{
    UNUSED(file_version);

    if (config_emulator.light_phaser)
        config_emulator.paddle_control = false;

    sync_shader_preset_parameter_defaults();
}

static void before_write(void)
{
    if (config_emulator.ffwd)
        config_audio.sync = true;
}

static void after_write(void)
{
    sync_shader_preset_parameter_defaults();
}

static void before_defaults(void)
{
}

static void after_defaults(void)
{
    config_emulator.paused = false;
    config_emulator.ffwd = false;
    config_emulator.show_info = false;
    config_emulator.capture_mouse = false;
}

static void normalize(void)
{
#if defined(GS_DISABLE_DISASSEMBLER)
    config_debug.debug = false;
#endif
#if !defined(_WIN32)
    if (config_video.sync_mode == config_VideoSync_VRR)
        config_video.sync_mode = config_VideoSync_Fixed;
#endif
}

static void migrate(int file_version)
{
    int sync_mode = -1;
    std::string stored;

    if (file_version < 6)
    {
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        bool old_default = get_setting("Video", "BackgroundColorDebuggerLightR", &stored) &&
                           parse_float_string(stored, &red) &&
                           get_setting("Video", "BackgroundColorDebuggerLightG", &stored) &&
                           parse_float_string(stored, &green) &&
                           get_setting("Video", "BackgroundColorDebuggerLightB", &stored) &&
                           parse_float_string(stored, &blue) &&
                           std::fabs(red - (160.0f / 255.0f)) < 0.005f &&
                           std::fabs(green - (160.0f / 255.0f)) < 0.005f &&
                           std::fabs(blue - (160.0f / 255.0f)) < 0.005f;

        if (old_default)
        {
            write_float("Video", "BackgroundColorDebuggerLightR", 233.0f / 255.0f);
            write_float("Video", "BackgroundColorDebuggerLightG", 232.0f / 255.0f);
            write_float("Video", "BackgroundColorDebuggerLightB", 230.0f / 255.0f);
        }
    }
    bool valid_sync_mode = get_setting("Video", "SyncMode", &stored) &&
        parse_int_string(stored, &sync_mode) && sync_mode >= config_VideoSync_Disabled &&
        sync_mode <= config_VideoSync_VRR;

    if (file_version < 4 || !valid_sync_mode)
    {
        bool sync = read_bool("Video", "Sync", true);
        bool vrr = read_bool("Video", "VRR", false);
        sync_mode = sync ? (vrr ? config_VideoSync_VRR : config_VideoSync_Fixed) : config_VideoSync_Disabled;
        write_int("Video", "SyncMode", sync_mode);
    }

    if (file_version < 5)
    {
        bool cpu = read_bool("Debug", "TraceCpu", true);
        bool cpu_irq = read_bool("Debug", "TraceCpuIrq", true);
        bool vdp_write = read_bool("Debug", "TraceVdpWrite", true);
        bool vdp_status = read_bool("Debug", "TraceVdpStatus", true);
        bool psg = read_bool("Debug", "TracePsg", true);
        bool ym2413 = read_bool("Debug", "TraceYm2413", true);
        bool io = read_bool("Debug", "TraceIoPort", true);
        bool mapper = read_bool("Debug", "TraceBankSwitch", true);
        bool old_default = cpu && cpu_irq && vdp_write && vdp_status && psg && ym2413 && io && mapper;

        write_bool("Debug", "TraceCpuEnabled", cpu || cpu_irq);
        write_bool("Debug", "TraceCpu", cpu);
        write_bool("Debug", "TraceCpuIrq", cpu_irq);
        write_bool("Debug", "TraceVdp", !old_default && (vdp_write || vdp_status));
        write_bool("Debug", "TraceInput", false);
        write_bool("Debug", "TraceIo", false);
        write_bool("Debug", "TracePsg", !old_default && psg);
        write_bool("Debug", "TraceYm2413", !old_default && ym2413);
        write_bool("Debug", "TraceMapper", !old_default && mapper);
        write_int("Debug", "TraceVdpEvents", (vdp_write ? TRACE_VDP_EVENT_REGISTERS : 0) |
              (vdp_status ? TRACE_VDP_EVENT_INTERRUPTS | TRACE_VDP_EVENT_STATUS | TRACE_VDP_EVENT_SPRITES | TRACE_VDP_EVENT_STATE : 0));
        write_int("Debug", "TraceInputEvents", TRACE_INPUT_EVENT_ALL);
        write_int("Debug", "TraceIoEvents", TRACE_IO_EVENT_ALL);
        write_int("Debug", "TracePsgEvents", TRACE_PSG_EVENT_ALL);
        write_int("Debug", "TraceYm2413Events", TRACE_YM2413_EVENT_ALL);
        write_int("Debug", "TraceMapperEvents", TRACE_MAPPER_EVENT_ALL);
        write_bool("Debug", "TraceCycles", false);
    }

    if (file_version < 7)
    {
        if (read_bool("Debug", "GearToGear", false))
        {
            write_bool("Debug", "GameGearSerial", true);
            write_bool("Debug", "GearToGearTransport", true);
        }

        if (read_int("Emulator", "GearToGearStallUs", 0) == 0)
        {
            write_int("Emulator", "GearToGearStallUs",
                config_geartogear_stall_default_us);
        }
    }

    if (file_version < 8 && read_bool("Debug", "GameGearSerial", false))
    {
        write_bool("Debug", "GameGearSerialRegisters", true);
        write_bool("Debug", "GameGearSerialStatus", true);
    }

    int scale = 0;
    if (get_setting("Video", "Scale", &stored) && parse_int_string(stored, &scale) && scale > 3)
        write_int("Video", "Scale", scale - 2);
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
