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

#include "KoreanFFFEMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanFFFEMemoryRule::KoreanFFFEMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanFFFEMemoryRule::~KoreanFFFEMemoryRule()
{
}

u8 KoreanFFFEMemoryRule::PerformRead(u16 address)
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

void KoreanFFFEMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address == 0xFFFE)
    {
        Debug("--> ** Writing to register $%X %X", address, value);

        if ((value & 0x40) == 0x40)
        {
            m_iPage[0] = (((value & 0x1E) * 2) + 0) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[1] = (((value & 0x1E) * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[2] = ((((value & 0x1E) + 1) * 2) + 0) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[3] = ((((value & 0x1E) + 1) * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);

        }
        else
        {
            m_iPage[0] = 0;
            m_iPage[1] = 1;
            m_iPage[2] = (((value & 0x1F) * 2) + 0) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[3] = (((value & 0x1F) * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
        }

        if ((value & 0x60) == 0x20) {
            m_iPage[4] = (((value & 0x1F) * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[5] = (((value & 0x1F) * 2) + 0) & (m_pCartridge->GetROMBankCount8k() - 1);
        }
        else
        {
            m_iPage[4] = 0x3F;
            m_iPage[5] = 0x3F;
        }

        m_iPageAddress[0] = 0x2000 * m_iPage[0];
        m_iPageAddress[1] = 0x2000 * m_iPage[1];
        m_iPageAddress[2] = 0x2000 * m_iPage[2];
        m_iPageAddress[3] = 0x2000 * m_iPage[3];
        m_iPageAddress[4] = 0x2000 * m_iPage[4];
        m_iPageAddress[5] = 0x2000 * m_iPage[5];

        return;
    }

    if (address >= 0xC000)
    {
        if (address < 0xE000)
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
}

void KoreanFFFEMemoryRule::Reset()
{
    m_iPage[0] = 0;
    m_iPage[1] = 1;
    m_iPage[2] = 2;
    m_iPage[3] = 3;
    m_iPage[4] = 0x3F;
    m_iPage[5] = 0x3F;

    for (int i = 0; i < 6; i++)
        m_iPageAddress[i] = 0x2000 * m_iPage[i];
}

u8* KoreanFFFEMemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iPageAddress[index];
}

int KoreanFFFEMemoryRule::GetBank(int index)
{
    return m_iPage[index];
}

bool KoreanFFFEMemoryRule::Has8kBanks()
{
    return true;
}

void KoreanFFFEMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPage), sizeof(m_iPage));
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
}

void KoreanFFFEMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPage), sizeof(m_iPage));
    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
}
