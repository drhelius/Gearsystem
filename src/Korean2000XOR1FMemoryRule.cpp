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

#include "Korean2000XOR1FMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

Korean2000XOR1FMemoryRule::Korean2000XOR1FMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

Korean2000XOR1FMemoryRule::~Korean2000XOR1FMemoryRule()
{
}

u8 Korean2000XOR1FMemoryRule::PerformRead(u16 address)
{
    if (address < 0xC000)
    {
        int slot = (address >> 13) & 0x07;
        if (slot > 5)
        {
            Debug("--> ** Invalid slot %d", slot);
            return 0xFF;
        }
        return m_pCartridge->GetROM()[m_iPageAddress[slot] + (address & 0x1FFF)];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void Korean2000XOR1FMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0xC000)
    {
        if ((address & 0x6000) == 0x2000)
        {
            Debug("--> ** Writing to register $%X %X", address, value);

            value = ((value ^ 0x1F) & (m_pCartridge->GetROMBankCount8k() - 1)) ^ 0x1F;

            m_iPageAddress[2] = 0x2000 * (value ^ 0x1F);
            m_iPageAddress[3] = 0x2000 * (value ^ 0x1E);
            m_iPageAddress[4] = 0x2000 * (value ^ 0x1D);
            m_iPageAddress[5] = 0x2000 * (value ^ 0x1C);
        }
        else
        {
            Debug("--> ** Attempting to write on ROM address $%X %X", address, value);
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

void Korean2000XOR1FMemoryRule::Reset()
{
    m_iPageAddress[0] = 0x2000 * 0;
    m_iPageAddress[1] = 0x2000 * 0;
    m_iPageAddress[2] = 0x2000 * 0x60;
    m_iPageAddress[3] = 0x2000 * 0x61;
    m_iPageAddress[4] = 0x2000 * 0x62;
    m_iPageAddress[5] = 0x2000 * 0x63;
}

u8* Korean2000XOR1FMemoryRule::GetPage(int)
{
    return m_pCartridge->GetROM();
}

int Korean2000XOR1FMemoryRule::GetBank(int)
{
    return 0;
}

void Korean2000XOR1FMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
}

void Korean2000XOR1FMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
}