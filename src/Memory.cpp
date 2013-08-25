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
#include "Processor.h"
#include "Video.h"

Memory::Memory()
{
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    InitPointer(m_pMap);
    InitPointer(m_pDisassembledMap);
}

Memory::~Memory()
{
    InitPointer(m_pProcessor);
    InitPointer(m_pVideo);
    SafeDeleteArray(m_pMap);
    SafeDeleteArray(m_pDisassembledMap);
}

void Memory::SetProcessor(Processor* pProcessor)
{
    m_pProcessor = pProcessor;
}

void Memory::SetVideo(Video* pVideo)
{
    m_pVideo = pVideo;
}

void Memory::Init()
{
    m_pMap = new u8[65536];
    m_pDisassembledMap = new stDisassemble[65536];
    Reset();
}

void Memory::Reset()
{
    for (int i = 0; i < 65536; i++)
    {
        m_pMap[i] = 0x00;
        m_pDisassembledMap[i].szDisString[0] = 0;
    }
}

void Memory::Disassemble(u16 address, const char* szDisassembled)
{
    strcpy(m_pDisassembledMap[address].szDisString, szDisassembled);
}

bool Memory::IsDisassembled(u16 address)
{
    return m_pDisassembledMap[address].szDisString[0] != 0;
}

void Memory::LoadBank0and1FromROM(u8* pTheROM)
{
    // loads the first 32KB only (bank 0 and 1)
    for (int i = 0; i < 0x8000; i++)
    {
        m_pMap[i] = pTheROM[i];
    }
}

void Memory::MemoryDump(const char* szFilePath)
{
    using namespace std;

    ofstream myfile(szFilePath, ios::out | ios::trunc);

    if (myfile.is_open())
    {
        for (int i = 0; i < 65536; i++)
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
