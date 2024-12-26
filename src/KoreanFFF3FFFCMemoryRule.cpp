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

#include "KoreanFFF3FFFCMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanFFF3FFFCMemoryRule::KoreanFFF3FFFCMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanFFF3FFFCMemoryRule::~KoreanFFF3FFFCMemoryRule()
{
}

u8 KoreanFFF3FFFCMemoryRule::PerformRead(u16 address)
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

void KoreanFFF3FFFCMemoryRule::PerformWrite(u16 address, u8 value)
{
    u16 address_assumed = address | 0x4000;

    if ((address_assumed == 0xFFF3) || (address_assumed == 0xFFFC))
    {
        Debug("--> ** Writing to register $%X %X", address, value);

        if (address_assumed == 0xFFF3)
            m_iRegister[0] = value;
        else if (address_assumed == 0xFFFC)
            m_iRegister[1] = value;

        int page0 = ((m_iRegister[1] & 0x10) * 8) + ((m_iRegister[0] & 0x3E) * 2);
        int page1 = page0 + 1;
        int page2 = page0 + 2;
        int page3 = page0 + 3;
        int odd = m_iRegister[0] & 0x01;
        int mask = m_pCartridge->GetROMBankCount8k() - 1;

        switch (m_iRegister[1] & 0xE0)
        {
            case 0x00:
                m_iPage[0] = (odd ? page2 : page0) & mask;
                m_iPage[1] = (odd ? page3 : page1) & mask;
                m_iPage[2] = (odd ? page2 : page0) & mask;
                m_iPage[3] = (odd ? page3 : page1) & mask;
                m_iPage[4] = 0xFF & mask;
                m_iPage[5] = 0xFF & mask;
                break;
            case 0x20:
                m_iPage[0] = page0 & mask;
                m_iPage[1] = page1 & mask;
                m_iPage[2] = page2 & mask;
                m_iPage[3] = page3 & mask;
                m_iPage[4] = 0xFF & mask;
                m_iPage[5] = 0xFF & mask;
                break;
            case 0x40:
                m_iPage[0] = 0x80 & mask;
                m_iPage[1] = 0x81 & mask;
                m_iPage[2] = (odd ? page2 : page0) & mask;
                m_iPage[3] = (odd ? page3 : page1) & mask;
                m_iPage[4] = 0xFF & mask;
                m_iPage[5] = 0xFF & mask;
                break;
            case 0x60:
                m_iPage[0] = 0x80 & mask;
                m_iPage[1] = 0x81 & mask;
                m_iPage[2] = 0xFE & mask;
                m_iPage[3] = 0xFF & mask;
                m_iPage[4] = (odd ? page2 : page0) & mask;
                m_iPage[5] = (odd ? page3 : page1) & mask;
                break;
            case 0x80:
                m_iPage[0] = 0x80 & mask;
                m_iPage[1] = 0x81 & mask;
                m_iPage[2] = (odd ? page2 : page0) & mask;
                m_iPage[3] = (odd ? page3 : page1) & mask;
                m_iPage[4] = (odd ? page3 : page1) & mask;
                m_iPage[5] = (odd ? page2 : page0) & mask;
                break;
            case 0xA0:
                m_iPage[0] = 0x80 & mask;
                m_iPage[1] = 0x81 & mask;
                m_iPage[2] = (odd ? page2 : page0) & mask;
                m_iPage[3] = (odd ? page3 : page1) & mask;
                m_iPage[4] = (odd ? page0 : page2) & mask;
                m_iPage[5] = (odd ? page1 : page3) & mask;
                break;
            default:
                m_iPage[0] = 0xFF & mask;
                m_iPage[1] = 0xFF & mask;
                m_iPage[2] = 0xFF & mask;
                m_iPage[3] = 0xFF & mask;
                m_iPage[4] = 0xFF & mask;
                m_iPage[5] = 0xFF & mask;
                break;
        }

        m_iPageAddress[0] = 0x2000 * m_iPage[0];
        m_iPageAddress[1] = 0x2000 * m_iPage[1];
        m_iPageAddress[2] = 0x2000 * m_iPage[2];
        m_iPageAddress[3] = 0x2000 * m_iPage[3];
        m_iPageAddress[4] = 0x2000 * m_iPage[4];
        m_iPageAddress[5] = 0x2000 * m_iPage[5];
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

void KoreanFFF3FFFCMemoryRule::Reset()
{
    m_iPage[0] = 0;
    m_iPage[1] = 1;
    m_iPage[2] = 0;
    m_iPage[3] = 1;
    m_iPage[4] = 0xFF & (m_pCartridge->GetROMBankCount8k() - 1);
    m_iPage[5] = 0xFF & (m_pCartridge->GetROMBankCount8k() - 1);

    for (int i = 0; i < 6; i++)
        m_iPageAddress[i] = 0x2000 * m_iPage[i];

    for (int i = 0; i < 2; i++)
        m_iRegister[i] = 0;
}

u8* KoreanFFF3FFFCMemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iPageAddress[index];
}

int KoreanFFF3FFFCMemoryRule::GetBank(int index)
{
    return m_iPage[index];
}

bool KoreanFFF3FFFCMemoryRule::Has8kBanks()
{
    return true;
}

void KoreanFFF3FFFCMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPage), sizeof(m_iPage));
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.write(reinterpret_cast<const char*> (m_iRegister), sizeof(m_iRegister));
}

void KoreanFFF3FFFCMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPage), sizeof(m_iPage));
    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.read(reinterpret_cast<char*> (m_iRegister), sizeof(m_iRegister));
}