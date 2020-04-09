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
static u16* frame_buffer_565;
static s16* audio_buffer;
static char base_save_path[260];
static bool audio_enabled;

static void save_ram(void);
static void load_ram(void);
static void generate_24bit_buffer(void);

void emu_init(const char* save_path)
{
    strcpy(base_save_path, save_path);

    frame_buffer_565 = new u16[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    emu_frame_buffer = new GS_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    
    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
    {
        emu_frame_buffer[i].red = 0;
        emu_frame_buffer[i].green = 0;
        emu_frame_buffer[i].blue = 0;
        frame_buffer_565[i] = 0;
    }

    gearsystem = new GearsystemCore();
    gearsystem->Init();

    sound_queue = new Sound_Queue();
    sound_queue->start(44100, 2);

    audio_buffer = new s16[AUDIO_BUFFER_SIZE];

    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;

    audio_enabled = true;
    emu_audio_sync = true;
}

void emu_destroy(void)
{
    save_ram();
    SafeDeleteArray(audio_buffer);
    SafeDelete(sound_queue);
    SafeDelete(gearsystem);
    SafeDeleteArray(emu_frame_buffer);
}

void emu_load_rom(const char* file_path, bool save_in_rom_dir)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearsystem->LoadROM(file_path);
    load_ram();
}

void emu_run_to_vblank(void)
{
    if (!emu_is_empty())
    {
        int sampleCount = 0;

        gearsystem->RunToVBlank(frame_buffer_565, audio_buffer, &sampleCount);

        generate_24bit_buffer();

        if ((sampleCount > 0) && !gearsystem->IsPaused())
        {
            sound_queue->write(audio_buffer, sampleCount, emu_audio_sync);
        }
    }
}

void emu_key_pressed(GS_Keys key)
{
    gearsystem->KeyPressed(key);
}

void emu_key_released(GS_Keys key)
{
    gearsystem->KeyReleased(key);
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
    return !gearsystem->GetCartridge()->IsLoadedROM();
}

void emu_reset(bool save_in_rom_dir)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearsystem->ResetROM();
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

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

void emu_save_ram(const char* file_path)
{
    if (!emu_is_empty())
        gearsystem->SaveRam(file_path, true);
}

void emu_load_ram(const char* file_path, bool force_dmg, bool save_in_rom_dir)
{
    if (!emu_is_empty())
    {
        save_files_in_rom_dir = save_in_rom_dir;
        save_ram();
        gearsystem->ResetROM(force_dmg);
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

static void generate_24bit_buffer(void)
{
    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
    {
        emu_frame_buffer[i].red = (((frame_buffer_565[i] >> 11) & 0x1F ) * 255 + 15) / 31;
        emu_frame_buffer[i].green = (((frame_buffer_565[i] >> 5) & 0x3F ) * 255 + 31) / 63;
        emu_frame_buffer[i].blue = ((frame_buffer_565[i] & 0x1F ) * 255 + 15) / 31;
    }
}