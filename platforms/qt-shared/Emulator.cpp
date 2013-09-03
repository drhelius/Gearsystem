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

#include "Emulator.h"

Emulator::Emulator()
{
    InitPointer(m_pGearsystemCore);
}

Emulator::~Emulator()
{
    SafeDelete(m_pGearsystemCore);
}

void Emulator::Init()
{
    m_pGearsystemCore = new GearsystemCore();
    m_pGearsystemCore->Init();
}

void Emulator::LoadRom(const char* szFilePath, bool forceDMG)
{
    m_Mutex.lock();
    m_pGearsystemCore->SaveRam();
    m_pGearsystemCore->LoadROM(szFilePath);
    m_pGearsystemCore->LoadRam();
    m_Mutex.unlock();
}

void Emulator::RunToVBlank(GS_Color* pFrameBuffer)
{
    m_Mutex.lock();
    m_pGearsystemCore->RunToVBlank(pFrameBuffer);
    m_Mutex.unlock();
}

void Emulator::KeyPressed(GS_Joypads joypad, GS_Keys key)
{
    m_Mutex.lock();
    m_pGearsystemCore->KeyPressed(joypad, key);
    m_Mutex.unlock();
}

void Emulator::KeyReleased(GS_Joypads joypad, GS_Keys key)
{
    m_Mutex.lock();
    m_pGearsystemCore->KeyReleased(joypad, key);
    m_Mutex.unlock();
}

void Emulator::Pause()
{
    m_Mutex.lock();
    m_pGearsystemCore->Pause(true);
    m_Mutex.unlock();
}

void Emulator::Resume()
{
    m_Mutex.lock();
    m_pGearsystemCore->Pause(false);
    m_Mutex.unlock();
}

bool Emulator::IsPaused()
{
    m_Mutex.lock();
    bool paused = m_pGearsystemCore->IsPaused();
    m_Mutex.unlock();
    return paused;
}

void Emulator::Reset(bool forceDMG)
{
    m_Mutex.lock();
    m_pGearsystemCore->SaveRam();
    m_pGearsystemCore->ResetROM();
    m_pGearsystemCore->LoadRam();
    m_Mutex.unlock();
}

void Emulator::MemoryDump()
{
    m_Mutex.lock();
    m_pGearsystemCore->GetMemory()->MemoryDump("memdump.txt");
    m_Mutex.unlock();
}

void Emulator::SetSoundSettings(bool enabled, int rate)
{
    m_Mutex.lock();
    m_pGearsystemCore->EnableSound(enabled);
    m_pGearsystemCore->SetSoundSampleRate(rate);
    m_Mutex.unlock();
}

void Emulator::SaveRam()
{
    m_Mutex.lock();
    m_pGearsystemCore->SaveRam();
    m_Mutex.unlock();
}
