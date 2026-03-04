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
#define EMU_H

#include "gearsystem.h"

#ifdef EMU_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

enum Debug_Command
{
    Debug_Command_Continue,
    Debug_Command_Step,
    Debug_Command_StepFrame,
    Debug_Command_None
};

enum Directory_Location
{
    Directory_Location_Default = 0,
    Directory_Location_ROM = 1,
    Directory_Location_Custom = 2
};

EXTERN u8* emu_frame_buffer;
EXTERN GS_SaveState_Header emu_savestates[5];
EXTERN GS_SaveState_Screenshot emu_savestates_screenshots[5];
EXTERN u8* emu_debug_sprite_buffers[64];
EXTERN u8* emu_debug_background_buffer;
EXTERN u8* emu_debug_tile_buffer;
EXTERN Debug_Command emu_debug_command;
EXTERN bool emu_debug_pc_changed;
EXTERN int emu_debug_step_frames_pending;
EXTERN int emu_debug_tile_palette;
EXTERN bool emu_audio_sync;
EXTERN bool emu_debug_disable_breakpoints;
EXTERN bool emu_debug_irq_breakpoints;

EXTERN bool emu_init(void);
EXTERN void emu_destroy(void);
EXTERN void emu_update(void);
EXTERN void emu_load_media_async(const char* file_path, Cartridge::ForceConfiguration config);
EXTERN bool emu_is_media_loading(void);
EXTERN bool emu_finish_media_loading(void);
EXTERN void emu_key_pressed(GS_Joypads pad, GS_Keys key);
EXTERN void emu_key_released(GS_Joypads pad, GS_Keys key);
EXTERN void emu_set_reset(bool pressed);
EXTERN void emu_set_phaser(int x, int y);
EXTERN void emu_set_phaser_offset(int x, int y);
EXTERN void emu_enable_phaser(bool enable);
EXTERN void emu_enable_phaser_crosshair(bool enable, int shape, int color);
EXTERN void emu_set_paddle(float x);
EXTERN void emu_enable_paddle(bool enable);
EXTERN void emu_pause(void);
EXTERN void emu_resume(void);
EXTERN bool emu_is_paused(void);
EXTERN bool emu_is_debug_idle(void);
EXTERN bool emu_is_empty(void);
EXTERN void emu_reset(Cartridge::ForceConfiguration config);
EXTERN void emu_audio_mute(bool mute);
EXTERN void emu_audio_reset(void);
EXTERN bool emu_is_audio_enabled(void);
EXTERN bool emu_is_audio_open(void);
EXTERN void emu_save_ram(const char* file_path);
EXTERN void emu_load_ram(const char* file_path, Cartridge::ForceConfiguration config);
EXTERN void emu_save_state_slot(int index);
EXTERN void emu_load_state_slot(int index);
EXTERN void emu_save_state_file(const char* file_path);
EXTERN void emu_load_state_file(const char* file_path);
EXTERN void update_savestates_data(void);
EXTERN void emu_add_cheat(const char* cheat);
EXTERN void emu_clear_cheats();
EXTERN void emu_get_runtime(GS_RuntimeInfo& runtime);
EXTERN void emu_get_info(char* info, int buffer_size);
EXTERN GearsystemCore* emu_get_core(void);
EXTERN void emu_debug_step_over(void);
EXTERN void emu_debug_step_into(void);
EXTERN void emu_debug_step_out(void);
EXTERN void emu_debug_step_frame(void);
EXTERN void emu_debug_break(void);
EXTERN void emu_debug_continue(void);
EXTERN void emu_debug_set_callback(GearsystemCore::GS_Debug_Callback callback);
EXTERN void emu_load_bootrom_sms(const char* file_path);
EXTERN void emu_load_bootrom_gg(const char* file_path);
EXTERN void emu_enable_bootrom_sms(bool enable);
EXTERN void emu_enable_bootrom_gg(bool enable);
EXTERN void emu_set_media_slot(int slot);
EXTERN void emu_set_3d_glasses_config(int config);
EXTERN void emu_set_overscan(int overscan);
EXTERN void emu_set_hide_left_bar(int hide_left_bar);
EXTERN void emu_disable_ym2413(bool disable);
EXTERN void emu_save_screenshot(const char* file_path);
EXTERN int emu_get_screenshot_png(unsigned char** out_buffer);
EXTERN int emu_get_sprite_png(int vdc, int sprite_index, unsigned char** out_buffer);
EXTERN void emu_save_sprite(const char* file_path, int vdc, int index);
EXTERN void emu_save_background(const char* file_path, int vdc);
EXTERN void emu_start_vgm_recording(const char* file_path);
EXTERN void emu_stop_vgm_recording(void);
EXTERN bool emu_is_vgm_recording(void);
EXTERN void emu_mcp_set_transport(int mode, int tcp_port);
EXTERN void emu_mcp_start(void);
EXTERN void emu_mcp_stop(void);
EXTERN bool emu_mcp_is_running(void);
EXTERN int emu_mcp_get_transport_mode(void);
EXTERN void emu_mcp_pump_commands(void);

#undef EMU_IMPORT
#undef EXTERN
#endif /* EMU_H */