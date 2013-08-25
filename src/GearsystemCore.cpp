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
#include "Video.h"
#include "Input.h"
#include "Cartridge.h"
#include "MemoryRule.h"
#include "SegaMemoryRule.h"

GearsystemCore::GearsystemCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pInput);
    InitPointer(m_pCartridge);
    InitPointer(m_pSegaMemoryRule);
    m_bPaused = true;
}

GearsystemCore::~GearsystemCore()
{
#ifdef DEBUG_GEARSYSTEM
    if (m_pCartridge->IsLoadedROM())
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

    SafeDelete(m_pSegaMemoryRule);
    SafeDelete(m_pCartridge);
    SafeDelete(m_pInput);
    SafeDelete(m_pVideo);
    SafeDelete(m_pProcessor);
    SafeDelete(m_pMemory);
}

void GearsystemCore::Init()
{
    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pVideo = new Video(m_pMemory, m_pProcessor);
    m_pInput = new Input(m_pMemory, m_pProcessor);
    m_pCartridge = new Cartridge();

    m_pMemory->Init();
    m_pProcessor->Init();
    m_pVideo->Init();
    m_pInput->Init();
    m_pCartridge->Init();

    InitMemoryRules();
}

void GearsystemCore::RunToVBlank(GS_Color* pFrameBuffer)
{
    if (!m_bPaused && m_pCartridge->IsLoadedROM())
    {
        bool vblank = false;
        while (!vblank)
        {
            unsigned int clockCycles = m_pProcessor->Tick();
            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
            m_pInput->Tick(clockCycles);
        }
    }
}

bool GearsystemCore::LoadROM(const char* szFilePath)
{
#ifdef DEBUG_GEARSYSTEM
    if (m_pCartridge->IsLoadedROM())
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
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetTheROM());
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

void GearsystemCore::KeyPressed(GS_Keys key)
{
    m_pInput->KeyPressed(key);
}

void GearsystemCore::KeyReleased(GS_Keys key)
{
    m_pInput->KeyReleased(key);
}

void GearsystemCore::Pause(bool paused)
{
    m_bPaused = paused;
}

bool GearsystemCore::IsPaused()
{
    return m_bPaused;
}

void GearsystemCore::ResetROM()
{
    if (m_pCartridge->IsLoadedROM())
    {
        Reset();
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetTheROM());
        AddMemoryRules();
    }
}

void GearsystemCore::EnableSound(bool enabled)
{
    //m_pAudio->Enable(enabled);
}

void GearsystemCore::SetSoundSampleRate(int rate)
{
    //m_pAudio->SetSampleRate(rate);
}

void GearsystemCore::SaveRam()
{
    SaveRam(NULL);
}

void GearsystemCore::SaveRam(const char* szPath)
{/*
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
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

        char signature[16] = SAVE_FILE_SIGNATURE;
        u8 version = SAVE_FILE_VERSION;
        u8 romType = m_pCartridge->GetType();
        u8 romSize = m_pCartridge->GetROMSize();
        u8 ramSize = m_pCartridge->GetRAMSize();
        u8 ramBanksSize = m_pMemory->GetCurrentRule()->GetRamBanksSize();
        u8 ramBanksStart = 39;
        u8 saveStateSize = 0;
        u8 saveStateStart = 0;

        ofstream file(path, ios::out | ios::binary);

        file.write(signature, 16);
        file.write(reinterpret_cast<const char*> (&version), 1);
        file.write(m_pCartridge->GetName(), 16);
        file.write(reinterpret_cast<const char*> (&romType), 1);
        file.write(reinterpret_cast<const char*> (&romSize), 1);
        file.write(reinterpret_cast<const char*> (&ramSize), 1);
        file.write(reinterpret_cast<const char*> (&ramBanksSize), 1);
        file.write(reinterpret_cast<const char*> (&ramBanksStart), 1);
        file.write(reinterpret_cast<const char*> (&saveStateSize), 1);
        file.write(reinterpret_cast<const char*> (&saveStateStart), 1);

        Log("Header saved");

        m_pMemory->GetCurrentRule()->SaveRam(file);

        Log("RAM saved");
    }*/
}

void GearsystemCore::LoadRam()
{
    LoadRam(NULL);
}

void GearsystemCore::LoadRam(const char* szPath)
{/*
    if (m_pCartridge->IsLoadedROM() && m_pCartridge->HasBattery() && IsValidPointer(m_pMemory->GetCurrentRule()))
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

        Log("Save file: %s", path);

        ifstream file(path, ios::in | ios::binary);

        if (!file.fail())
        {
            char signature[16];
            u8 version;
            char romName[16];
            u8 romType;
            u8 romSize;
            u8 ramSize;
            u8 ramBanksSize;
            u8 ramBanksStart;
            u8 saveStateSize;
            u8 saveStateStart;

            file.read(signature, 16);
            file.read(reinterpret_cast<char*> (&version), 1);
            file.read(romName, 16);
            file.read(reinterpret_cast<char*> (&romType), 1);
            file.read(reinterpret_cast<char*> (&romSize), 1);
            file.read(reinterpret_cast<char*> (&ramSize), 1);
            file.read(reinterpret_cast<char*> (&ramBanksSize), 1);
            file.read(reinterpret_cast<char*> (&ramBanksStart), 1);
            file.read(reinterpret_cast<char*> (&saveStateSize), 1);
            file.read(reinterpret_cast<char*> (&saveStateStart), 1);

            Log("Header loaded");

            if ((strcmp(signature, SAVE_FILE_SIGNATURE) == 0) && (strcmp(romName, m_pCartridge->GetName()) == 0) &&
                    (version == SAVE_FILE_VERSION) && (romType == m_pCartridge->GetType()) &&
                    (romSize == m_pCartridge->GetROMSize()) && (ramSize == m_pCartridge->GetRAMSize()))
            {
                m_pMemory->GetCurrentRule()->LoadRam(file);

                Log("RAM loaded");
            }
            else
            {
                Log("Integrity check failed loading save file");
            }
        }
        else
        {
            Log("Save file doesn't exist");
        }
    }*/
}

void GearsystemCore::InitMemoryRules()
{
    m_pSegaMemoryRule = new SegaMemoryRule(m_pMemory, m_pCartridge);
}

bool GearsystemCore::AddMemoryRules()
{
    Cartridge::CartridgeTypes type = m_pCartridge->GetType();

    bool notSupported = false;

    switch (type)
    {
        case Cartridge::CartridgeSegaMapper:
            m_pMemory->SetCurrentRule(m_pSegaMemoryRule);
            break;
        case Cartridge::CartridgeNotSupported:
            notSupported = true;
            break;
        default:
            notSupported = true;
    }

    return !notSupported;
}

void GearsystemCore::Reset()
{
    m_pMemory->Reset();
    m_pProcessor->Reset();
    m_pVideo->Reset();
    m_pInput->Reset();
    
    m_pSegaMemoryRule->Reset();

    m_bPaused = false;
}

