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
#include <math.h>
#include <signal.h>
#include "application_headless.h"
#include "config.h"
#include "emu.h"
#include "gui.h"
#include "gui_debug.h"
#include "gui_debug_disassembler.h"
#include "log.h"

static volatile bool headless_running = true;

static void headless_signal_handler(int sig)
{
    (void)sig;
    headless_running = false;
}

int application_headless_init(const char* rom_file, const char* symbol_file, int mcp_mode, int mcp_tcp_port)
{
    Log("\n%s", GS_TITLE_ASCII);
    Log("%s %s Headless Mode", GS_TITLE, GS_VERSION);

    if (mcp_mode < 0)
    {
        Error("Headless mode requires --mcp-stdio or --mcp-http");
        return 1;
    }

    if (!SDL_Init(0))
    {
        Error("Failed to initialize SDL (headless)");
        return 1;
    }

    if (!emu_init())
    {
        Error("Failed to initialize emulator");
        return 2;
    }

    config_debug.debug = true;

    emu_set_overscan(0);
    emu_audio_mute(true);

    gui_debug_init();

    if (strlen(config_emulator.sms_bootrom_path.c_str()) > 0)
    {
        Log("Loading SMS bootrom: %s", config_emulator.sms_bootrom_path.c_str());
        emu_load_bootrom_sms(config_emulator.sms_bootrom_path.c_str());
    }
    if (strlen(config_emulator.gg_bootrom_path.c_str()) > 0)
    {
        Log("Loading GG bootrom: %s", config_emulator.gg_bootrom_path.c_str());
        emu_load_bootrom_gg(config_emulator.gg_bootrom_path.c_str());
    }
    emu_enable_bootrom_sms(config_emulator.sms_bootrom);
    emu_enable_bootrom_gg(config_emulator.gg_bootrom);
    emu_set_media_slot(config_emulator.media);
    emu_set_hide_left_bar(config_video.hide_left_bar);
    emu_disable_ym2413(config_audio.ym2413 == 1);

    if (IsValidPointer(rom_file) && (strlen(rom_file) > 0))
    {
        Log("Rom file argument: %s", rom_file);
        gui_load_rom(rom_file);
    }

    if (IsValidPointer(symbol_file) && (strlen(symbol_file) > 0))
    {
        Log("Symbol file argument: %s", symbol_file);
        gui_debug_reset_symbols();
        gui_debug_load_symbols_file(symbol_file);
    }

    Log("Starting MCP server (mode: %s, port: %d)...", mcp_mode == 0 ? "stdio" : "http", mcp_tcp_port);
    emu_mcp_set_transport(mcp_mode, mcp_tcp_port);
    emu_mcp_start();

    signal(SIGINT, headless_signal_handler);
    signal(SIGTERM, headless_signal_handler);

    return 0;
}

void application_headless_destroy(void)
{
    gui_debug_destroy();
    emu_destroy();
    SDL_Quit();
}

void application_headless_mainloop(void)
{
    Log("Running headless main loop...");

    while (headless_running)
    {
        Uint64 frame_start = SDL_GetPerformanceCounter();

        emu_update();

        if (!emu_mcp_is_running())
        {
            Log("MCP server stopped, exiting headless mode");
            break;
        }

        Uint64 frame_end = SDL_GetPerformanceCounter();
        float elapsed_ms = (float)(frame_end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

        GS_RuntimeInfo runtime;
        emu_get_runtime(runtime);
        float target_ms = (runtime.region == Region_PAL) ? 20.0f : 16.666f;

        if (elapsed_ms < target_ms)
            SDL_Delay((Uint32)ceilf(target_ms - elapsed_ms));
    }
}
