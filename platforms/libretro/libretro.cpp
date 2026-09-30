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
#include <string>
#include <sstream>
#include <vector>

#include "libretro.h"
#include "../../src/gearsystem.h"
#include "libretro_core_options.h"
#include "libretro_link.h"

#ifdef _WIN32
static const char slash = '\\';
#else
static const char slash = '/';
#endif

#define RETRO_DEVICE_SMS_GG_PAD     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)
#define RETRO_DEVICE_LIGHT_PHASER   RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 0)
#define RETRO_DEVICE_PADDLE         RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 0)
#define RETRO_DEVICE_SPORTS_PAD     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 1)

static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static struct retro_log_callback logging;
retro_log_printf_t log_cb;

static char retro_system_directory[4096];
static char retro_game_path[4096];

static LibretroInstance instances[2];
static unsigned instance_count = 0;
static LibretroLink* link_cable = NULL;
static bool link_enabled = false;
static bool link_vertical = false;
static bool link_switched = false;
static int link_screen = 0;
static int link_audio = 0;
static bool link_subsystem = false;
static bool game_loaded = false;

static unsigned input_device[2] = {
    RETRO_DEVICE_SMS_GG_PAD,
    RETRO_DEVICE_SMS_GG_PAD
};
static bool allow_up_down = false;
static int8_t dpad_vertical_latch[2] = { 0, 0 };
static int8_t dpad_horizontal_latch[2] = { 0, 0 };
static bool lightgun_touchscreen = false;
static bool lightgun_crosshair = false;
static Video::LightPhaserCrosshairShape lightgun_crosshair_shape = Video::LightPhaserCrosshairCross;
static Video::LightPhaserCrosshairColor lightgun_crosshair_color = Video::LightPhaserCrosshairWhite;
static int paddle_sensitivity = 0;
static int sports_pad_sensitivity = 8;
static bool bootrom_sms = false;
static bool bootrom_gg = false;
static bool libretro_supports_bitmasks = false;
static bool categories_supported = false;
static float aspect_ratio = 0.0f;
static int current_screen_width = 0;
static int current_screen_height = 0;
static float current_aspect_ratio = 0;

static GearsystemCore* core;
static Cartridge::ForceConfiguration config;
static GearsystemCore::GlassesConfig glasses_config;
static const retro_vfs_interface* vfs_interface = NULL;
static std::vector<std::string> libretro_cheats;

static void init_instances(unsigned count);
static void load_bootroms(GearsystemCore* target);
static bool load_game(const struct retro_game_info* first, const struct retro_game_info* second);
static void apply_variables(GearsystemCore* core);
static void set_controller_info(void);
static void clear_input_state(void);
static void reset_controller_devices(void);
static void apply_controller_device(unsigned port, unsigned device, bool log_device);
static void release_controller_input(unsigned port);
static void update_input(void);
static void check_variables(void);

static void apply_cheats(void)
{
    for (unsigned i = 0; i < instance_count; i++)
    {
        instances[i].core->ClearCheats();

        for (size_t j = 0; j < libretro_cheats.size(); j++)
        {
            if (!libretro_cheats[j].empty())
                instances[i].core->SetCheat(libretro_cheats[j].c_str());
        }
    }
}

static void clear_cheats(void)
{
    libretro_cheats.clear();
    for (unsigned i = 0; i < instance_count; i++)
        instances[i].core->ClearCheats();
}

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
    (void)cb;
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
    static const struct retro_subsystem_memory_info memory1[] = {
        { "srm", GEARSYSTEM_LINK_RAM_1 }
    };
    static const struct retro_subsystem_memory_info memory2[] = {
        { "srm2", GEARSYSTEM_LINK_RAM_2 }
    };
    static const struct retro_subsystem_rom_info roms[] = {
        { "Screen 1", "gg|bin|rom", false, false, true, memory1, 1 },
        { "Screen 2", "gg|bin|rom", false, false, true, memory2, 1 }
    };
    static const struct retro_subsystem_info subsystems[] = {
        { "2 Player Game Gear Link", "gg_link_2p", roms, 2, GEARSYSTEM_LINK_SUBSYSTEM },
        { NULL, NULL, NULL, 0, 0 }
    };
    environ_cb(RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO, (void*)subsystems);
    libretro_set_core_options(environ_cb, &categories_supported);
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

    log_cb(RETRO_LOG_INFO, "%s (%s) libretro\n", GS_TITLE, EMULATOR_BUILD);

    struct retro_vfs_interface_info vfs_interface_info = {};
    vfs_interface_info.required_interface_version = 1;
    vfs_interface_info.iface = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_interface_info) &&
        vfs_interface_info.iface && vfs_interface_info.iface->open &&
        vfs_interface_info.iface->close && vfs_interface_info.iface->size &&
        vfs_interface_info.iface->read)
        vfs_interface = vfs_interface_info.iface;
    else
        vfs_interface = NULL;

    init_instances(1);

    config.type = Cartridge::CartridgeNotSupported;
    config.zone = Cartridge::CartridgeUnknownZone;
    config.region = Cartridge::CartridgeUnknownRegion;
    config.system = Cartridge::CartridgeUnknownSystem;

    glasses_config = GearsystemCore::GlassesBothEyes;

    clear_input_state();

    for (int i = 0; i < 2; i++)
        apply_controller_device(i, input_device[i], false);

    libretro_supports_bitmasks = environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL);
}

static void init_instances(unsigned count)
{
    for (unsigned i = instance_count; i < count; i++)
    {
        memset(&instances[i], 0, sizeof(instances[i]));
        instances[i].core = new GearsystemCore();
#ifdef PS2
        instances[i].core->Init(GS_PIXEL_BGR555);
#else
        instances[i].core->Init(GS_PIXEL_RGB565);
#endif
    }
    instance_count = count;
    core = instances[0].core;
}

void retro_deinit(void)
{
    retro_unload_game();
    vfs_interface = NULL;

    current_screen_width = 0;
    current_screen_height = 0;
    current_aspect_ratio = 0.0f;
    aspect_ratio = 0.0f;
    libretro_supports_bitmasks = false;

    reset_controller_devices();
    clear_input_state();
}

void retro_reset(void)
{
    if (!game_loaded)
        return;

    log_cb(RETRO_LOG_DEBUG, "Resetting...\n");
    check_variables();
    clear_input_state();

    for (unsigned i = 0; i < instance_count; i++)
    {
        GearsystemCore* target = instances[i].core;
        if (link_cable)
        {
            // Preserve frontend-loaded saves even before the cartridge enables RAM.
            MemoryRule* rule = target->GetMemory()->GetCurrentRule();
            std::stringstream ram;
            rule->SaveRam(ram);
            s32 size = (s32)ram.tellp();
            target->ResetROM();
            if (size > 0)
                rule->LoadRam(ram, size);
        }
        else
        {
            load_bootroms(target);
            target->ResetROMPreservingRAM(&config);
        }

        memset(instances[i].frame_buffer, 0, sizeof(instances[i].frame_buffer));
        instances[i].sample_count = 0;
    }

    if (link_cable)
        link_cable->Reset();
    apply_cheats();
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    if (port > 1)
    {
        if (log_cb)
            log_cb(RETRO_LOG_DEBUG, "retro_set_controller_port_device invalid port number: %u\n", port);
        return;
    }

    if ((input_device[port] != device) && core)
        release_controller_input(port);

    input_device[port] = device;

    apply_controller_device(port, device, true);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = GS_TITLE;
    info->library_version  = GS_VERSION;
    info->need_fullpath    = false;
    info->valid_extensions = "sms|gg|sg|mv|bin|rom";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    GS_RuntimeInfo runtime_info = {};
    runtime_info.screen_width = GS_RESOLUTION_SMS_WIDTH;
    runtime_info.screen_height = GS_RESOLUTION_SMS_HEIGHT;
    runtime_info.fps = (double)GS_MASTER_CLOCK_NTSC / (GS_LINES_PER_FRAME_NTSC * GS_CYCLES_PER_LINE);
    if (game_loaded)
        core->GetRuntimeInfo(runtime_info);

    unsigned width = runtime_info.screen_width;
    unsigned height = runtime_info.screen_height;
    if (link_cable)
        link_cable->Geometry(link_vertical, link_screen, &width, &height);

    info->geometry.base_width   = width;
    info->geometry.base_height  = height;
    info->geometry.max_width    = GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN;
    info->geometry.max_height   = GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN;
    info->geometry.aspect_ratio = link_cable ? (float)width / height : aspect_ratio;
    info->timing.fps            = runtime_info.fps;
    info->timing.sample_rate    = 44100.0;
}

void retro_run(void)
{
    if (!game_loaded)
        return;

    bool core_options_updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &core_options_updated) && core_options_updated)
        check_variables();

    update_input();
    for (unsigned i = 0; i < instance_count; i++)
        instances[i].sample_count = 0;

    if (link_cable)
        link_cable->RunFrame();
    else
        core->RunToVBlank((u8*)instances[0].frame_buffer, instances[0].audio_buffer, &instances[0].sample_count);

    struct retro_system_av_info info;
    retro_get_system_av_info(&info);
    if ((int)info.geometry.base_width != current_screen_width ||
        (int)info.geometry.base_height != current_screen_height ||
        info.geometry.aspect_ratio != current_aspect_ratio)
    {
        current_screen_width = info.geometry.base_width;
        current_screen_height = info.geometry.base_height;
        current_aspect_ratio = info.geometry.aspect_ratio;
        environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &info.geometry);
    }

    const u16* video = instances[0].frame_buffer;
    const s16* audio = instances[0].audio_buffer;
    int samples = instances[0].sample_count;
    if (link_cable)
    {
        video = link_cable->Video(link_vertical, link_switched, link_screen);
        audio = link_cable->Audio(link_audio, &samples);
    }

    video_cb(video, current_screen_width, current_screen_height, current_screen_width * sizeof(u16));
    if (samples > 0)
        audio_batch_cb(audio, samples / 2);
}

static bool load_rom(GearsystemCore* target, const struct retro_game_info* info)
{
    if (!info)
        return false;

    const char* path = info->path ? info->path : "";

    if (info->size > 0x7FFFFFFF)
        return false;

    if (IsValidPointer(info->data) && (info->size > 0))
        return target->LoadROMFromBuffer(reinterpret_cast<const u8*>(info->data), info->size, &config, path);

    if (!path[0])
        return false;

    if (!vfs_interface)
        return target->LoadROM(path, &config);

    retro_vfs_file_handle* file = vfs_interface->open(path, RETRO_VFS_FILE_ACCESS_READ,
        RETRO_VFS_FILE_ACCESS_HINT_NONE);
    if (!file)
        return false;

    s64 size = (s64)vfs_interface->size(file);
    if ((size <= 0) || (size > 0x7FFFFFFF))
    {
        vfs_interface->close(file);
        return false;
    }

    u8* buffer = new u8[(int)size];
    s64 total = 0;

    while (total < size)
    {
        s64 read = (s64)vfs_interface->read(file, buffer + total, size - total);
        if (read <= 0)
            break;

        total += read;
    }

    bool loaded = vfs_interface->close(file) == 0 && total == size;
    if (loaded)
        loaded = target->LoadROMFromBuffer(buffer, (int)size, &config, path);

    SafeDeleteArray(buffer);
    return loaded;
}

bool retro_load_game(const struct retro_game_info* info)
{
    return load_game(info, NULL);
}

static bool load_game(const struct retro_game_info* info, const struct retro_game_info* second)
{
    retro_unload_game();
    if (!info)
        return false;

    init_instances(1);
    check_variables();
    load_bootroms(core);
    link_subsystem = second != NULL;
    snprintf(retro_game_path, sizeof(retro_game_path), "%s", info->path ? info->path : "");
    log_cb(RETRO_LOG_INFO, "Loading game: %s\n", retro_game_path);

    if (!load_rom(core, info))
    {
        log_cb(RETRO_LOG_ERROR, "Invalid or corrupted ROM for screen 1.\n");
        retro_unload_game();
        return false;
    }

    if (link_subsystem || (link_enabled && core->IsNativeGameGearMode()))
    {
        init_instances(2);
        check_variables();
        load_bootroms(instances[1].core);
        if (!load_rom(instances[1].core, second ? second : info) ||
            !core->IsNativeGameGearMode() || !instances[1].core->IsNativeGameGearMode() ||
            core->GetCartridge()->IsPAL() != instances[1].core->GetCartridge()->IsPAL())
        {
            log_cb(RETRO_LOG_ERROR, "Game Gear linking requires two native Game Gear ROMs with matching timing.\n");
            retro_unload_game();
            return false;
        }
    }

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_ERROR, "RGB565 is not supported.\n");
        retro_unload_game();
        return false;
    }

    if (instance_count == 2)
    {
        link_cable = new LibretroLink(instances, dpad_vertical_latch, dpad_horizontal_latch);
        link_cable->Reset();
    }
    game_loaded = true;
    clear_input_state();
    set_controller_info();
    for (unsigned i = 0; i < 2; i++)
        retro_set_controller_port_device(i, input_device[i]);

    struct retro_system_av_info av_info;
    retro_get_system_av_info(&av_info);
    current_screen_width = av_info.geometry.base_width;
    current_screen_height = av_info.geometry.base_height;
    current_aspect_ratio = av_info.geometry.aspect_ratio;

    if (link_cable && !link_subsystem)
    {
        const char* directory = NULL;
        environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &directory);
        link_cable->SetSavePath(info->path, directory);
        link_cable->PersistentMemory(false, vfs_interface);
    }

    bool achievements = !link_cable;
    environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS, &achievements);

    Cartridge* cart = core->GetCartridge();

    log_cb(RETRO_LOG_INFO, "CRC: %08X\n", cart->GetCRC());
    const char* system_name = "Master System";
    if (cart->IsGameGear())
    {
        if (cart->GetGameGearASIC() == 1)
            system_name = cart->IsGameGearInSMSMode() ? "Game Gear (1 ASIC) SMS Mode" : "Game Gear (1 ASIC)";
        else
            system_name = cart->IsGameGearInSMSMode() ? "Game Gear (2 ASIC) SMS Mode" : "Game Gear (2 ASIC)";
    }
    else if (cart->IsSG1000())
        system_name = "SG-1000";
    log_cb(RETRO_LOG_INFO, "System: %s\n", system_name);
    log_cb(RETRO_LOG_INFO, "Refresh Rate: %s\n", cart->IsPAL() ? "PAL" : "NTSC");
    log_cb(RETRO_LOG_INFO, "Cartridge Header: %s\n", cart->IsValidROM() ? "VALID" : "FAILED");
    log_cb(RETRO_LOG_INFO, "Battery: %s\n", core->GetMemory()->GetCurrentRule()->PersistedRAM() ? "YES" : "NO");

    return true;
}

void retro_unload_game(void)
{
    if (game_loaded && link_cable && !link_subsystem)
        link_cable->PersistentMemory(true, vfs_interface);

    clear_input_state();
    clear_cheats();
    SafeDelete(link_cable);
    for (unsigned i = 0; i < instance_count; i++)
    {
        SafeDelete(instances[i].core);
        instances[i].sample_count = 0;
    }
    core = NULL;
    instance_count = 0;
    game_loaded = false;
    link_subsystem = false;
    retro_game_path[0] = 0;
    current_screen_width = 0;
    current_screen_height = 0;
    current_aspect_ratio = 0.0f;
    set_controller_info();
}

unsigned retro_get_region(void)
{
    return game_loaded && core->GetCartridge()->IsPAL() ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info* info, size_t num)
{
    if (type != GEARSYSTEM_LINK_SUBSYSTEM || !info || num != 2)
        return false;
    return load_game(&info[0], &info[1]);
}

size_t retro_serialize_size(void)
{
    if (!game_loaded)
        return 0;
    if (link_cable)
        return link_cable->StateSize();

    size_t size = 0;
    core->SaveState(NULL, size);
    return size;
}

bool retro_serialize(void* data, size_t size)
{
    if (!game_loaded || !data)
        return false;
    return link_cable ? link_cable->SaveState(data, size) : core->SaveState((u8*)data, size);
}

bool retro_unserialize(const void* data, size_t size)
{
    if (!game_loaded)
        return false;
    return link_cable ? link_cable->LoadState(data, size) : core->LoadState((const u8*)data, size);
}

static GearsystemCore* memory_instance(unsigned id)
{
    if (!game_loaded)
        return NULL;
    if (id < 0x100)
        return instances[0].core;

    unsigned index = (id >> 8) - 1;
    if (index >= instance_count || (id & 0xFF) != RETRO_MEMORY_SAVE_RAM)
        return NULL;
    return instances[index].core;
}

void* retro_get_memory_data(unsigned id)
{
    GearsystemCore* target = memory_instance(id);
    if (!target)
        return NULL;

    switch (id & 0xFF)
    {
        case RETRO_MEMORY_SAVE_RAM:
            return target->GetMemory()->GetCurrentRule()->GetRamBanks();
        case RETRO_MEMORY_SYSTEM_RAM:
            return target->GetMemory()->GetMemoryMap() + 0xC000;
    }
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    GearsystemCore* target = memory_instance(id);
    if (!target)
        return 0;

    switch (id & 0xFF)
    {
        case RETRO_MEMORY_SAVE_RAM:
            return target->GetMemory()->GetCurrentRule()->GetRamSize();
        case RETRO_MEMORY_SYSTEM_RAM:
            return 0x2000;
    }
    return 0;
}

void retro_cheat_reset(void)
{
    clear_cheats();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    if (enabled)
    {
        if (index >= libretro_cheats.size())
            libretro_cheats.resize(index + 1);

        libretro_cheats[index] = code ? code : "";
    }
    else if (index < libretro_cheats.size())
    {
        libretro_cheats[index].clear();
    }

    apply_cheats();
}

static bool load_bootrom_file(GearsystemCore* target, const char* path, bool gg)
{
    if (!vfs_interface)
    {
        if (gg)
            target->GetMemory()->LoadBootromGG(path);
        else
            target->GetMemory()->LoadBootromSMS(path);

        return target->GetMemory()->IsBootromLoaded(gg);
    }

    target->GetMemory()->UnloadBootrom(gg);

    retro_vfs_file_handle* file = vfs_interface->open(path, RETRO_VFS_FILE_ACCESS_READ,
        RETRO_VFS_FILE_ACCESS_HINT_NONE);
    if (!file)
    {
        log_cb(RETRO_LOG_ERROR, "There was a problem opening the file %s\n", path);
        return false;
    }

    s64 size = (s64)vfs_interface->size(file);
    if ((size <= 0) || (size > 0x7FFFFFFF))
    {
        log_cb(RETRO_LOG_ERROR, "Invalid bootrom size %lld: %s\n", (long long)size, path);
        vfs_interface->close(file);
        return false;
    }

    u8* bootrom = new u8[(int)size];
    s64 total = 0;

    while (total < size)
    {
        s64 read = (s64)vfs_interface->read(file, bootrom + total, size - total);
        if (read <= 0)
            break;

        total += read;
    }

    vfs_interface->close(file);

    bool loaded = (total == size) && target->GetMemory()->LoadBootromFromBuffer(bootrom, (int)size, gg);
    SafeDeleteArray(bootrom);

    if (!loaded)
    {
        log_cb(RETRO_LOG_ERROR, "There was a problem reading the bootrom file %s\n", path);
        return false;
    }

    log_cb(RETRO_LOG_INFO, "Bootrom %s loaded (%lld bytes)\n", path, (long long)size);
    return true;
}

static void load_bootroms(GearsystemCore* target)
{
    char bootrom_sms_path[4112];
    char bootrom_gg_path[4112];

    snprintf(bootrom_sms_path, sizeof(bootrom_sms_path), "%s%cbios.sms", retro_system_directory, slash);
    snprintf(bootrom_gg_path, sizeof(bootrom_gg_path), "%s%cbios.gg", retro_system_directory, slash);

    load_bootrom_file(target, bootrom_sms_path, false);
    load_bootrom_file(target, bootrom_gg_path, true);
    target->GetMemory()->EnableBootromSMS(bootrom_sms);
    target->GetMemory()->EnableBootromGG(bootrom_gg);
}

static void set_controller_info(void)
{
    static const struct retro_controller_description port_1[] = {
        { "Joypad Auto", RETRO_DEVICE_JOYPAD },
        { "Joypad Port Empty", RETRO_DEVICE_NONE },
        { "Master System / Game Gear Pad", RETRO_DEVICE_SMS_GG_PAD },
        { "Sega Light Phaser", RETRO_DEVICE_LIGHT_PHASER },
        { "Paddle Control", RETRO_DEVICE_PADDLE },
        { "Sports Pad", RETRO_DEVICE_SPORTS_PAD },
    };

    static const struct retro_controller_description port_2[] = {
        { "Joypad Auto", RETRO_DEVICE_JOYPAD },
        { "Joypad Port Empty", RETRO_DEVICE_NONE },
        { "Master System / Game Gear Pad", RETRO_DEVICE_SMS_GG_PAD },
        { "Sports Pad", RETRO_DEVICE_SPORTS_PAD },
    };

    static const struct retro_controller_info ports[] = {
        { port_1, 6 },
        { port_2, 4 },
        { NULL, 0 },
    };

    static const struct retro_controller_info linked_ports[] = {
        { port_1, 3 },
        { port_2, 3 },
        { NULL, 0 },
    };
    environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)(link_cable ? linked_ports : ports));

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

static void clear_input_state(void)
{
    for (unsigned i = 0; i < 2; i++)
        release_controller_input(i);
}

static void reset_controller_devices(void)
{
    for (int i = 0; i < 2; i++)
        input_device[i] = RETRO_DEVICE_SMS_GG_PAD;
}

static void apply_controller_device(unsigned port, unsigned device, bool log_device)
{
    if (!core)
        return;

    if (link_cable)
    {
        GearsystemCore* target = instances[port].core;
        target->EnablePhaser(false);
        target->EnablePaddle(false);
        target->EnableSportsPad(Joypad_1, false);
        target->EnableSportsPad(Joypad_2, false);
        return;
    }

    bool phaser = false;
    bool paddle = false;
    bool sports_pad = false;

    switch (device)
    {
        case RETRO_DEVICE_NONE:
            if (log_device && log_cb)
                log_cb(RETRO_LOG_INFO, "Controller %u: Unplugged\n", port);
            break;
        case RETRO_DEVICE_SMS_GG_PAD:
        case RETRO_DEVICE_JOYPAD:
            if (log_device && log_cb)
                log_cb(RETRO_LOG_INFO, "Controller %u: SMS/GG Pad\n", port);
            break;
        case RETRO_DEVICE_LIGHT_PHASER:
            if (log_device && log_cb)
                log_cb(RETRO_LOG_INFO, "Controller %u: Light Phaser\n", port);
            phaser = true;
            break;
        case RETRO_DEVICE_PADDLE:
            if (log_device && log_cb)
                log_cb(RETRO_LOG_INFO, "Controller %u: Paddle\n", port);
            paddle = true;
            break;
        case RETRO_DEVICE_SPORTS_PAD:
            if (log_device && log_cb)
                log_cb(RETRO_LOG_INFO, "Controller %u: Sports Pad\n", port);
            sports_pad = true;
            break;
        default:
            if (log_device && log_cb)
                log_cb(RETRO_LOG_DEBUG, "Setting descriptors for unsupported device.\n");
            break;
    }

    if (port == 0)
    {
        core->EnablePhaser(phaser);
        core->EnablePaddle(paddle);
    }

    core->EnableSportsPad((GS_Joypads)port, sports_pad);
}

static void release_controller_input(unsigned port)
{
    GearsystemCore* target = link_cable ? instances[port].core : core;
    GS_Joypads joypad = link_cable ? Joypad_1 : (GS_Joypads)port;

    if (target)
    {
        target->KeyReleased(joypad, Key_Up);
        target->KeyReleased(joypad, Key_Down);
        target->KeyReleased(joypad, Key_Left);
        target->KeyReleased(joypad, Key_Right);
        target->KeyReleased(joypad, Key_1);
        target->KeyReleased(joypad, Key_2);
        target->KeyReleased(joypad, Key_Start);
    }

    dpad_vertical_latch[port] = 0;
    dpad_horizontal_latch[port] = 0;

    if (target)
        target->SetReset(false);
}

static void update_input(void)
{
    input_poll_cb();
    bool reset_pressed = false;

    for (int player = 0; player < 2; player++)
    {
        GearsystemCore* target = link_cable ? instances[player].core : core;
        GS_Joypads joypad = link_cable ? Joypad_1 : (GS_Joypads)player;

        unsigned device = link_cable && input_device[player] != RETRO_DEVICE_NONE ? RETRO_DEVICE_SMS_GG_PAD : input_device[player];

        switch (device)
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

            bool raw_up = (ib & (1 << RETRO_DEVICE_ID_JOYPAD_UP)) != 0;
            bool raw_down = (ib & (1 << RETRO_DEVICE_ID_JOYPAD_DOWN)) != 0;
            bool raw_left = (ib & (1 << RETRO_DEVICE_ID_JOYPAD_LEFT)) != 0;
            bool raw_right = (ib & (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT)) != 0;
            bool up = raw_up;
            bool down = raw_down;
            bool left = raw_left;
            bool right = raw_right;

            if (!allow_up_down)
            {
                if (raw_up && raw_down)
                {
                    if (dpad_vertical_latch[player] > 0)
                    {
                        up = true;
                        down = false;
                    }
                    else if (dpad_vertical_latch[player] < 0)
                    {
                        up = false;
                        down = true;
                    }
                    else
                    {
                        up = true;
                        down = false;
                        dpad_vertical_latch[player] = 1;
                    }
                }
                else if (raw_up)
                {
                    up = true;
                    down = false;
                    dpad_vertical_latch[player] = 1;
                }
                else if (raw_down)
                {
                    up = false;
                    down = true;
                    dpad_vertical_latch[player] = -1;
                }
                else
                {
                    up = false;
                    down = false;
                    dpad_vertical_latch[player] = 0;
                }

                if (raw_left && raw_right)
                {
                    if (dpad_horizontal_latch[player] > 0)
                    {
                        left = true;
                        right = false;
                    }
                    else if (dpad_horizontal_latch[player] < 0)
                    {
                        left = false;
                        right = true;
                    }
                    else
                    {
                        left = true;
                        right = false;
                        dpad_horizontal_latch[player] = 1;
                    }
                }
                else if (raw_left)
                {
                    left = true;
                    right = false;
                    dpad_horizontal_latch[player] = 1;
                }
                else if (raw_right)
                {
                    left = false;
                    right = true;
                    dpad_horizontal_latch[player] = -1;
                }
                else
                {
                    left = false;
                    right = false;
                    dpad_horizontal_latch[player] = 0;
                }
            }

            if (up)
                target->KeyPressed(joypad, Key_Up);
            else
                target->KeyReleased(joypad, Key_Up);

            if (down)
                target->KeyPressed(joypad, Key_Down);
            else
                target->KeyReleased(joypad, Key_Down);

            if (left)
                target->KeyPressed(joypad, Key_Left);
            else
                target->KeyReleased(joypad, Key_Left);

            if (right)
                target->KeyPressed(joypad, Key_Right);
            else
                target->KeyReleased(joypad, Key_Right);

            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_B))
                target->KeyPressed(joypad, Key_1);
            else
                target->KeyReleased(joypad, Key_1);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_A))
                target->KeyPressed(joypad, Key_2);
            else
                target->KeyReleased(joypad, Key_2);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_START))
                target->KeyPressed(joypad, Key_Start);
            else
                target->KeyReleased(joypad, Key_Start);
            if (ib & (1 << RETRO_DEVICE_ID_JOYPAD_SELECT))
                reset_pressed = true;

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

                    target->SetPhaser(x, y);

                    if (input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED))
                        target->KeyPressed(static_cast<GS_Joypads>(0), Key_1);
                    else
                        target->KeyReleased(static_cast<GS_Joypads>(0), Key_1);
                }
                else
                {
                    if (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN) )
                    {
                        target->SetPhaser(-1000, -1000);
                    }
                    else
                    {
                        s16 x = input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X);
                        s16 y = input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y);
                        x = ((x + 0x7fff) * current_screen_width) / 0xfffe;
                        y = ((y + 0x7fff) * current_screen_height) / 0xfffe;

                        target->SetPhaser(x, y);
                    }

                    if (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER))
                        target->KeyPressed(static_cast<GS_Joypads>(0), Key_1);
                    else
                        target->KeyReleased(static_cast<GS_Joypads>(0), Key_1);
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
                target->SetPaddle(relx);

                if (input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT))
                    target->KeyPressed(static_cast<GS_Joypads>(0), Key_1);
                else
                    target->KeyReleased(static_cast<GS_Joypads>(0), Key_1);
            }

            break;
        }
        case RETRO_DEVICE_SPORTS_PAD:
        {
            s16 x = input_state_cb(player, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);
            s16 y = input_state_cb(player, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);

            if ((x > -4096) && (x < 4096))
                x = 0;
            if ((y > -4096) && (y < 4096))
                y = 0;

            int sen = sports_pad_sensitivity;
            if (sen < 1)
                sen = 1;
            float sensitivity = (float)sen / 32768.0f;
            target->MoveSportsPad(joypad, x * sensitivity, y * sensitivity);

            if (input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
                target->KeyPressed(joypad, Key_1);
            else
                target->KeyReleased(joypad, Key_1);
            if (input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
                target->KeyPressed(joypad, Key_2);
            else
                target->KeyReleased(joypad, Key_2);
            if (input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
                target->KeyPressed(joypad, Key_Start);
            else
                target->KeyReleased(joypad, Key_Start);
            if (input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT))
                reset_pressed = true;

            break;
        }
        default:
            break;
        }
    }

    for (unsigned i = 0; i < instance_count; i++)
        instances[i].core->SetReset(reset_pressed);
}

static void apply_variables(GearsystemCore* core)
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

    var.key = "gearsystem_sports_pad_sensitivity";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        sports_pad_sensitivity = atoi(var.value);
    }

    var.key = "gearsystem_system";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Auto") == 0)
            config.system = Cartridge::CartridgeUnknownSystem;
        else if (strcmp(var.value, "Master System / Mark III") == 0)
            config.system = Cartridge::CartridgeSMS;
        else if (strcmp(var.value, "Game Gear (2 ASIC)") == 0)
            config.system = Cartridge::CartridgeGG2ASIC;
        else if (strcmp(var.value, "Game Gear (2 ASIC) SMS Mode") == 0)
            config.system = Cartridge::CartridgeGG2ASICSMSMode;
        else if (strcmp(var.value, "Game Gear (1 ASIC)") == 0)
            config.system = Cartridge::CartridgeGG1ASIC;
        else if (strcmp(var.value, "Game Gear (1 ASIC) SMS Mode") == 0)
            config.system = Cartridge::CartridgeGG1ASICSMSMode;
        else if (strcmp(var.value, "SG-1000 / Multivision") == 0)
            config.system = Cartridge::CartridgeSG1000;
        else if (strcmp(var.value, "SG-1000 II") == 0)
            config.system = Cartridge::CartridgeSG1000II;
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

    var.key = "gearsystem_no_sprite_limit";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Enabled") == 0)
            core->GetVideo()->SetNoSpriteLimit(true);
        else
            core->GetVideo()->SetNoSpriteLimit(false);
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

    var.key = "gearsystem_psg_volume";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int volume = atoi(var.value);
        if (volume < 0 || volume > 200)
            volume = 100;
        float volume_f = (float)volume / 100.0f;
        core->GetAudio()->SetPSGVolume(volume_f);
    }

    var.key = "gearsystem_fm_volume";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int volume = atoi(var.value);
        if (volume < 0 || volume > 200)
            volume = 100;
        float volume_f = (float)volume / 100.0f;
        core->GetAudio()->SetFMVolume(volume_f);
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

static void check_variables(void)
{
    struct retro_variable var = {};

    var.key = "gearsystem_link_enable";
    var.value = NULL;
    link_enabled = environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && strcmp(var.value, "Enabled") == 0;

    var.key = "gearsystem_link_placement";
    var.value = NULL;
    link_vertical = environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && strcmp(var.value, "Vertical") == 0;

    var.key = "gearsystem_link_switch";
    var.value = NULL;
    link_switched = environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value && strcmp(var.value, "Enabled") == 0;

    var.key = "gearsystem_link_screen";
    var.value = NULL;
    link_screen = 0;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Screen 1") == 0)
            link_screen = 1;
        else if (strcmp(var.value, "Screen 2") == 0)
            link_screen = 2;
        else
            link_screen = 0;
    }

    var.key = "gearsystem_link_audio";
    var.value = NULL;
    link_audio = 0;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Screen 2") == 0)
            link_audio = 1;
        else if (strcmp(var.value, "Mix") == 0)
            link_audio = 2;
        else
            link_audio = 0;
    }

    for (unsigned i = 0; i < instance_count; i++)
        apply_variables(instances[i].core);
}
