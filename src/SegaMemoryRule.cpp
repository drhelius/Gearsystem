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

#include "SegaMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

SegaMemoryRule::SegaMemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    m_pRAMBanks = new u8[0x8000];
    Reset();
}

SegaMemoryRule::~SegaMemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

u8 SegaMemoryRule::PerformRead(u16 address)
{
    if (address < 0x400)
    {
        // First 1KB (fixed)
        return m_pMemory->Retrieve(address);
    }
    else if (address < 0x4000)
    {
        // ROM page 0
        u8* pROM = m_pCartridge->GetROM();
        return pROM[address + m_iMapperSlotAddress[0]];
    }
    else if (address < 0x8000)
    {
        // ROM page 1
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0x4000) + m_iMapperSlotAddress[1]];
    }
    else if (address < 0xC000)
    {
        if (m_bRAMEnabled)
        {
            // External RAM
            return m_pRAMBanks [(address - 0x8000) + m_RAMBankStartAddress];
        }
        else
        {
            // ROM page 2
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

void SegaMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        // ROM page 0 and 1
        Log("--> ** Attempting to write on ROM address $%X %X", address, value);
    }
    else if (address < 0xC000)
    {
        if (m_bRAMEnabled)
        {
            // External RAM
            m_pRAMBanks[(address - 0x8000) + m_RAMBankStartAddress] = value;
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

        switch (address)
        {
            case 0xFFFC:
            {
                m_RAMBankStartAddress = IsSetBit(value, 2) ? 0x4000 : 0x0000;
                m_bRAMEnabled = IsSetBit(value, 3);
                if (m_bRAMEnabled && !m_pCartridge->HasRAMWithoutBattery())
                    m_iPersistRAM = 1;
                break;
            }
            case 0xFFFD:
            {
                m_iMapperSlot[0] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x4000;
                break;
            }
            case 0xFFFE:
            {
                m_iMapperSlot[1] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x4000;
                break;
            }
            case 0xFFFF:
            {
                m_iMapperSlot[2] = value & (m_pCartridge->GetROMBankCount() - 1);
                m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x4000;
                break;
            }
        }
    }

    if (m_iPersistRAM < 0)
        m_iPersistRAM = 0;
}

void SegaMemoryRule::Reset()
{
    m_RAMBankStartAddress = 0;
    m_bRAMEnabled = false;
    m_iPersistRAM = -1;

    for (int i = 0; i < 3; i++)
    {
        m_iMapperSlot[i] = i;
        m_iMapperSlotAddress[i] = i * 0x4000;
    }
}

void SegaMemoryRule::SaveRam(std::ostream & file)
{
    Log("SegaMemoryRule save RAM...");

    for (int i = 0x0000; i < 0x8000; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Log("SegaMemoryRule save RAM done");
}

bool SegaMemoryRule::LoadRam(std::istream & file, s32 fileSize)
{
    Log("SegaMemoryRule load RAM...");

    if ((fileSize > 0) && (fileSize != 0x8000))
    {
        Log("SegaMemoryRule incorrect size. Expected: 512 Found: %d", fileSize);
        return false;
    }

    for (int i = 0x0000; i < 0x8000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    Log("SegaMemoryRule load RAM done");

    return true;
}

bool SegaMemoryRule::PersistedRAM()
{
    return (m_iPersistRAM == 1);
}

size_t SegaMemoryRule::GetRamSize()
{
    if (m_iPersistRAM == 0)
        return 0;
    else
        return 0x8000;
}

u8* SegaMemoryRule::GetRamBanks()
{
    if (m_iPersistRAM == 0)
        return NULL;
    else
        return m_pRAMBanks;
}

u8* SegaMemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
            return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
        case 2:
            if (m_bRAMEnabled)
                return m_pRAMBanks + m_RAMBankStartAddress;
            else
                return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
        default:
            return NULL;
    }
}

void SegaMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), 0x8000);
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*> (&m_RAMBankStartAddress), sizeof(m_RAMBankStartAddress));
    stream.write(reinterpret_cast<const char*> (&m_bRAMEnabled), sizeof(m_bRAMEnabled));
    stream.write(reinterpret_cast<const char*> (&m_iPersistRAM), sizeof(m_iPersistRAM));
}

void SegaMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pRAMBanks), 0x8000);
    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*> (&m_RAMBankStartAddress), sizeof(m_RAMBankStartAddress));
    stream.read(reinterpret_cast<char*> (&m_bRAMEnabled), sizeof(m_bRAMEnabled));
    stream.read(reinterpret_cast<char*> (&m_iPersistRAM), sizeof(m_iPersistRAM));
}
