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

#include "RomOnlyMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

RomOnlyMemoryRule::RomOnlyMemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    Reset();
}

RomOnlyMemoryRule::~RomOnlyMemoryRule()
{
}

u8 RomOnlyMemoryRule::PerformRead(u16 address)
{
    return m_pMemory->Retrieve(address);
}

void RomOnlyMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        // ROM page 0 and 1
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0xC000)
    {
        // ROM page 2
        Log("--> ** Attempting to write on ROM page 2 $%X %X", address, value);

        // this space is also available to external ram
        // m_pMemory->Load(address, value);
    }
    else if (address < 0xE000)
    {
        // RAM
        m_pMemory->Load(address, value);
        m_pMemory->Load(address + 0x2000, value);
    }
    else
    {
        // RAM (mirror)
        m_pMemory->Load(address, value);
        m_pMemory->Load(address - 0x2000, value);

        if (address >= 0xFFCC)
        {
            Log("--> ** Attempting to write to Control reg $%X %X", address, value);
        }
        else
        {
            Log("--> ** Attempting to write on mirrored RAM $%X %X", address, value);
        }
    }
}

void RomOnlyMemoryRule::Reset()
{
}
