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

#include "SG1000MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

SG1000MemoryRule::SG1000MemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    Reset();
}

SG1000MemoryRule::~SG1000MemoryRule()
{
}

u8 SG1000MemoryRule::PerformRead(u16 address)
{
    if (!m_pCartridge->HasRAMWithoutBattery() && (address >= 0x4000) && (address < 0x8000))
    {
        return m_pMemory->Retrieve(address - 0x4000);
    }

    return m_pMemory->Retrieve(address);
}

void SG1000MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x3000)
    {
        // ROM
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0x4000)
    {
        // May contain some RAM
        m_pMemory->Load(address, value);
    }
    else if (address < 0x8000)
    {
        // ROM
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else
    {
        // RAM
        m_pMemory->Load(address, value);
    }
}

void SG1000MemoryRule::Reset()
{
}

u8* SG1000MemoryRule::GetPage(int index)
{
    if ((index >= 0) && (index < 3))
        return m_pMemory->GetMemoryMap() + (0x4000 * index);
    else
        return NULL;
}
