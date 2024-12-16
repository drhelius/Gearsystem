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

RomOnlyMemoryRule::RomOnlyMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
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
    if (address < 0xC000)
    {
        // ROM page 0, 1 and 2
        Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
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
    }
}

void RomOnlyMemoryRule::Reset()
{
}

u8* RomOnlyMemoryRule::GetPage(int index)
{
    if ((index >= 0) && (index < 3))
        return m_pMemory->GetMemoryMap() + (0x4000 * index);
    else
        return NULL;
}

int RomOnlyMemoryRule::GetBank(int index)
{
    if ((index >= 0) && (index < 3))
        return index;
    else
        return 0;
}
