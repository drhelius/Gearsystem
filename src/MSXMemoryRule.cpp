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

#include "MSXMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

MSXMemoryRule::MSXMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

MSXMemoryRule::~MSXMemoryRule()
{
}

u8 MSXMemoryRule::PerformRead(u16 address)
{
    if (address < 0x4000)
    {
        u8* pROM = m_pCartridge->GetROM();

        // nemesis
        if ((m_pCartridge->GetCRC() == 0xE316C06D) && (address < 0x2000))
        {
            return pROM[m_pCartridge->GetROMSize() - 0x2000 + address];
        }
        else
        {
            return pROM[address];
        }
    }
    else if (address < 0x6000)
    {
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0x4000) + m_iMapperSlotAddress[2]];
    }
    else if (address < 0x8000)
    {
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0x6000) + m_iMapperSlotAddress[3]];
    }
    else if (address < 0xA000)
    {
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0x8000) + m_iMapperSlotAddress[0]];
    }
    else if (address < 0xC000)
    {
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0xA000) + m_iMapperSlotAddress[1]];
    }
    else
    {
        return m_pMemory->Retrieve(address);
    }
}

void MSXMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x0004)
    {
        m_iMapperSlot[address] = value;
        m_iMapperSlotAddress[address] = m_iMapperSlot[address] * 0x2000;
    }
    else if (address < 0xC000)
    {
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
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

void MSXMemoryRule::Reset()
{
    for (int i = 0; i < 4; i++)
    {
        m_iMapperSlot[i] = 0;
        m_iMapperSlotAddress[i] = m_iMapperSlot[i] * 0x2000;
    }
}

u8* MSXMemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
            return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
        case 2:
            return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
        default:
            return NULL;
    }
}

int MSXMemoryRule::GetBank(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
            return m_iMapperSlot[index];
        default:
            return 0;
    }
}

void MSXMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
}

void MSXMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
}
