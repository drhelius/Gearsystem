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

static void save_ram(void);
static void load_ram(void);

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

    gearsystem = new GearsystemCore();
    gearsystem->Init();

    sound_queue = new Sound_Queue();
    sound_queue->start(44100, 2);

    audio_buffer = new s16[GS_AUDIO_BUFFER_SIZE];

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
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

        gearsystem->RunToVBlank(emu_frame_buffer, audio_buffer, &sampleCount);

        if ((sampleCount > 0) && !gearsystem->IsPaused())
        {
            sound_queue->write(audio_buffer, sampleCount, emu_audio_sync);
        }
    }
}

void emu_key_pressed(GS_Keys key)
{
    gearsystem->KeyPressed(Joypad_1, key);
}

void emu_key_released(GS_Keys key)
{
    gearsystem->KeyReleased(Joypad_1, key);
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

void emu_load_ram(const char* file_path, bool save_in_rom_dir)
{
    if (!emu_is_empty())
    {
        save_files_in_rom_dir = save_in_rom_dir;
        save_ram();
        gearsystem->ResetROM();
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
