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

#include "HomebrewMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

HomebrewMemoryRule::HomebrewMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    // This is only done on POR
    Debug("Resetting game slot and mapper");
    m_iGameSlot = 0;
    m_iMapperSlot = 0;
    Reset();
}

HomebrewMemoryRule::~HomebrewMemoryRule()
{
}

u8 HomebrewMemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
    {
        // First 48KB (fixed)
        return m_pMemory->Retrieve(address);
    }
    else if (address < 0xC000)
    {
        // ROM slot 2
        u8* pROM = m_pCartridge->GetROM();
        return pROM[((address - 0x8000) + m_iMapperSlotAddress) | (m_iGameSlot << 17)];
    }

    return m_pMemory->Retrieve(address);
}

void HomebrewMemoryRule::PerformWrite(u16 address, u8 value)
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

        switch (address)
        {
            case 0xFFFE:
            {
                // Flash address lines A18-A17
                m_iGameSlot = value & 3;
                Debug("Setting game mapper to %d", m_iGameSlot);
                m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), (1024 * 48), (m_iGameSlot << 17));
                break;
            }
            case 0xFFFF:
            {
                m_iMapperSlot = value & 31;
                m_iMapperSlotAddress = m_iMapperSlot * 0x4000;
                Debug("Setting slot 2 mapper to %d", m_iMapperSlot);
                break;
            }
        }
    }
}

void HomebrewMemoryRule::Reset()
{
}

u8* HomebrewMemoryRule::GetPage(int index)
{
    if ((index >= 0) && (index < 3))
        return m_pMemory->GetMemoryMap() + (0x4000 * index);
    else
        return NULL;
}

int HomebrewMemoryRule::GetBank(int index)
{
    if ((index >= 0) && (index < 3))
        return index;
    else
        return 0;
}
