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

#include "IratahackMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

// The Iratahack mapper is a 512KB flash multi-cart with:
// 4 "game slots" of 128KB each (selected via register 0xFFFE)
// Within each game slot, slot 2 (0x8000–0xBFFF) is further
// bankable with 8 × 16KB banks (selected via register 0xFFFF)
// Flash chip emulation (ID, erase, write) for save data

IratahackMemoryRule::IratahackMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput) : MemoryRule(pMemory, pCartridge, pInput)
{
    m_pFlash = new u8[0x80000];
    Reset();
}

IratahackMemoryRule::~IratahackMemoryRule()
{
    SafeDeleteArray(m_pFlash);
}

u8 IratahackMemoryRule::PerformRead(u16 address)
{
    if (address < 0x4000)
    {
        if (m_iFlashMode == FlashSeqID)
        {
            switch (address)
            {
                case 0x0000:
                    return 0xBF;
                case 0x0001:
                    return 0xB7;
                default:
                    return 0xFF;
            }
        }
        // Slot 0
        return m_pFlash[address + m_iMapperSlotAddress[0]];
    }
    else if (address < 0x8000)
    {
        // Slot 1
        return m_pFlash[(address - 0x4000) + m_iMapperSlotAddress[1]];
    }
    else if (address < 0xC000)
    {
        // Slot 2
        return m_pFlash[(address - 0x8000) + m_iMapperSlotAddress[2]];
    }
    else
    {
        // RAM + RAM mirror
        return m_pMemory->Retrieve(address);
    }
}

void IratahackMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address)
    {
        case 0x5555:
        case 0x2AAA:
            ProcessFlashAccess(address, value);
            break;
        default:
            if (address < 0xC000)
            {
                if (m_iFlashMode == FlashSeqID)
                {
                    Debug("Exiting Flash ID mode");
                    ResetFlashState();
                }
                else if (m_iFlashMode == FlashSeqErase)
                {
                    if (value == 0x30)
                    {
                        Debug("Erasing flash sector");
                        u32 sectorBase = m_iMapperSlotAddress[2] + (address - 0x8000);
                        sectorBase &= ~0x3FFF;
                        if (sectorBase + 0x4000 <= 0x80000u)
                        {
                            memset(m_pFlash + sectorBase, 0xFF, 0x4000);
                        }
                    }
                    ResetFlashState();
                }
                else if (m_iFlashMode == FlashSeqWrite)
                {
                    u32 flashIndex;
                    if (address < 0x4000)
                        flashIndex = address + m_iMapperSlotAddress[0];
                    else if (address < 0x8000)
                        flashIndex = (address - 0x4000) + m_iMapperSlotAddress[1];
                    else
                        flashIndex = (address - 0x8000) + m_iMapperSlotAddress[2];

                    if (flashIndex < 0x80000u)
                    {
                        m_pFlash[flashIndex] = value;
                    }
                    ResetFlashState();
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

                switch (address)
                {
                    case 0xFFFE:
                    {
                        m_iGameSlot = value & 3;
                        m_iMapperSlotAddress[0] = (m_iGameSlot << 17);
                        m_iMapperSlotAddress[1] = (m_iGameSlot << 17) + 0x4000;
                        m_iMapperSlotAddress[2] = (m_iGameSlot << 17) + (m_iMapperSlot[2] * 0x4000);
                        break;
                    }
                    case 0xFFFF:
                    {
                        m_iMapperSlot[2] = value & 7;
                        m_iMapperSlotAddress[2] = (m_iGameSlot << 17) + (m_iMapperSlot[2] * 0x4000);
                        break;
                    }
                }
            }
            break;
    }
}

void IratahackMemoryRule::ProcessFlashAccess(u16 address, u8 value)
{
    if (AdvanceSequence(FlashSeqWrite, m_iFlashStep, address, value))
    {
        m_iFlashMode = FlashSeqWrite;
        Debug("Entering Flash Write mode");
    }

    if (AdvanceSequence(FlashSeqErase, m_iFlashStep, address, value))
    {
        m_iFlashMode = FlashSeqErase;
        Debug("Entering Flash Erase mode");
    }

    if (AdvanceSequence(FlashSeqID, m_iFlashStep, address, value))
    {
        m_iFlashMode = FlashSeqID;
        Debug("Entering Flash ID mode");
    }
}

const int IratahackMemoryRule::kFlashSeqs[FlashSeqCount][11] =
{
    { 0x5555, 0xAA, 0x2AAA, 0x55, 0x5555, 0x90, -1 },
    { 0x5555, 0xAA, 0x2AAA, 0x55, 0x5555, 0x80, 0x5555, 0xAA, 0x2AAA, 0x55, -1 },
    { 0x5555, 0xAA, 0x2AAA, 0x55, 0x5555, 0xA0, -1 },
};

bool IratahackMemoryRule::AdvanceSequence(int seqIndex, int* steps, u16 address, u8 value)
{
    int step = steps[seqIndex];

    if (kFlashSeqs[seqIndex][step] == -1)
    {
        steps[seqIndex] = 0;
        return false;
    }

    if ((address == kFlashSeqs[seqIndex][step]) && (value == kFlashSeqs[seqIndex][step + 1]))
    {
        step += 2;
        steps[seqIndex] = (kFlashSeqs[seqIndex][step] == -1) ? 0 : step;
        return (kFlashSeqs[seqIndex][step] == -1);
    }

    steps[seqIndex] = 0;
    return false;
}

void IratahackMemoryRule::Reset()
{
    m_iGameSlot = 0;

    m_iMapperSlot[0] = 0;
    m_iMapperSlot[1] = 1;
    m_iMapperSlot[2] = 0;

    m_iMapperSlotAddress[0] = 0x0000;
    m_iMapperSlotAddress[1] = 0x4000;
    m_iMapperSlotAddress[2] = 0x0000;

    ResetFlashState();

    int romSize = m_pCartridge->GetROMSize();
    u8* pROM = m_pCartridge->GetROM();
    int copySize = (romSize < 0x80000) ? romSize : 0x80000;
    memcpy(m_pFlash, pROM, copySize);
    if (copySize < 0x80000)
        memset(m_pFlash + copySize, 0xFF, 0x80000 - copySize);
}

void IratahackMemoryRule::ResetFlashState()
{
    m_iFlashMode = FlashSeqCount;
    m_iFlashStep[0] = 0;
    m_iFlashStep[1] = 0;
    m_iFlashStep[2] = 0;
}

void IratahackMemoryRule::SaveRam(std::ostream & file)
{
    Debug("IratahackMemoryRule save RAM...");

    for (int i = 0; i < 0x80000; i++)
    {
        u8 ram_byte = m_pFlash[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Debug("IratahackMemoryRule save RAM done");
}

bool IratahackMemoryRule::LoadRam(std::istream & file, s32 fileSize)
{
    Debug("IratahackMemoryRule load RAM...");

    if ((fileSize > 0) && (fileSize != 0x80000))
    {
        Log("IratahackMemoryRule incorrect size. Expected: %d Found: %d", 0x80000, fileSize);
        return false;
    }

    for (int i = 0; i < 0x80000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pFlash[i] = ram_byte;
    }

    Debug("IratahackMemoryRule load RAM done");

    return true;
}

bool IratahackMemoryRule::PersistedRAM()
{
    return true;
}

size_t IratahackMemoryRule::GetRamSize()
{
    return 0x80000;
}

u8* IratahackMemoryRule::GetRamBanks()
{
    return m_pFlash;
}

u8* IratahackMemoryRule::GetPage(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
            return m_pFlash + m_iMapperSlotAddress[index];
        default:
            return NULL;
    }
}

int IratahackMemoryRule::GetBank(int index)
{
    switch (index)
    {
        case 0:
        case 1:
        case 2:
            return m_iMapperSlot[index];
        default:
            return 0;
    }
}

void IratahackMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*> (m_pFlash), 0x80000);
    stream.write(reinterpret_cast<const char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.write(reinterpret_cast<const char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.write(reinterpret_cast<const char*> (&m_iGameSlot), sizeof(m_iGameSlot));
    stream.write(reinterpret_cast<const char*> (&m_iFlashMode), sizeof(m_iFlashMode));
    stream.write(reinterpret_cast<const char*> (m_iFlashStep), sizeof(m_iFlashStep));
}

void IratahackMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pFlash), 0x80000);
    stream.read(reinterpret_cast<char*> (m_iMapperSlot), sizeof(m_iMapperSlot));
    stream.read(reinterpret_cast<char*> (m_iMapperSlotAddress), sizeof(m_iMapperSlotAddress));
    stream.read(reinterpret_cast<char*> (&m_iGameSlot), sizeof(m_iGameSlot));
    stream.read(reinterpret_cast<char*> (&m_iFlashMode), sizeof(m_iFlashMode));
    stream.read(reinterpret_cast<char*> (m_iFlashStep), sizeof(m_iFlashStep));
}
