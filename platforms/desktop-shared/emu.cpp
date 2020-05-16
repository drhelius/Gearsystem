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
static void update_debug_background_buffer(void);
static void update_debug_tile_buffer(void);
static void update_debug_sprite_buffers(void);

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
    emu_debug_background_buffer = new GS_Color[256 * 256];
    emu_debug_tile_buffer = new GS_Color[32 * 16 * 64];

    for (int i=0; i < (32 * 16 * 64); i++)
    {
        emu_debug_tile_buffer[i].red = 0;
        emu_debug_tile_buffer[i].green = 0;
        emu_debug_tile_buffer[i].blue = 0;
    }

    for (int s = 0; s < 64; s++)
    {
        emu_debug_sprite_buffers[s] = new GS_Color[8 * 16];

        for (int i=0; i < (8 * 16); i++)
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
    update_debug_background_buffer();
    update_debug_tile_buffer();
    update_debug_sprite_buffers();
}

static void update_debug_background_buffer(void)
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
            if (!tile_hflip)
                offset_x = 7 - offset_x;
            if (tile_vflip)
                offset_y = 7 - offset_y;


            int tile_data_addr = (tile_number * 32) + (4 * offset_y);
            int color_index = ((vram[tile_data_addr] >> offset_x) & 1) | (((vram[tile_data_addr + 1] >> offset_x) & 1) << 1) | (((vram[tile_data_addr + 2] >> offset_x) & 1) << 2) | (((vram[tile_data_addr + 3] >> offset_x) & 1) << 3);

            emu_debug_background_buffer[pixel] = video->ConvertTo8BitColor(color_index + tile_palette);
        }
    }
}

static void update_debug_tile_buffer(void)
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
            int offset_x = x & 0x7;
            offset_x = 7 - offset_x;
            int pixel = width_y + x;

            int tile_number = (tile_y * 32) + tile_x;
            int tile_palette = emu_debug_tile_palette * 16;

            int tile_data_addr = (tile_number * 32) + (4 * offset_y);
            int color_index = ((vram[tile_data_addr] >> offset_x) & 1) | (((vram[tile_data_addr + 1] >> offset_x) & 1) << 1) | (((vram[tile_data_addr + 2] >> offset_x) & 1) << 2) | (((vram[tile_data_addr + 3] >> offset_x) & 1) << 3);

            emu_debug_tile_buffer[pixel] = video->ConvertTo8BitColor(color_index + tile_palette);
        }
    }
}

static void update_debug_sprite_buffers(void)
{
    /*
    Memory* memory = gearboy->GetMemory();
    Video* video = gearboy->GetVideo();
    u16 address = 0xFE00;
    u16* dmg_palette = gearboy->GetDMGInternalPalette();
    PaletteMatrix sprite_palettes = video->GetCGBSpritePalettes();
    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_16 = IsSetBit(lcdc, 2);

    for (int s = 0; s < 40; s++)
    {
        u8 tile = memory->Retrieve(address + 2);
        u8 flags = memory->Retrieve(address + 3);
        int palette = IsSetBit(flags, 4) ? 1 : 0;
        bool xflip = IsSetBit(flags, 5);
        bool yflip = IsSetBit(flags, 6);
        bool cgb_bank = IsSetBit(flags, 3);
        int cgb_pal = flags & 0x07;

        for (int pixel = 0; pixel < (8 * 16); pixel++)
        {
            u16 tile_addr = 0x8000 + (tile * 16);

            int pixel_x = pixel & 0x7;
            int pixel_y = pixel / 8;

            u16 line_addr = tile_addr + (2 * pixel_y);

            if (xflip)
                pixel_x = 7 - pixel_x;
            if (yflip)
                line_addr = (sprites_16 ? 15 : 7) - line_addr;

            u8 byte1 = 0;
            u8 byte2 = 0;

            if (gearboy->IsCGB() && cgb_bank)
            {
                byte1 = memory->ReadCGBLCDRAM(line_addr, true);
                byte2 = memory->ReadCGBLCDRAM(line_addr + 1, true);
            }
            else
            {
                byte1 = memory->Retrieve(line_addr);
                byte2 = memory->Retrieve(line_addr + 1);
            }

            int tile_bit = 0x1 << (7 - pixel_x);
            int pixel_data = (byte1 & tile_bit) ? 1 : 0;
            pixel_data |= (byte2 & tile_bit) ? 2 : 0;

            if (gearboy->IsCGB())
            {
                pixel_data = (*sprite_palettes)[cgb_pal][pixel_data][1];
                debug_oam_buffers_565[s][pixel] = pixel_data;
            }
            else
            {
                u8 final_palette = memory->Retrieve(0xFF48 + palette);
                pixel_data = (final_palette >> (pixel_data << 1)) & 0x03;
                debug_oam_buffers_565[s][pixel] = dmg_palette[pixel_data];
            }
        }

        address += 4;
    }
    */
}

