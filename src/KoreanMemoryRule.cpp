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

#include "KoreanMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanMemoryRule::KoreanMemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    Reset();
}

KoreanMemoryRule::~KoreanMemoryRule()
{
}

u8 KoreanMemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
    {
        // ROM page 0 and 1
        return m_pCartridge->GetROM()[address];
    }
    else if (address < 0xC000)
    {
        return m_pCartridge->GetROM()[(address - 0x8000) + m_iMapperSlot2Address];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void KoreanMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        // ROM page 0 and 1
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0xC000)
    {
        if (address == 0xA000)
        {
            m_iMapperSlot2 = value % m_pCartridge->GetROMBankCount();
            m_iMapperSlot2Address = m_iMapperSlot2 * 0x4000;
        }
        else
        {
            // ROM page 2
            Log("--> ** Attempting to write on ROM page 2 $%X %X", address, value);
        }
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

void KoreanMemoryRule::Reset()
{
    m_iMapperSlot2 = 2;
    m_iMapperSlot2Address = m_iMapperSlot2 * 0x4000;
}

u8* KoreanMemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
            return m_pCartridge->GetROM() + (index * 0x4000);
        case 2:
            return m_pCartridge->GetROM() + m_iMapperSlot2Address;
        default:
            return NULL;
    }
}

void KoreanMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot2), sizeof(m_iMapperSlot2));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot2Address), sizeof(m_iMapperSlot2Address));
}

void KoreanMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iMapperSlot2), sizeof(m_iMapperSlot2));
    stream.read(reinterpret_cast<char*> (m_iMapperSlot2Address), sizeof(m_iMapperSlot2Address));
}
