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
static void update_debug_background_buffer_smsgg(void);
static void update_debug_tile_buffer_smsgg(void);
static void update_debug_sprite_buffers_smsgg(void);
static void update_debug_background_buffer_sg1000(void);
static void update_debug_tile_buffer_sg1000(void);
static void update_debug_sprite_buffers_sg1000(void);

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
    emu_debug_tile_palette = 0;
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
    gearsystem->LoadROM(file_path, &config);
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
    gearsystem->ResetROM(&config);
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
        gearsystem->ResetROM(&config);
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

        sprintf(info, "File Name: %s\nMapper: %s\nRegion: %s\nSystem: %s\nRefresh Rate: %s\nCartridge Header: %s\nROM Banks: %d\nBattery: %s\nScreen Resolution: %dx%d", filename, mapper, zone, system, pal, checksum, rom_banks, battery, runtime.screen_width, runtime.screen_height);
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
    case Cartridge::CartridgeMSXMapper:
        return "MSX";
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
    emu_debug_background_buffer = new GS_Color[256 * 256];
    emu_debug_tile_buffer = new GS_Color[32 * 32 * 64];

    for (int i=0; i < (32 * 32 * 64); i++)
    {
        emu_debug_tile_buffer[i].red = 0;
        emu_debug_tile_buffer[i].green = 0;
        emu_debug_tile_buffer[i].blue = 0;
    }

    for (int s = 0; s < 64; s++)
    {
        emu_debug_sprite_buffers[s] = new GS_Color[16 * 16];

        for (int i=0; i < (16 * 16); i++)
        {
            emu_debug_sprite_buffers[s][i].red = 0;
            emu_debug_sprite_buffers[s][i].green = 0;
            emu_debug_sprite_buffers[s][i].blue = 0;
        }
    }

    for (int i=0; i < (256 * 256); i++)
    {
        emu_debug_background_buffer[i].red = 0;
        emu_debug_background_buffer[i].green = 0;
        emu_debug_background_buffer[i].blue = 0;
    }
}

static void update_debug(void)
{
    if (gearsystem->GetVideo()->IsSG1000Mode())
    {
        update_debug_background_buffer_sg1000();
        update_debug_tile_buffer_sg1000();
        update_debug_sprite_buffers_sg1000();
    }
    else
    {
        update_debug_background_buffer_smsgg();
        update_debug_tile_buffer_smsgg();
        update_debug_sprite_buffers_smsgg();
    }
}

static void update_debug_background_buffer_smsgg(void)
{
    Video* video = gearsystem->GetVideo();
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();

    for (int y = 0; y < 256; y++)
    {
        int width_y = (y * 256);
        int tile_y = y / 8;
        int offset_y = y & 0x7;

        for (int x = 0; x < 256; x++)
        {
            int tile_x = x / 8;
            int offset_x = x & 0x7;
            int pixel = width_y + x;

            int name_table_addr = (regs[2] & (video->IsExtendedMode224() ? 0x0C : 0x0E)) << 10;
            if (video->IsExtendedMode224())
                name_table_addr |= 0x700;
            u16 map_addr = name_table_addr + (64 * tile_y) + (tile_x * 2);

            u16 tile_info_lo = vram[map_addr];
            u16 tile_info_hi = vram[map_addr + 1];

            int tile_number = ((tile_info_hi & 1) << 8) | tile_info_lo;
            bool tile_hflip = IsSetBit(tile_info_hi, 1);
            bool tile_vflip = IsSetBit(tile_info_hi, 2);
            int tile_palette = IsSetBit(tile_info_hi, 3) ? 16 : 0;
            int final_offset_y = offset_y;
            if (!tile_hflip)
                offset_x = 7 - offset_x;
            if (tile_vflip)
                final_offset_y = 7 - offset_y;

            int tile_data_addr = (tile_number * 32) + (4 * final_offset_y);
            int color_index = ((vram[tile_data_addr] >> offset_x) & 1) | (((vram[tile_data_addr + 1] >> offset_x) & 1) << 1) | (((vram[tile_data_addr + 2] >> offset_x) & 1) << 2) | (((vram[tile_data_addr + 3] >> offset_x) & 1) << 3);

            emu_debug_background_buffer[pixel] = video->ConvertTo8BitColor(color_index + tile_palette);
        }
    }
}

static void update_debug_background_buffer_sg1000(void)
{
    Video* video = gearsystem->GetVideo();
    u8* vram = video->GetVRAM();
    u8* regs = video->GetRegisters();
    int mode = video->GetSG1000Mode();
    GS_Color* pal = video->GetSG1000Palette();

    int pattern_table_addr = 0;
    int color_table_addr = 0;
    int name_table_addr = (regs[2] & 0x0F) << 10;
    int region = (regs[4] & 0x03) << 8;
    int backdrop_color = regs[7] & 0x0F;

    if (mode == 0x200)
    {
        pattern_table_addr = (regs[4] & 0x04) << 11;
        color_table_addr = (regs[3] & 0x80) << 6;
    }
    else
    {
        pattern_table_addr = (regs[4] & 0x07) << 11;
        color_table_addr = regs[3] << 6;
    }

    for (int y = 0; y < 256; y++)
    {
        int width_y = (y * 256);
        int tile_y = y / 8;
        int offset_y = y & 0x7;

        for (int x = 0; x < 256; x++)
        {
            int tile_x = x / 8;
            int offset_x = 7 - (x & 0x7);
            int pixel = width_y + x;

            int tile_number = (tile_y * 32) + tile_x;

            int name_tile_addr = name_table_addr + tile_number;

            int name_tile = 0;

            if (mode == 0x200)
                name_tile = vram[name_tile_addr] | (region & 0x300 & tile_number);
            else
                name_tile = vram[name_tile_addr];

            u8 pattern_line = vram[pattern_table_addr + (name_tile << 3) + offset_y];

            u8 color_line = 0;

            if (mode == 0x200)
                color_line = vram[color_table_addr + (name_tile << 3) + offset_y];
            else
                color_line = vram[color_table_addr + (name_tile >> 3)];

            int bg_color = color_line & 0x0F;
            int fg_color = color_line >> 4;
            int final_color = IsSetBit(pattern_line, offset_x) ? fg_color : bg_color;

            emu_debug_background_buffer[pixel] = pal[(final_color > 0) ? final_color : backdrop_color];
        }
    }
}

static void update_debug_tile_buffer_smsgg(void)
{
    Video* video = gearsystem->GetVideo();
    u8* vram = video->GetVRAM();

    for (int y = 0; y < 128; y++)
    {
        int width_y = (y * 256);
        int tile_y = y / 8;
        int offset_y = y & 0x7;

        for (int x = 0; x < 256; x++)
        {
            int tile_x = x / 8;
            int offset_x = 7 - (x & 0x7);
            int pixel = width_y + x;

            int tile_number = (tile_y * 32) + tile_x;
            int tile_palette = emu_debug_tile_palette * 16;

            int tile_data_addr = (tile_number * 32) + (4 * offset_y);
            int color_index = ((vram[tile_data_addr] >> offset_x) & 1) | (((vram[tile_data_addr + 1] >> offset_x) & 1) << 1) | (((vram[tile_data_addr + 2] >> offset_x) & 1) << 2) | (((vram[tile_data_addr + 3] >> offset_x) & 1) << 3);

            emu_debug_tile_buffer[pixel] = video->ConvertTo8BitColor(color_index + tile_palette);
        }
    }
}

static void update_debug_tile_buffer_sg1000(void)
{
    Video* video = gearsystem->GetVideo();
    u8* vram = video->GetVRAM();
    u8* regs = video->GetRegisters();
    int mode = video->GetSG1000Mode();

    int pattern_table_addr = 0;
    int color_table_addr = 0;

    if (mode == 0x200)
    {
        pattern_table_addr = (regs[4] & 0x04) << 11;
        color_table_addr = (regs[3] & 0x80) << 6;
    }
    else
    {
        pattern_table_addr = (regs[4] & 0x07) << 11;
        color_table_addr = regs[3] << 6;
    }

    for (int y = 0; y < 256; y++)
    {
        int width_y = (y * 256);
        int tile_y = y / 8;
        int offset_y = y & 0x7;

        for (int x = 0; x < 256; x++)
        {
            int tile_x = x / 8;
            int offset_x = 7 - (x & 0x7);
            int pixel = width_y + x;

            int tile_number = (tile_y * 32) + tile_x;

            int tile_data_addr = pattern_table_addr + (tile_number * 8) + (1 * offset_y);
            bool color = IsSetBit(vram[tile_data_addr], offset_x);

            GS_Color black;
            black.red = 0;
            black.green = 0;
            black.blue = 0;

            GS_Color white;
            black.red = 255;
            black.green = 255;
            black.blue = 255;

            emu_debug_tile_buffer[pixel] = color ? white : black;
        }
    }
}

static void update_debug_sprite_buffers_smsgg(void)
{
    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);

    bool sprites_16 = IsSetBit(regs[1], 1);
    u16 sprite_table_address = (regs[5] << 7) & 0x3F00;
    u16 sprite_table_address_2 = sprite_table_address + 0x80;
    u16 sprite_tiles_address = (regs[6] << 11) & 0x2000;

    for (int s = 0; s < 64; s++)
    {
        u16 sprite_info_address = sprite_table_address_2 + (s << 1);
        int tile = vram[sprite_info_address + 1];
        tile &= sprites_16 ? 0xFE : 0xFF;
        int tile_addr = sprite_tiles_address + (tile << 5);

        int padding = 0;
        for (int pixel = 0; pixel < (8 * 16); pixel++)
        {
            if ((pixel != 0) && (pixel % 8 == 0))
                padding += 8;

            int pixel_x = 7 - (pixel & 0x7);
            int pixel_y = pixel / 8;

            u16 line_addr = tile_addr + (4 * pixel_y);

            int color_index = ((vram[line_addr] >> pixel_x) & 1) | (((vram[line_addr + 1] >> pixel_x) & 1) << 1) | (((vram[line_addr + 2] >> pixel_x) & 1) << 2) | (((vram[line_addr + 3] >> pixel_x) & 1) << 3);

            emu_debug_sprite_buffers[s][pixel + padding] = video->ConvertTo8BitColor(color_index + 16);
        }
    }
}

static void update_debug_sprite_buffers_sg1000(void)
{
    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();
    GS_Color* pal = video->GetSG1000Palette();
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);

    int sprite_size = IsSetBit(regs[1], 1) ? 16 : 8;
    u16 sprite_attribute_addr = (regs[5] & 0x7F) << 7;
    u16 sprite_pattern_addr = (regs[6] & 0x07) << 11;

    for (int s = 0; s < 64; s++)
    {
        int sprite_attribute_offset = sprite_attribute_addr + (s << 2);
        int sprite_color = vram[sprite_attribute_offset + 3] & 0x0F;
        int sprite_tile = vram[sprite_attribute_offset + 2];
        sprite_tile &= (sprite_size == 16) ? 0xFC : 0xFF;

        for (int pixel_y = 0; pixel_y < sprite_size; pixel_y++)
        {
            int sprite_line_addr = sprite_pattern_addr + (sprite_tile << 3) + pixel_y;

            for (int pixel_x = 0; pixel_x < 16; pixel_x++)
            {
                if ((sprite_size == 8) && (pixel_x == 8))
                    break;

                int pixel = (pixel_y * 16) + pixel_x;

                bool sprite_pixel = false;

                if (pixel_x < 8)
                    sprite_pixel = IsSetBit(vram[sprite_line_addr], 7 - pixel_x);
                else
                    sprite_pixel = IsSetBit(vram[sprite_line_addr + 16], 15 - pixel_x);

                emu_debug_sprite_buffers[s][pixel] = pal[sprite_pixel ? sprite_color : 0];
            }
        }
    }
}
