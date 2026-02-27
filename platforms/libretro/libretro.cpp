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

#include "libretro.h"
#include "../../src/gearsystem.h"

#ifdef _WIN32
static const char slash = '\\';
#else
static const char slash = '/';
#endif

#define RETRO_DEVICE_SMS_GG_PAD     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)
#define RETRO_DEVICE_LIGHT_PHASER   RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 0)
#define RETRO_DEVICE_PADDLE         RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 0)

static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static struct retro_log_callback logging;
retro_log_printf_t log_cb;

static char retro_system_directory[4096];
static char retro_game_path[4096];

static s16 audio_buf[GS_AUDIO_BUFFER_SIZE];
static int audio_sample_count = 0;

static unsigned input_device[2];
static bool allow_up_down = false;
static bool lightgun_touchscreen = false;
static bool lightgun_crosshair = false;
static Video::LightPhaserCrosshairShape lightgun_crosshair_shape = Video::LightPhaserCrosshairCross;
static Video::LightPhaserCrosshairColor lightgun_crosshair_color = Video::LightPhaserCrosshairWhite;
static int paddle_sensitivity = 0;
static bool bootrom_sms = false;
static bool bootrom_gg = false;
static bool libretro_supports_bitmasks;
static float aspect_ratio = 0.0f;
static int current_screen_width = 0;
static int current_screen_height = 0;
static float current_aspect_ratio = 0;

static GearsystemCore* core;
static u8* frame_buffer;
static Cartridge::ForceConfiguration config;
static GearsystemCore::GlassesConfig glasses_config;

static void load_bootroms(void);
static void set_controller_info(void);
static void update_input(void);
static void set_variabless(void);
static void check_variables(void);

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
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

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;
    set_controller_info();
    set_variabless();
}

void retro_init(void)
{
    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;

    const char *dir = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
        snprintf(retro_system_directory, sizeof(retro_system_directory), "%s", dir);
    else
        snprintf(retro_system_directory, sizeof(retro_system_directory), "%s", ".");

    log_cb(RETRO_LOG_INFO, "%s (%s) libretro\n", GEARSYSTEM_TITLE, EMULATOR_BUILD);

    core = new GearsystemCore();

#ifdef PS2
    core->Init(GS_PIXEL_BGR555);
#else
    core->Init(GS_PIXEL_RGB565);
#endif

    frame_buffer = new u8[GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN * 2];

    config.type = Cartridge::CartridgeNotSupported;
    config.zone = Cartridge::CartridgeUnknownZone;
    config.region = Cartridge::CartridgeUnknownRegion;
    config.system = Cartridge::CartridgeUnknownSystem;

    glasses_config = GearsystemCore::GlassesBothEyes;

    input_device[0] = RETRO_DEVICE_SMS_GG_PAD;
    input_device[1] = RETRO_DEVICE_SMS_GG_PAD;

    libretro_supports_bitmasks = environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL);
}

void retro_deinit(void)
{
    SafeDeleteArray(frame_buffer);
    SafeDelete(core);
}

void retro_reset(void)
{
    log_cb(RETRO_LOG_DEBUG, "Resetting...\n");

    check_variables();
    load_bootroms();
    core->ResetROMPreservingRAM(&config);
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    if (port > 1)
    {
        log_cb(RETRO_LOG_DEBUG, "retro_set_controller_port_device invalid port number: %u\n", port);
        return;
    }

    bool phaser = false;
    bool paddle = false;
    input_device[port] = device;

    switch (device)
    {
        case RETRO_DEVICE_NONE:
            log_cb(RETRO_LOG_INFO, "Controller %u: Unplugged\n", port);
            break;
        case RETRO_DEVICE_SMS_GG_PAD:
        case RETRO_DEVICE_JOYPAD:
            log_cb(RETRO_LOG_INFO, "Controller %u: SMS/GG Pad\n", port);
            break;
        case RETRO_DEVICE_LIGHT_PHASER:
            log_cb(RETRO_LOG_INFO, "Controller %u: Light Phaser\n", port);
            phaser = true;
            break;
        case RETRO_DEVICE_PADDLE:
            log_cb(RETRO_LOG_INFO, "Controller %u: Paddle\n", port);
            paddle = true;
            break;
        default:
            log_cb(RETRO_LOG_DEBUG, "Setting descriptors for unsupported device.\n");
            break;
    }

    if (port == 0)
    {
        core->EnablePhaser(phaser);
        core->EnablePaddle(paddle);
    }
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "Gearsystem";
    info->library_version  = GEARSYSTEM_VERSION;
    info->need_fullpath    = false;
    info->valid_extensions = "sms|gg|sg|mv|bin|rom";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    GS_RuntimeInfo runtime_info;
    core->GetRuntimeInfo(runtime_info);

    current_screen_width = runtime_info.screen_width;
    current_screen_height = runtime_info.screen_height;

    info->geometry.base_width   = current_screen_width;
    info->geometry.base_height  = current_screen_height;
    info->geometry.max_width    = GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN;
    info->geometry.max_height   = GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN;
    info->geometry.aspect_ratio = aspect_ratio;
    info->timing.fps            = runtime_info.region == Region_NTSC ? 60.0 : 50.0;
    info->timing.sample_rate    = 44100.0;
}

void retro_run(void)
{
    bool core_options_updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &core_options_updated) && core_options_updated)
    {
        check_variables();
    }

    update_input();

    audio_sample_count = 0;
    core->RunToVBlank(frame_buffer, audio_buf, &audio_sample_count);

    GS_RuntimeInfo runtime_info;
    core->GetRuntimeInfo(runtime_info);

    if ((runtime_info.screen_width != current_screen_width) ||
        (runtime_info.screen_height != current_screen_height) ||
        (aspect_ratio != current_aspect_ratio))
    {
        current_screen_width = runtime_info.screen_width;
        current_screen_height = runtime_info.screen_height;
        current_aspect_ratio = aspect_ratio;

        retro_system_av_info info;
        info.geometry.base_width   = runtime_info.screen_width;
        info.geometry.base_height  = runtime_info.screen_height;
        info.geometry.max_width    = runtime_info.screen_width;
        info.geometry.max_height   = runtime_info.screen_height;
        info.geometry.aspect_ratio = aspect_ratio;

        environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &info.geometry);
    }

    video_cb((uint8_t*)frame_buffer, runtime_info.screen_width, runtime_info.screen_height, runtime_info.screen_width * sizeof(u8) * 2);

    if (audio_sample_count > 0)
        audio_batch_cb(audio_buf, audio_sample_count / 2);
}

bool retro_load_game(const struct retro_game_info *info)
{
    core->GetCartridge()->Reset();
    check_variables();
    load_bootroms();

    snprintf(retro_game_path, sizeof(retro_game_path), "%s", info->path);

    log_cb(RETRO_LOG_INFO, "Loading game: %s\n", retro_game_path);

    if (!core->LoadROMFromBuffer(reinterpret_cast<const u8*>(info->data), info->size, &config, retro_game_path))
    {
        log_cb(RETRO_LOG_ERROR, "Invalid or corrupted ROM.\n");
        return false;
    }

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_ERROR, "RGB565 is not supported.\n");
        return false;
    }

    bool achievements = true;
    environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS, &achievements);

    Cartridge* cart = core->GetCartridge();

    log_cb(RETRO_LOG_INFO, "CRC: %08X\n", cart->GetCRC());
    log_cb(RETRO_LOG_INFO, "System: %s\n", cart->IsGameGear() ? "Game Gear" : (cart->IsSG1000() ? "SG-1000" : "Master System"));
    log_cb(RETRO_LOG_INFO, "Refresh Rate: %s\n", cart->IsPAL() ? "PAL" : "NTSC");
    log_cb(RETRO_LOG_INFO, "Cartridge Header: %s\n", cart->IsValidROM() ? "VALID" : "FAILED");
    log_cb(RETRO_LOG_INFO, "Battery: %s\n", core->GetMemory()->GetCurrentRule()->PersistedRAM() ? "YES" : "NO");

    return true;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return core->GetCartridge()->IsPAL() ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
   (void)game_type;
   (void)info;
   (void)num_info;
   return false;
}

size_t retro_serialize_size(void)
{
    size_t size = 0;
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
    if (code == NULL)
        return;

    core->SetCheat(code);
}

static void load_bootroms(void)
{
    char bootrom_sms_path[4112];
    char bootrom_gg_path[4112];

    snprintf(bootrom_sms_path, sizeof(bootrom_sms_path), "%s%cbios.sms", retro_system_directory, slash);
    snprintf(bootrom_gg_path, sizeof(bootrom_gg_path), "%s%cbios.gg", retro_system_directory, slash);

    core->GetMemory()->LoadBootromSMS(bootrom_sms_path);
    core->GetMemory()->LoadBootromGG(bootrom_gg_path);
    core->GetMemory()->EnableBootromSMS(bootrom_sms);
    core->GetMemory()->EnableBootromGG(bootrom_gg);
}

static void set_controller_info(void)
{
    static const struct retro_controller_description port_1[] = {
        { "Joypad Auto", RETRO_DEVICE_JOYPAD },
        { "Joypad Port Empty", RETRO_DEVICE_NONE },
        { "Master System / Game Gear Pad", RETRO_DEVICE_SMS_GG_PAD },
        { "Sega Light Phaser", RETRO_DEVICE_LIGHT_PHASER },
        { "Paddle Control", RETRO_DEVICE_PADDLE },
    };

    static const struct retro_controller_description port_2[] = {
        { "Joypad Auto", RETRO_DEVICE_JOYPAD },
        { "Joypad Port Empty", RETRO_DEVICE_NONE },
        { "Master System / Game Gear Pad", RETRO_DEVICE_SMS_GG_PAD },
    };

    static const struct retro_controller_info ports[] = {
        { port_1, 5 },
        { port_2, 3 },
        { NULL, 0 },
    };

    environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);

    struct retro_input_descriptor joypad[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Reset" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "1" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "2" },

        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Reset" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "1" },
        { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "2" },

        { 0, 0, 0, 0, NULL }
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, joypad);
}

static void update_input(void)
{
    input_poll_cb();

    for (int player=0; player<2; player++)
    {
        switch (input_device[player])
        {
        case RETRO_DEVICE_SMS_GG_PAD:
        case RETRO_DEVICE_JOYPAD:
        {
            int16_t ib;

            if (libretro_supports_bitmasks)
                ib = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
            else
            {
                unsigned int i;
                ib = 0;
                for (i = 0; i <= RETRO_DEVICE_ID_JOYPAD_R3; i++)
                    ib |= input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, i) ? (1 << i) : 0;
            }

            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_UP))
            {
                if (allow_up_down || !(ib & (1 << RETRO_DEVICE_ID_JOYPAD_DOWN)))
                    core->KeyPressed(static_cast<GS_Joypads>(player), Key_Up);
            }
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_Up);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_DOWN))
            {
                if (allow_up_down || !(ib & (1 << RETRO_DEVICE_ID_JOYPAD_UP)))
                    core->KeyPressed(static_cast<GS_Joypads>(player), Key_Down);
            }
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_Down);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_LEFT))
            {
                if (allow_up_down || !(ib & (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT)))
                    core->KeyPressed(static_cast<GS_Joypads>(player), Key_Left);
            }
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_Left);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT))
            {
                if (allow_up_down || !(ib & (1 << RETRO_DEVICE_ID_JOYPAD_LEFT)))
                    core->KeyPressed(static_cast<GS_Joypads>(player), Key_Right);
            }
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_Right);

            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_B))
                core->KeyPressed(static_cast<GS_Joypads>(player), Key_1);
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_1);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_A))
                core->KeyPressed(static_cast<GS_Joypads>(player), Key_2);
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_2);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_START))
                core->KeyPressed(static_cast<GS_Joypads>(player), Key_Start);
            else
                core->KeyReleased(static_cast<GS_Joypads>(player), Key_Start);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT))
                core->SetReset(true);
            else
                core->SetReset(false);

            break;
        }
        case RETRO_DEVICE_LIGHT_PHASER:
        {
            if (player == 0)
            {
                if (lightgun_touchscreen)
                {
                    s16 x = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
                    s16 y = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);
                    x = ((x + 0x7fff) * current_screen_width) / 0xfffe;
                    y = ((y + 0x7fff) * current_screen_height) / 0xfffe;

                    core->SetPhaser(x, y);

                    if (input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED))
                        core->KeyPressed(static_cast<GS_Joypads>(0), Key_1);
                    else
                        core->KeyReleased(static_cast<GS_Joypads>(0), Key_1);
                }
                else
                {
                    if (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN) )
                    {
                        core->SetPhaser(-1000, -1000);
                    }
                    else
                    {
                        s16 x = input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X);
                        s16 y = input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y);
                        x = ((x + 0x7fff) * current_screen_width) / 0xfffe;
                        y = ((y + 0x7fff) * current_screen_height) / 0xfffe;

                        core->SetPhaser(x, y);
                    }

                    if (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER))
                        core->KeyPressed(static_cast<GS_Joypads>(0), Key_1);
                    else
                        core->KeyReleased(static_cast<GS_Joypads>(0), Key_1);
                }
            }

            break;
        }
        case RETRO_DEVICE_PADDLE:
        {
            if (player == 0)
            {
                int mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);

                int sen = paddle_sensitivity;
                if (sen < 1)
                    sen = 1;
                float relx = (float)(mouse_x) * ((float)(sen) / 6.0f);
                core->SetPaddle(relx);

                if (input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT))
                    core->KeyPressed(static_cast<GS_Joypads>(0), Key_1);
                else
                    core->KeyReleased(static_cast<GS_Joypads>(0), Key_1);
            }

            break;
        }
        default:
            break;
        }
    }
}

static void set_variabless(void)
{
    struct retro_variable vars[] = {
        { "gearsystem_system", "System (restart); Auto|Master System / Mark III|Game Gear|SG-1000 / Multivision" },
        { "gearsystem_region", "Region (restart); Auto|Master System Japan|Master System Export|Game Gear Japan|Game Gear Export|Game Gear International" },
        { "gearsystem_mapper", "Mapper (restart); Auto|ROM|SEGA|Codemasters|Korean|SG-1000|MSX|Janggun|Korean 2000 XOR 1F|Korean MSX 32KB 2000|Korean MSX SMS 8000|Korean SMS 32KB 2000|Korean MSX 8KB 0300|Korean 0000 XOR FF|Korean FFFF HiCom|Korean FFFE|Korean BFFC|Korean FFF3 FFFC|Korean MD FFF5|Korean MD FFF0|Jumbo Dahjee|EEPROM 93C46|Multi 4PAK All Action|Iratahack" },
        { "gearsystem_timing", "Refresh Rate (restart); Auto|NTSC (60 Hz)|PAL (50 Hz)" },
        { "gearsystem_aspect_ratio", "Aspect Ratio; 1:1 PAR|4:3 DAR|16:9 DAR|16:10 DAR" },
        { "gearsystem_overscan", "Overscan; Disabled|Top+Bottom|Full (284 width)|Full (320 width)" },
        { "gearsystem_hide_left_bar", "Hide Left Bar (SMS only); No|Auto|Always" },
        { "gearsystem_bios_sms", "Master System BIOS (restart); Disabled|Enabled" },
        { "gearsystem_bios_gg", "Game Gear BIOS (restart); Disabled|Enabled" },
        { "gearsystem_ym2413", "YM2413 (restart); Auto|Disabled"},
        { "gearsystem_glasses", "3D Glasses; Both Eyes / OFF|Left Eye|Right Eye" },
        { "gearsystem_up_down_allowed", "Allow Up+Down / Left+Right; Disabled|Enabled" },
        { "gearsystem_lightgun_input", "Light Gun Input; Light Gun|Touchscreen" },
        { "gearsystem_lightgun_crosshair", "Light Gun Crosshair; Disabled|Enabled" },
        { "gearsystem_lightgun_shape", "Light Gun Crosshair Shape; Cross|Square" },
        { "gearsystem_lightgun_color", "Light Gun Crosshair Color; White|Black|Red|Green|Blue|Yellow|Magenta|Cyan" },
        { "gearsystem_lightgun_crosshair_offset_x", "Light Gun Crosshair Offset X; 0|-10|-9|-8|-7|-6|-5|-4|-3|-2|-1|0|1|2|3|4|5|6|7|8|9|10" },
        { "gearsystem_lightgun_crosshair_offset_y", "Light Gun Crosshair Offset Y; 0|-10|-9|-8|-7|-6|-5|-4|-3|-2|-1|0|1|2|3|4|5|6|7|8|9|10" },
        { "gearsystem_paddle_sensitivity", "Paddle Sensitivity; 1|2|3|4|5|6|7|8|9|10|11|12|13|14|15" },
        
        { NULL }
    };

    environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void *)vars);
}

static void check_variables(void)
{
    struct retro_variable var = {0};

    var.key = "gearsystem_up_down_allowed";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Enabled") == 0)
            allow_up_down = true;
        else
            allow_up_down = false;
    }

    var.key = "gearsystem_lightgun_input";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Touchscreen") == 0)
            lightgun_touchscreen = true;
        else
            lightgun_touchscreen = false;
    }

    var.key = "gearsystem_lightgun_crosshair";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Enabled") == 0)
            lightgun_crosshair = true;
        else
            lightgun_crosshair = false;
    }

    var.key = "gearsystem_lightgun_shape";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Cross") == 0)
            lightgun_crosshair_shape = Video::LightPhaserCrosshairCross;
        else
            lightgun_crosshair_shape = Video::LightPhaserCrosshairSquare;
    }

    var.key = "gearsystem_lightgun_color";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "White") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairWhite;
        else if (strcmp(var.value, "Black") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairBlack;
        else if (strcmp(var.value, "Red") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairRed;
        else if (strcmp(var.value, "Green") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairGreen;
        else if (strcmp(var.value, "Blue") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairBlue;
        else if (strcmp(var.value, "Yellow") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairYellow;
        else if (strcmp(var.value, "Magenta") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairMagenta;
        else if (strcmp(var.value, "Cyan") == 0)
            lightgun_crosshair_color = Video::LightPhaserCrosshairCyan;
    }

    core->EnablePhaserCrosshair(lightgun_crosshair, lightgun_crosshair_shape, lightgun_crosshair_color);

    int phaser_offset_x = 0;
    int phaser_offset_y = 0;

    var.key = "gearsystem_lightgun_crosshair_offset_x";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        phaser_offset_x = atoi(var.value);
    }

    var.key = "gearsystem_lightgun_crosshair_offset_y";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        phaser_offset_y = atoi(var.value);
    }

    core->SetPhaserOffset(phaser_offset_x, phaser_offset_y);

    var.key = "gearsystem_paddle_sensitivity";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        paddle_sensitivity = atoi(var.value);
    }

    var.key = "gearsystem_system";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Auto") == 0)
            config.system = Cartridge::CartridgeUnknownSystem;
        else if (strcmp(var.value, "Master System / Mark III") == 0)
            config.system = Cartridge::CartridgeSMS;
        else if (strcmp(var.value, "Game Gear") == 0)
            config.system = Cartridge::CartridgeGG;
        else if (strcmp(var.value, "SG-1000 / Multivision") == 0)
            config.system = Cartridge::CartridgeSG1000;
        else 
            config.system = Cartridge::CartridgeUnknownSystem;
    }

    var.key = "gearsystem_region";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Auto") == 0)
            config.zone = Cartridge::CartridgeUnknownZone;
        else if (strcmp(var.value, "Master System Japan") == 0)
            config.zone = Cartridge::CartridgeJapanSMS;
        else if (strcmp(var.value, "Master System Export") == 0)
            config.zone = Cartridge::CartridgeExportSMS;
        else if (strcmp(var.value, "Game Gear Japan") == 0)
            config.zone = Cartridge::CartridgeJapanGG;
        else if (strcmp(var.value, "Game Gear Export") == 0)
            config.zone = Cartridge::CartridgeExportGG;
        else if (strcmp(var.value, "Game Gear International") == 0)
            config.zone = Cartridge::CartridgeInternationalGG;
        else
            config.zone = Cartridge::CartridgeUnknownZone;
    }

    var.key = "gearsystem_mapper";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Auto") == 0)
            config.type = Cartridge::CartridgeNotSupported;
        else if (strcmp(var.value, "ROM") == 0)
            config.type = Cartridge::CartridgeRomOnlyMapper;
        else if (strcmp(var.value, "SEGA") == 0)
            config.type = Cartridge::CartridgeSegaMapper;
        else if (strcmp(var.value, "Codemasters") == 0)
            config.type = Cartridge::CartridgeCodemastersMapper;
        else if (strcmp(var.value, "Korean") == 0)
            config.type = Cartridge::CartridgeKoreanMapper;
        else if (strcmp(var.value, "SG-1000") == 0)
            config.type = Cartridge::CartridgeSG1000Mapper;
        else if (strcmp(var.value, "MSX") == 0)
            config.type = Cartridge::CartridgeMSXMapper;
        else if (strcmp(var.value, "Janggun") == 0)
            config.type = Cartridge::CartridgeJanggunMapper;
        else if (strcmp(var.value, "Korean 2000 XOR 1F") == 0)
            config.type = Cartridge::CartridgeKorean2000XOR1FMapper;
        else if (strcmp(var.value, "Korean MSX 32KB 2000") == 0)
            config.type = Cartridge::CartridgeKoreanMSX32KB2000Mapper;
        else if (strcmp(var.value, "Korean MSX SMS 8000") == 0)
            config.type = Cartridge::CartridgeKoreanMSXSMS8000Mapper;
        else if (strcmp(var.value, "Korean SMS 32KB 2000") == 0)
            config.type = Cartridge::CartridgeKoreanSMS32KB2000Mapper;
        else if (strcmp(var.value, "Korean MSX 8KB 0300") == 0)
            config.type = Cartridge::CartridgeKoreanMSX8KB0300Mapper;
        else if (strcmp(var.value, "Korean 0000 XOR FF") == 0)
            config.type = Cartridge::CartridgeKorean0000XORFFMapper;
        else if (strcmp(var.value, "Korean FFFF HiCom") == 0)
            config.type = Cartridge::CartridgeKoreanFFFFHiComMapper;
        else if (strcmp(var.value, "Korean FFFE") == 0)
            config.type = Cartridge::CartridgeKoreanFFFEMapper;
        else if (strcmp(var.value, "Korean BFFC") == 0)
            config.type = Cartridge::CartridgeKoreanBFFCMapper;
        else if (strcmp(var.value, "Korean FFF3 FFFC") == 0)
            config.type = Cartridge::CartridgeKoreanFFF3FFFCMapper;
        else if (strcmp(var.value, "Korean MD FFF5") == 0)
            config.type = Cartridge::CartridgeKoreanMDFFF5Mapper;
        else if (strcmp(var.value, "Korean MD FFF0") == 0)
            config.type = Cartridge::CartridgeKoreanMDFFF0Mapper;
        else if (strcmp(var.value, "Jumbo Dahjee") == 0)
            config.type = Cartridge::CartridgeJumboDahjeeMapper;
        else if (strcmp(var.value, "EEPROM 93C46") == 0)
            config.type = Cartridge::CartridgeEeprom93C46Mapper;
        else if (strcmp(var.value, "Multi 4PAK All Action") == 0)
            config.type = Cartridge::CartridgeMulti4PAKAllActionMapper;
        else if (strcmp(var.value, "Iratahack") == 0)
            config.type = Cartridge::CartridgeIratahackMapper;
        else
            config.type = Cartridge::CartridgeNotSupported;
    }

    var.key = "gearsystem_timing";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Auto") == 0)
            config.region = Cartridge::CartridgeUnknownRegion;
        else if (strcmp(var.value, "NTSC (60 Hz)") == 0)
            config.region = Cartridge::CartridgeNTSC;
        else if (strcmp(var.value, "PAL (50 Hz)") == 0)
            config.region = Cartridge::CartridgePAL;
        else
            config.region = Cartridge::CartridgeUnknownRegion;
    }

    var.key = "gearsystem_aspect_ratio";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "1:1 PAR") == 0)
            aspect_ratio = 0.0f;
        else if (strcmp(var.value, "4:3 DAR") == 0)
            aspect_ratio = 4.0f / 3.0f;
        else if (strcmp(var.value, "16:9 DAR") == 0)
            aspect_ratio = 16.0f / 9.0f;
        else if (strcmp(var.value, "16:10 DAR") == 0)
            aspect_ratio = 16.0f / 10.0f;
        else
            aspect_ratio = 0.0f;
    }

    var.key = "gearsystem_overscan";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Disabled") == 0)
            core->GetVideo()->SetOverscan(Video::OverscanDisabled);
        else if (strcmp(var.value, "Top+Bottom") == 0)
            core->GetVideo()->SetOverscan(Video::OverscanTopBottom);
        else if (strcmp(var.value, "Full (284 width)") == 0)
            core->GetVideo()->SetOverscan(Video::OverscanFull284);
        else if (strcmp(var.value, "Full (320 width)") == 0)
            core->GetVideo()->SetOverscan(Video::OverscanFull320);
        else
            core->GetVideo()->SetOverscan(Video::OverscanDisabled);
    }

    var.key = "gearsystem_hide_left_bar";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "No") == 0)
            core->GetVideo()->SetHideLeftBar(Video::HideLeftBarNo);
        else if (strcmp(var.value, "Auto") == 0)
            core->GetVideo()->SetHideLeftBar(Video::HideLeftBarAuto);
        else if (strcmp(var.value, "Always") == 0)
            core->GetVideo()->SetHideLeftBar(Video::HideLeftBarAlways);
        else
            core->GetVideo()->SetHideLeftBar(Video::HideLeftBarNo);
    }

    var.key = "gearsystem_bios_sms";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Enabled") == 0)
            bootrom_sms = true;
        else
            bootrom_sms = false;
    }

    var.key = "gearsystem_bios_gg";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Enabled") == 0)
            bootrom_gg = true;
        else
            bootrom_gg = false;
    }

    var.key = "gearsystem_ym2413";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Auto") == 0)
            core->GetAudio()->DisableYM2413(false);
        else if (strcmp(var.value, "Disabled") == 0)
            core->GetAudio()->DisableYM2413(true);
        else
            core->GetAudio()->DisableYM2413(false);
    }

    var.key = "gearsystem_glasses";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Both Eyes / OFF") == 0)
            glasses_config = GearsystemCore::GlassesBothEyes;
        else if (strcmp(var.value, "Left Eye") == 0)
            glasses_config = GearsystemCore::GlassesLeftEye;
        else if (strcmp(var.value, "Right Eye") == 0)
            glasses_config = GearsystemCore::GlassesRightEye;
        else
            glasses_config = GearsystemCore::GlassesBothEyes;

        core->SetGlassesConfig(glasses_config);
    }
}
