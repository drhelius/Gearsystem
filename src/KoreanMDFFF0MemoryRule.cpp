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

#include "KoreanMDFFF0MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanMDFFF0MemoryRule::KoreanMDFFF0MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput)
    : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanMDFFF0MemoryRule::~KoreanMDFFF0MemoryRule()
{
}

u8 KoreanMDFFF0MemoryRule::PerformRead(u16 address)
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

void KoreanMDFFF0MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address)
    {
        case 0xFFF0:
        {
            Debug("--> ** Writing to register 0xFFF0 %X", value);
            m_iRegister[0] = value;
            m_iRegister[1] = 0;
            m_iRegister[2] = 1;
            int bank = value << 2;
            m_iPage[0] = bank & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[1] = (bank + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[2] = (bank + 2) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[3] = (bank + 3) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[4] = bank & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[5] = (bank + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
            break;
        }
        case 0xFFFE:
        {
            Debug("--> ** Writing to register 0xFFFE %X", value);
            m_iRegister[1] = value;
            int bank = m_iRegister[0] << 2;
            m_iPage[2] = (bank + ((value & 0x0F) << 1)) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[3] = (bank + ((value & 0x0F) << 1) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
            break;
        }
        case 0xFFFF:
        {
            Debug("--> ** Writing to register 0xFFFF %X", value);
            m_iRegister[2] = value;
            int bank = m_iRegister[0] << 2;
            m_iPage[4] = (bank + ((value & 0x0F) << 1)) & (m_pCartridge->GetROMBankCount8k() - 1);
            m_iPage[5] = (bank + ((value & 0x0F) << 1) + 1) & (m_pCartridge->GetROMBankCount8k() - 1);
            break;
        }
    }

    m_iPageAddress[0] = 0x2000 * m_iPage[0];
    m_iPageAddress[1] = 0x2000 * m_iPage[1];
    m_iPageAddress[2] = 0x2000 * m_iPage[2];
    m_iPageAddress[3] = 0x2000 * m_iPage[3];
    m_iPageAddress[4] = 0x2000 * m_iPage[4];
    m_iPageAddress[5] = 0x2000 * m_iPage[5];

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

void KoreanMDFFF0MemoryRule::Reset()
{
    m_iPage[0] = 0;
    m_iPage[1] = 1;
    m_iPage[2] = 2;
    m_iPage[3] = 3;
    m_iPage[4] = 0;
    m_iPage[5] = 1;

    for (int i = 0; i < 6; i++)
        m_iPageAddress[i] = 0x2000 * m_iPage[i];

    m_iRegister[0] = 0;
    m_iRegister[1] = 0;
    m_iRegister[2] = 1;
}

u8* KoreanMDFFF0MemoryRule::GetPage(int index)
{
    return m_pCartridge->GetROM() + m_iPageAddress[index];
}

int KoreanMDFFF0MemoryRule::GetBank(int index)
{
    return m_iPage[index];
}

bool KoreanMDFFF0MemoryRule::Has8kBanks()
{
    return true;
}

void KoreanMDFFF0MemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPage), sizeof(m_iPage));
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.write(reinterpret_cast<const char*> (m_iRegister), sizeof(m_iRegister));
}

void KoreanMDFFF0MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPage), sizeof(m_iPage));
    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.read(reinterpret_cast<char*> (m_iRegister), sizeof(m_iRegister));
}
