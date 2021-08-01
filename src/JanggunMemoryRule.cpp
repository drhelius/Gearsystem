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

#include "JanggunMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"
#include "Input.h"

JanggunMemoryRule::JanggunMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    Reset();
}

JanggunMemoryRule::~JanggunMemoryRule()
{
}

u8 JanggunMemoryRule::PerformRead(u16 address)
{
    u8 ret = 0;

    if (address < 0x4000)
    {
        // First 16KB (fixed)
        u8* pROM = m_pCartridge->GetROM();
        ret = pROM[address];
    }
    else if (address < 0x6000)
    {
        // ROM slot 0
        u8* pROM = m_pCartridge->GetROM();
        ret = pROM[(address & 0x1FFF) + m_iMapperSlotAddress[0]];
    }
    else if (address < 0x8000)
    {
        // ROM slot 1
        u8* pROM = m_pCartridge->GetROM();
        ret = pROM[(address & 0x1FFF) + m_iMapperSlotAddress[1]];
    }
    else if (address < 0xA000)
    {
        // ROM slot 2
        u8* pROM = m_pCartridge->GetROM();
        ret = pROM[(address & 0x1FFF) + m_iMapperSlotAddress[2]];
    }
    else if (address < 0xC000)
    {
        // ROM slot 3
        u8* pROM = m_pCartridge->GetROM();
        ret = pROM[(address & 0x1FFF) + m_iMapperSlotAddress[3]];

    }
    else
    {
        // RAM + RAM mirror
        ret = m_pMemory->Retrieve(address);
    }

    if (m_bReverseFlags[(address >> 14) & 0x03])
    {
        ret = ReverseBits(ret);
    }

    return ret;
}

void JanggunMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address)
    {
        case 0x4000:
        {
            // page 0
            m_iMapperSlot[0] = value & 0x3F;
            m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x2000;
            break;
        }
        case 0x6000:
        {
            // page 1
            m_iMapperSlot[1] = value & 0x3F;
            m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x2000;
            break;
        }
        case 0x8000:
        {
            // page 2
            m_iMapperSlot[2] = value & 0x3F;
            m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x2000;
            break;
        }
        case 0xA000:
        {
            // page 3
            m_iMapperSlot[3] = value & 0x3F;
            m_iMapperSlotAddress[3] = m_iMapperSlot[3] * 0x2000;
            break;
        }
        default:
        {
            if (address >= 0xC000 && address < 0xE000)
            {
                m_pMemory->Load(address, value);
                m_pMemory->Load(address + 0x2000, value);
            }
            else if (address >= 0xE000)
            {
                m_pMemory->Load(address, value);
                m_pMemory->Load(address - 0x2000, value);

                switch (address)
                {
                    case 0xFFFE:
                    {
                        m_iMapperSlot[0] = (value & 0x3F) << 1;
                        m_iMapperSlot[1] = ((value & 0x3F) + 1) << 1;
                        m_iMapperSlotAddress[0] = m_iMapperSlot[0] * 0x2000;
                        m_iMapperSlotAddress[1] = m_iMapperSlot[1] * 0x2000;
                        m_bReverseFlags[1] = IsSetBit(value, 6);
                        break;
                    }
                    case 0xFFFF:
                    {
                        m_iMapperSlot[2] = (value & 0x3F) << 1;
                        m_iMapperSlot[3] = ((value & 0x3F) + 1) << 1;
                        m_iMapperSlotAddress[2] = m_iMapperSlot[2] * 0x2000;
                        m_iMapperSlotAddress[3] = m_iMapperSlot[3] * 0x2000;
                        m_bReverseFlags[2] = IsSetBit(value, 6);
                        break;
                    }
                }
            }
        }
    }
}

void JanggunMemoryRule::Reset()
{
    for (int i = 0; i < 4; i++)
    {
        m_iMapperSlot[i] = i;
        m_iMapperSlotAddress[i] = i * 0x4000;
        m_bReverseFlags[i] = false;
    }
}

u8* JanggunMemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            return m_pCartridge->GetROM() + m_iMapperSlotAddress[index];
        default:
            return NULL;
    }
}

int JanggunMemoryRule::GetBank(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            return m_iMapperSlot[index];
        default:
            return 0;
    }
}

void JanggunMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
}

void JanggunMemoryRule::LoadState(std::istream& stream)
{
    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
}
