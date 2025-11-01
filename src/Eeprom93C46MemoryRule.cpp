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

#include "Eeprom93C46MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"
#include <cstring>

Eeprom93C46MemoryRule::Eeprom93C46MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) 
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

Eeprom93C46MemoryRule::~Eeprom93C46MemoryRule()
{
}

u8 Eeprom93C46MemoryRule::PerformRead(u16 address)
{
    // Addresses in the [8000h] range
    if ((address >> 13) == 0x04 && m_EEPROM.Enabled)
    {
        // 93c46 Serial Access
        if (address == 0x8000)
        {
            return EEPROM_93c46_Read();
        }
        // 93c46 Direct Access
        if (address >= 0x8008 && address < 0x8088)
        {
            return EEPROM_93c46_DirectRead(address - 0x8008);
        }
    }

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
        // ROM page 2
        u8* pROM = m_pCartridge->GetROM();
        return pROM[(address - 0x8000) + m_iMapperSlotAddress[2]];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void Eeprom93C46MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address)
    {
        case 0x8000:
            // 93c46 Write (Set Lines)
            EEPROM_93c46_SetLines(value);
            return;
        case 0xFFFC:
            // 93c46 Control Register
            EEPROM_93c46_Control(value);
            m_pMemory->Load(address, value);
            m_pMemory->Load(address + 0x2000, value);
            return;
        case 0xFFFD:
        {
            // Frame 0
            int romBankCount = m_pCartridge->GetROMBankCount();
            m_iMapperSlot[0] = value & (romBankCount - 1);
            m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x4000;
            m_pMemory->Load(address, value);
            m_pMemory->Load(address + 0x2000, value);
            return;
        }
        case 0xFFFE:
        {
            // Frame 1
            int romBankCount = m_pCartridge->GetROMBankCount();
            m_iMapperSlot[1] = value & (romBankCount - 1);
            m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x4000;
            m_pMemory->Load(address, value);
            m_pMemory->Load(address + 0x2000, value);
            return;
        }
        case 0xFFFF:
        {
            // Frame 2
            int romBankCount = m_pCartridge->GetROMBankCount();
            m_iMapperSlot[2] = value & (romBankCount - 1);
            m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x4000;
            m_pMemory->Load(address, value);
            m_pMemory->Load(address + 0x2000, value);
            return;
        }
    }

    // 93c46 Direct Access
    if ((address >> 13) == 4 && address >= 0x8008 && address < 0x8088)
    {
        EEPROM_93c46_DirectWrite(address - 0x8008, value);
        return;
    }

    // RAM [0xC000] = [0xE000]
    if (address >= 0xC000 && address < 0xE000)
    {
        m_pMemory->Load(address, value);
        m_pMemory->Load(address + 0x2000, value);
        return;
    }
    
    if (address >= 0xE000)
    {
        m_pMemory->Load(address, value);
        m_pMemory->Load(address - 0x2000, value);
        return;
    }

    Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
}

void Eeprom93C46MemoryRule::Reset()
{
    for (int i = 0; i < 3; i++)
    {
        m_iMapperSlot[i] = i;
        m_iMapperSlotAddress[i] = i * 0x4000;
    }

    EEPROM_93c46_Clear();
    EEPROM_93c46_Init(EEPROM_93C46_INIT_ALL);
}

void Eeprom93C46MemoryRule::SaveRam(std::ostream& file)
{
    Debug("Eeprom93C46MemoryRule save EEPROM...");

    for (int i = 0; i < EEPROM_93C46_DATA_SIZE; i++)
    {
        u8 eeprom_byte = (i % 2 == 0) ? (m_EEPROM.Data[i / 2] & 0xFF) : (m_EEPROM.Data[i / 2] >> 8);
        file.write(reinterpret_cast<const char*>(&eeprom_byte), 1);
    }

    Debug("Eeprom93C46MemoryRule save EEPROM done");
}

bool Eeprom93C46MemoryRule::LoadRam(std::istream& file, s32 fileSize)
{
    Debug("Eeprom93C46MemoryRule load EEPROM...");

    if ((fileSize > 0) && (fileSize != EEPROM_93C46_DATA_SIZE))
    {
        Log("Eeprom93C46MemoryRule incorrect size. Expected: %d Found: %d", EEPROM_93C46_DATA_SIZE, fileSize);
        return false;
    }

    for (int i = 0; i < EEPROM_93C46_DATA_SIZE; i++)
    {
        u8 eeprom_byte = 0;
        file.read(reinterpret_cast<char*>(&eeprom_byte), 1);
        
        if (i % 2 == 0)
        {
            m_EEPROM.Data[i / 2] = eeprom_byte;
        }
        else
        {
            m_EEPROM.Data[i / 2] |= (eeprom_byte << 8);
        }
    }

    Debug("Eeprom93C46MemoryRule load EEPROM done");

    return true;
}

bool Eeprom93C46MemoryRule::PersistedRAM()
{
    return true;
}

size_t Eeprom93C46MemoryRule::GetRamSize()
{
    return EEPROM_93C46_DATA_SIZE;
}

u8* Eeprom93C46MemoryRule::GetRamBanks()
{
    return reinterpret_cast<u8*>(m_EEPROM.Data);
}

u8* Eeprom93C46MemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
            return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
        default:
            return NULL;
    }
}

int Eeprom93C46MemoryRule::GetBank(int index)
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

void Eeprom93C46MemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.write(reinterpret_cast<const char*>(m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*>(&m_EEPROM), sizeof(m_EEPROM));
}

void Eeprom93C46MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*>(m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.read(reinterpret_cast<char*>(m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*>(&m_EEPROM), sizeof(m_EEPROM));
}

// EEPROM 93C46 Implementation

void Eeprom93C46MemoryRule::EEPROM_93c46_Clear()
{
    memset(m_EEPROM.Data, 0xFF, EEPROM_93C46_DATA_SIZE);
}

void Eeprom93C46MemoryRule::EEPROM_93c46_Init(int init)
{
    if (init == EEPROM_93C46_INIT_ALL)
    {
        m_EEPROM.Enabled = 0;
        m_EEPROM.Lines = 0x00 | EEPROM_93C46_LINE_DATA_OUT;
        m_EEPROM.ReadOnly = 1;
    }
    m_EEPROM.Status = EEPROM_93C46_STATUS_START;
    m_EEPROM.Opcode = 0x0000;
    m_EEPROM.Position = 0;
}

void Eeprom93C46MemoryRule::EEPROM_93c46_Control(u8 value)
{
    if (value & 0x80)
    {
        EEPROM_93c46_Init(EEPROM_93C46_INIT_ALL);
        return;
    }
    m_EEPROM.Enabled = (value & 0x08) ? 1 : 0;
}

void Eeprom93C46MemoryRule::EEPROM_93c46_SetLines(u8 lines)
{
    // CS Line (Reset), 1->0 edge
    if (!(lines & EEPROM_93C46_LINE_CS))
    {
        if (m_EEPROM.Lines & EEPROM_93C46_LINE_CS)
        {
            EEPROM_93c46_Init(EEPROM_93C46_INIT_NORMAL);
        }
        m_EEPROM.Lines = (m_EEPROM.Lines & ~0x07) | (lines & 0x07) | EEPROM_93C46_LINE_DATA_OUT;
        return;
    }

    // Clock Line, 0->1 edge
    if ((lines & EEPROM_93C46_LINE_CLOCK) && !(m_EEPROM.Lines & EEPROM_93C46_LINE_CLOCK))
    {
        u8 data = lines & EEPROM_93C46_LINE_DATA_IN;
        m_EEPROM.Lines = (m_EEPROM.Lines & ~0x07) | (lines & 0x07);
        
        switch (m_EEPROM.Status)
        {
            // Start Bit
            case EEPROM_93C46_STATUS_START:
                if (data)
                {
                    m_EEPROM.Status = EEPROM_93C46_STATUS_OPCODE;
                    m_EEPROM.Opcode = 0x0000;
                    m_EEPROM.Position = 0;
                }
                return;
                
            // Opcode (2 Bits) + Address (6 Bits) Read
            case EEPROM_93C46_STATUS_OPCODE:
                m_EEPROM.Opcode = (m_EEPROM.Opcode << 1) | data;
                if (++m_EEPROM.Position == 8)
                {
                    switch (m_EEPROM.Opcode & 0xC0)
                    {
                        case 0x00: // 00: EXTENDED
                            switch (m_EEPROM.Opcode & 0x30)
                            {
                                case 0x00: // ERASE/WRITE DISABLE
                                    m_EEPROM.ReadOnly = 1;
                                    m_EEPROM.Status = EEPROM_93C46_STATUS_START;
                                    return;
                                case 0x10: // WRITE ALL
                                    m_EEPROM.Position = 0;
                                    m_EEPROM.Latch = 0x0000;
                                    m_EEPROM.Status = EEPROM_93C46_STATUS_WRITING;
                                    return;
                                case 0x20: // ERASE ALL
                                    if (m_EEPROM.ReadOnly == 0)
                                    {
                                        memset(m_EEPROM.Data, 0xFF, EEPROM_93C46_DATA_SIZE);
                                    }
                                    m_EEPROM.Lines |= EEPROM_93C46_LINE_DATA_OUT;
                                    m_EEPROM.Status = EEPROM_93C46_STATUS_START;
                                    return;
                                case 0x30: // ERASE/WRITE ENABLE
                                    m_EEPROM.ReadOnly = 0;
                                    m_EEPROM.Status = EEPROM_93C46_STATUS_START;
                                    return;
                            }
                            return;
                        case 0x40: // 01: WRITE
                            m_EEPROM.Position = 0;
                            m_EEPROM.Latch = 0x0000;
                            m_EEPROM.Status = EEPROM_93C46_STATUS_WRITING;
                            return;
                        case 0x80: // 10: READ
                            m_EEPROM.Position = 0;
                            m_EEPROM.Status = EEPROM_93C46_STATUS_READING;
                            m_EEPROM.Lines &= ~EEPROM_93C46_LINE_DATA_OUT;
                            return;
                        case 0xC0: // 11: ERASE
                            if (m_EEPROM.ReadOnly == 0)
                            {
                                m_EEPROM.Data[m_EEPROM.Opcode & 0x3F] = 0xFFFF;
                                m_EEPROM.Lines |= EEPROM_93C46_LINE_DATA_OUT;
                                m_EEPROM.Status = EEPROM_93C46_STATUS_START;
                            }
                            return;
                    }
                }
                return;
                
            // Reading
            case EEPROM_93C46_STATUS_READING:
                if (m_EEPROM.Data[m_EEPROM.Opcode & 0x3F] & (0x8000 >> m_EEPROM.Position))
                    m_EEPROM.Lines |= EEPROM_93C46_LINE_DATA_OUT;
                else
                    m_EEPROM.Lines &= ~EEPROM_93C46_LINE_DATA_OUT;
                    
                if (++m_EEPROM.Position == 16)
                {
                    m_EEPROM.Position = 0;
                    m_EEPROM.Opcode = 0x80 | ((m_EEPROM.Opcode + 1) & 0x3F);
                }
                return;
                
            // Writing
            case EEPROM_93C46_STATUS_WRITING:
                m_EEPROM.Latch = (m_EEPROM.Latch << 1) | data;
                if (++m_EEPROM.Position == 16)
                {
                    if (m_EEPROM.ReadOnly == 0)
                    {
                        if ((m_EEPROM.Opcode & 0x40) == 0x40)
                        {
                            // WRITE
                            m_EEPROM.Data[m_EEPROM.Opcode & 0x3F] = m_EEPROM.Latch;
                        }
                        else
                        {
                            // WRITE ALL
                            for (int i = 0; i < 64; i++)
                            {
                                m_EEPROM.Data[i] = m_EEPROM.Latch;
                            }
                        }
                    }
                    m_EEPROM.Lines |= EEPROM_93C46_LINE_DATA_OUT;
                    m_EEPROM.Status = EEPROM_93C46_STATUS_START;
                }
                return;
        }
        return;
    }

    // Data Line
    m_EEPROM.Lines = (m_EEPROM.Lines & ~0x07) | (lines & 0x07);
}

u8 Eeprom93C46MemoryRule::EEPROM_93c46_Read()
{
    int ret = (m_EEPROM.Lines & EEPROM_93C46_LINE_CS)
            | ((m_EEPROM.Lines & EEPROM_93C46_LINE_DATA_OUT) >> EEPROM_93C46_LINE_DATA_OUT_POS)
            | EEPROM_93C46_LINE_CLOCK;
    return ret;
}

void Eeprom93C46MemoryRule::EEPROM_93c46_DirectWrite(int addr, u8 data)
{
    if (addr >= 0 && addr < 64)
    {
        if (addr % 2 == 0)
        {
            m_EEPROM.Data[addr / 2] = (m_EEPROM.Data[addr / 2] & 0xFF00) | data;
        }
        else
        {
            m_EEPROM.Data[addr / 2] = (m_EEPROM.Data[addr / 2] & 0x00FF) | (data << 8);
        }
    }
}

u8 Eeprom93C46MemoryRule::EEPROM_93c46_DirectRead(int addr)
{
    if (addr >= 0 && addr < 64)
    {
        if (addr % 2 == 0)
        {
            return m_EEPROM.Data[addr / 2] & 0xFF;
        }
        else
        {
            return m_EEPROM.Data[addr / 2] >> 8;
        }
    }
    return 0xFF;
}
