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

#include "KoreanBFFCMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanBFFCMemoryRule::KoreanBFFCMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanBFFCMemoryRule::~KoreanBFFCMemoryRule()
{
}

u8 KoreanBFFCMemoryRule::PerformRead(u16 address)
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

void KoreanBFFCMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address == 0xBFFC)
    {
        Debug("--> ** Writing to register $%X %X", address, value);

        uint8_t lower = 0x00;
        uint8_t upper = 0x00;

        switch(value & 0xC0)
        {
            case 0x00:
                lower = value & 0x3E;
                upper = (value & 0x3E) | 1;
                m_iPage[0] = (lower * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[1] = m_iPage[0] + 1;
                m_iPage[2] = (upper * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[3] = m_iPage[2] + 1;
                m_iPage[4] = ((0x3F * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[5] = m_iPage[4];
                break;
            case 0x40:
                lower = value & 0x3F;
                upper = value & 0x3F;
                m_iPage[0] = (lower * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[1] = m_iPage[0] + 1;
                m_iPage[2] = (upper * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[3] = m_iPage[2] + 1;
                m_iPage[4] = ((0x3F * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[5] = m_iPage[4];
                break;
            case 0x80:
                lower = 0x20;
                upper = value & 0x3F;
                m_iPage[0] = (lower * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[1] = m_iPage[0] + 1;
                m_iPage[2] = (upper * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[3] = m_iPage[2] + 1;
                m_iPage[4] = ((0x3F * 2) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[5] = m_iPage[4];
                break;
            case 0xC0:
                lower = 0x20;
                upper = value & 0x3F;
                m_iPage[0] = (lower * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[1] = m_iPage[0] + 1;
                m_iPage[2] = (upper * 2) & (m_pCartridge->GetROMBankCount8k() - 1);
                m_iPage[3] = m_iPage[2] + 1;
                m_iPage[4] = m_iPage[3];
                m_iPage[5] = m_iPage[2];
                break;
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

void KoreanBFFCMemoryRule::Reset()
{
    for (int i = 0; i < 6; i++)
    {
        m_iPage[i] = i;
        m_iPageAddress[i] = 0x2000 * m_iPage[i];
    }
}

u8* KoreanBFFCMemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iPageAddress[index];
}

int KoreanBFFCMemoryRule::GetBank(int index)
{
    return m_iPage[index];
}

bool KoreanBFFCMemoryRule::Has8kBanks()
{
    return true;
}

void KoreanBFFCMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPage), sizeof(m_iPage));
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
}

void KoreanBFFCMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPage), sizeof(m_iPage));
    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
}
