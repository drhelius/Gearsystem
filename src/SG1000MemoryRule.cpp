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

SG1000MemoryRule::SG1000MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

SG1000MemoryRule::~SG1000MemoryRule()
{
}

u8 SG1000MemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
    {
        // Cartridge ROM ($0000-$7FFF) with mirroring for small ROMs
        int romSize = m_pCartridge->GetROMSize();
        if (romSize > 0 && romSize < 0x8000 && address >= (u16)romSize)
            return m_pMemory->Retrieve(address % romSize);
        return m_pMemory->Retrieve(address);
    }
    else if (address < 0xC000)
    {
        // Expansion port ($8000-$BFFF)
        if (m_pCartridge->HasRAMWithoutBattery() || m_pCartridge->GetROMSize() > 0x8000)
            return m_pMemory->Retrieve(address);
        else
            return 0xFF;
    }
    else
    {
        // System RAM ($C000-$FFFF) - 1KB mirrored every $0400
        return m_pMemory->Retrieve(0xC000 + (address & 0x03FF));
    }
}

void SG1000MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x2000)
    {
        // ROM
        Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0x4000)
    {
        // On-cartridge RAM ($2000-$3FFF)
        m_pMemory->Load(address, value);
    }
    else if (address < 0x8000)
    {
        // ROM ($4000-$7FFF)
        Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0xC000)
    {
        // Expansion port ($8000-$BFFF) - writable only if cart RAM present
        if (m_pCartridge->HasRAMWithoutBattery())
            m_pMemory->Load(address, value);
    }
    else
    {
        // System RAM ($C000-$FFFF) - 1KB mirrored every $0400
        m_pMemory->Load(0xC000 + (address & 0x03FF), value);
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

int SG1000MemoryRule::GetBank(int index)
{
    if ((index >= 0) && (index < 3))
        return index;
    else
        return 0;
}
