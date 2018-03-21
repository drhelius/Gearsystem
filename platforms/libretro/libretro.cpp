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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#if defined(_WIN32) && !defined(_XBOX)
#include <windows.h>
#endif
#include "libretro.h"

#include "../../src/gearsystem.h"

GS_Color *frame_buf;

static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static char retro_base_directory[4096];
static char retro_game_path[4096];

static s16 audio_buf[GS_AUDIO_BUFFER_SIZE];
static int audio_sample_count;
static int current_screen_width = 0;
static int current_screen_height = 0;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

GearsystemCore* core;

static retro_environment_t environ_cb;

void retro_init(void)
{
    const char *dir = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
        snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
    }

    core = new GearsystemCore();
    core->Init();

    frame_buf = new GS_Color[GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT];

    audio_sample_count = 0;
}

void retro_deinit(void)
{
    SafeDeleteArray(frame_buf);
    SafeDelete(core);
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "Gearsystem";
    info->library_version  = GEARSYSTEM_VERSION;
    info->need_fullpath    = false;
    info->valid_extensions = "sms|gg|bin|rom";
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    GS_RuntimeInfo runtime_info;
    core->GetRuntimeInfo(runtime_info);

    current_screen_width = runtime_info.screen_width;
    current_screen_height = runtime_info.screen_height;

    info->geometry.base_width   = runtime_info.screen_width;
    info->geometry.base_height  = runtime_info.screen_height;
    info->geometry.max_width    = runtime_info.screen_width;
    info->geometry.max_height   = runtime_info.screen_height;
    info->geometry.aspect_ratio = 0.0;
    info->timing.fps            = runtime_info.region == Region_NTSC ? 60 : 50;
    info->timing.sample_rate    = 44100.0f;
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;

    static const struct retro_controller_description port_1[] = {
        { "Sega Master System / Game Gear", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0) },
    };

    static const struct retro_controller_description port_2[] = {
        { "Sega Master System / Game Gear", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0) },
    };

    static const struct retro_controller_info ports[] = {
        { port_1, 1 },
        { port_2, 1 },
        { NULL, 0 },
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

static void update_input(void)
{
    input_poll_cb();

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
        core->KeyPressed(Joypad_1, Key_Up);
    else
        core->KeyReleased(Joypad_1,Key_Up);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
        core->KeyPressed(Joypad_1, Key_Down);
    else
        core->KeyReleased(Joypad_1, Key_Down);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
        core->KeyPressed(Joypad_1, Key_Left);
    else
        core->KeyReleased(Joypad_1, Key_Left);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
        core->KeyPressed(Joypad_1, Key_Right);
    else
        core->KeyReleased(Joypad_1, Key_Right);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
        core->KeyPressed(Joypad_1, Key_1);
    else
        core->KeyReleased(Joypad_1, Key_1);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
        core->KeyPressed(Joypad_1, Key_2);
    else
        core->KeyReleased(Joypad_1, Key_2);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
        core->KeyPressed(Joypad_1, Key_Start);
    else
        core->KeyReleased(Joypad_1, Key_Start);
}

static void check_variables(void)
{

}

void retro_run(void)
{
    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
    {
        check_variables();
    }

    update_input();

    core->RunToVBlank(frame_buf, audio_buf, &audio_sample_count);

    GS_RuntimeInfo runtime_info;
    core->GetRuntimeInfo(runtime_info);

    if ((runtime_info.screen_width != current_screen_width) || (runtime_info.screen_height != current_screen_height))
    {
        current_screen_width = runtime_info.screen_width;
        current_screen_height = runtime_info.screen_height;

        retro_system_av_info info;
        info.geometry.base_width   = runtime_info.screen_width;
        info.geometry.base_height  = runtime_info.screen_height;
        info.geometry.max_width    = runtime_info.screen_width;
        info.geometry.max_height   = runtime_info.screen_height;
        info.geometry.aspect_ratio = 0.0;

        environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &info.geometry);
    }

    video_cb((uint8_t*)frame_buf, runtime_info.screen_width, runtime_info.screen_height, runtime_info.screen_width * sizeof(GS_Color));

    if (audio_sample_count > 0)
        audio_batch_cb(audio_buf, audio_sample_count / 2);

    audio_sample_count = 0;
}

void retro_reset(void)
{
    check_variables();
    core->ResetROMPreservingRAM();
}

bool retro_load_game(const struct retro_game_info *info)
{
    check_variables();

    core->LoadROMFromBuffer(reinterpret_cast<const u8*>(info->data), info->size);

    struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "1" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "2" },
        { 0 },
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    snprintf(retro_game_path, sizeof(retro_game_path), "%s", info->path);

    bool achievements = true;
    environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS, &achievements);

    return true;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    return false;
}

size_t retro_serialize_size(void)
{
    size_t size;
    core->SaveState(NULL, size);
    return size;
}

bool retro_serialize(void *data, size_t size)
{
    return core->SaveState(reinterpret_cast<u8*>(data), size);
}

bool retro_unserialize(const void *data, size_t size)
{
    return core->LoadState(reinterpret_cast<const u8*>(data), size);
}

void *retro_get_memory_data(unsigned id)
{
    switch (id)
    {
        case RETRO_MEMORY_SAVE_RAM:
            return core->GetMemory()->GetCurrentRule()->GetRamBanks();
        case RETRO_MEMORY_SYSTEM_RAM:
            return core->GetMemory()->GetMemoryMap() + 0xC000;
    }

    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    switch (id)
    {
        case RETRO_MEMORY_SAVE_RAM:
           return core->GetMemory()->GetCurrentRule()->GetRamSize();
        case RETRO_MEMORY_SYSTEM_RAM:
           return 0x2000;
    }

    return 0;
}

void retro_cheat_reset(void)
{
    core->ClearCheats();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    core->SetCheat(code);
}
