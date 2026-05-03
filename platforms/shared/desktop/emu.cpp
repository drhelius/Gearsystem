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

#define EMU_IMPORT
#include "emu.h"

#include <thread>
#include <atomic>
#include <string.h>
#include "gearsystem.h"
#include "sound_queue.h"
#include "config.h"
#include "rewind.h"
#include "events.h"
#include "mcp/mcp_manager.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(_WIN32)
#define STBIW_WINDOWS_UTF8
#endif
#include "stb_image_write.h"

static GearsystemCore* gearsystem;
static s16* audio_buffer;
static bool audio_enabled;
static McpManager* mcp_manager;
static Uint64 rewind_last_counter = 0;
static double rewind_pop_accumulator = 0.0;

enum Loading_State
{
    Loading_State_None = 0,
    Loading_State_Loading,
    Loading_State_Finished
};

static std::atomic<int> loading_state(Loading_State_None);
static std::thread loading_thread;
static bool loading_thread_active;
static bool loading_result;
static char loading_file_path[4096];
static Cartridge::ForceConfiguration loading_config;
static int emu_debug_halt_step_frames_pending;
static const int kDebugHaltStepMaxFrames = 4;

u16* debug_background_buffer;
u16* debug_tile_buffer;
u16* debug_sprite_buffers[64];

static void save_ram(void);
static void load_ram(void);
static void reset_buffers(void);
static const char* get_mapper(Cartridge::CartridgeTypes type);
static const char* get_zone(Cartridge::CartridgeZones zone);
static const char* get_configurated_dir(int option, const char* path); 
static void init_debug(void);
static void destroy_debug(void);
static void update_debug(void);
static void update_debug_background(void);
static void update_debug_tiles(void);
static void update_debug_sprites(void);
static void update_debug_background_buffer_smsgg(void);
static void update_debug_tile_buffer_smsgg(void);
static void update_debug_sprite_buffers_smsgg(void);
static void update_debug_background_buffer_sg1000(void);
static void update_debug_tile_buffer_sg1000(void);
static void update_debug_sprite_buffers_sg1000(void);
static void debug_step_instruction(void);
static void reset_rewind_timing(void);
static int get_rewind_pop_budget(void);

bool emu_init(void)
{
    emu_frame_buffer = new u8[512 * 512 * 4];
    audio_buffer = new s16[GS_AUDIO_BUFFER_SIZE];

    init_debug();
    reset_buffers();

    gearsystem = new GearsystemCore();
    gearsystem->Init();

    sound_queue_init();

    for (int i = 0; i < 5; i++)
        InitPointer(emu_savestates_screenshots[i].data);

    audio_enabled = true;
    emu_audio_sync = true;
    emu_debug_disable_breakpoints = false;
    emu_debug_irq_breakpoints = false;
    emu_debug_command = Debug_Command_None;
    emu_debug_halt_step_frames_pending = 0;
    emu_debug_pc_changed = false;
    emu_debug_step_frames_pending = 0;
    emu_debug_tile_palette = 0;

    mcp_manager = new McpManager();
    mcp_manager->Init(gearsystem);

    rewind_init();

    return true;
}

void emu_destroy(void)
{
    if (loading_thread_active)
    {
        loading_thread.join();
        loading_thread_active = false;
    }
    loading_state.store(Loading_State_None);

    save_ram();
    rewind_destroy();
    SafeDelete(mcp_manager);
    SafeDeleteArray(audio_buffer);
    sound_queue_destroy();
    SafeDelete(gearsystem);
    SafeDeleteArray(emu_frame_buffer);
    destroy_debug();

    for (int i = 0; i < 5; i++)
        SafeDeleteArray(emu_savestates_screenshots[i].data);
}

static void load_media_thread_func(void)
{
    loading_result = gearsystem->LoadROM(loading_file_path, &loading_config);
    loading_state.store(Loading_State_Finished);
}

void emu_load_media_async(const char* file_path, Cartridge::ForceConfiguration config)
{
    if (loading_state.load() != Loading_State_None)
        return;

    emu_debug_command = Debug_Command_None;
    reset_buffers();

    save_ram();

    strncpy(loading_file_path, file_path, sizeof(loading_file_path) - 1);
    loading_file_path[sizeof(loading_file_path) - 1] = '\0';
    loading_result = false;
    loading_config = config;
    loading_state.store(Loading_State_Loading);
    if (loading_thread_active)
        loading_thread.join();
    loading_thread = std::thread(load_media_thread_func);
    loading_thread_active = true;
}

bool emu_is_media_loading(void)
{
    return loading_state.load() == Loading_State_Loading;
}

bool emu_finish_media_loading(void)
{
    if (loading_state.load() != Loading_State_Finished)
        return false;

    if (loading_thread_active)
    {
        loading_thread.join();
        loading_thread_active = false;
    }

    loading_state.store(Loading_State_None);

    if (!loading_result)
        return false;

    emu_audio_reset();
    load_ram();

    if (config_debug.debug && (config_debug.dis_look_ahead_count > 0))
        gearsystem->GetProcessor()->DisassembleAhead(config_debug.dis_look_ahead_count);

    update_savestates_data();
    rewind_reset();

    return true;
}

void emu_render_current_frame(void)
{
    if (emu_is_empty())
        return;

    gearsystem->RenderFrameBuffer(emu_frame_buffer);

    if (config_debug.debug)
        update_debug();
}

void emu_reset_rewind_timing(void)
{
    reset_rewind_timing();
}

void emu_update(void)
{
    emu_mcp_pump_commands();

    if (loading_state.load() != Loading_State_None)
        return;

    if (emu_is_empty())
        return;

    int sampleCount = 0;
    bool frame_executed = false;

    if (rewind_is_active())
    {
        int to_pop = get_rewind_pop_budget();

        for (int i = 0; i < to_pop; i++)
        {
            if (!rewind_pop())
                break;
        }

        int silence_count = GS_AUDIO_QUEUE_SIZE;
        memset(audio_buffer, 0, silence_count * sizeof(s16));
        sound_queue_write(audio_buffer, silence_count, false);
        return;
    }

    reset_rewind_timing();

    if (config_debug.debug)
    {
        bool breakpoint_hit = false;
        GearsystemCore::GS_Debug_Run debug_run;
        debug_run.step_debugger = (emu_debug_command == Debug_Command_Step);
        debug_run.stop_on_breakpoint = !emu_debug_disable_breakpoints && (emu_debug_halt_step_frames_pending == 0);
        debug_run.stop_on_run_to_breakpoint = true;
        debug_run.stop_on_irq = emu_debug_irq_breakpoints && (emu_debug_halt_step_frames_pending == 0);

        bool executed = (emu_debug_command != Debug_Command_None);

        if (executed)
        {
            rewind_commit_seek();
            breakpoint_hit = gearsystem->RunToVBlank(emu_frame_buffer, audio_buffer, &sampleCount, &debug_run);
            frame_executed = true;
        }

        if (breakpoint_hit || emu_debug_command == Debug_Command_StepFrame || emu_debug_command == Debug_Command_Step)
        {
            emu_debug_pc_changed = true;

            if (config_debug.dis_look_ahead_count > 0)
                gearsystem->GetProcessor()->DisassembleAhead(config_debug.dis_look_ahead_count);
        }

        if (emu_debug_halt_step_frames_pending > 0)
        {
            if (breakpoint_hit)
                emu_debug_halt_step_frames_pending = 0;
            else if (emu_debug_command == Debug_Command_Continue)
            {
                emu_debug_halt_step_frames_pending--;

                if (emu_debug_halt_step_frames_pending == 0)
                {
                    emu_debug_command = Debug_Command_None;
                    emu_debug_pc_changed = true;

                    if (config_debug.dis_look_ahead_count > 0)
                        gearsystem->GetProcessor()->DisassembleAhead(config_debug.dis_look_ahead_count);
                }
            }
        }

        if (breakpoint_hit)
            emu_debug_command = Debug_Command_None;

        if (emu_debug_command == Debug_Command_StepFrame && emu_debug_step_frames_pending > 0)
        {
            emu_debug_step_frames_pending--;
            if (emu_debug_step_frames_pending > 0)
                emu_debug_command = Debug_Command_StepFrame;
            else
                emu_debug_command = Debug_Command_None;
        }
        else if (emu_debug_command != Debug_Command_Continue)
            emu_debug_command = Debug_Command_None;

        update_debug();
    }
    else
    {
        if (!gearsystem->IsPaused())
        {
            rewind_commit_seek();
            gearsystem->RunToVBlank(emu_frame_buffer, audio_buffer, &sampleCount);
            frame_executed = true;
        }
    }

    if (frame_executed)
        rewind_push();

    if ((sampleCount > 0) && !gearsystem->IsPaused())
    {
        sound_queue_write(audio_buffer, sampleCount, emu_audio_sync);
    }
    else if (gearsystem->IsPaused())
    {
        int silence_count = GS_AUDIO_QUEUE_SIZE;
        memset(audio_buffer, 0, silence_count * sizeof(s16));
        sound_queue_write(audio_buffer, silence_count, false);
    }
}

static void reset_rewind_timing(void)
{
    rewind_last_counter = 0;
    rewind_pop_accumulator = 0.0;
}

static int get_rewind_pop_budget(void)
{
    Uint64 now = SDL_GetPerformanceCounter();

    if (rewind_last_counter == 0)
    {
        rewind_last_counter = now;
        return 0;
    }

    double elapsed = (double)(now - rewind_last_counter) / (double)SDL_GetPerformanceFrequency();
    rewind_last_counter = now;

    if (elapsed < 0.0)
        elapsed = 0.0;
    else if (elapsed > 0.25)
        elapsed = 0.25;

    int frames_per_snapshot = rewind_get_frames_per_snapshot();
    if (frames_per_snapshot < 1)
        frames_per_snapshot = 1;

    double snapshots_per_second = (60.0 * (double)config_rewind.speed) / (double)frames_per_snapshot;
    rewind_pop_accumulator += elapsed * snapshots_per_second;

    int to_pop = (int)rewind_pop_accumulator;
    if (to_pop > 0)
        rewind_pop_accumulator -= (double)to_pop;

    return to_pop;
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

bool emu_is_debug_idle(void)
{
    return config_debug.debug && (emu_debug_command == Debug_Command_None);
}

bool emu_is_empty(void)
{
    if (loading_state.load() != Loading_State_None)
        return true;
    return !gearsystem->GetCartridge()->IsReady();
}

void emu_reset(Cartridge::ForceConfiguration config)
{
    emu_debug_command = Debug_Command_None;
    reset_buffers();
    emu_audio_reset();
    save_ram();
    gearsystem->ResetROM(&config);
    load_ram();
    rewind_reset();
}

void emu_audio_mute(bool mute)
{
    audio_enabled = !mute;
    gearsystem->GetAudio()->Mute(mute);
}

void emu_audio_set_master_volume(float volume)
{
    gearsystem->GetAudio()->SetMasterVolume(volume);
}

void emu_audio_reset(void)
{
    sound_queue_stop();
    sound_queue_start(GS_AUDIO_SAMPLE_RATE, 2, GS_AUDIO_QUEUE_SIZE, config_audio.buffer_count);
}

void emu_audio_psg_volume(float volume)
{
    gearsystem->GetAudio()->SetPSGVolume(volume);
}

void emu_audio_fm_volume(float volume)
{
    gearsystem->GetAudio()->SetFMVolume(volume);
}

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

bool emu_is_audio_open(void)
{
    return sound_queue_is_open();
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
        rewind_reset();
    }
}

void emu_save_state_slot(int index)
{
    if (!emu_is_empty())
    {
        const char* dir = get_configurated_dir(config_emulator.savestates_dir_option, config_emulator.savestates_path.c_str());
        gearsystem->SaveState(dir, index, true);
        update_savestates_data();
    }
}

void emu_load_state_slot(int index)
{
    if (!emu_is_empty())
    {
        const char* dir = get_configurated_dir(config_emulator.savestates_dir_option, config_emulator.savestates_path.c_str());
        if (gearsystem->LoadState(dir, index))
        {
            events_sync_input();
            rewind_reset();
        }
    }
}

void emu_save_state_file(const char* file_path)
{
    if (!emu_is_empty())
        gearsystem->SaveState(file_path, -1, true);
}

void emu_load_state_file(const char* file_path)
{
    if (!emu_is_empty())
    {
        if (gearsystem->LoadState(file_path))
        {
            events_sync_input();
            rewind_reset();
        }
    }
}

void update_savestates_data(void)
{
    if (emu_is_empty())
        return;

    for (int i = 0; i < 5; i++)
    {
        emu_savestates[i].rom_name[0] = 0;
        SafeDeleteArray(emu_savestates_screenshots[i].data);

        const char* dir = get_configurated_dir(config_emulator.savestates_dir_option, config_emulator.savestates_path.c_str());

        if (!gearsystem->GetSaveStateHeader(i + 1, dir, &emu_savestates[i]))
            continue;

        if (emu_savestates[i].screenshot_size > 0)
        {
            emu_savestates_screenshots[i].data = new u8[emu_savestates[i].screenshot_size];
            emu_savestates_screenshots[i].size = emu_savestates[i].screenshot_size;
            gearsystem->GetSaveStateScreenshot(i + 1, dir, &emu_savestates_screenshots[i]);
        }
    }
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

void emu_get_info(char* info, int buffer_size)
{
    if (!emu_is_empty())
    {
        Cartridge* cart = gearsystem->GetCartridge();
        GS_RuntimeInfo runtime;
        gearsystem->GetRuntimeInfo(runtime);

        const char* filename = cart->GetFileName();
        int crc = cart->GetCRC();
        const char* system = "Master System";
        if (cart->IsGameGear())
        {
            if (cart->GetGameGearASIC() == 1)
                system = cart->IsGameGearInSMSMode() ? "Game Gear (1 ASIC) SMS Mode" : "Game Gear (1 ASIC)";
            else
                system = cart->IsGameGearInSMSMode() ? "Game Gear (2 ASIC) SMS Mode" : "Game Gear (2 ASIC)";
        }
        else if (cart->IsSG1000II())
            system = "SG-1000 II";
        else if (cart->IsSG1000())
            system = "SG-1000";
        const char* pal = cart->IsPAL() ? "PAL" : "NTSC";
        const char* checksum = cart->IsValidROM() ? "VALID" : "FAILED";
        const char* battery = gearsystem->GetMemory()->GetCurrentRule()->PersistedRAM() ? "YES" : "NO";
        int rom_banks = cart->GetROMBankCount();
        const char* mapper = get_mapper(cart->GetType());
        const char* zone = get_zone(cart->GetZone());

        snprintf(info, buffer_size, "File Name: %s\nCRC: %08X\nMapper: %s\nRegion: %s\nSystem: %s\nRefresh Rate: %s\nCartridge Header: %s\nROM Banks: %d\nBattery: %s\nScreen Resolution: %dx%d", filename, crc, mapper, zone, system, pal, checksum, rom_banks, battery, runtime.screen_width, runtime.screen_height);
    }
    else
    {
        snprintf(info, buffer_size, "There is no ROM loaded!");
    }
}

GearsystemCore* emu_get_core(void)
{
    return gearsystem;
}

void emu_debug_step_over(void)
{
    Processor* processor = emu_get_core()->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();
    Memory* memory = emu_get_core()->GetMemory();
    u16 pc = proc_state->PC->GetValue();
    GS_Disassembler_Record* record = memory->GetDisassemblerRecord(pc);

    if (IsValidPointer(record) && record->subroutine)
    {
        u16 return_address = pc + record->size;
        processor->AddRunToBreakpoint(return_address);
        emu_debug_command = Debug_Command_Continue;
    }
    else
    {
        debug_step_instruction();
        return;
    }

    gearsystem->Pause(false);
}

void emu_debug_step_into(void)
{
    debug_step_instruction();
}

void emu_debug_step_out(void)
{
    Processor* processor = emu_get_core()->GetProcessor();
    std::stack<Processor::GS_CallStackEntry>* call_stack = processor->GetDisassemblerCallStack();

    if (call_stack->size() > 0)
    {
        Processor::GS_CallStackEntry entry = call_stack->top();
        u16 return_address = entry.back;
        processor->AddRunToBreakpoint(return_address);
        emu_debug_command = Debug_Command_Continue;
    }
    else
    {
        debug_step_instruction();
        return;
    }

    gearsystem->Pause(false);
}

void emu_debug_step_frame(void)
{
    gearsystem->Pause(false);
    emu_debug_step_frames_pending++;
    emu_debug_command = Debug_Command_StepFrame;
}

void emu_debug_break(void)
{
    gearsystem->Pause(false);
    if (emu_debug_command == Debug_Command_Continue)
        emu_debug_command = Debug_Command_Step;
}

void emu_debug_continue(void)
{
    gearsystem->Pause(false);
    emu_debug_halt_step_frames_pending = 0;
    emu_debug_command = Debug_Command_Continue;
}

bool emu_debug_halt_step_active(void)
{
    return emu_debug_halt_step_frames_pending > 0;
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

    stbi_write_png(file_path, runtime.screen_width, runtime.screen_height, 4, emu_frame_buffer, runtime.screen_width * 4);

    Log("Screenshot saved to %s", file_path);
}

int emu_get_screenshot_png(unsigned char** out_buffer)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return 0;

    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);

    int stride = runtime.screen_width * 4;
    int len = 0;

    *out_buffer = stbi_write_png_to_mem(emu_frame_buffer, stride, 
                                         runtime.screen_width, runtime.screen_height, 
                                         4, &len);

    return len;
}

static void get_sprite_size(int* width, int* height)
{
    Video* video = gearsystem->GetVideo();
    u8* regs = video->GetRegisters();
    bool sprites_16 = IsSetBit(regs[1], 1);
    bool isSG1000 = video->IsSG1000Mode();

    if (isSG1000)
    {
        *width = sprites_16 ? 16 : 8;
        *height = sprites_16 ? 16 : 8;
    }
    else
    {
        *width = 8;
        *height = sprites_16 ? 16 : 8;
    }
}

int emu_get_sprite_png(int sprite_index, unsigned char** out_buffer)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return 0;

    if (sprite_index < 0 || sprite_index > 63)
        return 0;

    update_debug();

    int width, height;
    get_sprite_size(&width, &height);

    u8* buffer = emu_debug_sprite_buffers[sprite_index];

    if (!buffer)
        return 0;

    int len = 0;
    *out_buffer = stbi_write_png_to_mem(buffer, 16 * 4, width, height, 4, &len);

    return len;
}

void emu_save_sprite(const char* file_path, int index)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return;

    update_debug_sprites();

    int width, height;
    get_sprite_size(&width, &height);

    u8* buffer = emu_debug_sprite_buffers[index];

    stbi_write_png(file_path, width, height, 4, buffer, 16 * 4);

    Log("Sprite saved to %s", file_path);
}

void emu_save_background(const char* file_path)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return;

    update_debug_background();

    stbi_write_png(file_path, 256, 256, 4, emu_debug_background_buffer, 256 * 4);

    Log("Background saved to %s", file_path);
}

void emu_save_tiles(const char* file_path)
{
    if (!gearsystem->GetCartridge()->IsReady())
        return;

    update_debug_tiles();

    bool isSG1000 = gearsystem->GetVideo()->IsSG1000Mode();
    int lines = isSG1000 ? 32 : 16;
    int width = 32 * 8;
    int height = lines * 8;

    stbi_write_png(file_path, width, height, 4, emu_debug_tile_buffer, width * 4);

    Log("Pattern table saved to %s", file_path);
}

static void save_ram(void)
{
    const char* dir = get_configurated_dir(config_emulator.savefiles_dir_option, config_emulator.savefiles_path.c_str());
    gearsystem->SaveRam(dir);
}

static void load_ram(void)
{
    const char* dir = get_configurated_dir(config_emulator.savefiles_dir_option, config_emulator.savefiles_path.c_str());
    gearsystem->LoadRam(dir);
}

static void reset_buffers(void)
{
    for (int i = 0; i < 512 * 512 * 4; i++)
        emu_frame_buffer[i] = 0;

    for (int i = 0; i < GS_AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;

}

static const char* get_configurated_dir(int location, const char* path)
{
    switch ((Directory_Location)location)
    {
        default:
        case Directory_Location_Default:
            return config_root_path;
        case Directory_Location_ROM:
            return NULL;
        case Directory_Location_Custom:
            return path;
    }
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

static void update_debug_background(void)
{
    if (gearsystem->GetVideo()->IsSG1000Mode())
        update_debug_background_buffer_sg1000();
    else
        update_debug_background_buffer_smsgg();

    Video* video = gearsystem->GetVideo();
    video->Render32bit(debug_background_buffer, emu_debug_background_buffer, GS_PIXEL_RGBA8888, 256 * 256);
}

static void update_debug_tiles(void)
{
    if (gearsystem->GetVideo()->IsSG1000Mode())
        update_debug_tile_buffer_sg1000();
    else
        update_debug_tile_buffer_smsgg();

    Video* video = gearsystem->GetVideo();
    video->Render32bit(debug_tile_buffer, emu_debug_tile_buffer, GS_PIXEL_RGBA8888, 32 * 32 * 64);
}

static void update_debug_sprites(void)
{
    if (gearsystem->GetVideo()->IsSG1000Mode())
        update_debug_sprite_buffers_sg1000();
    else
        update_debug_sprite_buffers_smsgg();

    Video* video = gearsystem->GetVideo();
    for (int s = 0; s < 64; s++)
    {
        video->Render32bit(debug_sprite_buffers[s], emu_debug_sprite_buffers[s], GS_PIXEL_RGBA8888, 16 * 16);
    }
}

static void update_debug(void)
{
    if (config_debug.show_video_nametable)
        update_debug_background();
    if (config_debug.show_video_tiles)
        update_debug_tiles();
    if (config_debug.show_video_sprites)
        update_debug_sprites();
}

static void debug_step_instruction(void)
{
    Processor* processor = emu_get_core()->GetProcessor();
    u16 pc = processor->GetState()->PC->GetValue();

    emu_debug_halt_step_frames_pending = 0;

    if (processor->Halted() || (emu_get_core()->GetMemory()->DebugRetrieve(pc) == 0x76))
    {
        processor->AddRunToBreakpoint(pc + 1);
        emu_debug_halt_step_frames_pending = kDebugHaltStepMaxFrames;
        emu_debug_command = Debug_Command_Continue;
    }
    else
        emu_debug_command = Debug_Command_Step;

    gearsystem->Pause(false);
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
    bool is_sg = gearsystem->GetCartridge()->IsSG1000();
    int clock_rate = is_pal ? (is_sg ? GS_MASTER_CLOCK_PAL_SG1000 : GS_MASTER_CLOCK_PAL) : GS_MASTER_CLOCK_NTSC;
    bool has_ym2413 = (gearsystem->GetAudio()->YM2413Read() != 0xFF);

    if (gearsystem->GetAudio()->StartVgmRecording(file_path, clock_rate, is_pal, has_ym2413))
    {
        Log("VGM recording started: %s", file_path);
    }
}

void emu_stop_vgm_recording(void)
{
    if (gearsystem->GetAudio()->IsVgmRecording())
    {
        gearsystem->GetAudio()->StopVgmRecording();
        Log("VGM recording stopped");
    }
}

bool emu_is_vgm_recording(void)
{
    return gearsystem->GetAudio()->IsVgmRecording();
}

void emu_mcp_set_transport(int mode, int tcp_port)
{
    if (mcp_manager)
        mcp_manager->SetTransportMode((McpTransportMode)mode, tcp_port);
}

void emu_mcp_start(void)
{
    if (mcp_manager)
        mcp_manager->Start();
}

void emu_mcp_stop(void)
{
    if (mcp_manager)
        mcp_manager->Stop();
}

bool emu_mcp_is_running(void)
{
    return mcp_manager && mcp_manager->IsRunning();
}

int emu_mcp_get_transport_mode(void)
{
    return mcp_manager ? mcp_manager->GetTransportMode() : -1;
}

void emu_mcp_pump_commands(void)
{
    if (mcp_manager && mcp_manager->IsRunning())
        mcp_manager->PumpCommands(gearsystem);
}

