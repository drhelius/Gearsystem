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
    Log("Loading %s core %s by Ignacio Sanchez", GEARSYSTEM_TITLE, GEARSYSTEM_VERSION);

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

bool GearsystemCore::RunToVBlank(u8* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount, bool step, bool stopOnBreakpoints)
{
    bool breakpoint = false;

    if (!m_bPaused && m_pCartridge->IsReady())
    {
        bool vblank = false;
        int totalClocks = 0;
        while (!vblank)
        {
#ifdef PERFORMANCE
            unsigned int clockCycles = m_pProcessor->RunFor(75);
#else
            unsigned int clockCycles = m_pProcessor->RunFor(1);
#endif
            vblank = m_pVideo->Tick(clockCycles);
            m_pAudio->Tick(clockCycles);
            totalClocks += clockCycles;

#ifndef GEARSYSTEM_DISABLE_DISASSEMBLER
            if ((step || (stopOnBreakpoints && m_pProcessor->BreakpointHit())))
            {
                vblank = true;
                if (m_pProcessor->BreakpointHit())
                    breakpoint = true;
            }
#endif

            if (totalClocks > 702240)
                vblank = true;
        }

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);
        RenderFrameBuffer(pFrameBuffer);
    }

    return breakpoint;
}

bool GearsystemCore::LoadROM(const char* szFilePath, Cartridge::ForceConfiguration* config)
{
    if (m_pCartridge->LoadFromFile(szFilePath))
    {
        if (IsValidPointer(config))
            m_pCartridge->ForceConfig(*config);
        Reset();
        m_pMemory->ResetDisassembledMemory();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();

        m_pProcessor->DisassembleNextOpcode();

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
        m_pMemory->ResetDisassembledMemory();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();

        m_pProcessor->DisassembleNextOpcode();

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
    Memory::stDisassembleRecord** romMap = m_pMemory->GetDisassembledROMMemoryMap();

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
        if (m_pCartridge->IsGameGear())
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
        m_pProcessor->DisassembleNextOpcode();
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

void GearsystemCore::SaveState(int index)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Log("Save states disabled when running BIOS");
        return;
    }

    Log("Creating save state %d...", index);

    SaveState(NULL, index);

    Debug("Save state %d created", index);
}

void GearsystemCore::SaveState(const char* szPath, int index)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Log("Save states disabled when running BIOS");
        return;
    }

    Log("Creating save state...");

    using namespace std;

    size_t size;
    SaveState(NULL, size);

    u8* buffer = new u8[size];
    string path = "";

    if (IsValidPointer(szPath))
    {
        path += szPath;
        path += "/";
        path += m_pCartridge->GetFileName();
    }
    else
    {
        path = m_pCartridge->GetFilePath();
    }

    string::size_type i = path.rfind('.', path.length());

    if (i != string::npos) {
        path.replace(i + 1, 3, "state");
    }

    std::stringstream sstm;

    if (index < 0)
        sstm << szPath;
    else
        sstm << path << index;

    Log("Save state file: %s", sstm.str().c_str());

    ofstream file;
    open_ofstream_utf8(file, sstm.str().c_str(), ios::out | ios::binary);

    SaveState(file, size);

    SafeDeleteArray(buffer);

    file.close();

    Debug("Save state created");
}

bool GearsystemCore::SaveState(u8* buffer, size_t& size)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Debug("Save states disabled when running BIOS");
        return false;
    }

    bool ret = false;

    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        using namespace std;

        stringstream stream;

        if (SaveState(stream, size))
            ret = true;

        if (IsValidPointer(buffer))
        {
            Log("Saving state to buffer [%d bytes]...", size);
            memcpy(buffer, stream.str().c_str(), size);
            ret = true;
        }
    }
    else
    {
        Log("Invalid rom or memory rule.");
    }

    return ret;
}

bool GearsystemCore::SaveState(std::ostream& stream, size_t& size)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Debug("Save states disabled when running BIOS");
        return false;
    }

    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Debug("Gathering save state data...");

        using namespace std;

        m_pMemory->SaveState(stream);
        m_pProcessor->SaveState(stream);
        m_pAudio->SaveState(stream);
        m_pVideo->SaveState(stream);
        m_pInput->SaveState(stream);
        m_pMemory->GetCurrentRule()->SaveState(stream);
        m_pProcessor->GetIOPOrts()->SaveState(stream);

        size = static_cast<size_t>(stream.tellp());
        size += (sizeof(u32) * 3);

        u32 header_magic = GS_SAVESTATE_MAGIC;
        u32 header_version = GS_SAVESTATE_VERSION;
        u32 header_size = static_cast<u32>(size);

        stream.write(reinterpret_cast<const char*> (&header_magic), sizeof(header_magic));
        stream.write(reinterpret_cast<const char*> (&header_version), sizeof(header_version));
        stream.write(reinterpret_cast<const char*> (&header_size), sizeof(header_size));

        Log("Save state size: %d", static_cast<size_t>(stream.tellp()));

        return true;
    }

    Log("Invalid rom or memory rule.");

    return false;
}

void GearsystemCore::LoadState(int index)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Debug("Save states disabled when running BIOS");
        return;
    }

    Log("Loading save state %d...", index);

    LoadState(NULL, index);

    Debug("State %d file loaded", index);
}

void GearsystemCore::LoadState(const char* szPath, int index)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Debug("Save states disabled when running BIOS");
        return;
    }

    Log("Loading save state...");

    using namespace std;

    string sav_path = "";

    if (IsValidPointer(szPath))
    {
        sav_path += szPath;
        sav_path += "/";
        sav_path += m_pCartridge->GetFileName();
    }
    else
    {
        sav_path = m_pCartridge->GetFilePath();
    }

    string rom_path = sav_path;

    string::size_type i = sav_path.rfind('.', sav_path.length());

    if (i != string::npos) {
        sav_path.replace(i + 1, 3, "state");
    }

    std::stringstream sstm;

    if (index < 0)
        sstm << szPath;
    else
        sstm << sav_path << index;

    Log("Opening save file: %s", sstm.str().c_str());

    ifstream file;

    open_ifstream_utf8(file, sstm.str().c_str(), ios::in | ios::binary);

    if (!file.fail())
    {
        if (LoadState(file))
        {
            Debug("Save state loaded");
        }
    }
    else
    {
        Log("Save state file doesn't exist");
    }

    file.close();
}

bool GearsystemCore::LoadState(const u8* buffer, size_t size)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Debug("Save states disabled when running BIOS");
        return false;
    }

    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()) && (size > 0) && IsValidPointer(buffer))
    {
        Debug("Gathering load state data [%d bytes]...", size);

        using namespace std;

        stringstream stream;

        stream.write(reinterpret_cast<const char*> (buffer), size);

        return LoadState(stream);
    }

    Log("Invalid rom or memory rule.");

    return false;
}

bool GearsystemCore::LoadState(std::istream& stream)
{
    if (m_pMemory->GetCurrentSlot() == Memory::BiosSlot)
    {
        Debug("Save states disabled when running BIOS");
        return false;
    }

    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        using namespace std;

        u32 header_magic = 0;
        u32 header_size = 0;
        u32 header_version = 0;

        stream.seekg(0, ios::end);
        size_t size = static_cast<size_t>(stream.tellg());
        stream.seekg(0, ios::beg);

        Debug("Load state stream size: %d", size);

        stream.seekg(size - (3 * sizeof(u32)), ios::beg);
        stream.read(reinterpret_cast<char*> (&header_magic), sizeof(header_magic));
        stream.read(reinterpret_cast<char*> (&header_version), sizeof(header_version));
        stream.read(reinterpret_cast<char*> (&header_size), sizeof(header_size));
        stream.seekg(0, ios::beg);

        Debug("Load state magic: 0x%08x", header_magic);
        Debug("Load state version: %d", header_version);
        Debug("Load state size: %d", header_size);

        if ((header_size == size) && (header_magic == GS_SAVESTATE_MAGIC) && (header_version == GS_SAVESTATE_VERSION))
        {
            Log("Loading state...");

            m_pMemory->LoadState(stream);
            m_pProcessor->LoadState(stream);
            m_pAudio->LoadState(stream);
            m_pVideo->LoadState(stream);
            m_pInput->LoadState(stream);
            m_pMemory->GetCurrentRule()->LoadState(stream);
            m_pProcessor->GetIOPOrts()->LoadState(stream);

            return true;
        }
        else
        {
            Log("Invalid save state size or header");
            if (header_version != GS_SAVESTATE_VERSION)
                Log("Save state version mismatch. Expected: %d, Found: %d", GS_SAVESTATE_VERSION, header_version);
        }
    }
    else
    {
        Log("Invalid rom or memory rule");
    }

    return false;
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
    m_pProcessor->Reset();
    m_pAudio->Reset(m_pCartridge->IsPAL());
    m_pVideo->Reset(m_pCartridge->IsGameGear(), m_pCartridge->IsPAL());
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
