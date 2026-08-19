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

#include "MemoryRule.h"

MemoryRule::MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
{
    m_pMemory = pMemory;
    m_pCartridge = pCartridge;
    m_pInput = pInput;
    InitPointer(m_pTraceLogger);
}

MemoryRule::~MemoryRule()
{
}

void MemoryRule::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void MemoryRule::LogBankSwitchEvent(u16 address, u8 value, u8 flags, u16 auxiliary, bool flags_valid)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    u8 event = TRACE_MAPPER_ROM;
    if (m_pCartridge->GetType() == Cartridge::CartridgeEeprom93C46Mapper &&
        (address == 0x8000 || address == 0xFFFC || (address >= 0x8008 && address < 0x8088)))
        event = TRACE_MAPPER_EEPROM;
    else if (m_pCartridge->GetType() == Cartridge::CartridgeIratahackMapper && address < 0xC000)
        event = TRACE_MAPPER_FLASH;

    if (m_pTraceLogger->IsEventEnabled(TRACE_MAPPER, event))
    {
        GS_Trace_Entry e = {};
        PopulateMapperTraceEntry(e, event, address, value, flags, auxiliary, flags_valid);
        m_pTraceLogger->TraceLog(e);
    }
#else
    UNUSED(address);
    UNUSED(value);
    UNUSED(flags);
    UNUSED(auxiliary);
    UNUSED(flags_valid);
#endif
}

void MemoryRule::LogMapperEvent(u8 event, u16 address, u8 value, u8 flags, u16 auxiliary, bool flags_valid)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    GS_Trace_Entry e = {};
    PopulateMapperTraceEntry(e, event, address, value, flags, auxiliary, flags_valid);
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(address);
    UNUSED(value);
    UNUSED(flags);
    UNUSED(auxiliary);
    UNUSED(flags_valid);
#endif
}

void MemoryRule::PopulateMapperTraceEntry(GS_Trace_Entry& e, u8 event, u16 address,
    u8 value, u8 flags, u16 auxiliary, bool flags_valid)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    e.type = TRACE_MAPPER;
    e.mapper.event = event;
    e.mapper.mapper = (u8)m_pCartridge->GetType();
    e.mapper.address = address;
    e.mapper.value = value;
    e.mapper.flags = flags;
    e.mapper.flags_valid = flags_valid ? 1 : 0;
    if (Has8kBanks())
    {
        for (int i = 0; i < 6; i++)
            e.mapper.banks[i] = (u16)GetBank(i);
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            u16 bank = (u16)GetBank(i);
            e.mapper.banks[i * 2] = bank * 2;
            e.mapper.banks[i * 2 + 1] = bank * 2 + 1;
        }
    }
    e.mapper.ram_bank = (s16)GetRamBank();
    e.mapper.auxiliary = auxiliary;
#else
    UNUSED(e);
    UNUSED(event);
    UNUSED(address);
    UNUSED(value);
    UNUSED(flags);
    UNUSED(auxiliary);
    UNUSED(flags_valid);
#endif
}

void MemoryRule::SaveRam(std::ostream&)
{
}

bool MemoryRule::LoadRam(std::istream&, s32)
{
    return false;
}

void MemoryRule::SetRamChangedCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

bool MemoryRule::PersistedRAM()
{
    return false;
}

size_t MemoryRule::GetRamSize()
{
    return 0;
}

u8* MemoryRule::GetRamBanks()
{
    return NULL;
}

int MemoryRule::GetRamBank()
{
    return 0;
}

u8* MemoryRule::GetPage(int)
{
    return NULL;
}

int MemoryRule::GetBank(int)
{
    return 0;
}

bool MemoryRule::Has8kBanks()
{
    return false;
}

void MemoryRule::SaveState(std::ostream&)
{
}

void MemoryRule::LoadState(std::istream&, int)
{
}
