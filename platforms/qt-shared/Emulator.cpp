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

#include <QStandardPaths>
#include "Emulator.h"

Emulator::Emulator()
{
    InitPointer(m_pGearsystemCore);
    InitPointer(m_pSoundQueue);
    m_bAudioEnabled = true;
    m_bSaveInROMFolder = false;
}

Emulator::~Emulator()
{
    SaveRam();
    SafeDelete(m_pSoundQueue);
    SafeDelete(m_pGearsystemCore);
}

void Emulator::Init()
{
    m_pGearsystemCore = new GearsystemCore();
    m_pGearsystemCore->Init();

    m_pSoundQueue = new Sound_Queue();
    m_pSoundQueue->start(44100, 2);

    m_Runtime_info.screen_width = GS_RESOLUTION_MAX_WIDTH;
    m_Runtime_info.screen_height = GS_RESOLUTION_MAX_HEIGHT;
}

void Emulator::LoadRom(const char* szFilePath, bool saveInROMFolder)
{
    m_Mutex.lock();
    m_bSaveInROMFolder = saveInROMFolder;
    SaveRam();
    m_pGearsystemCore->LoadROM(szFilePath);
    LoadRam();
    m_pGearsystemCore->GetRuntimeInfo(m_Runtime_info);
    m_Mutex.unlock();
}

void Emulator::RunToVBlank(GS_Color* pFrameBuffer)
{
    m_Mutex.lock();

    s16 sampleBufer[GS_AUDIO_BUFFER_SIZE];
    int sampleCount = 0;

    m_pGearsystemCore->RunToVBlank(pFrameBuffer, sampleBufer, &sampleCount);

    if (m_bAudioEnabled && (sampleCount > 0))
    {
        m_pSoundQueue->write(sampleBufer, sampleCount);
    }

    m_pGearsystemCore->GetRuntimeInfo(m_Runtime_info);

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
    m_bAudioEnabled = false;
    m_Mutex.unlock();
}

void Emulator::Resume()
{
    m_Mutex.lock();
    m_pGearsystemCore->Pause(false);
    m_bAudioEnabled = true;
    m_Mutex.unlock();
}

bool Emulator::IsPaused()
{
    m_Mutex.lock();
    bool paused = m_pGearsystemCore->IsPaused();
    m_Mutex.unlock();
    return paused;
}

bool Emulator::IsAudioEnabled()
{
    return m_bAudioEnabled;
}

void Emulator::Reset(bool saveInROMFolder)
{
    m_Mutex.lock();
    m_bSaveInROMFolder = saveInROMFolder;
    SaveRam();
    m_pGearsystemCore->ResetROM();
    LoadRam();
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
    m_bAudioEnabled = enabled;
    m_pGearsystemCore->SetSoundSampleRate(rate);
    m_pSoundQueue->stop();
    m_pSoundQueue->start(rate, 2);
    m_Mutex.unlock();
}

void Emulator::SaveRam()
{
    if (m_bSaveInROMFolder)
        m_pGearsystemCore->SaveRam();
    else
        m_pGearsystemCore->SaveRam(QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdString().c_str());
}

void Emulator::LoadRam()
{
    if (m_bSaveInROMFolder)
        m_pGearsystemCore->LoadRam();
    else
        m_pGearsystemCore->LoadRam(QStandardPaths::writableLocation(QStandardPaths::DataLocation).toStdString().c_str());
}

void Emulator::SaveState(int index)
{
    m_Mutex.lock();
    m_pGearsystemCore->SaveState(index);
    m_Mutex.unlock();
}

void Emulator::LoadState(int index)
{
    m_Mutex.lock();
    m_pGearsystemCore->LoadState(index);
    m_Mutex.unlock();
}

void Emulator::GetRuntimeInfo(GS_RuntimeInfo& runtime_info)
{
    runtime_info.region = m_Runtime_info.region;
    runtime_info.screen_width = m_Runtime_info.screen_width;
    runtime_info.screen_height = m_Runtime_info.screen_height;
}
