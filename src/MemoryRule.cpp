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

#include "MemoryRule.h"

MemoryRule::MemoryRule(Memory* pMemory, Cartridge* pCartridge)
{
    m_pMemory = pMemory;
    m_pCartridge = pCartridge;
}

MemoryRule::~MemoryRule()
{

}

void MemoryRule::SaveRam(std::ostream&)
{
    Log("MemoryRule::SaveRam not implemented");
}

bool MemoryRule::LoadRam(std::istream&, s32)
{
    Log("MemoryRule::LoadRam not implemented");
    return false;
}

void MemoryRule::SetRamChangedCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

bool MemoryRule::PersistedRAM()
{
    Log("MemoryRule::PersistedRAM not implemented");
    return false;
}

size_t MemoryRule::GetRamSize()
{
    Log("MemoryRule::GetRamSize not implemented");
    return 0;
}

u8* MemoryRule::GetRamBanks()
{
    Log("MemoryRule::GetRamBanks not implemented");
    return NULL;
}

u8* MemoryRule::GetPage(int)
{
    Log("MemoryRule::GetPage not implemented");
    return NULL;
}

int MemoryRule::GetBank(int)
{
    Log("MemoryRule::GetBank not implemented");
    return 0;
}

void MemoryRule::SaveState(std::ostream&)
{
    Log("MemoryRule::SaveState not implemented");
}

void MemoryRule::LoadState(std::istream&)
{
    Log("MemoryRule::LoadState not implemented");
}
