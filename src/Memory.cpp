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

#include <iostream>
#include <iomanip>
#include <fstream>
#include "Memory.h"

Memory::Memory()
{
    InitPointer(m_pMap);
    InitPointer(m_pCurrentMemoryRule);
    InitPointer(m_pDisassembledMap);
    InitPointer(m_pDisassembledROMMap);
    InitPointer(m_pRunToBreakpoint);
}

Memory::~Memory()
{
    SafeDeleteArray(m_pMap);
    InitPointer(m_pCurrentMemoryRule);
    SafeDeleteArray(m_pDisassembledMap);
    SafeDeleteArray(m_pDisassembledROMMap);
}

void Memory::Init()
{
    m_pMap = new u8[0x10000];
    m_pDisassembledMap = new stDisassembleRecord[0x10000];
    m_pDisassembledROMMap = new stDisassembleRecord[MAX_ROM_SIZE];
    m_Breakpoints.clear();
    InitPointer(m_pRunToBreakpoint);
    Reset();
}

void Memory::Reset()
{
    for (int i = 0; i < 0x10000; i++)
    {
        m_pMap[i] = 0x00;
        m_pDisassembledMap[i].address = i;
        m_pDisassembledMap[i].bank = 0;
        m_pDisassembledMap[i].name[0] = 0;
        m_pDisassembledMap[i].bytes[0] = 0;
        m_pDisassembledMap[i].size = 0;
    }

    for (int i = 0; i < MAX_ROM_SIZE; i++)
    {
        m_pDisassembledROMMap[i].address = i & 0x3FFF;
        m_pDisassembledROMMap[i].bank = i >> 14;
        m_pDisassembledROMMap[i].name[0] = 0;
        m_pDisassembledROMMap[i].bytes[0] = 0;
        m_pDisassembledROMMap[i].size = 0;
    }
}

void Memory::SetCurrentRule(MemoryRule* pRule)
{
    m_pCurrentMemoryRule = pRule;
}

MemoryRule* Memory::GetCurrentRule()
{
    return m_pCurrentMemoryRule;
}

u8* Memory::GetMemoryMap()
{
    return m_pMap;
}

void Memory::LoadSlotsFromROM(u8* pTheROM, int size)
{
    // loads the first 48KB only (bank 0, 1 and 2)
    int i;
    for (i = 0; ((i < 0xC000) && (i < size)); i++)
    {
        m_pMap[i] = pTheROM[i];
    }
    Log("%d bytes copied from cartridge", i);
}

void Memory::MemoryDump(const char* szFilePath)
{
    using namespace std;

    ofstream myfile(szFilePath, ios::out | ios::trunc);

    if (myfile.is_open())
    {
        for (int i = 0; i < 0x10000; i++)
        {
            if (m_pDisassembledMap[i].name[0] != 0)
            {
                myfile << "0x" << hex << i << "\t " << m_pDisassembledMap[i].name << "\n";
                i += (m_pDisassembledMap[i].size - 1);
            }
            else
            {
                myfile << "0x" << hex << i << "\t [0x" << hex << (int) m_pMap[i] << "]\n";
            }
        }

        myfile.close();
    }
}

void Memory::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_pMap), 0x10000);
}

void Memory::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pMap), 0x10000);
}

std::vector<Memory::stDisassembleRecord*>* Memory::GetBreakpoints()
{
    return &m_Breakpoints;
}

Memory::stDisassembleRecord* Memory::GetRunToBreakpoint()
{
    return m_pRunToBreakpoint;
}

void Memory::SetRunToBreakpoint(Memory::stDisassembleRecord* pBreakpoint)
{
    m_pRunToBreakpoint = pBreakpoint;
}

