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
#include "RomOnlyMemoryRule.h"
#include "SmsIOPorts.h"

GearsystemCore::GearsystemCore()
{
    InitPointer(m_pMemory);
    InitPointer(m_pProcessor);
    InitPointer(m_pAudio);
    InitPointer(m_pVideo);
    InitPointer(m_pInput);
    InitPointer(m_pCartridge);
    InitPointer(m_pSegaMemoryRule);
    InitPointer(m_pRomOnlyMemoryRule);
    InitPointer(m_pSmsIOPorts);
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

    SafeDelete(m_pSmsIOPorts);
    SafeDelete(m_pRomOnlyMemoryRule);
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
    m_pMemory = new Memory();
    m_pProcessor = new Processor(m_pMemory);
    m_pAudio = new Audio();
    m_pVideo = new Video(m_pMemory, m_pProcessor);
    m_pInput = new Input(m_pMemory, m_pProcessor);
    m_pCartridge = new Cartridge();
    m_pSmsIOPorts = new SmsIOPorts(m_pAudio, m_pVideo, m_pInput);
    m_pProcessor->SetIOPOrts(m_pSmsIOPorts);

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
    if (!m_bPaused && m_pCartridge->IsLoadedROM())
    {
        bool vblank = false;
        while (!vblank)
        {
            unsigned int clockCycles = m_pProcessor->Tick();
            vblank = m_pVideo->Tick(clockCycles, pFrameBuffer);
            m_pAudio->Tick(clockCycles);
            //m_pInput->Tick(clockCycles);
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
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetTheROM(), m_pCartridge->GetROMSize());
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
        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetTheROM(), m_pCartridge->GetROMSize());
        AddMemoryRules();
    }
}

void GearsystemCore::EnableSound(bool enabled)
{
    m_pAudio->Enable(enabled);
}

void GearsystemCore::SetSoundSampleRate(int rate)
{
    m_pAudio->SetSampleRate(rate);
}

void GearsystemCore::SaveRam()
{
    SaveRam(NULL);
}

void GearsystemCore::SaveRam(const char* szPath)
{
}

void GearsystemCore::LoadRam()
{
    LoadRam(NULL);
}

void GearsystemCore::LoadRam(const char* szPath)
{
}

void GearsystemCore::InitMemoryRules()
{
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
    m_pAudio->Reset();
    m_pVideo->Reset();
    m_pInput->Reset();
    m_pSegaMemoryRule->Reset();
    m_pRomOnlyMemoryRule->Reset();
    m_bPaused = false;
}

