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
    m_pCartRAM = new u8[0x2000];
    Reset();
}

CodemastersMemoryRule::~CodemastersMemoryRule()
{
    SafeDeleteArray(m_pCartRAM);
}

u8 CodemastersMemoryRule::PerformRead(u16 address)
{
    if (address < 0x4000)
    {
        // ROM page 0
        u8* pROM = m_pCartridge->GetROM();
        return pROM[address + m_iMapperSlotAddress[0]];
    }
    else if (address < 0x8000)
    {
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0x4000) + m_iMapperSlotAddress[1]];
    }
    else if (address < 0xC000)
    {
        // ROM page 2
        if (m_bRAMBankActive && (address >= 0xA000))
        {
            return m_pCartRAM[address - 0xA000];
        }
        else
        {
            u8* pROM = m_pCartridge->GetROM();
            return pROM[(address - 0x8000) + m_iMapperSlotAddress[2]];
        }
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
                m_iMapperSlot[0] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x4000;
                break;
            }
            case 0x4000:
            {
                m_bRAMBankActive = ((value & 0x80) != 0) && m_pCartridge->HasRAMWithoutBattery();
                m_iMapperSlot[1] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x4000;
                break;
            }
            case 0x8000:
            {
                m_iMapperSlot[2] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x4000;
                break;
            }
            default:
            {
                if (!m_pCartridge->HasRAMWithoutBattery())
                {
                    Log("--> ** Attempting to write on ROM address $%X %X", address, value);
                }
            }
        }

        if (m_pCartridge->HasRAMWithoutBattery())
        {
            if ((address >= 0xA000) && (address < 0xC000) && m_bRAMBankActive)
            {
                m_pCartRAM[address - 0xA000] = value;
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
    m_bRAMBankActive = false;
}

u8* CodemastersMemoryRule::GetPage(int index)
{
    if ((index >= 0) && (index < 3))
        return m_pMemory->GetMemoryMap() + (0x4000 * index);
    else
        return NULL;
}

void CodemastersMemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*> (m_pCartRAM), 0x2000);
    stream.write(reinterpret_cast<const char*> (&m_bRAMBankActive), sizeof(m_bRAMBankActive));
}

void CodemastersMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*> (m_pCartRAM), 0x2000);
    stream.read(reinterpret_cast<char*> (&m_bRAMBankActive), sizeof(m_bRAMBankActive));
}
