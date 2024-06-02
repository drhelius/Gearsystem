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

#include "BootromMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

BootromMemoryRule::BootromMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

BootromMemoryRule::~BootromMemoryRule()
{
}

u8 BootromMemoryRule::PerformRead(u16 address)
{
    if (!IsValidPointer(m_pBootrom))
        return 0x00;

    if (address < 0x400)
    {
        // First 1KB (fixed)
        return m_pBootrom[address];
    }
    else if (address < 0x4000)
    {
        // ROM page 0
        return m_pBootromBanks[address + m_iMapperSlotAddress[0]];
    }
    else if (address < 0x8000)
    {
        // ROM page 1
        return m_pBootromBanks[(address - 0x4000) + m_iMapperSlotAddress[1]];
    }
    else if (address < 0xC000)
    {
        // ROM page 2
        return m_pBootromBanks[(address - 0x8000) + m_iMapperSlotAddress[2]];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void BootromMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (!IsValidPointer(m_pBootrom))
        return;

    if (address < 0x8000)
    {
        // ROM page 0 and 1
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0xC000)
    {
        // ROM page 2
        Log("--> ** Attempting to write on ROM page 2 $%X %X", address, value);
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

        switch (address)
        {
            case 0xFFFC:
            {
                Log("--> ** Boot Attempting to write on RAM setup registry $%X %X", address, value);
                break;
            }
            case 0xFFFD:
            {
                m_iMapperSlot[0] = value & m_iBankMax;
                m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x4000;
                break;
            }
            case 0xFFFE:
            {
                m_iMapperSlot[1] = value & m_iBankMax;
                m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x4000;
                break;
            }
            case 0xFFFF:
            {
                m_iMapperSlot[2] = value & m_iBankMax;
                m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x4000;
                break;
            }
        }
    }
}

void BootromMemoryRule::Reset()
{
    m_pBootrom = m_pMemory->GetBootrom();
    m_pBootromBanks = m_pCartridge->IsGameGear() ? m_pCartridge->GetROM() : m_pBootrom;
    m_iBankMax = m_pCartridge->IsGameGear() ? (m_pCartridge->GetROMBankCount() - 1) : (m_pMemory->GetBootromBankCount() - 1);

    for (int i = 0; i < 3; i++)
    {
        m_iMapperSlot[i] = i;
        m_iMapperSlotAddress[i] = i * 0x4000;
    }
}

u8* BootromMemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
            return m_pBootrom + m_iMapperSlotAddress[index];
        default:
            return NULL;
    }
}

int BootromMemoryRule::GetBank(int index)
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
