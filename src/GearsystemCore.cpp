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

#include "GearsystemCore.h"
#include "Memory.h"
#include "Processor.h"
#include "Audio.h"
#include "Input.h"
#include "Cartridge.h"
#include "MemoryRule.h"
#include "SegaMemoryRule.h"
#include "CodemastersMemoryRule.h"
#include "RomOnlyMemoryRule.h"
#include "KoreanMemoryRule.h"
#include "KoreanMSXSMS8000MemoryRule.h"
#include "KoreanSMS32KB2000MemoryRule.h"
#include "KoreanMSX32KB2000MemoryRule.h"
#include "Korean2000XOR1FMemoryRule.h"
#include "KoreanMSX8KB0300MemoryRule.h"
#include "Korean0000XORFFMemoryRule.h"
#include "KoreanFFFFHiComMemoryRule.h"
#include "KoreanFFFEMemoryRule.h"
#include "KoreanBFFCMemoryRule.h"
#include "KoreanFFF3FFFCMemoryRule.h"
#include "KoreanMDFFF5MemoryRule.h"
#include "KoreanMDFFF0MemoryRule.h"
#include "MSXMemoryRule.h"
#include "JanggunMemoryRule.h"
#include "Multi4PAKAllActionMemoryRule.h"
#include "JumboDahjeeMemoryRule.h"
#include "IratahackMemoryRule.h"
#include "Eeprom93C46MemoryRule.h"
#include "SG1000MemoryRule.h"
#include "SmsIOPorts.h"
#include "GameGearIOPorts.h"
#include "BootromMemoryRule.h"
#include "common.h"
#include "memory_stream.h"

GearsystemCore::GearsystemCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pAudio);
    InitPointer(m_pVideo);
    InitPointer(m_pInput);
    InitPointer(m_pCartridge);
    InitPointer(m_pSegaMemoryRule);
    InitPointer(m_pCodemastersMemoryRule);
    InitPointer(m_pSG1000MemoryRule);
    InitPointer(m_pRomOnlyMemoryRule);
    InitPointer(m_pKoreanMemoryRule);
    InitPointer(m_pKoreanMSXSMS8000MemoryRule);
    InitPointer(m_pKoreanSMS32KB2000MemoryRule);
    InitPointer(m_pKoreanMSX32KB2000MemoryRule);
    InitPointer(m_pKorean2000XOR1FMemoryRule);
    InitPointer(m_pKoreanMSX8KB0300MemoryRule);
    InitPointer(m_pKorean0000XORFFMemoryRule);
    InitPointer(m_pKoreanFFFFHiComMemoryRule);
    InitPointer(m_pKoreanFFFEMemoryRule);
    InitPointer(m_pKoreanBFFCMemoryRule);
    InitPointer(m_pKoreanFFF3FFFCMemoryRule);
    InitPointer(m_pKoreanMDFFF5MemoryRule);
    InitPointer(m_pKoreanMDFFF0MemoryRule);
    InitPointer(m_pMSXMemoryRule);
    InitPointer(m_pJanggunMemoryRule);
    InitPointer(m_pMulti4PAKAllActionMemoryRule);
    InitPointer(m_pJumboDahjeeMemoryRule);
    InitPointer(m_pEeprom93C46MemoryRule);
    InitPointer(m_pSmsIOPorts);
    InitPointer(m_pGameGearIOPorts);
    InitPointer(m_pBootromMemoryRule);
    InitPointer(m_pFrameBuffer);
    InitPointer(m_debug_callback);
    InitPointer(m_pIratahackMemoryRule);
    m_bPaused = true;
    m_pixelFormat = GS_PIXEL_RGBA8888;
    m_GlassesConfig = GearsystemCore::GlassesBothEyes;
}

GearsystemCore::~GearsystemCore()
{
    SafeDelete(m_pBootromMemoryRule);
    SafeDelete(m_pGameGearIOPorts);
    SafeDelete(m_pSmsIOPorts);
    SafeDelete(m_pRomOnlyMemoryRule);
    SafeDelete(m_pCodemastersMemoryRule);
    SafeDelete(m_pSG1000MemoryRule);
    SafeDelete(m_pSegaMemoryRule);
    SafeDelete(m_pKoreanMemoryRule);
    SafeDelete(m_pKoreanMSXSMS8000MemoryRule);
    SafeDelete(m_pKoreanSMS32KB2000MemoryRule);
    SafeDelete(m_pKoreanMSX32KB2000MemoryRule);
    SafeDelete(m_pKorean2000XOR1FMemoryRule);
    SafeDelete(m_pKoreanMSX8KB0300MemoryRule);
    SafeDelete(m_pKorean0000XORFFMemoryRule);
    SafeDelete(m_pKoreanFFFFHiComMemoryRule);
    SafeDelete(m_pKoreanFFFEMemoryRule);
    SafeDelete(m_pKoreanBFFCMemoryRule);
    SafeDelete(m_pKoreanFFF3FFFCMemoryRule);
    SafeDelete(m_pKoreanMDFFF5MemoryRule);
    SafeDelete(m_pKoreanMDFFF0MemoryRule);
    SafeDelete(m_pMSXMemoryRule);
    SafeDelete(m_pJanggunMemoryRule);
    SafeDelete(m_pMulti4PAKAllActionMemoryRule);
    SafeDelete(m_pJumboDahjeeMemoryRule);
    SafeDelete(m_pIratahackMemoryRule);
    SafeDelete(m_pEeprom93C46MemoryRule);
    SafeDelete(m_pCartridge);
    SafeDelete(m_pInput);
    SafeDelete(m_pVideo);
    SafeDelete(m_pAudio);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearsystemCore::Init(GS_Color_Format pixelFormat)
{
    Log("Loading %s core %s by Ignacio Sanchez", GS_TITLE, GS_VERSION);

    m_pixelFormat = pixelFormat;

    m_pCartridge = new Cartridge();
    m_pMemory = new Memory(m_pCartridge);
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video(m_pMemory, m_pProcessor, m_pCartridge);
    m_pInput = new Input(m_pProcessor, m_pVideo);
    m_pAudio = new Audio(m_pCartridge);
    m_pSmsIOPorts = new SmsIOPorts(m_pAudio, m_pVideo, m_pInput, m_pCartridge, m_pMemory, m_pProcessor);
    m_pGameGearIOPorts = new GameGearIOPorts(m_pAudio, m_pVideo, m_pInput, m_pCartridge, m_pMemory);

    m_pMemory->Init();
    m_pProcessor->Init();
    m_pAudio->Init();
    m_pVideo->Init();
    m_pInput->Init();
    m_pCartridge->Init();

    InitMemoryRules();
}

bool GearsystemCore::RunToVBlank(u8* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount, GS_Debug_Run* debug)
{
    m_pFrameBuffer = pFrameBuffer;

    if (!m_bPaused && m_pCartridge->IsReady())
    {
#if !defined(GS_DISABLE_DISASSEMBLER)
        bool debug_enable = false;
        bool instruction_completed = false;
        if (IsValidPointer(debug))
        {
            debug_enable = true;
            m_pProcessor->EnableBreakpoints(debug->stop_on_breakpoint, debug->stop_on_irq);
        }

        bool vblank = false;
        int totalClocks = 0;

        do
        {
            if (debug_enable && (IsValidPointer(m_debug_callback)))
                m_debug_callback();

            unsigned int clockCycles = debug_enable && debug->step_debugger ? m_pProcessor->RunInstruction() : m_pProcessor->RunFor(1);
            instruction_completed = true;
            vblank = m_pVideo->Tick(clockCycles);
            m_pAudio->Tick(clockCycles);
            m_master_clock_cycles += clockCycles;
            totalClocks += clockCycles;

            if (debug_enable)
            {
                if (debug->step_debugger)
                    vblank = instruction_completed;

                if (m_pProcessor->MemoryBreakpointHit())
                    vblank = true;

                if (instruction_completed)
                {
                    if (m_pProcessor->BreakpointHit())
                        vblank = true;

                    if (debug->stop_on_run_to_breakpoint && m_pProcessor->RunToBreakpointHit())
                        vblank = true;
                }
            }

            if (totalClocks > 702240)
                vblank = true;
        }
        while (!vblank);

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);
        RenderFrameBuffer(pFrameBuffer);

        return m_pProcessor->BreakpointHit() || m_pProcessor->RunToBreakpointHit();
#else
        UNUSED(debug);
        bool vblank = false;
        int totalClocks = 0;

        do
        {
            unsigned int clockCycles = m_pProcessor->RunFor(1);
            vblank = m_pVideo->Tick(clockCycles);
            m_pAudio->Tick(clockCycles);
            m_master_clock_cycles += clockCycles;
            totalClocks += clockCycles;

            if (totalClocks > 702240)
                vblank = true;
        }
        while (!vblank);

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);
        RenderFrameBuffer(pFrameBuffer);

        return false;
#endif
    }

    return false;
}

bool GearsystemCore::LoadROM(const char* szFilePath, Cartridge::ForceConfiguration* config)
{
    if (m_pCartridge->LoadFromFile(szFilePath))
    {
        if (IsValidPointer(config))
            m_pCartridge->ForceConfig(*config);
        Reset();
        m_pMemory->ResetDisassemblerRecords();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();

        m_pProcessor->DisassembleNextOPCode();

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header. File: %s...", szFilePath);
        }

        return romTypeOK;
    }
    else
        return false;
}

bool GearsystemCore::LoadROMFromBuffer(const u8* buffer, int size, Cartridge::ForceConfiguration* config, const char* szFilePath)
{
    if (m_pCartridge->LoadFromBuffer(buffer, size, szFilePath))
    {
        if (IsValidPointer(config))
            m_pCartridge->ForceConfig(*config);
        Reset();
        m_pMemory->ResetDisassemblerRecords();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();

        m_pProcessor->DisassembleNextOPCode();

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header.");
        }

        return romTypeOK;
    }
    else
        return false;
}

void GearsystemCore::SaveMemoryDump()
{
    if (m_pCartridge->IsReady() && (strlen(m_pCartridge->GetFilePath()) > 0))
    {
        using namespace std;

        char path[512];

        strcpy(path, m_pCartridge->GetFilePath());
        strcat(path, ".dump");

        Log("Saving Memory Dump %s...", path);

        m_pMemory->MemoryDump(path);

        Debug("Memory Dump Saved");
    }
}

void GearsystemCore::SaveDisassembledROM()
{
    GS_Disassembler_Record** romMap = m_pMemory->GetAllDisassemblerRecords();

    if (m_pCartridge->IsReady() && (strlen(m_pCartridge->GetFilePath()) > 0) && IsValidPointer(romMap))
    {
        using namespace std;

        char path[512];

        strcpy(path, m_pCartridge->GetFilePath());
        strcat(path, ".dis");

        Log("Saving Disassembled ROM %s...", path);

        ofstream myfile;
        open_ofstream_utf8(myfile, path, ios::out | ios::trunc);

        if (myfile.is_open())
        {
            for (int i = 0; i < 65536; i++)
            {
                if (IsValidPointer(romMap[i]) && (romMap[i]->name[0] != 0))
                {
                    myfile << "0x" << hex << i << "\t " << romMap[i]->name << "\n";
                    i += (romMap[i]->size - 1);
                }
            }

            myfile.close();
        }

        Debug("Disassembled ROM Saved");
    }
}

bool GearsystemCore::GetRuntimeInfo(GS_RuntimeInfo& runtime_info)
{
    if (m_pCartridge->IsReady())
    {
        if (m_pCartridge->IsGameGear() && !m_pCartridge->IsGameGearInSMSMode())
        {
            runtime_info.screen_width = GS_RESOLUTION_GG_WIDTH;
            runtime_info.screen_height = GS_RESOLUTION_GG_HEIGHT;
        }
        else
        {
            runtime_info.screen_width = GS_RESOLUTION_SMS_WIDTH - m_pVideo->GetHideLeftBarOffset();
            runtime_info.screen_height = m_pVideo->IsExtendedMode224() ? GS_RESOLUTION_SMS_HEIGHT_EXTENDED : GS_RESOLUTION_SMS_HEIGHT;

            if (m_pVideo->GetOverscan() == Video::OverscanFull284)
                runtime_info.screen_width = GS_RESOLUTION_SMS_WIDTH + GS_RESOLUTION_SMS_OVERSCAN_H_284_L + GS_RESOLUTION_SMS_OVERSCAN_H_284_R;
            if (m_pVideo->GetOverscan() == Video::OverscanFull320)
                runtime_info.screen_width = GS_RESOLUTION_SMS_WIDTH + GS_RESOLUTION_SMS_OVERSCAN_H_320_L + GS_RESOLUTION_SMS_OVERSCAN_H_320_R;
            if (m_pVideo->GetOverscan() != Video::OverscanDisabled)
                runtime_info.screen_height = GS_RESOLUTION_SMS_HEIGHT + (2 * (m_pCartridge->IsPAL() ? GS_RESOLUTION_SMS_OVERSCAN_V_PAL : GS_RESOLUTION_SMS_OVERSCAN_V));
        }

        runtime_info.region = m_pCartridge->IsPAL() ? Region_PAL : Region_NTSC;
        return true;
    }

    runtime_info.screen_width = GS_RESOLUTION_MAX_WIDTH;
    runtime_info.screen_height = GS_RESOLUTION_MAX_HEIGHT;
    runtime_info.region = Region_NTSC;

    return false;
}

Memory* GearsystemCore::GetMemory()
{
    return m_pMemory;
}

Cartridge* GearsystemCore::GetCartridge()
{
    return m_pCartridge;
}

Processor* GearsystemCore::GetProcessor()
{
    return m_pProcessor;
}

Audio* GearsystemCore::GetAudio()
{
    return m_pAudio;
}

Video* GearsystemCore::GetVideo()
{
    return m_pVideo;
}

void GearsystemCore::SetGlassesConfig(GlassesConfig config)
{
    m_GlassesConfig = config;
}

u64 GearsystemCore::GetMasterClockCycles()
{
    return m_master_clock_cycles;
}

void GearsystemCore::SetDebugCallback(GS_Debug_Callback callback)
{
    m_debug_callback = callback;
}

void GearsystemCore::KeyPressed(GS_Joypads joypad, GS_Keys key)
{
    m_pInput->KeyPressed(joypad, key);
}

void GearsystemCore::KeyReleased(GS_Joypads joypad, GS_Keys key)
{
    m_pInput->KeyReleased(joypad, key);
}

void GearsystemCore::SetReset(bool pressed)
{
    m_pInput->SetReset(pressed);
}

void GearsystemCore::SetPhaser(int x, int y)
{
    int y_adjust = m_pCartridge->IsPAL() ? GS_RESOLUTION_SMS_OVERSCAN_V_PAL : GS_RESOLUTION_SMS_OVERSCAN_V;

    switch (m_pVideo->GetOverscan())
    {
    case Video::OverscanTopBottom:
        y -= y_adjust;
        break;
    case Video::OverscanFull320:
        x -= GS_RESOLUTION_SMS_OVERSCAN_H_320_L;
        y -= y_adjust;
        break;
    case Video::OverscanFull284:
        x -= GS_RESOLUTION_SMS_OVERSCAN_H_284_L;
        y -= y_adjust;
        break;
    default:
        break;
    }

    m_pInput->SetPhaser(x, y);
}

void GearsystemCore::SetPhaserOffset(int x, int y)
{
    m_pInput->SetPhaserOffset(x, y);
}

void GearsystemCore::EnablePhaser(bool enable)
{
    m_pInput->EnablePhaser(enable);
}

void GearsystemCore::EnablePhaserCrosshair(bool enable, Video::LightPhaserCrosshairShape shape, Video::LightPhaserCrosshairColor color)
{
    m_pVideo->SetLightPhaserCrosshair(enable, shape, color);
}

void GearsystemCore::SetPaddle(float x)
{
    m_pInput->SetPaddle(x);
}

void GearsystemCore::EnablePaddle(bool enable)
{
    m_pInput->EnablePaddle(enable);
}

void GearsystemCore::Pause(bool paused)
{
    if (paused)
    {
        Log("Gearsystem PAUSED");
    }
    else
    {
        Log("Gearsystem RESUMED");
    }
    m_bPaused = paused;
}

bool GearsystemCore::IsPaused()
{
    return m_bPaused;
}

void GearsystemCore::ResetROM(Cartridge::ForceConfiguration* config)
{
    if (m_pCartridge->IsReady())
    {
        Log("Gearsystem RESET");
        if (IsValidPointer(config))
            m_pCartridge->ForceConfig(*config);
        Reset();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        AddMemoryRules();
        m_pProcessor->DisassembleNextOPCode();
    }
}

void GearsystemCore::ResetROMPreservingRAM(Cartridge::ForceConfiguration* config)
{
    if (m_pCartridge->IsReady())
    {
        if (m_pMemory->GetCurrentRule()->PersistedRAM())
        {
            Debug("Resetting preserving RAM...");

            using namespace std;
            stringstream stream;

            m_pMemory->GetCurrentRule()->SaveRam(stream);

            ResetROM(config);

            stream.seekg(0, stream.end);
            s32 size = (s32)stream.tellg();
            stream.seekg(0, stream.beg);

            m_pMemory->GetCurrentRule()->LoadRam(stream, size);
        }
        else
        {
            ResetROM();
        }
    }
}

void GearsystemCore::ResetSound()
{
    m_pAudio->Reset(m_pCartridge->IsPAL());
}

void GearsystemCore::SaveRam()
{
    SaveRam(NULL);
}

void GearsystemCore::SaveRam(const char* szPath, bool fullPath)
{
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()) && m_pMemory->GetCurrentRule()->PersistedRAM())
    {
        Log("Saving RAM...");

        using namespace std;

        string path = "";

        if (IsValidPointer(szPath))
        {
            path += szPath;

            if (!fullPath)
            {
                path += "/";
                path += m_pCartridge->GetFileName();
            }
        }
        else
        {
            path = m_pCartridge->GetFilePath();
        }

        string::size_type i = path.rfind('.', path.length());

        if (i != string::npos) {
            path.replace(i + 1, 3, "sav");
        }

        Log("Save file: %s", path.c_str());

        ofstream file;
        open_ofstream_utf8(file, path.c_str(), ios::out | ios::binary);

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Debug("RAM saved");
    }
}

void GearsystemCore::LoadRam()
{
    LoadRam(NULL);
}

void GearsystemCore::LoadRam(const char* szPath, bool fullPath)
{
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Loading RAM...");

        using namespace std;

        string sav_path = "";

        if (IsValidPointer(szPath))
        {
            sav_path += szPath;

            if (!fullPath)
            {
                sav_path += "/";
                sav_path += m_pCartridge->GetFileName();
            }
        }
        else
        {
            sav_path = m_pCartridge->GetFilePath();
        }

        string rom_path = sav_path;

        string::size_type i = sav_path.rfind('.', sav_path.length());

        if (i != string::npos) {
            sav_path.replace(i + 1, 3, "sav");
        }

        Log("Opening save file: %s", sav_path.c_str());

        ifstream file;

        open_ifstream_utf8(file, sav_path.c_str(), ios::in | ios::binary);

        // check for old .gearsystem saves
        if (file.fail())
        {
            Log("Save file doesn't exist");
            string old_sav_file = rom_path + ".gearsystem";

            Log("Opening old save file: %s", old_sav_file.c_str());
            open_ifstream_utf8(file, old_sav_file.c_str(), ios::in | ios::binary);
        }

        if (!file.fail())
        {
            file.seekg(0, file.end);
            s32 fileSize = (s32)file.tellg();
            file.seekg(0, file.beg);

            if (m_pMemory->GetCurrentRule()->LoadRam(file, fileSize))
            {
                Debug("RAM loaded");
            }
            else
            {
                Log("Save file size incorrect: %d", fileSize);
            }
        }
        else
        {
            Log("Save file doesn't exist");
        }
    }
}

std::string GearsystemCore::GetSaveStatePath(const char* path, int index)
{
    if (index < 0)
        return path;

    using namespace std;
    string full_path;

    if (IsValidPointer(path))
    {
        full_path = path;
        full_path += "/";
        full_path += m_pCartridge->GetFileName();
    }
    else
        full_path = m_pCartridge->GetFilePath();

    string::size_type dot_index = full_path.rfind('.');

    if (dot_index != string::npos)
        full_path.replace(dot_index + 1, full_path.length() - dot_index - 1, "state");

    stringstream ss;
    ss << index;
    full_path += ss.str();

    return full_path;
}

bool GearsystemCore::SaveState(const char* path, int index, bool screenshot)
{
    using namespace std;

    string full_path = GetSaveStatePath(path, index);
    Debug("Saving state to %s...", full_path.c_str());

    ofstream stream;
    open_ofstream_utf8(stream, full_path.c_str(), ios::out | ios::binary);

    size_t size;
    bool ret = SaveState(stream, size, screenshot);
    if (ret)
        Log("Saved state to %s", full_path.c_str());
    else
        Error("Failed to save state to %s", full_path.c_str());
    return ret;
}

bool GearsystemCore::SaveState(u8* buffer, size_t& size, bool screenshot)
{
    using namespace std;

    Debug("Saving state to buffer [%d bytes]...", size);

    if (!m_pCartridge->IsReady())
    {
        Error("Cartridge is not ready when trying to save state");
        return false;
    }

    if (!IsValidPointer(buffer))
    {
        stringstream stream;
        if (!SaveState(stream, size, screenshot))
        {
            Error("Failed to save state to stream to calculate size");
            return false;
        }
        return true;
    }
    else
    {
        memory_stream direct_stream(reinterpret_cast<char*>(buffer), size);

        if (!SaveState(direct_stream, size, screenshot))
        {
            Error("Failed to save state to buffer");
            return false;
        }

        size = direct_stream.size();
        return true;
    }
}

bool GearsystemCore::SaveState(std::ostream& stream, size_t& size, bool screenshot)
{
    using namespace std;

    if (!m_pCartridge->IsReady())
    {
        Error("Cartridge is not ready when trying to save state");
        return false;
    }

    Debug("Serializing save state...");

    m_pMemory->SaveState(stream);
    m_pProcessor->SaveState(stream);
    m_pAudio->SaveState(stream);
    m_pVideo->SaveState(stream);
    m_pInput->SaveState(stream);
    m_pMemory->GetCurrentRule()->SaveState(stream);
    m_pProcessor->GetIOPOrts()->SaveState(stream);

#if defined(__LIBRETRO__)
    GS_SaveState_Header_Libretro header;
    header.magic = GS_SAVESTATE_MAGIC;
    header.version = GS_SAVESTATE_VERSION;
    Debug("Save state header magic: 0x%08x", header.magic);
    Debug("Save state header version: %d", header.version);
#else
    GS_SaveState_Header header;
    header.magic = GS_SAVESTATE_MAGIC;
    header.version = GS_SAVESTATE_VERSION;

    header.timestamp = time(NULL);
    strncpy_fit(header.rom_name, m_pCartridge->GetFileName(), sizeof(header.rom_name));
    header.rom_crc = m_pCartridge->GetCRC();
    strncpy_fit(header.emu_build, GS_VERSION, sizeof(header.emu_build));

    Debug("Save state header magic: 0x%08x", header.magic);
    Debug("Save state header version: %d", header.version);
    Debug("Save state header timestamp: %d", header.timestamp);
    Debug("Save state header rom name: %s", header.rom_name);
    Debug("Save state header rom crc: 0x%08x", header.rom_crc);
    Debug("Save state header emu build: %s", header.emu_build);

    if (screenshot)
    {
        GS_RuntimeInfo runtime_info;
        GetRuntimeInfo(runtime_info);
        header.screenshot_width = runtime_info.screen_width;
        header.screenshot_height = runtime_info.screen_height;

        int bytes_per_pixel = 2;
        if (m_pixelFormat == GS_PIXEL_RGBA8888 || m_pixelFormat == GS_PIXEL_BGRA8888)
            bytes_per_pixel = 4;

        u8* frame_buffer = m_pFrameBuffer;

        header.screenshot_size = header.screenshot_width * header.screenshot_height * bytes_per_pixel;
        stream.write(reinterpret_cast<const char*>(frame_buffer), header.screenshot_size);
    }
    else
    {
        header.screenshot_size = 0;
        header.screenshot_width = 0;
        header.screenshot_height = 0;
    }

    Debug("Save state header screenshot size: %d", header.screenshot_size);
    Debug("Save state header screenshot width: %d", header.screenshot_width);
    Debug("Save state header screenshot height: %d", header.screenshot_height);
#endif

    size = static_cast<size_t>(stream.tellp());
    size += sizeof(header);

#if !defined(__LIBRETRO__)
    header.size = static_cast<u32>(size);
    Debug("Save state header size: %d", header.size);
#endif

    stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
    return true;
}

bool GearsystemCore::LoadState(const char* path, int index)
{
    using namespace std;
    bool ret = false;

    string full_path = GetSaveStatePath(path, index);
    Debug("Loading state from %s...", full_path.c_str());

    ifstream stream;
    open_ifstream_utf8(stream, full_path.c_str(), ios::in | ios::binary);

    if (!stream.fail())
    {
        ret = LoadState(stream);

        if (ret)
            Log("Loaded state from %s", full_path.c_str());
        else
            Error("Failed to load state from %s", full_path.c_str());
    }
    else
    {
        Error("Load state file doesn't exist: %s", full_path.c_str());
    }

    stream.close();
    return ret;
}

bool GearsystemCore::LoadState(const u8* buffer, size_t size)
{
    using namespace std;

    Debug("Loading state to buffer [%d bytes]...", size);

    if (!m_pCartridge->IsReady())
    {
        Error("Cartridge is not ready when trying to load state");
        return false;
    }

    if (!IsValidPointer(buffer) || (size == 0))
    {
        Error("Invalid load state buffer");
        return false;
    }

    memory_input_stream direct_stream(reinterpret_cast<const char*>(buffer), size);
    return LoadState(direct_stream);
}

bool GearsystemCore::LoadState(std::istream& stream)
{
    using namespace std;

    if (!m_pCartridge->IsReady())
    {
        Error("Cartridge is not ready when trying to load state");
        return false;
    }

#if defined(__LIBRETRO__)
    GS_SaveState_Header_Libretro header;
#else
    GS_SaveState_Header header;
#endif

    stream.seekg(0, ios::end);
    size_t size = static_cast<size_t>(stream.tellg());
    stream.seekg(0, ios::beg);

    if (size < sizeof(header))
    {
        Log("Save state too small for current header (%d bytes), trying V1 format...", static_cast<int>(size));
        return LoadStateV1(stream, size);
    }

    stream.seekg(size - sizeof(header), ios::beg);
    stream.read(reinterpret_cast<char*> (&header), sizeof(header));
    stream.seekg(0, ios::beg);

    Debug("Load state header magic: 0x%08x", header.magic);
    Debug("Load state header version: %d", header.version);

    if (header.magic != GS_SAVESTATE_MAGIC || header.version != GS_SAVESTATE_VERSION)
    {
        Log("Save state header does not match current version, trying V1 format...");
        return LoadStateV1(stream, size);
    }

#if !defined(__LIBRETRO__)
    Debug("Load state header size: %d", header.size);
    Debug("Load state header timestamp: %d", header.timestamp);
    Debug("Load state header rom name: %s", header.rom_name);
    Debug("Load state header rom crc: 0x%08x", header.rom_crc);
    Debug("Load state header screenshot size: %d", header.screenshot_size);
    Debug("Load state header screenshot width: %d", header.screenshot_width);
    Debug("Load state header screenshot height: %d", header.screenshot_height);
    Debug("Load state header emu build: %s", header.emu_build);

    if (header.size != size)
    {
        Error("Invalid save state size: %d", header.size);
        return false;
    }

    if (header.rom_crc != m_pCartridge->GetCRC())
    {
        Error("Invalid save state rom crc: 0x%08x", header.rom_crc);
        return false;
    }
#endif

    Debug("Unserializing save state...");

    m_pMemory->LoadState(stream);
    m_pProcessor->LoadState(stream);
    m_pAudio->LoadState(stream);
    m_pVideo->LoadState(stream);
    m_pInput->LoadState(stream);
    m_pMemory->GetCurrentRule()->LoadState(stream);
    m_pProcessor->GetIOPOrts()->LoadState(stream);

    return true;
}

bool GearsystemCore::LoadStateV1(std::istream& stream, size_t size)
{
    using namespace std;

    if (size < (3 * sizeof(u32)))
    {
        Error("Save state too small for V1 header (%d bytes)", static_cast<int>(size));
        return false;
    }

    u32 v1_magic = 0;
    u32 v1_version = 0;
    u32 v1_size = 0;

    stream.seekg(size - (3 * sizeof(u32)), ios::beg);
    stream.read(reinterpret_cast<char*>(&v1_magic), sizeof(v1_magic));
    stream.read(reinterpret_cast<char*>(&v1_version), sizeof(v1_version));
    stream.read(reinterpret_cast<char*>(&v1_size), sizeof(v1_size));
    stream.seekg(0, ios::beg);

    Debug("Load state V1 magic: 0x%08x", v1_magic);
    Debug("Load state V1 version: %d", v1_version);
    Debug("Load state V1 size: %d", v1_size);

    if (v1_magic != GS_SAVESTATE_MAGIC)
    {
        Error("Invalid V1 save state magic: 0x%08x", v1_magic);
        return false;
    }

    if (v1_version != GS_SAVESTATE_VERSION_V1)
    {
        Error("Invalid V1 save state version: %d", v1_version);
        return false;
    }

    if (v1_size != size)
    {
        Error("Invalid V1 save state size: %d (expected %d)", v1_size, static_cast<int>(size));
        return false;
    }

    Log("Loading V1 save state (%d bytes)...", static_cast<int>(size));

    m_pMemory->LoadState(stream);
    m_pProcessor->LoadState(stream);
    m_pAudio->LoadStateV1(stream);
    m_pVideo->LoadState(stream);
    m_pInput->LoadState(stream);
    m_pMemory->GetCurrentRule()->LoadState(stream);
    m_pProcessor->GetIOPOrts()->LoadState(stream);

    return true;
}

bool GearsystemCore::GetSaveStateHeader(int index, const char* path, GS_SaveState_Header* header)
{
    using namespace std;

    string full_path = GetSaveStatePath(path, index);
    Debug("Loading state header from %s...", full_path.c_str());

    ifstream stream;
    open_ifstream_utf8(stream, full_path.c_str(), ios::in | ios::binary);

    if (stream.fail())
    {
        Debug("ERROR: Savestate file doesn't exist %s", full_path.c_str());
        stream.close();
        return false;
    }

    stream.seekg(0, ios::end);
    size_t savestate_size = static_cast<size_t>(stream.tellg());
    stream.seekg(0, ios::beg);

    if (savestate_size < sizeof(GS_SaveState_Header))
    {
        if (savestate_size < (3 * sizeof(u32)))
        {
            stream.close();
            return false;
        }

        u32 v1_magic = 0;
        u32 v1_version = 0;
        u32 v1_size = 0;

        stream.seekg(savestate_size - (3 * sizeof(u32)), ios::beg);
        stream.read(reinterpret_cast<char*>(&v1_magic), sizeof(v1_magic));
        stream.read(reinterpret_cast<char*>(&v1_version), sizeof(v1_version));
        stream.read(reinterpret_cast<char*>(&v1_size), sizeof(v1_size));
        stream.close();

        if (v1_magic != GS_SAVESTATE_MAGIC || v1_version != GS_SAVESTATE_VERSION_V1 || v1_size != savestate_size)
            return false;

        memset(header, 0, sizeof(GS_SaveState_Header));
        header->magic = v1_magic;
        header->version = v1_version;
        header->size = v1_size;
        strncpy_fit(header->rom_name, m_pCartridge->GetFileName(), sizeof(header->rom_name));
        return true;
    }

    stream.seekg(savestate_size - sizeof(GS_SaveState_Header), ios::beg);
    stream.read(reinterpret_cast<char*> (header), sizeof(GS_SaveState_Header));
    stream.seekg(0, ios::beg);

    // for older versions of the save state without build in header
    if (header->magic != GS_SAVESTATE_MAGIC)
    {
        if (savestate_size >= sizeof(GS_SaveState_Header) - 32)
        {
            stream.seekg(savestate_size - sizeof(GS_SaveState_Header) + 32, ios::beg);
            stream.read(reinterpret_cast<char*> (header), sizeof(GS_SaveState_Header));
            stream.seekg(0, ios::beg);
        }

        if (header->magic != GS_SAVESTATE_MAGIC)
        {
            // try V1 format (12-byte footer: magic + version + size)
            stream.clear();

            u32 v1_magic = 0;
            u32 v1_version = 0;
            u32 v1_size = 0;

            stream.seekg(savestate_size - (3 * sizeof(u32)), ios::beg);
            stream.read(reinterpret_cast<char*>(&v1_magic), sizeof(v1_magic));
            stream.read(reinterpret_cast<char*>(&v1_version), sizeof(v1_version));
            stream.read(reinterpret_cast<char*>(&v1_size), sizeof(v1_size));
            stream.close();

            if (v1_magic != GS_SAVESTATE_MAGIC || v1_version != GS_SAVESTATE_VERSION_V1 || v1_size != savestate_size)
                return false;

            memset(header, 0, sizeof(GS_SaveState_Header));
            header->magic = v1_magic;
            header->version = v1_version;
            header->size = v1_size;
            strncpy_fit(header->rom_name, m_pCartridge->GetFileName(), sizeof(header->rom_name));
            return true;
        }

        header->size += 32;
        header->emu_build[0] = 0;
    }

    return true;
}

bool GearsystemCore::GetSaveStateScreenshot(int index, const char* path, GS_SaveState_Screenshot* screenshot)
{
    using namespace std;

    if (!IsValidPointer(screenshot->data) || (screenshot->size == 0))
    {
        Error("Invalid save state screenshot buffer");
        return false;
    }

    string full_path = GetSaveStatePath(path, index);
    Debug("Loading state screenshot from %s...", full_path.c_str());

    ifstream stream;
    open_ifstream_utf8(stream, full_path.c_str(), ios::in | ios::binary);

    if (stream.fail())
    {
        Error("Savestate file doesn't exist %s", full_path.c_str());
        stream.close();
        return false;
    }

    GS_SaveState_Header header;
    GetSaveStateHeader(index, path, &header);

    if (header.screenshot_size == 0)
    {
        Debug("No screenshot data");
        stream.close();
        return false;
    }

    if (screenshot->size < header.screenshot_size)
    {
        Error("Invalid screenshot buffer size %d < %d", screenshot->size, header.screenshot_size);
        stream.close();
        return false;
    }

    screenshot->size = header.screenshot_size;
    screenshot->width = header.screenshot_width;
    screenshot->height = header.screenshot_height;

    Debug("Screenshot size: %d bytes", screenshot->size);
    Debug("Screenshot width: %d", screenshot->width);
    Debug("Screenshot height: %d", screenshot->height);

    if (header.size < sizeof(header) + screenshot->size)
    {
        Error("Invalid screenshot offset: header size %d too small for screenshot %d", header.size, screenshot->size);
        stream.close();
        return false;
    }

    stream.seekg(header.size - sizeof(header) - screenshot->size, ios::beg);
    stream.read(reinterpret_cast<char*> (screenshot->data), screenshot->size);
    stream.close();

    return true;
}

void GearsystemCore::SetCheat(const char* szCheat)
{
    std::string s = szCheat;
    if ((s.length() == 7) || (s.length() == 11))
    {
        m_pCartridge->SetGameGenieCheat(szCheat);
        if (m_pCartridge->IsReady())
            m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
    }
    else
    {
        m_pProcessor->SetProActionReplayCheat(szCheat);
    }
}

void GearsystemCore::ClearCheats()
{
    m_pCartridge->ClearGameGenieCheats();
    m_pProcessor->ClearProActionReplayCheats();
    if (m_pCartridge->IsReady())
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
}

void GearsystemCore::SetRamModificationCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

void GearsystemCore::InitMemoryRules()
{
    m_pSG1000MemoryRule = new SG1000MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pCodemastersMemoryRule = new CodemastersMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pSegaMemoryRule = new SegaMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanMemoryRule = new KoreanMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanMSXSMS8000MemoryRule = new KoreanMSXSMS8000MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanSMS32KB2000MemoryRule = new KoreanSMS32KB2000MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanMSX32KB2000MemoryRule = new KoreanMSX32KB2000MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKorean2000XOR1FMemoryRule = new Korean2000XOR1FMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanMSX8KB0300MemoryRule = new KoreanMSX8KB0300MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKorean0000XORFFMemoryRule = new Korean0000XORFFMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanFFFFHiComMemoryRule = new KoreanFFFFHiComMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanFFFEMemoryRule = new KoreanFFFEMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanBFFCMemoryRule = new KoreanBFFCMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanFFF3FFFCMemoryRule = new KoreanFFF3FFFCMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanMDFFF5MemoryRule = new KoreanMDFFF5MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pKoreanMDFFF0MemoryRule = new KoreanMDFFF0MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pMSXMemoryRule = new MSXMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pJanggunMemoryRule = new JanggunMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pMulti4PAKAllActionMemoryRule = new Multi4PAKAllActionMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pJumboDahjeeMemoryRule = new JumboDahjeeMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pEeprom93C46MemoryRule = new Eeprom93C46MemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pBootromMemoryRule = new BootromMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pIratahackMemoryRule = new IratahackMemoryRule(m_pMemory, m_pCartridge, m_pInput);
    m_pMemory->SetCurrentRule(m_pRomOnlyMemoryRule);
    m_pMemory->SetBootromRule(m_pBootromMemoryRule);
    m_pProcessor->SetIOPOrts(m_pSmsIOPorts);
}

bool GearsystemCore::AddMemoryRules()
{
    Cartridge::CartridgeTypes type = m_pCartridge->GetType();

    bool notSupported = false;

    switch (type)
    {
        case Cartridge::CartridgeRomOnlyMapper:
            m_pMemory->SetCurrentRule(m_pRomOnlyMemoryRule);
            break;
        case Cartridge::CartridgeSegaMapper:
            m_pMemory->SetCurrentRule(m_pSegaMemoryRule);
            break;
        case Cartridge::CartridgeCodemastersMapper:
            m_pMemory->SetCurrentRule(m_pCodemastersMemoryRule);
            break;
        case Cartridge::CartridgeSG1000Mapper:
            m_pMemory->SetCurrentRule(m_pSG1000MemoryRule);
            break;
        case Cartridge::CartridgeKoreanMapper:
            m_pMemory->SetCurrentRule(m_pKoreanMemoryRule);
            break;
        case Cartridge::CartridgeKoreanMSXSMS8000Mapper:
            m_pMemory->SetCurrentRule(m_pKoreanMSXSMS8000MemoryRule);
            break;
        case Cartridge::CartridgeKoreanSMS32KB2000Mapper:
            m_pMemory->SetCurrentRule(m_pKoreanSMS32KB2000MemoryRule);
            break;
        case Cartridge::CartridgeKoreanMSX32KB2000Mapper:
            m_pMemory->SetCurrentRule(m_pKoreanMSX32KB2000MemoryRule);
            break;
        case Cartridge::CartridgeKorean2000XOR1FMapper:
            m_pMemory->SetCurrentRule(m_pKorean2000XOR1FMemoryRule);
            break;
        case Cartridge::CartridgeKoreanMSX8KB0300Mapper:
            m_pMemory->SetCurrentRule(m_pKoreanMSX8KB0300MemoryRule);
            break;
        case Cartridge::CartridgeKorean0000XORFFMapper:
            m_pMemory->SetCurrentRule(m_pKorean0000XORFFMemoryRule);
            break;
        case Cartridge::CartridgeKoreanFFFFHiComMapper:
            m_pMemory->SetCurrentRule(m_pKoreanFFFFHiComMemoryRule);
            break;
        case Cartridge::CartridgeKoreanFFFEMapper:
            m_pMemory->SetCurrentRule(m_pKoreanFFFEMemoryRule);
            break;
        case Cartridge::CartridgeKoreanBFFCMapper:
            m_pMemory->SetCurrentRule(m_pKoreanBFFCMemoryRule);
            break;
        case Cartridge::CartridgeKoreanFFF3FFFCMapper:
            m_pMemory->SetCurrentRule(m_pKoreanFFF3FFFCMemoryRule);
            break;
        case Cartridge::CartridgeKoreanMDFFF5Mapper:
            m_pMemory->SetCurrentRule(m_pKoreanMDFFF5MemoryRule);
            break;
        case Cartridge::CartridgeKoreanMDFFF0Mapper:
            m_pMemory->SetCurrentRule(m_pKoreanMDFFF0MemoryRule);
            break;
        case Cartridge::CartridgeMSXMapper:
            m_pMemory->SetCurrentRule(m_pMSXMemoryRule);
            break;
        case Cartridge::CartridgeJanggunMapper:
            m_pMemory->SetCurrentRule(m_pJanggunMemoryRule);
            break;
        case Cartridge::CartridgeMulti4PAKAllActionMapper:
            m_pMemory->SetCurrentRule(m_pMulti4PAKAllActionMemoryRule);
            break;
        case Cartridge::CartridgeJumboDahjeeMapper:
            m_pMemory->SetCurrentRule(m_pJumboDahjeeMemoryRule);
            break;
        case Cartridge::CartridgeIratahackMapper:
            m_pMemory->SetCurrentRule(m_pIratahackMemoryRule);
            break;
        case Cartridge::CartridgeEeprom93C46Mapper:
            m_pMemory->SetCurrentRule(m_pEeprom93C46MemoryRule);
            break;
        case Cartridge::CartridgeNotSupported:
            notSupported = true;
            break;
        default:
            notSupported = true;
    }

    if (m_pCartridge->IsGameGear())
    {
        Log("Game Gear IO ports enabled");
        m_pProcessor->SetIOPOrts(m_pGameGearIOPorts);
    }
    else
    {
        Log("Master System IO ports enabled");
        m_pProcessor->SetIOPOrts(m_pSmsIOPorts);
    }

    return !notSupported;
}

void GearsystemCore::Reset()
{
    m_pMemory->Reset(m_pCartridge->IsGameGear());
    m_pProcessor->Reset(m_pCartridge->GetGameGearASIC() == 1);
    m_pAudio->Reset(m_pCartridge->IsPAL());
    m_pVideo->Reset(m_pCartridge->IsGameGear(), m_pCartridge->IsPAL(), m_pCartridge->GetGameGearASIC(), m_pCartridge->IsGameGearInSMSMode());
    m_pInput->Reset(m_pCartridge->IsGameGear());
    m_pSegaMemoryRule->Reset();
    m_pCodemastersMemoryRule->Reset();
    m_pSG1000MemoryRule->Reset();
    m_pRomOnlyMemoryRule->Reset();
    m_pKoreanMemoryRule->Reset();
    m_pKoreanMSXSMS8000MemoryRule->Reset();
    m_pKoreanSMS32KB2000MemoryRule->Reset();
    m_pKoreanMSX32KB2000MemoryRule->Reset();
    m_pKorean2000XOR1FMemoryRule->Reset();
    m_pKoreanMSX8KB0300MemoryRule->Reset();
    m_pKorean0000XORFFMemoryRule->Reset();
    m_pKoreanFFFFHiComMemoryRule->Reset();
    m_pKoreanFFFEMemoryRule->Reset();
    m_pKoreanBFFCMemoryRule->Reset();
    m_pKoreanFFF3FFFCMemoryRule->Reset();
    m_pKoreanMDFFF5MemoryRule->Reset();
    m_pKoreanMDFFF0MemoryRule->Reset();
    m_pMSXMemoryRule->Reset();
    m_pJanggunMemoryRule->Reset();
    m_pIratahackMemoryRule->Reset();
    m_pMulti4PAKAllActionMemoryRule->Reset();
    m_pJumboDahjeeMemoryRule->Reset();
    m_pEeprom93C46MemoryRule->Reset();
    m_pBootromMemoryRule->Reset();
    m_pGameGearIOPorts->Reset();
    m_pSmsIOPorts->Reset();
    m_bPaused = false;
    m_master_clock_cycles = 0;
}

void GearsystemCore::RenderFrameBuffer(u8* finalFrameBuffer)
{
    if (m_pInput->IsPhaserEnabled())
    {
        Input::stPhaser* phaser = m_pInput->GetPhaser();
        m_pVideo->DrawPhaserCrosshair(phaser->x, phaser->y);
    }

    if (m_GlassesConfig != GearsystemCore::GlassesBothEyes)
    {
        bool left = IsSetBit(m_pInput->GetGlassesRegistry(), 0);

        if ((m_GlassesConfig == GearsystemCore::GlassesLeftEye) && !left)
            return;
        else if ((m_GlassesConfig == GearsystemCore::GlassesRightEye) && left)
            return;
    }

    int size = GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN;

    switch (m_pixelFormat)
    {
        case GS_PIXEL_RGB555:
        case GS_PIXEL_BGR555:
        case GS_PIXEL_RGB565:
        case GS_PIXEL_BGR565:
        {
            m_pVideo->Render16bit(m_pVideo->GetFrameBuffer(), finalFrameBuffer, m_pixelFormat, size, true);
            break;
        }
        case GS_PIXEL_RGBA8888:
        case GS_PIXEL_BGRA8888:
        {
            m_pVideo->Render32bit(m_pVideo->GetFrameBuffer(), finalFrameBuffer, m_pixelFormat, size, true);
            break;
        }
    }
}
