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

#ifndef MEMORY_H
#define	MEMORY_H

#include "definitions.h"
#include <vector>

class Processor;
class Video;

class Memory
{
public:
    Memory();
    ~Memory();
    void SetProcessor(Processor* pProcessor);
    void SetVideo(Video* pVideo);
    void Init();
    void Reset();
    u8 Read(u16 address);
    void Write(u16 address, u8 value);
    u8 Retrieve(u16 address);
    void Load(u16 address, u8 value);
    void Disassemble(u16 address, const char* szDisassembled);
    bool IsDisassembled(u16 address);
    void LoadBank0and1FromROM(u8* pTheROM);
    void MemoryDump(const char* szFilePath);

private:

    struct stDisassemble
    {
        char szDisString[32];
    };

private:
    Processor* m_pProcessor;
    Video* m_pVideo;
    u8* m_pMap;
    stDisassemble* m_pDisassembledMap;
};

#include "Memory_inline.h"

#endif	/* MEMORY_H */

