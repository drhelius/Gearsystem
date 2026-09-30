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

#ifndef LIBRETRO_LINK_H
#define LIBRETRO_LINK_H

#include "../../src/GearsystemCore.h"
#include "libretro.h"

#define GEARSYSTEM_LINK_SUBSYSTEM 0x101
#define GEARSYSTEM_LINK_RAM_1 ((1 << 8) | RETRO_MEMORY_SAVE_RAM)
#define GEARSYSTEM_LINK_RAM_2 ((2 << 8) | RETRO_MEMORY_SAVE_RAM)

struct LibretroInstance
{
    GearsystemCore* core;
    u16 frame_buffer[GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN];
    s16 audio_buffer[GS_AUDIO_BUFFER_SIZE];
    int sample_count;
};

class LibretroLink
{
public:
    LibretroLink(LibretroInstance* instances, s8* vertical_latch, s8* horizontal_latch);
    ~LibretroLink();
    void Reset();
    void RunFrame();
    void Geometry(bool vertical, int selection, unsigned* width, unsigned* height);
    const u16* Video(bool vertical, bool switched, int selection);
    const s16* Audio(int selection, int* count);
    size_t StateSize();
    bool SaveState(void* data, size_t size);
    bool LoadState(const void* data, size_t size);
    void SetSavePath(const char* content_path, const char* save_directory);
    void PersistentMemory(bool write, const retro_vfs_interface* vfs);
    bool PersistentMemory(const char* path, unsigned id, bool write, const retro_vfs_interface* vfs);

private:
    // The scheduler keeps peers within one CPU step of each other.
    static const unsigned EventCapacity = 64;

    struct Runtime
    {
        u64 frame_cycle;
        u64 origin[2];
        GS_LinkCable_WireEvent events[2][EventCapacity];
        GS_LinkCable_WireState sampled[2];
        u32 head[2];
        u32 count[2];
        s8 dpad_vertical_latch[2];
        s8 dpad_horizontal_latch[2];
    };

    struct Endpoint
    {
        LibretroLink* link;
        unsigned index;
    };

    static void PublishCallback(u64 cycle, const GS_LinkCable_WireState* state, void* data);
    static bool SampleCallback(u64 cycle, GS_LinkCable_WireState* state, void* data);
    static bool PollCallback(u64 cycle, GS_LinkCable_WireEvent* event, void* data);
    u64 Cycle(unsigned index);
    bool ReadState(const u8* data, size_t size);

    LibretroInstance* m_instances;
    s8* m_dpad_vertical_latch;
    s8* m_dpad_horizontal_latch;
    Endpoint m_endpoint[2];
    Runtime m_runtime;
    u16 m_video[2 * 160 * 144];
    s16 m_mix[GS_AUDIO_BUFFER_SIZE];
    char m_save_path[4096];
    u32 m_rom_hash[2];
    u32 m_bootrom_hash[2];
    bool m_has_rom_identity;
};

#endif /* LIBRETRO_LINK_H */
