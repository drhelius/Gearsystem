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

#include "CodemastersMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

CodemastersMemoryRule::CodemastersMemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    Reset();
}

CodemastersMemoryRule::~CodemastersMemoryRule()
{
}

u8 CodemastersMemoryRule::PerformRead(u16 address)
{
    if (address < 0x4000)
    {
        // ROM page 0
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[address + m_iMapperSlotAddress[0]];
    }
    else if (address < 0x8000)
    {
        // ROM page 1
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[(address - 0x4000) + m_iMapperSlotAddress[1]];
    }
    else if (address < 0xC000)
    {
        // ROM page 2
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[(address - 0x8000) + m_iMapperSlotAddress[2]];

        // this space is also available to external ram
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void CodemastersMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0xC000)
    {
        // ROM page 0, 1 and 2
        switch (address)
        {
            case 0x0000:
            {
                //Log("--> ** Selecting bank %d for slot 0", value);
                m_iMapperSlot[0] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x4000;
                break;
            }
            case 0x4000:
            {
                //Log("--> ** Selecting bank %d for slot 1", value);
                m_iMapperSlot[1] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x4000;
                break;
            }
            case 0x8000:
            {
                //Log("--> ** Selecting bank %d for slot 2", value);
                m_iMapperSlot[2] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x4000;
                break;
            }
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

void CodemastersMemoryRule::Reset()
{
    m_iMapperSlot[0] = 0;
    m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x4000;
    m_iMapperSlot[1] = 1;
    m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x4000;
    m_iMapperSlot[2] = 0;
    m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x4000;
}

