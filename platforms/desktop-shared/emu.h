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

#ifndef EMU_H
#define	EMU_H

#include "../../src/gearsystem.h"

#ifdef EMU_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN u8* emu_frame_buffer;
EXTERN u8* emu_debug_background_buffer;
EXTERN u8* emu_debug_tile_buffer;
EXTERN u8* emu_debug_sprite_buffers[64];

EXTERN bool emu_audio_sync;
EXTERN bool emu_debug_disable_breakpoints_cpu;
EXTERN bool emu_debug_disable_breakpoints_mem;
EXTERN int emu_debug_tile_palette;
EXTERN bool emu_savefiles_dir_option;
EXTERN bool emu_savestates_dir_option;
EXTERN char emu_savefiles_path[4096];
EXTERN char emu_savestates_path[4096];

EXTERN void emu_init(void);
EXTERN void emu_destroy(void);
EXTERN void emu_update(void);
EXTERN void emu_load_rom(const char* file_path, Cartridge::ForceConfiguration config);
EXTERN void emu_key_pressed(GS_Joypads pad, GS_Keys key);
EXTERN void emu_key_released(GS_Joypads pad, GS_Keys key);
EXTERN void emu_pause(void);
EXTERN void emu_resume(void);
EXTERN bool emu_is_paused(void);
EXTERN bool emu_is_empty(void);
EXTERN void emu_reset(Cartridge::ForceConfiguration config);
EXTERN void emu_memory_dump(void);
EXTERN void emu_dissasemble_rom(void);
EXTERN void emu_audio_volume(float volume);
EXTERN void emu_audio_reset(void);
EXTERN bool emu_is_audio_enabled(void);
EXTERN void emu_save_ram(const char* file_path);
EXTERN void emu_load_ram(const char* file_path, Cartridge::ForceConfiguration config);
EXTERN void emu_save_state_slot(int index);
EXTERN void emu_load_state_slot(int index);
EXTERN void emu_save_state_file(const char* file_path);
EXTERN void emu_load_state_file(const char* file_path);
EXTERN void emu_add_cheat(const char* cheat);
EXTERN void emu_clear_cheats();
EXTERN void emu_get_runtime(GS_RuntimeInfo& runtime);
EXTERN void emu_get_info(char* info);
EXTERN GearsystemCore* emu_get_core(void);
EXTERN void emu_debug_step(void);
EXTERN void emu_debug_continue(void);
EXTERN void emu_debug_next_frame(void);
EXTERN void emu_load_bootrom_sms(const char* file_path);
EXTERN void emu_load_bootrom_gg(const char* file_path);
EXTERN void emu_enable_bootrom_sms(bool enable);
EXTERN void emu_enable_bootrom_gg(bool enable);
EXTERN void emu_set_media_slot(int slot);
EXTERN void emu_set_3d_glasses_config(int config);

#undef EMU_IMPORT
#undef EXTERN
#endif	/* EMU_H */
