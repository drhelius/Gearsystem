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
}

Memory::~Memory()
{
    SafeDeleteArray(m_pMap);
    InitPointer(m_pCurrentMemoryRule);
    SafeDeleteArray(m_pDisassembledMap);
}

void Memory::Init()
{
    m_pMap = new u8[0x10000];
    m_pDisassembledMap = new stDisassemble[0x10000];
    Reset();
}

void Memory::Reset()
{
    for (int i = 0; i < 0x10000; i++)
    {
        m_pMap[i] = 0x00;
        m_pDisassembledMap[i].szDisString[0] = 0;
    }
}

void Memory::SetCurrentRule(MemoryRule* pRule)
{
    m_pCurrentMemoryRule = pRule;
}

void Memory::Disassemble(u16 address, const char* szDisassembled)
{
    strcpy(m_pDisassembledMap[address].szDisString, szDisassembled);
}

bool Memory::IsDisassembled(u16 address)
{
    return m_pDisassembledMap[address].szDisString[0] != 0;
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
            if (IsDisassembled(i))
            {
                myfile << "$" << uppercase << hex << setw(4) << setfill('0') << i << "\t " << nouppercase << m_pDisassembledMap[i].szDisString << endl;
            }
            else
            {
                myfile << "$" << uppercase << hex << setw(4) << setfill('0') << i << "\t [" << hex << setw(2) << setfill('0') << (int) m_pMap[i] << "]" << endl;
            }
        }

        myfile.close();
    }
}
