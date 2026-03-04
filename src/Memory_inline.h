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

#ifndef MEMORY_INLINE_H
#define	MEMORY_INLINE_H

#include "Processor.h"

inline u8 Memory::Read(u16 address)
{
    #ifndef GS_DISABLE_DISASSEMBLER
    m_pProcessor->CheckMemoryBreakpoints(Processor::GS_BREAKPOINT_TYPE_ROMRAM, address, true);
    #endif

    if (m_MediaSlot == m_DesiredMediaSlot)
        return m_pCurrentMemoryRule->PerformRead(address);

    if (m_MediaSlot == BiosSlot)
        return m_pBootromMemoryRule->PerformRead(address);

    if (address < 0xC000)
        return 0xFF;
    else
        return m_pBootromMemoryRule->PerformRead(address);
}

inline void Memory::Write(u16 address, u8 value)
{
    #ifndef GS_DISABLE_DISASSEMBLER
    m_pProcessor->CheckMemoryBreakpoints(Processor::GS_BREAKPOINT_TYPE_ROMRAM, address, false);
    #endif

    if (m_MediaSlot == m_DesiredMediaSlot)
        m_pCurrentMemoryRule->PerformWrite(address, value);
    else if (m_MediaSlot == BiosSlot)
        m_pBootromMemoryRule->PerformWrite(address, value);
    else if (address >= 0xC000)
        m_pBootromMemoryRule->PerformWrite(address, value);
}

inline u8 Memory::Retrieve(u16 address)
{
    return m_pMap[address];
}

inline u8 Memory::DebugRetrieve(u16 address)
{
    if (m_MediaSlot == BiosSlot && address < 0xC000)
        return m_pBootromMemoryRule->PerformRead(address);
    return m_pMap[address];
}

inline void Memory::Load(u16 address, u8 value)
{
    m_pMap[address] = value;
}

inline u32 Memory::GetPhysicalAddress(u16 address)
{
    if (address >= 0xC000)
        return (u32)address;

    MemoryRule* rule = (m_MediaSlot == BiosSlot) ? m_pBootromMemoryRule : m_pCurrentMemoryRule;

    if (rule->Has8kBanks())
    {
        int slot = (address >> 13) & 0x07;
        int bank = rule->GetBank(slot);
        return (u32)(0x2000 * bank) + (address & 0x1FFF);
    }
    else
    {
        int slot = (address >> 14) & 0x03;
        int bank = rule->GetBank(slot);
        return (u32)(0x4000 * bank) + (address & 0x3FFF);
    }
}

inline u8 Memory::GetBank(u16 address)
{
    if (address >= 0xC000)
        return 0;

    MemoryRule* rule = (m_MediaSlot == BiosSlot) ? m_pBootromMemoryRule : m_pCurrentMemoryRule;

    if (rule->Has8kBanks())
    {
        int slot = (address >> 13) & 0x07;
        return (u8)rule->GetBank(slot);
    }
    else
    {
        int slot = (address >> 14) & 0x03;
        return (u8)rule->GetBank(slot);
    }
}

inline GS_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address)
{
    u32 physical_address = GetPhysicalAddress(address);
    bool rom = (address < 0xC000);

    if (rom)
    {
        if (physical_address >= MAX_ROM_SIZE)
            return NULL;
        return m_pDisassembledROMMap[physical_address];
    }
    else
    {
        return m_pDisassembledMap[physical_address];
    }
}

inline GS_Disassembler_Record* Memory::GetDisassemblerRecord(u16 address, u8 bank)
{
    if (address >= 0xC000)
        return m_pDisassembledMap[address];

    MemoryRule* rule = (m_MediaSlot == BiosSlot) ? m_pBootromMemoryRule : m_pCurrentMemoryRule;

    u32 physical_address;
    if (rule->Has8kBanks())
        physical_address = (u32)(0x2000 * bank) + (address & 0x1FFF);
    else
        physical_address = (u32)(0x4000 * bank) + (address & 0x3FFF);

    if (physical_address >= MAX_ROM_SIZE)
        return NULL;
    return m_pDisassembledROMMap[physical_address];
}

inline GS_Disassembler_Record** Memory::GetAllDisassemblerRecords()
{
    return m_pDisassembledROMMap;
}

#endif	/* MEMORY_INLINE_H */

