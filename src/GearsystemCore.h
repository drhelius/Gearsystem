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

#ifndef CORE_H
#define	CORE_H

#include "definitions.h"

class Memory;
class Processor;
class Audio;
class Video;
class Input;
class Cartridge;
class SegaMemoryRule;
class RomOnlyMemoryRule;
class MemoryRule;
class SmsIOPorts;

class GearsystemCore
{
public:
    GearsystemCore();
    ~GearsystemCore();
    void Init();
    void RunToVBlank(GS_Color* pFrameBuffer);
    bool LoadROM(const char* szFilePath);
    Memory* GetMemory();
    Cartridge* GetCartridge();
    void KeyPressed(GS_Joypads joypad, GS_Keys key);
    void KeyReleased(GS_Joypads joypad, GS_Keys key);
    void Pause(bool paused);
    bool IsPaused();
    void ResetROM();
    void EnableSound(bool enabled);
    void SetSoundSampleRate(int rate);
    void SaveRam();
    void SaveRam(const char* szPath);
    void LoadRam();
    void LoadRam(const char* szPath);

private:
    void InitMemoryRules();
    bool AddMemoryRules();
    void Reset();

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    SegaMemoryRule* m_pSegaMemoryRule;
    RomOnlyMemoryRule* m_pRomOnlyMemoryRule;
    SmsIOPorts* m_pSmsIOPorts;
    bool m_bPaused;
};

#endif	/* CORE_H */

