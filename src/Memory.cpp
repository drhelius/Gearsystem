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

    if (IsValidPointer(m_pDisassembledROMMap))
    {
        for (int i = 0; i < MAX_ROM_SIZE; i++)
        {
            SafeDelete(m_pDisassembledROMMap[i]);
        }
        SafeDeleteArray(m_pDisassembledROMMap);
    }

    if (IsValidPointer(m_pDisassembledMap))
    {
        for (int i = 0; i < 0x10000; i++)
        {
            SafeDelete(m_pDisassembledMap[i]);
        }
        SafeDeleteArray(m_pDisassembledMap);
    }
}

void Memory::Init()
{
    m_pMap = new u8[0x10000];
#ifndef GEARSYSTEM_DISABLE_DISASSEMBLER
    m_pDisassembledMap = new stDisassembleRecord*[0x10000];
    for (int i = 0; i < 0x10000; i++)
    {
        InitPointer(m_pDisassembledMap[i]);
    }

    m_pDisassembledROMMap = new stDisassembleRecord*[MAX_ROM_SIZE];
    for (int i = 0; i < MAX_ROM_SIZE; i++)
    {
        InitPointer(m_pDisassembledROMMap[i]);
    }
#endif
    m_Breakpoints.clear();
    InitPointer(m_pRunToBreakpoint);
    Reset();
}

void Memory::Reset()
{
    for (int i = 0; i < 0x10000; i++)
    {
        m_pMap[i] = 0x00;
        if (IsValidPointer(m_pDisassembledMap))
        {
            SafeDelete(m_pDisassembledMap[i]);
        }
    }

    if (IsValidPointer(m_pDisassembledROMMap))
    {
        for (int i = 0; i < MAX_ROM_SIZE; i++)
        {
            SafeDelete(m_pDisassembledROMMap[i]);
        }
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
    if (!IsValidPointer(m_pDisassembledMap))
        return;

    using namespace std;

    ofstream myfile(szFilePath, ios::out | ios::trunc);

    if (myfile.is_open())
    {
        for (int i = 0; i < 0x10000; i++)
        {
            if (IsValidPointer(m_pDisassembledMap[i]) && (m_pDisassembledMap[i]->name[0] != 0))
            {
                myfile << "0x" << hex << i << "\t " << m_pDisassembledMap[i]->name << "\n";
                i += (m_pDisassembledMap[i]->size - 1);
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

