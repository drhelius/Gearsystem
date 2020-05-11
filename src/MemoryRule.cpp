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
}

bool MemoryRule::LoadRam(std::istream&, s32)
{
    return false;
}

void MemoryRule::SetRamChangedCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

bool MemoryRule::PersistedRAM()
{
    return false;
}

size_t MemoryRule::GetRamSize()
{
    return 0;
}

u8* MemoryRule::GetRamBanks()
{
    return NULL;
}

int MemoryRule::GetRamBank()
{
    return 0;
}

u8* MemoryRule::GetPage(int)
{
    return NULL;
}

int MemoryRule::GetBank(int)
{
    return 0;
}

void MemoryRule::SaveState(std::ostream&)
{
}

void MemoryRule::LoadState(std::istream&)
{
}
