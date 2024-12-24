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

#include "KoreanFFFFHiComMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanFFFFHiComMemoryRule::KoreanFFFFHiComMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanFFFFHiComMemoryRule::~KoreanFFFFHiComMemoryRule()
{
}

u8 KoreanFFFFHiComMemoryRule::PerformRead(u16 address)
{
    if (address < 0xC000)
    {
        int slot = (address >> 14) & 0x03;
        return m_pCartridge->GetROM()[m_iMapperSlotAddress[slot] + (address & 0x3FFF)];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void KoreanFFFFHiComMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address >= 0xC000)
    {
        if (address == 0xFFFF)
        {
            int bank = (value << 1) & (m_pCartridge->GetROMBankCount() - 1);

            m_iMapperSlot[0] = bank;
            m_iMapperSlot[1] = bank + 1;

            m_iMapperSlotAddress[0] = 0x4000 * m_iMapperSlot[0];
            m_iMapperSlotAddress[1] = 0x4000 * m_iMapperSlot[1];
        }

        if (address < 0xE000)
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
    else
    {
        Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
}

void KoreanFFFFHiComMemoryRule::Reset()
{
    m_iMapperSlot[0] = 0;
    m_iMapperSlot[1] = 1;
    m_iMapperSlot[2] = 0;

    for (int i = 0; i < 3; i++)
        m_iMapperSlotAddress[i] = 0x4000 * m_iMapperSlot[i];
}

u8* KoreanFFFFHiComMemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
}

int KoreanFFFFHiComMemoryRule::GetBank(int index)
{
    return m_iMapperSlot[index];
}

void KoreanFFFFHiComMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
}

void KoreanFFFFHiComMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
}
