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

#include "Korean0000XORFFMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

Korean0000XORFFMemoryRule::Korean0000XORFFMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

Korean0000XORFFMemoryRule::~Korean0000XORFFMemoryRule()
{
}

u8 Korean0000XORFFMemoryRule::PerformRead(u16 address)
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

void Korean0000XORFFMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0xC000)
    {
        if (address == 0x0000)
        {
            Debug("--> ** Writing to register $%X %X", address, value);

            if ((value & 0xF0) == 0xF0)
            {
                m_iPage[2] = 2;
                m_iPage[3] = 3;
                m_iPage[4] = 2;
                m_iPage[5] = 3;
            }
            else
            {
                int segment = ((value ^ 0xF0) & 0xF0) >> 2;
                m_iPage[2] = (segment + 0) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[3] = (segment + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[4] = (segment + 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[5] = (segment + 3) & (m_pCartridge->GetROMBankCount8k() - 1);
            }

            m_iPageAddress[2] = 0x2000 * m_iPage[2];
            m_iPageAddress[3] = 0x2000 * m_iPage[3];
            m_iPageAddress[4] = 0x2000 * m_iPage[4];
            m_iPageAddress[5] = 0x2000 * m_iPage[5];
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

void Korean0000XORFFMemoryRule::Reset()
{
    for (int i = 0; i < 6; i++)
    {
        m_iPage[i] = i;
        m_iPageAddress[i] = 0x2000 * m_iPage[i];
    }
}

u8* Korean0000XORFFMemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iPageAddress[index];
}

int Korean0000XORFFMemoryRule::GetBank(int index)
{
    return m_iPage[index];
}

bool Korean0000XORFFMemoryRule::Has8kBanks()
{
    return true;
}

void Korean0000XORFFMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPage), sizeof(m_iPage));
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
}

void Korean0000XORFFMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPage), sizeof(m_iPage));
    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
}
