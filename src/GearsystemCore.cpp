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
#include "Video.h"
#include "Input.h"
#include "Cartridge.h"
#include "MemoryRule.h"
#include "SegaMemoryRule.h"
#include "CodemastersMemoryRule.h"
#include "RomOnlyMemoryRule.h"
#include "KoreanMemoryRule.h"
#include "MSXMemoryRule.h"
#include "SG1000MemoryRule.h"
#include "SmsIOPorts.h"
#include "GameGearIOPorts.h"

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
    InitPointer(m_pMSXMemoryRule);
    InitPointer(m_pSmsIOPorts);
    InitPointer(m_pGameGearIOPorts);
    m_bPaused = true;
}

GearsystemCore::~GearsystemCore()
{
    SafeDelete(m_pGameGearIOPorts);
    SafeDelete(m_pSmsIOPorts);
    SafeDelete(m_pRomOnlyMemoryRule);
    SafeDelete(m_pCodemastersMemoryRule);
    SafeDelete(m_pSG1000MemoryRule);
    SafeDelete(m_pSegaMemoryRule);
    SafeDelete(m_pKoreanMemoryRule);
    SafeDelete(m_pMSXMemoryRule);
    SafeDelete(m_pCartridge);
    SafeDelete(m_pInput);
    SafeDelete(m_pVideo);
    SafeDelete(m_pAudio);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearsystemCore::Init()
{
    Log("--== %s %s by Ignacio Sanchez ==--", GEARSYSTEM_TITLE, GEARSYSTEM_VERSION);

    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pAudio = new Audio();
    m_pVideo = new Video(m_pMemory, m_pProcessor);
    m_pInput = new Input(m_pProcessor);
    m_pCartridge = new Cartridge();
    m_pSmsIOPorts = new SmsIOPorts(m_pAudio, m_pVideo, m_pInput, m_pCartridge);
    m_pGameGearIOPorts = new GameGearIOPorts(m_pAudio, m_pVideo, m_pInput, m_pCartridge);

    m_pMemory->Init();
    m_pProcessor->Init();
    m_pAudio->Init();
    m_pVideo->Init();
    m_pInput->Init();
    m_pCartridge->Init();

    InitMemoryRules();
}

bool GearsystemCore::RunToVBlank(GS_Color* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount, bool step, bool stopOnBreakpoints)
{
    bool breakpoint = false;

    if (!m_bPaused && m_pCartridge->IsReady())
    {
        bool vblank = false;
        int totalClocks = 0;
        while (!vblank)
        {
            unsigned int clockCycles = m_pProcessor->Tick();
            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
            m_pAudio->Tick(clockCycles);
            m_pInput->Tick(clockCycles);
            

            if ((step || (stopOnBreakpoints && m_pProcessor->BreakpointHit())))
            {
                vblank = true;
                if (m_pProcessor->BreakpointHit())
                    breakpoint = true;
            }

            totalClocks += clockCycles;

            if (totalClocks > 702240)
                vblank = true;
        }

        m_pAudio->EndFrame(pSampleBuffer, pSampleCount);
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
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();
        m_pProcessor->Disassemble(m_pProcessor->GetState()->PC->GetValue());

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header. File: %s...", szFilePath);
        }

        return romTypeOK;
    }
    else
        return false;
}

bool GearsystemCore::LoadROMFromBuffer(const u8* buffer, int size, Cartridge::ForceConfiguration* config)
{
    if (m_pCartridge->LoadFromBuffer(buffer, size))
    {
        if (IsValidPointer(config))
            m_pCartridge->ForceConfig(*config);
        Reset();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();

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

        Log("Memory Dump Saved");
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

        ofstream myfile(path, ios::out | ios::trunc);

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

        Log("Disassembled ROM Saved");
    }
}

bool GearsystemCore::GetRuntimeInfo(GS_RuntimeInfo& runtime_info)
{
    if (m_pCartridge->IsReady())
    {
        runtime_info.screen_width = m_pCartridge->IsGameGear() ? GS_RESOLUTION_GG_WIDTH : GS_RESOLUTION_SMS_WIDTH;
        runtime_info.screen_height = m_pCartridge->IsGameGear() ? GS_RESOLUTION_GG_HEIGHT : (m_pVideo->IsExtendedMode224() ? GS_RESOLUTION_SMS_HEIGHT_EXTENDED : GS_RESOLUTION_SMS_HEIGHT);
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


void GearsystemCore::SetSG1000Palette(GS_Color* pSG1000Palette)
{
    m_pVideo->SetSG1000Palette(pSG1000Palette);
}

void GearsystemCore::KeyPressed(GS_Joypads joypad, GS_Keys key)
{
    m_pInput->KeyPressed(joypad, key);
}

void GearsystemCore::KeyReleased(GS_Joypads joypad, GS_Keys key)
{
    m_pInput->KeyReleased(joypad, key);
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
        m_pProcessor->Disassemble(m_pProcessor->GetState()->PC->GetValue());
    }
}

void GearsystemCore::ResetROMPreservingRAM(Cartridge::ForceConfiguration* config)
{
    if (m_pCartridge->IsReady())
    {
        if (m_pMemory->GetCurrentRule()->PersistedRAM())
        {
            Log("Resetting preserving RAM...");

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

void GearsystemCore::SetSoundSampleRate(int rate)
{
    Log("Gearsystem sound sample rate: %d", rate);
    m_pAudio->SetSampleRate(rate);
}

void GearsystemCore::SetSoundVolume(float volume)
{
    m_pAudio->SetVolume(volume);
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

        ofstream file(path.c_str(), ios::out | ios::binary);

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Log("RAM saved");
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

        file.open(sav_path.c_str(), ios::in | ios::binary);

        // check for old .gearsystem saves
        if (file.fail())
        {
            Log("Save file doesn't exist");
            string old_sav_file = rom_path + ".gearsystem";

            Log("Opening old save file: %s", old_sav_file.c_str());
            file.open(old_sav_file.c_str(), ios::in | ios::binary);
        }

        if (!file.fail())
        {
            file.seekg(0, file.end);
            s32 fileSize = (s32)file.tellg();
            file.seekg(0, file.beg);

            if (m_pMemory->GetCurrentRule()->LoadRam(file, fileSize))
            {
                Log("RAM loaded");
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
    Log("Creating save state %d...", index);

    SaveState(NULL, index);

    Log("Save state %d created", index);
}

void GearsystemCore::SaveState(const char* szPath, int index)
{
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

    ofstream file(sstm.str().c_str(), ios::out | ios::binary);

    SaveState(file, size);

    SafeDeleteArray(buffer);

    file.close();

    Log("Save state created");
}

bool GearsystemCore::SaveState(u8* buffer, size_t& size)
{
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
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Gathering save state data...");

        using namespace std;

        m_pMemory->SaveState(stream);
        m_pProcessor->SaveState(stream);
        m_pAudio->SaveState(stream);
        m_pVideo->SaveState(stream);
        m_pInput->SaveState(stream);
        m_pMemory->GetCurrentRule()->SaveState(stream);
        m_pProcessor->GetIOPOrts()->SaveState(stream);

        size = static_cast<size_t>(stream.tellp());
        size += (sizeof(u32) * 2);

        u32 header_magic = GS_SAVESTATE_MAGIC;
        u32 header_size = static_cast<u32>(size);

        stream.write(reinterpret_cast<const char*> (&header_magic), sizeof(header_magic));
        stream.write(reinterpret_cast<const char*> (&header_size), sizeof(header_size));

        Log("Save state size: %d", static_cast<size_t>(stream.tellp()));

        return true;
    }

    Log("Invalid rom or memory rule.");

    return false;
}

void GearsystemCore::LoadState(int index)
{
    Log("Loading save state %d...", index);

    LoadState(NULL, index);

    Log("State %d file loaded", index);
}

void GearsystemCore::LoadState(const char* szPath, int index)
{
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

    file.open(sstm.str().c_str(), ios::in | ios::binary);

    if (!file.fail())
    {
        if (LoadState(file))
        {
            Log("Save state loaded");
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
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()) && (size > 0) && IsValidPointer(buffer))
    {
        Log("Gathering load state data [%d bytes]...", size);

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
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        using namespace std;

        u32 header_magic = 0;
        u32 header_size = 0;

        stream.seekg(0, ios::end);
        size_t size = static_cast<size_t>(stream.tellg());
        stream.seekg(0, ios::beg);

        Log("Load state stream size: %d", size);

        stream.seekg(size - (2 * sizeof(u32)), ios::beg);
        stream.read(reinterpret_cast<char*> (&header_magic), sizeof(header_magic));
        stream.read(reinterpret_cast<char*> (&header_size), sizeof(header_size));
        stream.seekg(0, ios::beg);

        Log("Load state magic: 0x%08x", header_magic);
        Log("Load state size: %d", header_size);

        if ((header_size == size) && (header_magic == GS_SAVESTATE_MAGIC))
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
    m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
}

void GearsystemCore::SetRamModificationCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

void GearsystemCore::InitMemoryRules()
{
    m_pSG1000MemoryRule = new SG1000MemoryRule(m_pMemory, m_pCartridge);
    m_pCodemastersMemoryRule = new CodemastersMemoryRule(m_pMemory, m_pCartridge);
    m_pSegaMemoryRule = new SegaMemoryRule(m_pMemory, m_pCartridge);
    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pMemory, m_pCartridge);
    m_pKoreanMemoryRule = new KoreanMemoryRule(m_pMemory, m_pCartridge);
    m_pMSXMemoryRule = new MSXMemoryRule(m_pMemory, m_pCartridge);

    m_pMemory->SetCurrentRule(m_pRomOnlyMemoryRule);
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
        case Cartridge::CartridgeMSXMapper:
            m_pMemory->SetCurrentRule(m_pMSXMemoryRule);
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
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pAudio->Reset(m_pCartridge->IsPAL());
    m_pVideo->Reset(m_pCartridge->IsGameGear(), m_pCartridge->IsPAL());
    m_pInput->Reset(m_pCartridge->IsGameGear());
    m_pSegaMemoryRule->Reset();
    m_pCodemastersMemoryRule->Reset();
    m_pSG1000MemoryRule->Reset();
    m_pRomOnlyMemoryRule->Reset();
    m_pGameGearIOPorts->Reset();
    m_pSmsIOPorts->Reset();
    m_bPaused = false;
}

void GearsystemCore::Get16BitFrameBuffer(GS_Color* pFrameBuffer, u16* p16BitFrameBuffer)
{
    if (IsValidPointer(pFrameBuffer) && IsValidPointer(p16BitFrameBuffer))
    {
        int pixels = GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT;
        

        for (int i = 0; i < pixels; i++)
        {
            GS_Color p = pFrameBuffer[i];

            p16BitFrameBuffer[i] = 0;
            p16BitFrameBuffer[i] = ((p.red >> 3) << 11) | ((p.green >> 2) << 5) | (p.blue >> 3);
        }
    }
}
