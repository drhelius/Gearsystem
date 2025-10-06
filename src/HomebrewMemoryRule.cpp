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
        if (m_bFlashIDMode)
        {
            switch(address)
            {
                case 0x0000:
                    // Manufacturer ID (MCHP)
                    return 0xbf;
                case 0x0001:
                    // Device ID (512KB flash)
                    return 0xb7;
                default:
                    return 0xFF;
            }
        }
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
    switch (address)
    {
        case 0x5555:
        case 0x2AAA:
            // Handle flash commands
            ProcessFlashAccess(address, value);
            break;
        default:
            if (address < 0xC000)
            {
                if (m_bFlashIDMode)
                {
                    // Any write exits flash ID mode
                    Debug("Exiting Flash ID mode");
                    Reset();
                }
                else if (m_bFlashEraseMode)
                {
                    if (value == 0x30)
                    {
                        // Erase sector command
                        Debug("Erasing flash sector");
                        u8* pROM = m_pCartridge->GetROM();
                        // Calculate base ROM offset for the sector being erased
                        int sectorOffset = ((address - 0x8000) + m_iMapperSlotAddress) | (m_iGameSlot << 17);
                        // Ensure sectorOffset points to the start of a 0x4000 sector
                        sectorOffset &= ~0x3FFF;
                        for (int i = 0; i < 0x4000; i++)
                        {
                            pROM[sectorOffset + i] = 0xFF;
                            // Update the emulator memory map for the visible address range
                            m_pMemory->Load(0x8000 + (sectorOffset + i - ((m_iGameSlot << 17) + m_iMapperSlotAddress)), 0xFF);
                        }
                    }
                    Reset();
                    Debug("Exiting Flash Erase mode");
                }
                else if (m_bFlashWriteMode)
                {
                    // Write byte command
                    Debug("Writing byte %X to flash address %X", value, address);
                    u8* pROM = m_pCartridge->GetROM();
                    pROM[((address - 0x8000) + m_iMapperSlotAddress) | (m_iGameSlot << 17)] = value;
                    m_pMemory->Load(address, value);
                    Reset();
                    Debug("Exiting Flash Write mode");
                }
                else
                {
                    Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
                }
            }
            else
            {
                // RAM or mapper
                switch (address)
                {
                    case 0xFFFE:
                        // Flash address lines A18-A17
                        m_iGameSlot = value & 3;
                        Debug("Setting game mapper to %d", m_iGameSlot);
                        m_pMemory->LoadSlotsFromROM(m_pCartridge->GetROM(), (1024 * 48), (m_iGameSlot << 17));
                        break;
                    case 0xFFFF:
                        m_iMapperSlot = value & 31;
                        m_iMapperSlotAddress = m_iMapperSlot * 0x4000;
                        break;
                }
                // RAM and shadow RAM
                m_pMemory->Load((address & ~0x2000), value);
                m_pMemory->Load((address | 0x2000), value);
            }
            break;
    }   
}

void HomebrewMemoryRule::ProcessFlashAccess(u16 address, u8 value)
{
    // Use helper to advance each of the sequences; if completed, enter the corresponding mode
    if (AdvanceSequence(m_iFlashWriteSequence, FLASH_WRITE_SEQUENCE_LENGTH, m_iFlashWriteStep, address, value))
    {
        m_bFlashWriteMode = true;
        m_iFlashWriteStep = 0;
        Debug("Entering Flash Write mode");
    }

    if (AdvanceSequence(m_iFlashEraseSequence, FLASH_ERASE_SEQUENCE_LENGTH, m_iFlashEraseStep, address, value))
    {
        m_bFlashEraseMode = true;
        m_iFlashEraseStep = 0;
        Debug("Entering Flash Erase mode");
    }

    if (AdvanceSequence(m_iFlashIDSequence, FLASH_ID_SEQUENCE_LENGTH, m_iFlashIDStep, address, value))
    {
        m_bFlashIDMode = true;
        m_iFlashIDStep = 0;
        Debug("Entering Flash ID mode");
    }
}

bool HomebrewMemoryRule::AdvanceSequence(const int seq[], int len, int &step, u16 address, u8 value)
{
    // Each sequence is address,value pairs. step is the index into seq (0..len-1)
    if (step < 0 || step >= len)
    {
        step = 0;
        return false;
    }

    int expectedAddr = seq[step];
    int expectedVal = seq[step + 1];

    if (address == expectedAddr)
    {
        if (value == expectedVal)
        {
            step += 2;
            if (step >= len)
            {
                // Sequence completed
                return true;
            }
            return false;
        }
        // wrong value - reset
        step = 0;
        return false;
    }

    // wrong address - reset
    step = 0;
    return false;
}

void HomebrewMemoryRule::Reset()
{
    m_bFlashIDMode = false;
    m_iFlashIDStep = 0;
    m_bFlashEraseMode = false;
    m_iFlashEraseStep = 0;
    m_bFlashWriteMode = false;
    m_iFlashWriteStep = 0;
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
