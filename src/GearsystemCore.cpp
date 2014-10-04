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
    InitPointer(m_pRomOnlyMemoryRule);
    InitPointer(m_pSmsIOPorts);
    InitPointer(m_pGameGearIOPorts);
    m_bPaused = true;
}

GearsystemCore::~GearsystemCore()
{
#ifdef DEBUG_GEARSYSTEM
    if (m_pCartridge->IsReady())
    {
        Log("Saving Memory Dump...");

        using namespace std;

        char path[512];

        strcpy(path, m_pCartridge->GetFilePath());
        strcat(path, ".dump");

        m_pMemory->MemoryDump(path);

        Log("Memory Dump Saved");
    }
#endif

    SafeDelete(m_pGameGearIOPorts);
    SafeDelete(m_pSmsIOPorts);
    SafeDelete(m_pRomOnlyMemoryRule);
    SafeDelete(m_pCodemastersMemoryRule);
    SafeDelete(m_pSegaMemoryRule);
    SafeDelete(m_pCartridge);
    SafeDelete(m_pInput);
    SafeDelete(m_pVideo);
    SafeDelete(m_pAudio);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearsystemCore::Init()
{
    Log("-=:: GEARSYSTEM %1.1f ::=-", GEARSYSTEM_VERSION);

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

void GearsystemCore::RunToVBlank(GS_Color* pFrameBuffer)
{
    if (!m_bPaused && m_pCartridge->IsReady())
    {
        bool vblank = false;
        while (!vblank)
        {
            unsigned int clockCycles = m_pProcessor->Tick();
            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
            m_pAudio->Tick(clockCycles);
            m_pInput->Tick(clockCycles);
        }
        m_pAudio->EndFrame();
    }
}

bool GearsystemCore::LoadROM(const char* szFilePath)
{
#ifdef DEBUG_GEARSYSTEM
    if (m_pCartridge->IsReady())
    {
        Log("Saving Memory Dump...");

        using namespace std;

        char path[512];

        strcpy(path, m_pCartridge->GetFilePath());
        strcat(path, ".dump");

        m_pMemory->MemoryDump(path);

        Log("Memory Dump Saved");
    }
#endif

    bool loaded = m_pCartridge->LoadFromFile(szFilePath);
    if (loaded)
    {
        Reset();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        bool romTypeOK = AddMemoryRules();

        if (!romTypeOK)
        {
            Log("There was a problem with the cartridge header. File: %s...", szFilePath);
        }

        return romTypeOK;
    }
    else
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

void GearsystemCore::ResetROM()
{
    if (m_pCartridge->IsReady())
    {
        Log("Gearsystem RESET");
        Reset();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), m_pCartridge->GetROMSize());
        AddMemoryRules();
    }
}

void GearsystemCore::EnableSound(bool enabled)
{
    if (enabled)
    {
        Log("Gearsystem sound ENABLED");
    }
    else
    {
        Log("Gearsystem sound DISABLED");
    }
    m_pAudio->Enable(enabled);
}

void GearsystemCore::ResetSound()
{
    m_pAudio->Reset();
}

void GearsystemCore::SetSoundSampleRate(int rate)
{
    Log("Gearsystem sound sample rate: %d", rate);
    m_pAudio->SetSampleRate(rate);
}

void GearsystemCore::SaveRam()
{
    SaveRam(NULL);
}

void GearsystemCore::SaveRam(const char* szPath)
{
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()) && m_pMemory->GetCurrentRule()->PersistedRAM())
    {
        Log("Saving RAM...");

        using namespace std;

        char path[512];

        if (IsValidPointer(szPath))
        {
            strcpy(path, szPath);
            strcat(path, "/");
            strcat(path, m_pCartridge->GetFileName());
        }
        else
        {
            strcpy(path, m_pCartridge->GetFilePath());
        }

        strcat(path, ".gearsystem");

        Log("Save file: %s", path);

        ofstream file(path, ios::out | ios::binary);

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Log("RAM saved");
    }
}

void GearsystemCore::LoadRam()
{
    LoadRam(NULL);
}

void GearsystemCore::LoadRam(const char* szPath)
{
    if (m_pCartridge->IsReady() && IsValidPointer(m_pMemory->GetCurrentRule()))
    {
        Log("Loading RAM...");

        using namespace std;

        char path[512];

        if (IsValidPointer(szPath))
        {
            strcpy(path, szPath);
            strcat(path, "/");
            strcat(path, m_pCartridge->GetFileName());
        }
        else
        {
            strcpy(path, m_pCartridge->GetFilePath());
        }

        strcat(path, ".gearsystem");

        Log("Opening save file: %s", path);

        ifstream file(path, ios::in | ios::binary);

        if (!file.fail())
        {
            char signature[16];

            file.read(signature, 16);

            file.seekg(0, file.end);
            s32 fileSize = (s32) file.tellg();
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

float GearsystemCore::GetVersion()
{
    return GEARSYSTEM_VERSION;
}

void GearsystemCore::InitMemoryRules()
{
    m_pCodemastersMemoryRule = new CodemastersMemoryRule(m_pMemory, m_pCartridge);
    m_pSegaMemoryRule = new SegaMemoryRule(m_pMemory, m_pCartridge);
    m_pRomOnlyMemoryRule = new RomOnlyMemoryRule(m_pMemory, m_pCartridge);
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
        case Cartridge::CartridgeNotSupported:
            notSupported = true;
            break;
        default:
            notSupported = true;
    }

    if (m_pCartridge->IsGameGear())
    {
        Log("Game Gear Mode enabled");
        m_pProcessor->SetIOPOrts(m_pGameGearIOPorts);
    }
    else
    {
        Log("Master System Mode enabled");
        m_pProcessor->SetIOPOrts(m_pSmsIOPorts);
    }

    return !notSupported;
}

void GearsystemCore::Reset()
{
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pAudio->Reset();
    m_pVideo->Reset(m_pCartridge->IsGameGear(), m_pCartridge->IsPAL());
    m_pInput->Reset(m_pCartridge->IsGameGear());
    m_pSegaMemoryRule->Reset();
    m_pCodemastersMemoryRule->Reset();
    m_pRomOnlyMemoryRule->Reset();
    m_pGameGearIOPorts->Reset();
    m_pSmsIOPorts->Reset();
    m_bPaused = false;
}

