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
#include "../audio-shared/sound_queue.h"
#include "config.h"

#define EMU_IMPORT
#include "emu.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#ifdef _WIN32
#define STBIW_WINDOWS_UTF8
#endif
#include "stb/stb_image_write.h"

static GearsystemCore* gearsystem;
static SoundQueue* sound_queue;
static s16* audio_buffer;
static bool audio_enabled;
static bool debugging = false;
static bool debug_step = false;
static bool debug_next_frame = false;

u16* debug_background_buffer;
u16* debug_tile_buffer;
u16* debug_sprite_buffers[64];

static void save_ram(void);
static void load_ram(void);
static void reset_buffers(void);
static const char* get_mapper(Cartridge::CartridgeTypes type);
static const char* get_zone(Cartridge::CartridgeZones zone);
static void init_debug(void);
static void destroy_debug(void);
static void update_debug(void);
static void update_debug_background_buffer_smsgg(void);
static void update_debug_tile_buffer_smsgg(void);
static void update_debug_sprite_buffers_smsgg(void);
static void update_debug_background_buffer_sg1000(void);
static void update_debug_tile_buffer_sg1000(void);
static void update_debug_sprite_buffers_sg1000(void);

bool emu_init(void)
{
    emu_frame_buffer = new u8[512 * 512 * 4];
    audio_buffer = new s16[GS_AUDIO_BUFFER_SIZE];

    init_debug();
    reset_buffers();

    gearsystem = new GearsystemCore();
    gearsystem->Init();

    sound_queue = new SoundQueue();
    if (!sound_queue->Start(GS_AUDIO_SAMPLE_RATE, 2))
        return false;

    audio_enabled = true;
    emu_audio_sync = true;
    emu_debug_disable_breakpoints_cpu = false;
    emu_debug_disable_breakpoints_mem = false;
    emu_debug_step_frames_pending = 0;
    emu_debug_tile_palette = 0;
    emu_savefiles_dir_option = 0;
    emu_savestates_dir_option = 0;
    emu_savefiles_path[0] = 0;
    emu_savestates_path[0] = 0;

    return true;
}

void emu_destroy(void)
{
    save_ram();
    SafeDeleteArray(audio_buffer);
    SafeDelete(sound_queue);
    SafeDelete(gearsystem);
    SafeDeleteArray(emu_frame_buffer);
    destroy_debug();
}

void emu_load_rom(const char* file_path, Cartridge::ForceConfiguration config)
{
    reset_buffers();
    emu_audio_reset();
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
            bool breakpoints = (!emu_debug_disable_breakpoints_cpu && !emu_debug_disable_breakpoints_mem) || IsValidPointer(gearsystem->GetMemory()->GetRunToBreakpoint());

            if (gearsystem->RunToVBlank(emu_frame_buffer, audio_buffer, &sampleCount, debug_step, breakpoints))
            {
                debugging = true;
            }

            if (emu_debug_step_frames_pending > 0)
            {
                emu_debug_step_frames_pending--;
                if (emu_debug_step_frames_pending > 0)
                    debug_next_frame = true;
                else
                    debug_next_frame = false;
            }
            else
            {
                debug_next_frame = false;
            }
            debug_step = false;
        }

        if (config_debug.debug)
        {
            update_debug();
        }

        if ((sampleCount > 0) && !gearsystem->IsPaused())
        {
            sound_queue->Write(audio_buffer, sampleCount, emu_audio_sync);
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

void emu_set_reset(bool pressed)
{
    gearsystem->SetReset(pressed);
}

void emu_set_phaser(int x, int y)
{
    gearsystem->SetPhaser(x, y);
}

void emu_set_phaser_offset(int x, int y)
{
    gearsystem->SetPhaserOffset(x, y);
}

void emu_enable_phaser(bool enable)
{
    gearsystem->EnablePhaser(enable);
}

void emu_enable_phaser_crosshair(bool enable, int shape, int color)
{
    gearsystem->EnablePhaserCrosshair(enable, (Video::LightPhaserCrosshairShape)shape, (Video::LightPhaserCrosshairColor)color);
}

void emu_set_paddle(float x)
{
    gearsystem->SetPaddle(x);
}

void emu_enable_paddle(bool enable)
{
    gearsystem->EnablePaddle(enable);
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

void emu_reset(Cartridge::ForceConfiguration config)
{
    reset_buffers();
    emu_audio_reset();
    save_ram();
    gearsystem->ResetROM(&config);
    load_ram();
}

void emu_memory_dump(void)
{
    gearsystem->SaveMemoryDump();
}

void emu_dissasemble_rom(void)
{
    gearsystem->SaveDisassembledROM();
}

void emu_audio_mute(bool mute)
{
    audio_enabled = !mute;
    gearsystem->GetAudio()->Mute(mute);
}

void emu_audio_reset(void)
{
    sound_queue->Stop();
    sound_queue->Start(GS_AUDIO_SAMPLE_RATE, 2);
}

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

bool emu_is_audio_open(void)
{
    return sound_queue->IsOpen();
}

void emu_save_ram(const char* file_path)
{
    if (!emu_is_empty())
        gearsystem->SaveRam(file_path, true);
}

void emu_load_ram(const char* file_path, Cartridge::ForceConfiguration config)
{
    if (!emu_is_empty())
    {
        save_ram();
        gearsystem->ResetROM(&config);
        gearsystem->LoadRam(file_path, true);
    }
}

void emu_save_state_slot(int index)
{
    if (!emu_is_empty())
    {
        if ((emu_savestates_dir_option == 0) && (strcmp(emu_savestates_path, "")))
            gearsystem->SaveState(emu_savestates_path, index);
        else
            gearsystem->SaveState(index);
    }
}

void emu_load_state_slot(int index)
{
    if (!emu_is_empty())
    {
        if ((emu_savestates_dir_option == 0) && (strcmp(emu_savestates_path, "")))
            gearsystem->LoadState(emu_savestates_path, index);
        else
            gearsystem->LoadState(index);
    }
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
        int crc = cart->GetCRC();
        const char* system = cart->IsGameGear() ? "Game Gear" : (cart->IsSG1000() ? "SG-1000" : "Master System");
        const char* pal = cart->IsPAL() ? "PAL" : "NTSC";
        const char* checksum = cart->IsValidROM() ? "VALID" : "FAILED";
        const char* battery = gearsystem->GetMemory()->GetCurrentRule()->PersistedRAM() ? "YES" : "NO";
        int rom_banks = cart->GetROMBankCount();
        const char* mapper = get_mapper(cart->GetType());
        const char* zone = get_zone(cart->GetZone());

        snprintf(info, 512, "File Name: %s\nCRC: %08X\nMapper: %s\nRegion: %s\nSystem: %s\nRefresh Rate: %s\nCartridge Header: %s\nROM Banks: %d\nBattery: %s\nScreen Resolution: %dx%d", filename, crc, mapper, zone, system, pal, checksum, rom_banks, battery, runtime.screen_width, runtime.screen_height);
    }
    else
    {
        snprintf(info, 512, "No data!");
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

void emu_load_bootrom_sms(const char* file_path)
{
    gearsystem->GetMemory()->LoadBootromSMS(file_path);
}

void emu_load_bootrom_gg(const char* file_path)
{
    gearsystem->GetMemory()->LoadBootromGG(file_path);
}

void emu_enable_bootrom_sms(bool enable)
{
    gearsystem->GetMemory()->EnableBootromSMS(enable);
}

void emu_enable_bootrom_gg(bool enable)
{
    gearsystem->GetMemory()->EnableBootromGG(enable);
}

void emu_set_media_slot(int slot)
{
    Memory::MediaSlots media_slot = Memory::CartridgeSlot;

    switch (slot)
    {
        case 1:
            media_slot = Memory::CardSlot;
            break;
        case 2:
            media_slot = Memory::ExpansionSlot;
            break;
        case 3:
            media_slot = Memory::NoSlot;
            break;
        default:
            media_slot = Memory::CartridgeSlot;
    }

    gearsystem->GetMemory()->SetMediaSlot(media_slot);
}

void emu_set_3d_glasses_config(int config)
{
    GearsystemCore::GlassesConfig glasses = GearsystemCore::GlassesBothEyes;

    switch (config)
    {
        case 1:
            glasses = GearsystemCore::GlassesLeftEye;
            break;
        case 2:
            glasses = GearsystemCore::GlassesRightEye;
            break;
        default:
            glasses = GearsystemCore::GlassesBothEyes;
    }
    gearsystem->SetGlassesConfig(glasses);
}

void emu_set_overscan(int overscan)
{
    switch (overscan)
    {
        case 0:
            gearsystem->GetVideo()->SetOverscan(Video::OverscanDisabled);
            break;
        case 1:
            gearsystem->GetVideo()->SetOverscan(Video::OverscanTopBottom);
            break;
        case 2:
            gearsystem->GetVideo()->SetOverscan(Video::OverscanFull284);
            break;
        case 3:
            gearsystem->GetVideo()->SetOverscan(Video::OverscanFull320);
            break;
        default:
            gearsystem->GetVideo()->SetOverscan(Video::OverscanDisabled);
    }
}

void emu_set_hide_left_bar(int hide_left_bar)
{
    switch (hide_left_bar)
    {
        case 0:
            gearsystem->GetVideo()->SetHideLeftBar(Video::HideLeftBarNo);
            break;
        case 1:
            gearsystem->GetVideo()->SetHideLeftBar(Video::HideLeftBarAuto);
            break;
        case 2:
            gearsystem->GetVideo()->SetHideLeftBar(Video::HideLeftBarAlways);
            break;
        default:
            gearsystem->GetVideo()->SetHideLeftBar(Video::HideLeftBarNo);
    }
}

void emu_disable_ym2413(bool disable)
{
    gearsystem->GetAudio()->DisableYM2413(disable);
}

void emu_save_screenshot(const char* file_path)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return;

    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);

    Log("Saving screenshot to %s", file_path);

    stbi_write_png(file_path, runtime.screen_width, runtime.screen_height, 4, emu_frame_buffer, runtime.screen_width * 4);

    Debug("Screenshot saved!");
}

static void save_ram(void)
{
#ifdef DEBUG_GEARSYSTEM
    emu_dissasemble_rom();
#endif

    if ((emu_savefiles_dir_option == 0) && (strcmp(emu_savefiles_path, "")))
        gearsystem->SaveRam(emu_savefiles_path);
    else
        gearsystem->SaveRam();
}

static void load_ram(void)
{
    if ((emu_savefiles_dir_option == 0) && (strcmp(emu_savefiles_path, "")))
        gearsystem->LoadRam(emu_savefiles_path);
    else
        gearsystem->LoadRam();
}

static void reset_buffers(void)
{
     for (int i = 0; i < 512 * 512 * 4; i++)
        emu_frame_buffer[i] = 0;

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;
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
    case Cartridge::CartridgeKoreanMSXSMS8000Mapper:
        return "Korean MSX/SMS 8000";
        break;
    case Cartridge::CartridgeKoreanSMS32KB2000Mapper:
        return "Korean SMS 32KB 2000";
        break;
    case Cartridge::CartridgeKoreanMSX32KB2000Mapper:
        return "Korean MSX 32KB 2000";
        break;
    case Cartridge::CartridgeKorean2000XOR1FMapper:
        return "Korean 2000 XOR 1F";
        break;
    case Cartridge::CartridgeKoreanMSX8KB0300Mapper:
        return "Korean MSX 8KB 0300";
        break;
    case Cartridge::CartridgeKorean0000XORFFMapper:
        return "Korean 0000 XOR FF";
        break;
    case Cartridge::CartridgeKoreanFFFFHiComMapper:
        return "Korean FFFF HiCom";
        break;
    case Cartridge::CartridgeKoreanFFFEMapper:
        return "Korean FFFE";
        break;
    case Cartridge::CartridgeKoreanBFFCMapper:
        return "Korean BFFC";
        break;
    case Cartridge::CartridgeKoreanFFF3FFFCMapper:
        return "Korean FFF3 FFFC";
        break;
    case Cartridge::CartridgeKoreanMDFFF5Mapper:
        return "Korean MD FFF5";
        break;
    case Cartridge::CartridgeKoreanMDFFF0Mapper:
        return "Korean MD FFF0";
        break;
    case Cartridge::CartridgeMSXMapper:
        return "MSX";
        break;
    case Cartridge::CartridgeJanggunMapper:
        return "Janggun";
        break;
    case Cartridge::CartridgeJumboDahjeeMapper:
        return "Jumbo Dahjee";
        break;
    case Cartridge::CartridgeEeprom93C46Mapper:
        return "EEPROM 93C46";
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
    emu_debug_background_buffer = new u8[256 * 256 * 4];
    emu_debug_tile_buffer = new u8[32 * 32 * 64 * 4];
    debug_background_buffer = new u16[256 * 256];    
    debug_tile_buffer = new u16[32 * 32 * 64];

    for (int i=0,j=0; i < (32 * 32 * 64); i++,j+=4)
    {
        debug_tile_buffer[i] = 0;
        emu_debug_tile_buffer[j] = 0;
        emu_debug_tile_buffer[j+1] = 0;
        emu_debug_tile_buffer[j+2] = 0;
        emu_debug_tile_buffer[j+3] = 0;
    }

    for (int s = 0; s < 64; s++)
    {
        emu_debug_sprite_buffers[s] = new u8[16 * 16 * 4];
        debug_sprite_buffers[s] = new u16[16 * 16];

        for (int i=0,j=0; i < (16 * 16); i++,j+=4)
        {
            debug_sprite_buffers[s][i] = 0;
            emu_debug_sprite_buffers[s][j] = 0;
            emu_debug_sprite_buffers[s][j+1] = 0;
            emu_debug_sprite_buffers[s][j+2] = 0;
            emu_debug_sprite_buffers[s][j+3] = 0;
        }
    }

    for (int i=0,j=0; i < (256 * 256); i++,j+=4)
    {
        debug_background_buffer[i] = 0;
        emu_debug_background_buffer[j] = 0;
        emu_debug_background_buffer[j+1] = 0;
        emu_debug_background_buffer[j+2] = 0;
        emu_debug_background_buffer[j+3] = 0;
    }
}

static void destroy_debug(void) 
{
    SafeDeleteArray(emu_debug_background_buffer);
    SafeDeleteArray(emu_debug_tile_buffer);
    SafeDeleteArray(debug_background_buffer);
    SafeDeleteArray(debug_tile_buffer);

    for (int s = 0; s < 64; s++)
    {
        SafeDeleteArray(emu_debug_sprite_buffers[s]);
        SafeDeleteArray(debug_sprite_buffers[s]);
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

    Video* video = gearsystem->GetVideo();

    video->Render32bit(debug_background_buffer, emu_debug_background_buffer, GS_PIXEL_RGBA8888, 256 * 256);
    video->Render32bit(debug_tile_buffer, emu_debug_tile_buffer, GS_PIXEL_RGBA8888, 32 * 32 * 64);

    for (int s = 0; s < 64; s++)
    {
        video->Render32bit(debug_sprite_buffers[s], emu_debug_sprite_buffers[s], GS_PIXEL_RGBA8888, 16 * 16);
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
            bool tile_hflip = IsSetBit((u8)tile_info_hi, 1);
            bool tile_vflip = IsSetBit((u8)tile_info_hi, 2);
            int tile_palette = IsSetBit((u8)tile_info_hi, 3) ? 16 : 0;
            int final_offset_y = offset_y;
            if (!tile_hflip)
                offset_x = 7 - offset_x;
            if (tile_vflip)
                final_offset_y = 7 - offset_y;

            int tile_data_addr = (tile_number * 32) + (4 * final_offset_y);
            int color_index = ((vram[tile_data_addr] >> offset_x) & 1) | (((vram[tile_data_addr + 1] >> offset_x) & 1) << 1) | (((vram[tile_data_addr + 2] >> offset_x) & 1) << 2) | (((vram[tile_data_addr + 3] >> offset_x) & 1) << 3);

            debug_background_buffer[pixel] = video->ColorFromPalette(color_index + tile_palette);
        }
    }
}

static void update_debug_background_buffer_sg1000(void)
{
    Video* video = gearsystem->GetVideo();
    u8* vram = video->GetVRAM();
    u8* regs = video->GetRegisters();
    int mode = video->GetTMS9918Mode();

    int pattern_table_addr = 0;
    int color_table_addr = 0;
    int name_table_addr = (regs[2] & 0x0F) << 10;
    int region = (regs[4] & 0x03) << 8;
    int backdrop_color = regs[7] & 0x0F;

    if (mode == 2)
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

            if (mode == 2)
                name_tile = vram[name_tile_addr] | (region & 0x300 & tile_number);
            else
                name_tile = vram[name_tile_addr];

            u8 pattern_line = vram[pattern_table_addr + (name_tile << 3) + offset_y];

            u8 color_line = 0;

            if (mode == 2)
                color_line = vram[color_table_addr + (name_tile << 3) + offset_y];
            else
                color_line = vram[color_table_addr + (name_tile >> 3)];

            int bg_color = color_line & 0x0F;
            int fg_color = color_line >> 4;
            int final_color = IsSetBit(pattern_line, offset_x) ? fg_color : bg_color;

            debug_background_buffer[pixel] = (final_color > 0) ? final_color : backdrop_color;
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

            debug_tile_buffer[pixel] = video->ColorFromPalette(color_index + tile_palette);
        }
    }
}

static void update_debug_tile_buffer_sg1000(void)
{
    Video* video = gearsystem->GetVideo();
    u8* vram = video->GetVRAM();
    u8* regs = video->GetRegisters();
    int mode = video->GetTMS9918Mode();

    int pattern_table_addr = (regs[4] & ((mode == 2) ? 0x04 : 0x07)) << 11;
    
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

            u16 black = 0;

            u16 white = 15;

            debug_tile_buffer[pixel] = color ? white : black;
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

            debug_sprite_buffers[s][pixel + padding] = video->ColorFromPalette(color_index + 16);
        }
    }
}

static void update_debug_sprite_buffers_sg1000(void)
{
    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();
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

                debug_sprite_buffers[s][pixel] = sprite_pixel ? sprite_color : 0;
            }
        }
    }
}

void emu_start_vgm_recording(const char* file_path)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return;

    if (gearsystem->GetAudio()->IsVgmRecording())
    {
        emu_stop_vgm_recording();
    }

    GS_RuntimeInfo runtime;
    gearsystem->GetRuntimeInfo(runtime);

    bool is_pal = (runtime.region == Region_PAL);
    int clock_rate = is_pal ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC;
    bool has_ym2413 = (gearsystem->GetAudio()->YM2413Read() != 0xFF);

    if (gearsystem->GetAudio()->StartVgmRecording(file_path, clock_rate, is_pal, has_ym2413))
    {
        Log("VGM recording started: %s", file_path);
    }
}

void emu_stop_vgm_recording()
{
    if (gearsystem->GetAudio()->IsVgmRecording())
    {
        gearsystem->GetAudio()->StopVgmRecording();
        Log("VGM recording stopped");
    }
}

bool emu_is_vgm_recording()
{
    return gearsystem->GetAudio()->IsVgmRecording();
}
