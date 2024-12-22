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

#include "KoreanMSXSMS8000MemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

KoreanMSXSMS8000MemoryRule::KoreanMSXSMS8000MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

KoreanMSXSMS8000MemoryRule::~KoreanMSXSMS8000MemoryRule()
{
}

u8 KoreanMSXSMS8000MemoryRule::PerformRead(u16 address)
{
    if (address < 0xC000)
    {
        int page = (address >> 13) & 0x03;
        if (page > 5)
        {
            Debug("--> ** Invalid page %d", page);
            return 0xFF;
        }
        return m_pCartridge->GetROM()[m_iPageAddress[page] + (address & 0x1FFF)];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void KoreanMSXSMS8000MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0xC000)
    {
        if (address == 0x8000)
        {
            Debug("--> ** Writing to register $%X %X", address, value);

            if (m_Register == 0xFF)
                value ^= 0x22;

            m_Register = value;

            if (value & 0x80)
            {
                m_iPageAddress[0] = 0x2000 * ((value ^ 0x03) & (m_pCartridge->GetROMBankCount8k() - 1));
                m_iPageAddress[1] = 0x2000 * ((value ^ 0x02) & (m_pCartridge->GetROMBankCount8k() - 1));
            }
            else
            {
                m_iPageAddress[0] = 0x2000 * (0x3C & (m_pCartridge->GetROMBankCount8k() - 1));
                m_iPageAddress[1] = 0x2000 * (0x3C & (m_pCartridge->GetROMBankCount8k() - 1));
            }

            m_iPageAddress[2] = 0x2000 * ((value ^ 0x01) & (m_pCartridge->GetROMBankCount8k() - 1));
            m_iPageAddress[3] = 0x2000 * ((value ^ 0x00) & (m_pCartridge->GetROMBankCount8k() - 1));
            m_iPageAddress[4] = 0x2000 * ((value ^ 0x03) & (m_pCartridge->GetROMBankCount8k() - 1));
            m_iPageAddress[5] = 0x2000 * ((value ^ 0x02) & (m_pCartridge->GetROMBankCount8k() - 1));
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

void KoreanMSXSMS8000MemoryRule::Reset()
{
    m_Register = 0;
    m_iPageAddress[0] = 0x2000 * (0x3C);
    m_iPageAddress[1] = 0x2000 * (0x3C);
    m_iPageAddress[2] = 0x2000 * (m_Register ^ 0x01);
    m_iPageAddress[3] = 0x2000 * (m_Register ^ 0x00);
    m_iPageAddress[4] = 0x2000 * (m_Register ^ 0x03);
    m_iPageAddress[5] = 0x2000 * (m_Register ^ 0x02);
}

u8* KoreanMSXSMS8000MemoryRule::GetPage(int index)
{
    return 0;
    // switch (index)
    // {
    //     case 0:
    //     case 1:
    //         return m_pCartridge->GetROM() + (index * 0x4000);
    //     case 2:
    //         return m_pCartridge->GetROM() + m_iMapperSlot2Address;
    //     default:
    //         return NULL;
    // }
}

int KoreanMSXSMS8000MemoryRule::GetBank(int index)
{
    return 0;

    // switch (index)
    // {
    //     case 0:
    //     case 1:
    //         return index;
    //     case 2:
    //         return m_iMapperSlot2;
    //     default:
    //         return 0;
    // }
}

void KoreanMSXSMS8000MemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.write(reinterpret_cast<const char*> (&m_Register), sizeof(m_Register));
}

void KoreanMSXSMS8000MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_iPageAddress), sizeof(m_iPageAddress));
    stream.read(reinterpret_cast<char*> (&m_Register), sizeof(m_Register));
}
