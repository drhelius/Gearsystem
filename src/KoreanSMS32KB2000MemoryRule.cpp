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

#include "KoreanSMS32KB2000MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanSMS32KB2000MemoryRule::KoreanSMS32KB2000MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanSMS32KB2000MemoryRule::~KoreanSMS32KB2000MemoryRule()
{
}

u8 KoreanSMS32KB2000MemoryRule::PerformRead(u16 address)
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

void KoreanSMS32KB2000MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0xC000)
    {
        if (address == 0x2000)
        {
            Debug("--> ** Writing to register $%X %X", address, value);

            m_iMapperSlot[0] = (value << 1) & (m_pCartridge->GetROMBankCount() - 1);
            m_iMapperSlot[1] = ((value << 1) + 1) & (m_pCartridge->GetROMBankCount() - 1);
            m_iMapperSlot[2] = (value << 1) & (m_pCartridge->GetROMBankCount() - 1);

            m_iMapperSlotAddress[0] = 0x4000 * m_iMapperSlot[0];
            m_iMapperSlotAddress[1] = 0x4000 * m_iMapperSlot[1];
            m_iMapperSlotAddress[2] = 0x4000 * m_iMapperSlot[2];
        }
        else
        {
            Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
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

void KoreanSMS32KB2000MemoryRule::Reset()
{
    m_iMapperSlot[0] = 0;
    m_iMapperSlot[1] = 1;
    m_iMapperSlot[2] = 0;

    m_iMapperSlotAddress[0] = 0x0000;
    m_iMapperSlotAddress[1] = 0x4000;
    m_iMapperSlotAddress[2] = 0x0000;
}

u8* KoreanSMS32KB2000MemoryRule::GetPage(int index)
{
    if (index < 0 || index > 2)
        return NULL;

    return m_pCartridge->GetROM() + (m_iMapperSlot[index] * 0x4000);
}

int KoreanSMS32KB2000MemoryRule::GetBank(int index)
{
    if (index < 0 || index > 2)
        return 0;

    return m_iMapperSlot[index];
}

void KoreanSMS32KB2000MemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
}

void KoreanSMS32KB2000MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
}
