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

inline u8 Memory::Read(u16 address)
{
    #ifndef GEARSYSTEM_DISABLE_DISASSEMBLER
    CheckBreakpoints(address, false);
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
    #ifndef GEARSYSTEM_DISABLE_DISASSEMBLER
    CheckBreakpoints(address, true);
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

inline void Memory::Load(u16 address, u8 value)
{
    m_pMap[address] = value;
}

inline Memory::stDisassembleRecord** Memory::GetDisassembledMemoryMap()
{
    return m_pDisassembledMap;
}

inline Memory::stDisassembleRecord** Memory::GetDisassembledROMMemoryMap()
{
    return m_pDisassembledROMMap;
}

#endif	/* MEMORY_INLINE_H */

