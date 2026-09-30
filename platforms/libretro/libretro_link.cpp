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

#include "libretro_link.h"
#include "../../src/common.h"
#include "../../src/Memory.h"
#include "../../src/memory_stream.h"
#include <string.h>
#include <sstream>
#include <assert.h>

static const size_t frame_size = GS_RESOLUTION_GG_WIDTH * GS_RESOLUTION_GG_HEIGHT * sizeof(u16);
static const size_t render_size = GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN * sizeof(u16);

static u32 StateChecksum(const u8* data, size_t size);

LibretroLink::LibretroLink(LibretroInstance* instances, s8* vertical_latch, s8* horizontal_latch)
{
    m_instances = instances;
    m_dpad_vertical_latch = vertical_latch;
    m_dpad_horizontal_latch = horizontal_latch;
    m_save_path[0] = 0;
    m_has_rom_identity = false;
    memset(m_rom_hash, 0, sizeof(m_rom_hash));
    memset(m_bootrom_hash, 0, sizeof(m_bootrom_hash));
    memset(&m_runtime, 0, sizeof(m_runtime));

    for (unsigned i = 0; i < 2; i++)
    {
        m_endpoint[i].link = this;
        m_endpoint[i].index = i;
        m_instances[i].core->SetLinkCableCallbacks(PublishCallback, SampleCallback, PollCallback, NULL, NULL, &m_endpoint[i]);
    }
}

LibretroLink::~LibretroLink()
{
    for (unsigned i = 0; i < 2; i++)
    {
        GearsystemCore* core = m_instances[i].core;
        core->SetLinkCableProtocol(LinkCableProtocolNone, core->GetLinkCableCycles());
        core->SetLinkCableCallbacks(NULL, NULL, NULL, NULL, NULL, NULL);
    }
}

void LibretroLink::Reset()
{
    for (unsigned i = 0; i < 2; i++)
    {
        GearsystemCore* core = m_instances[i].core;
        core->SetLinkCableConnected(false, core->GetLinkCableCycles());
        core->SetLinkCableTransportActive(false, core->GetLinkCableCycles());
    }

    memset(&m_runtime, 0, sizeof(m_runtime));

    for (unsigned i = 0; i < 2; i++)
    {
        GearsystemCore* core = m_instances[i].core;
        m_runtime.origin[i] = core->GetLinkCableCycles();
        m_runtime.sampled[i].levels = 0x7F;
        if (!m_has_rom_identity)
        {
            m_rom_hash[i] = StateChecksum(core->GetCartridge()->GetROM(), core->GetCartridge()->GetROMSize());
            m_bootrom_hash[i] = StateChecksum(core->GetMemory()->GetBootrom(), core->GetMemory()->GetBootromSize());
        }
    }

    m_has_rom_identity = true;

    for (unsigned i = 0; i < 2; i++)
    {
        GearsystemCore* core = m_instances[i].core;
        core->SetLinkCableProtocol(LinkCableProtocolGearToGear, m_runtime.origin[i]);
        core->SetLinkCableTransportActive(true, m_runtime.origin[i]);
    }

    for (unsigned i = 0; i < 2; i++)
        m_instances[i].core->SetLinkCableConnected(true, m_runtime.origin[i]);
}

u64 LibretroLink::Cycle(unsigned index)
{
    return m_instances[index].core->GetLinkCableCycles() - m_runtime.origin[index];
}

void LibretroLink::PublishCallback(u64 cycle, const GS_LinkCable_WireState* state, void* data)
{
    Endpoint* endpoint = (Endpoint*)data;
    Runtime& runtime = endpoint->link->m_runtime;
    unsigned index = endpoint->index;

    assert(runtime.count[index] < EventCapacity);
    if (runtime.count[index] >= EventCapacity)
        return;

    GS_LinkCable_WireEvent& event = runtime.events[index][(runtime.head[index] + runtime.count[index]) % EventCapacity];
    event.cycle = cycle - runtime.origin[index];
    event.state = *state;
    runtime.count[index]++;
}

bool LibretroLink::SampleCallback(u64 cycle, GS_LinkCable_WireState* state, void* data)
{
    Endpoint* endpoint = (Endpoint*)data;
    GS_LinkCable_WireEvent event;

    while (PollCallback(cycle, &event, data))
    {
    }

    *state = endpoint->link->m_runtime.sampled[endpoint->index ^ 1];
    return true;
}

bool LibretroLink::PollCallback(u64 cycle, GS_LinkCable_WireEvent* event, void* data)
{
    Endpoint* endpoint = (Endpoint*)data;
    Runtime& runtime = endpoint->link->m_runtime;
    unsigned index = endpoint->index;
    unsigned peer = index ^ 1;

    if (!runtime.count[peer])
        return false;

    const GS_LinkCable_WireEvent& next = runtime.events[peer][runtime.head[peer]];
    if (next.cycle > cycle - runtime.origin[index])
        return false;

    *event = next;
    event->cycle += runtime.origin[index];
    runtime.sampled[peer] = next.state;
    runtime.head[peer] = (runtime.head[peer] + 1) % EventCapacity;
    runtime.count[peer]--;
    return true;
}

void LibretroLink::RunFrame()
{
    unsigned lines = m_instances[0].core->GetCartridge()->IsPAL() ? GS_LINES_PER_FRAME_PAL : GS_LINES_PER_FRAME_NTSC;
    m_runtime.frame_cycle += lines * GS_CYCLES_PER_LINE;

    while (Cycle(0) < m_runtime.frame_cycle || Cycle(1) < m_runtime.frame_cycle)
    {
        unsigned index = Cycle(0) <= Cycle(1) ? 0 : 1;
        unsigned int clocks;
        u8* frame_buffer = (u8*)m_instances[index].frame_buffer;

        if (m_instances[index].core->RunCycle(frame_buffer, clocks))
            m_instances[index].core->RenderFrameBuffer(frame_buffer);
    }

    for (unsigned i = 0; i < 2; i++)
        m_instances[i].core->EndFrame(m_instances[i].audio_buffer, &m_instances[i].sample_count);
}

void LibretroLink::Geometry(bool vertical, int selection, unsigned* width, unsigned* height)
{
    *width = (selection == 0 && !vertical) ? 320 : 160;
    *height = (selection == 0 && vertical) ? 288 : 144;
}

const u16* LibretroLink::Video(bool vertical, bool switched, int selection)
{
    if (selection != 0)
        return m_instances[selection - 1].frame_buffer;

    unsigned first = switched ? 1 : 0;

    if (vertical)
    {
        memcpy(m_video, m_instances[first].frame_buffer, frame_size);
        memcpy(m_video + 160 * 144, m_instances[first ^ 1].frame_buffer, frame_size);
    }
    else
    {
        for (unsigned y = 0; y < 144; y++)
        {
            memcpy(m_video + y * 320, m_instances[first].frame_buffer + y * 160, 160 * sizeof(u16));
            memcpy(m_video + y * 320 + 160, m_instances[first ^ 1].frame_buffer + y * 160, 160 * sizeof(u16));
        }
    }

    return m_video;
}

const s16* LibretroLink::Audio(int selection, int* count)
{
    if (selection < 2)
    {
        *count = m_instances[selection].sample_count;
        return m_instances[selection].audio_buffer;
    }

    *count = MAX(m_instances[0].sample_count, m_instances[1].sample_count);

    for (int i = 0; i < *count; i++)
    {
        int first = i < m_instances[0].sample_count ? m_instances[0].audio_buffer[i] : 0;
        int second = i < m_instances[1].sample_count ? m_instances[1].audio_buffer[i] : 0;
        m_mix[i] = (s16)((first + second) / 2);
    }

    return m_mix;
}

struct LinkStateHeader
{
    u32 magic;
    u32 version;
    u32 core_size[2];
    u32 cable_size[2];
    u32 runtime_size;
    u32 asic[2];
    u32 mapper[2];
    u32 pal[2];
    u32 zone[2];
    u32 rom_hash[2];
    u32 bootrom_size[2];
    u32 bootrom_hash[2];
    u32 bootrom_enabled[2];
    u32 checksum;
};

static u32 StateChecksum(const u8* data, size_t size)
{
    u32 hash = 2166136261u;

    for (size_t i = 0; i < size; i++)
        hash = (hash ^ data[i]) * 16777619u;

    return hash;
}

size_t LibretroLink::StateSize()
{
    size_t size = sizeof(LinkStateHeader) + sizeof(m_runtime) + 2 * (frame_size + render_size);

    for (unsigned i = 0; i < 2; i++)
    {
        size_t core_size = 0;
        if (!m_instances[i].core->SaveState(NULL, core_size))
            return 0;

        std::ostringstream stream;
        m_instances[i].core->SaveLinkCableState(stream);
        size += core_size + stream.str().size();
    }

    return size;
}

bool LibretroLink::SaveState(void* data, size_t size)
{
    size_t required = StateSize();
    if (!data || !required || size < required)
        return false;

    u8* bytes = (u8*)data;
    memset(bytes, 0, size);

    LinkStateHeader header = {};
    header.magic = 0x4B4C5347;
    header.version = 2;
    header.runtime_size = sizeof(m_runtime);

    memcpy(m_runtime.dpad_vertical_latch, m_dpad_vertical_latch, sizeof(m_runtime.dpad_vertical_latch));
    memcpy(m_runtime.dpad_horizontal_latch, m_dpad_horizontal_latch, sizeof(m_runtime.dpad_horizontal_latch));

    size_t offset = sizeof(header);
    memcpy(bytes + offset, &m_runtime, sizeof(m_runtime));
    offset += sizeof(m_runtime);

    for (unsigned i = 0; i < 2; i++)
    {
        memcpy(bytes + offset, m_instances[i].frame_buffer, frame_size);
        offset += frame_size;
        memcpy(bytes + offset, m_instances[i].core->GetVideo()->GetFrameBuffer(), render_size);
        offset += render_size;
    }

    for (unsigned i = 0; i < 2; i++)
    {
        size_t core_size = required - offset;

        if (!m_instances[i].core->SaveState(bytes + offset, core_size))
            return false;

        header.core_size[i] = (u32)core_size;
        header.rom_hash[i] = m_rom_hash[i];
        Cartridge* cart = m_instances[i].core->GetCartridge();
        header.asic[i] = cart->GetGameGearASIC();
        header.mapper[i] = (u32)cart->GetType();
        header.pal[i] = cart->IsPAL() ? 1 : 0;
        header.zone[i] = (u32)cart->GetZone();
        Memory* memory = m_instances[i].core->GetMemory();
        header.bootrom_size[i] = (u32)memory->GetBootromSize();
        header.bootrom_hash[i] = m_bootrom_hash[i];
        header.bootrom_enabled[i] = memory->IsBootromEnabled() ? 1 : 0;
        offset += core_size;

        memory_stream stream((char*)(bytes + offset), required - offset);

        m_instances[i].core->SaveLinkCableState(stream);

        if (!stream.good())
            return false;

        header.cable_size[i] = (u32)stream.size();
        offset += stream.size();
    }

    header.checksum = StateChecksum(bytes + sizeof(header), required - sizeof(header));
    memcpy(bytes, &header, sizeof(header));

    return true;
}

bool LibretroLink::ReadState(const u8* data, size_t size)
{
    LinkStateHeader header;
    memcpy(&header, data, sizeof(header));

    size_t offset = sizeof(header);
    memcpy(&m_runtime, data + offset, sizeof(m_runtime));
    offset += sizeof(m_runtime);
    memcpy(m_dpad_vertical_latch, m_runtime.dpad_vertical_latch, sizeof(m_runtime.dpad_vertical_latch));
    memcpy(m_dpad_horizontal_latch, m_runtime.dpad_horizontal_latch, sizeof(m_runtime.dpad_horizontal_latch));

    for (unsigned i = 0; i < 2; i++)
    {
        memcpy(m_instances[i].frame_buffer, data + offset, frame_size);
        offset += frame_size;
        memcpy(m_instances[i].core->GetVideo()->GetFrameBuffer(), data + offset, render_size);
        offset += render_size;
        m_instances[i].sample_count = 0;
    }

    for (unsigned i = 0; i < 2; i++)
    {
        // Input state loading rebases peripheral timestamps against this clock.
        u64 master_clock;
        memcpy(&master_clock, data + offset + header.core_size[i], sizeof(master_clock));
        m_instances[i].core->SetMasterClockCycles(master_clock);

        if (!m_instances[i].core->LoadState(data + offset, header.core_size[i]))
            return false;

        offset += header.core_size[i];
        memory_input_stream stream((const char*)(data + offset), header.cable_size[i]);

        m_instances[i].core->LoadLinkCableState(stream);

        if (!stream.good() || (size_t)stream.tellg() != header.cable_size[i])
            return false;

        offset += header.cable_size[i];
    }

    return offset == size;
}

bool LibretroLink::LoadState(const void* data, size_t size)
{
    size_t required = StateSize();

    if (!data || !required || size < required)
        return false;

    LinkStateHeader header;
    memcpy(&header, data, sizeof(header));

    if (header.magic != 0x4B4C5347 || header.version != 2 || header.runtime_size != sizeof(m_runtime))
        return false;

    const u8* bytes = (const u8*)data;

    if (header.checksum != StateChecksum(bytes + sizeof(header), required - sizeof(header)))
        return false;

    Runtime runtime;
    memcpy(&runtime, bytes + sizeof(header), sizeof(runtime));
    size_t offset = sizeof(header) + sizeof(runtime) + 2 * (frame_size + render_size);

    for (unsigned i = 0; i < 2; i++)
    {
        GearsystemCore* core = m_instances[i].core;
        Cartridge* cart = core->GetCartridge();
        Memory* memory = core->GetMemory();
        size_t core_size = 0;
        core->SaveState(NULL, core_size);
        std::ostringstream stream;
        core->SaveLinkCableState(stream);

        if (header.core_size[i] != core_size || header.cable_size[i] != stream.str().size() ||
            header.rom_hash[i] != m_rom_hash[i] || header.asic[i] != (u32)cart->GetGameGearASIC() ||
            header.mapper[i] != (u32)cart->GetType() || header.pal[i] != (cart->IsPAL() ? 1u : 0u) ||
            header.zone[i] != (u32)cart->GetZone() || !core->IsNativeGameGearMode() ||
            header.bootrom_size[i] != (u32)memory->GetBootromSize() || header.bootrom_hash[i] != m_bootrom_hash[i] ||
            header.bootrom_enabled[i] != (memory->IsBootromEnabled() ? 1u : 0u) ||
            runtime.head[i] >= EventCapacity || runtime.count[i] > EventCapacity ||
            runtime.dpad_vertical_latch[i] < -1 || runtime.dpad_vertical_latch[i] > 1 ||
            runtime.dpad_horizontal_latch[i] < -1 || runtime.dpad_horizontal_latch[i] > 1)
            return false;

        GS_SaveState_Header_Libretro core_header;
        memcpy(&core_header, bytes + offset + core_size - sizeof(core_header), sizeof(core_header));
        if (core_header.magic != GS_SAVESTATE_MAGIC || core_header.version != GS_SAVESTATE_VERSION)
            return false;

        u64 link_cycle;
        memcpy(&link_cycle, bytes + offset + core_size + sizeof(u64), sizeof(link_cycle));
        if (link_cycle < runtime.origin[i] || link_cycle - runtime.origin[i] < runtime.frame_cycle)
            return false;

        u64 previous_cycle = 0;
        for (u32 n = 0; n < runtime.count[i]; n++)
        {
            const GS_LinkCable_WireEvent& event = runtime.events[i][(runtime.head[i] + n) % EventCapacity];
            if (event.cycle < previous_cycle || event.cycle > link_cycle - runtime.origin[i])
                return false;
            previous_cycle = event.cycle;
        }
        offset += core_size + header.cable_size[i];
    }

    u8* backup = new u8[required];

    bool saved = SaveState(backup, required);
    bool loaded = saved && ReadState(bytes, required);

    if (saved && !loaded)
        ReadState(backup, required);

    delete[] backup;
    return loaded;
}

void LibretroLink::SetSavePath(const char* content_path, const char* save_directory)
{
    m_save_path[0] = 0;

    if (!content_path || !content_path[0])
        return;

    const char* filename = strrchr(content_path, '/');
    const char* backslash = strrchr(content_path, '\\');

    if (backslash && (!filename || backslash > filename))
        filename = backslash;

    filename = filename ? filename + 1 : content_path;
    int count;

    if (save_directory && save_directory[0])
        count = snprintf(m_save_path, sizeof(m_save_path), "%s/%s", save_directory, filename);
    else
        count = snprintf(m_save_path, sizeof(m_save_path), "%s", content_path);

    if (count < 0 || (size_t)count >= sizeof(m_save_path))
    {
        m_save_path[0] = 0;
        Log("Screen 2 save path is too long");
        return;
    }

    char* extension = strrchr(m_save_path, '.');
    char* separator = strrchr(m_save_path, '/');
    char* windows_separator = strrchr(m_save_path, '\\');

    if (windows_separator && (!separator || windows_separator > separator))
        separator = windows_separator;

    if (extension && (!separator || extension > separator))
        *extension = 0;
}

void LibretroLink::PersistentMemory(bool write, const retro_vfs_interface* vfs)
{
    if (!m_save_path[0])
        return;

    char path[4112];
    snprintf(path, sizeof(path), "%s.srm2", m_save_path);

    if (!PersistentMemory(path, RETRO_MEMORY_SAVE_RAM, write, vfs) && write)
        Log("Could not save screen 2 memory: %s", path);
}

bool LibretroLink::PersistentMemory(const char* path, unsigned id, bool write, const retro_vfs_interface* vfs)
{
    MemoryRule* rule = m_instances[1].core->GetMemory()->GetCurrentRule();
    if (id != RETRO_MEMORY_SAVE_RAM)
        return false;

    size_t size = rule->GetRamSize();
    void* data = rule->GetRamBanks();

    if (!size || !data)
        return true;

    u8* buffer = new u8[size];

    if (write)
        memcpy(buffer, data, size);

    size_t total = 0;
    bool closed = false;

    if (vfs)
    {
        if (!vfs->open || !vfs->close || (write ? !vfs->write : !vfs->read))
        {
            delete[] buffer;
            return false;
        }

        retro_vfs_file_handle* file = vfs->open(path, write ? RETRO_VFS_FILE_ACCESS_WRITE : RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

        if (file)
        {
            while (total < size)
            {
                int64_t count = write ? vfs->write(file, buffer + total, size - total) : vfs->read(file, buffer + total, size - total);

                if (count <= 0 || (uint64_t)count > size - total)
                    break;

                total += (size_t)count;
            }

            closed = vfs->close(file) == 0;
        }
    }
    else
    {
        FILE* file = fopen_utf8(path, write ? "wb" : "rb");

        if (file)
        {
            total = write ? fwrite(buffer, 1, size, file) : fread(buffer, 1, size, file);
            closed = fclose(file) == 0;
        }
    }

    bool success = closed && total == size;

    if (!write && success)
        memcpy(data, buffer, size);

    delete[] buffer;
    return success;
}
