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

#include "../../src/gearsystem.h"
#include "../audio-shared/Sound_Queue.h"

#define EMU_IMPORT
#include "emu.h"

static GearsystemCore* gearsystem;
static Sound_Queue* sound_queue;
static bool save_files_in_rom_dir = false;
static s16* audio_buffer;
static char base_save_path[260];
static bool audio_enabled;
static bool debugging = false;
static bool debug_step = false;
static bool debug_next_frame = false;

static void save_ram(void);
static void load_ram(void);
static const char* get_mapper(Cartridge::CartridgeTypes type);
static const char* get_zone(Cartridge::CartridgeZones zone);
static void init_debug(void);
static void update_debug(void);

void emu_init(const char* save_path)
{
    strcpy(base_save_path, save_path);

    int screen_size = GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT;

    emu_frame_buffer = new GS_Color[screen_size];
    
    for (int i=0; i < screen_size; i++)
    {
        emu_frame_buffer[i].red = 0;
        emu_frame_buffer[i].green = 0;
        emu_frame_buffer[i].blue = 0;
    }

    init_debug();

    gearsystem = new GearsystemCore();
    gearsystem->Init();

    sound_queue = new Sound_Queue();
    sound_queue->start(44100, 2);

    audio_buffer = new s16[GS_AUDIO_BUFFER_SIZE];

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;

    audio_enabled = true;
    emu_audio_sync = true;
    emu_debug_disable_breakpoints = false;

}

void emu_destroy(void)
{
    save_ram();
    SafeDeleteArray(audio_buffer);
    SafeDelete(sound_queue);
    SafeDelete(gearsystem);
    SafeDeleteArray(emu_frame_buffer);
}

void emu_load_rom(const char* file_path, bool save_in_rom_dir, Cartridge::ForceConfiguration config)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearsystem->LoadROM(file_path, config);
    load_ram();
    emu_debug_continue();
}

void emu_update(void)
{
    if (!emu_is_empty())
    {
        int sampleCount = 0;

        if (!debugging || debug_step || debug_next_frame)
        {
            bool breakpoints = !emu_debug_disable_breakpoints || IsValidPointer(gearsystem->GetMemory()->GetRunToBreakpoint());

            if (gearsystem->RunToVBlank(emu_frame_buffer, audio_buffer, &sampleCount, debug_step, breakpoints))
            {
                debugging = true;
            }

            debug_next_frame = false;
            debug_step = false;
        }

        update_debug();

        if ((sampleCount > 0) && !gearsystem->IsPaused())
        {
            sound_queue->write(audio_buffer, sampleCount, emu_audio_sync);
        }
    }
}

void emu_key_pressed(GS_Joypads pad, GS_Keys key)
{
    gearsystem->KeyPressed(pad, key);
}

void emu_key_released(GS_Joypads pad, GS_Keys key)
{
    gearsystem->KeyReleased(pad, key);
}

void emu_pause(void)
{
    gearsystem->Pause(true);
}

void emu_resume(void)
{
    gearsystem->Pause(false);
}

bool emu_is_paused(void)
{
    return gearsystem->IsPaused();
}

bool emu_is_empty(void)
{
    return !gearsystem->GetCartridge()->IsReady();
}

void emu_reset(bool save_in_rom_dir, Cartridge::ForceConfiguration config)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearsystem->ResetROM(config);
    load_ram();
}

void emu_memory_dump(void)
{
    gearsystem->GetMemory()->MemoryDump("memdump.txt");
}

void emu_audio_volume(float volume)
{
    audio_enabled = (volume > 0.0f);
    gearsystem->SetSoundVolume(volume);
}

void emu_audio_reset(void)
{
    sound_queue->stop();
    sound_queue->start(44100, 2);
}

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

void emu_save_ram(const char* file_path)
{
    if (!emu_is_empty())
        gearsystem->SaveRam(file_path, true);
}

void emu_load_ram(const char* file_path, bool save_in_rom_dir, Cartridge::ForceConfiguration config)
{
    if (!emu_is_empty())
    {
        save_files_in_rom_dir = save_in_rom_dir;
        save_ram();
        gearsystem->ResetROM(config);
        gearsystem->LoadRam(file_path, true);
    }
}

void emu_save_state_slot(int index)
{
    if (!emu_is_empty())
        gearsystem->SaveState(index);
}

void emu_load_state_slot(int index)
{
    if (!emu_is_empty())
        gearsystem->LoadState(index);
}

void emu_save_state_file(const char* file_path)
{
    if (!emu_is_empty())
        gearsystem->SaveState(file_path, -1);
}

void emu_load_state_file(const char* file_path)
{
    if (!emu_is_empty())
        gearsystem->LoadState(file_path, -1);
}

void emu_add_cheat(const char* cheat)
{
    gearsystem->SetCheat(cheat);
}

void emu_clear_cheats()
{
    gearsystem->ClearCheats();
}

void emu_get_runtime(GS_RuntimeInfo& runtime)
{
    gearsystem->GetRuntimeInfo(runtime);
}

void emu_get_info(char* info)
{
    if (!emu_is_empty())
    {
        Cartridge* cart = gearsystem->GetCartridge();
        GS_RuntimeInfo runtime;
        gearsystem->GetRuntimeInfo(runtime);

        const char* filename = cart->GetFileName();

        const char* system = cart->IsGameGear() ? "Game Gear" : (cart->IsSG1000() ? "SG-1000" : "Master System");
        const char* pal = cart->IsPAL() ? "PAL" : "NTSC";
        const char* checksum = cart->IsValidROM() ? "VALID" : "FAILED";
        const char* battery = gearsystem->GetMemory()->GetCurrentRule()->PersistedRAM() ? "YES" : "NO";
        int rom_banks = cart->GetROMBankCount();
        const char* mapper = get_mapper(cart->GetType());
        const char* zone = get_zone(cart->GetZone());

        sprintf(info, "File Name: %s\nMapper: %s\nRegion: %s\nSystem: %s\nRefresh Rate: %s\nCartridge Checksum: %s\nROM Banks: %d\nBattery: %s\nScreen Resolution: %dx%d", filename, mapper, zone, system, pal, checksum, rom_banks, battery, runtime.screen_width, runtime.screen_height);
    }
    else
    {
        sprintf(info, "No data!");
    }
}

GearsystemCore* emu_get_core(void)
{
    return gearsystem;
}

void emu_debug_step(void)
{
    debugging = debug_step = true;
    debug_next_frame = false;
    gearsystem->Pause(false);
}

void emu_debug_continue(void)
{
    debugging = debug_step = debug_next_frame = false;
    gearsystem->Pause(false);
}

void emu_debug_next_frame(void)
{
    debugging = debug_next_frame = true;
    debug_step = false;
    gearsystem->Pause(false);
}

static void save_ram(void)
{
    if (save_files_in_rom_dir)
        gearsystem->SaveRam();
    else
        gearsystem->SaveRam(base_save_path);
}

static void load_ram(void)
{
    if (save_files_in_rom_dir)
        gearsystem->LoadRam();
    else
        gearsystem->LoadRam(base_save_path);
}

static const char* get_mapper(Cartridge::CartridgeTypes type)
{
    switch (type)
    {
    case Cartridge::CartridgeRomOnlyMapper:
        return "ROM Only";
        break;
    case Cartridge::CartridgeSegaMapper:
        return "SEGA";
        break;
    case Cartridge::CartridgeCodemastersMapper:
        return "Codemasters";
        break;
    case Cartridge::CartridgeSG1000Mapper:
        return "SG-1000";
        break;
    case Cartridge::CartridgeKoreanMapper:
        return "Korean";
        break;
    case Cartridge::CartridgeNotSupported:
        return "Not Supported";
        break;
    default:
        return "Undefined";
        break;
    }
}

static const char* get_zone(Cartridge::CartridgeZones zone)
{
    switch (zone)
    {
    case Cartridge::CartridgeJapanSMS:
        return "Japan";
        break;
    case Cartridge::CartridgeExportSMS:
        return "Export";
        break;
    case Cartridge::CartridgeJapanGG:
        return "Game Gear Japan";
        break;
    case Cartridge::CartridgeExportGG:
        return "Game Gear Export";
        break;
    case Cartridge::CartridgeInternationalGG:
        return "Game Gear International";
        break;
    case Cartridge::CartridgeUnknownZone:
        return "Unknown";
        break;
    default:
        return "Undefined";
        break;
    }
}

static void init_debug(void)
{
    
}

static void update_debug(void)
{
    

}

