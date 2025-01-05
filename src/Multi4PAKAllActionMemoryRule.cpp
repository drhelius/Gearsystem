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

#include "Multi4PAKAllActionMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

Multi4PAKAllActionMemoryRule::Multi4PAKAllActionMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

Multi4PAKAllActionMemoryRule::~Multi4PAKAllActionMemoryRule()
{
}

u8 Multi4PAKAllActionMemoryRule::PerformRead(u16 address)
{
    if (address < 0xC000)
    {
        int slot = (address >> 14) & 0x03;
        if (slot > 3)
        {
            Debug("--> ** Invalid slot %d", slot);
            return 0xFF;
        }
        return m_pCartridge->GetROM()[m_iMapperSlotAddress[slot] + (address & 0x3FFF)];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void Multi4PAKAllActionMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0xC000)
    {
        switch (address)
        {
            case 0x3FFE:
                Debug("--> ** Writing to register $%X %X", address, value);
                m_iMapperSlot[0] = value;
                m_iMapperSlotAddress[0] = 0x4000 * (m_iMapperSlot[0] & (m_pCartridge->GetROMBankCount() - 1));
                return;
            case 0x7FFF:
                Debug("--> ** Writing to register $%X %X", address, value);
                m_iMapperSlot[1] = value;
                m_iMapperSlotAddress[1] = 0x4000 * (m_iMapperSlot[1] & (m_pCartridge->GetROMBankCount() - 1));
                return;
            case 0xBFFF:
                Debug("--> ** Writing to register $%X %X", address, value);
                m_iMapperSlot[2] = value;
                m_iMapperSlotAddress[2] = 0x4000 * (((m_iMapperSlot[0] & 0x30) + value) & (m_pCartridge->GetROMBankCount() - 1));
                return;
            default:
                Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
                return;
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

void Multi4PAKAllActionMemoryRule::Reset()
{
    m_iMapperSlot[0] = 0;
    m_iMapperSlot[1] = 1;
    m_iMapperSlot[2] = 2;

    m_iMapperSlotAddress[0] = 0x0000;
    m_iMapperSlotAddress[1] = 0x4000;
    m_iMapperSlotAddress[2] = 0x8000;
}

u8* Multi4PAKAllActionMemoryRule::GetPage(int index)
{
    if (index < 0 || index > 2)
        return m_pCartridge->GetROM();

    return m_pCartridge->GetROM() + (m_iMapperSlot[index] * 0x4000);
}

int Multi4PAKAllActionMemoryRule::GetBank(int index)
{
    if (index < 0 || index > 2)
        return 0;

    return m_iMapperSlot[index];
}

void Multi4PAKAllActionMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
}

void Multi4PAKAllActionMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
}
