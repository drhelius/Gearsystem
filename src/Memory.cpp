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

#include <iostream>
#include <iomanip>
#include <fstream>
#include "Memory.h"
#include "Processor.h"
#include "Cartridge.h"
#include "common.h"

Memory::Memory(Cartridge* pCartridge)
{
    m_pCartridge = pCartridge;
    InitPointer(m_pProcessor);
    InitPointer(m_pMap);
    InitPointer(m_pCurrentMemoryRule);
    InitPointer(m_pDisassembledMap);
    InitPointer(m_pDisassembledROMMap);
    InitPointer(m_pBootromSMS);
    InitPointer(m_pBootromGG);
    m_bBootromSMSEnabled = false;
    m_bBootromGGEnabled = false;
    m_bBootromSMSLoaded = false;
    m_bBootromGGLoaded = false;
    m_MediaSlot = CartridgeSlot;
    m_DesiredMediaSlot = CartridgeSlot;
    m_StoredMediaSlot = ExpansionSlot;
    m_bGameGear = false;
    m_iBootromBankCountSMS = 1;
    m_iBootromBankCountGG = 1;
    m_iBootromSMSSize = 0;
    m_iBootromGGSize = 0;
    m_bIOEnabled = true;
}

Memory::~Memory()
{
    SafeDeleteArray(m_pMap);
    InitPointer(m_pCurrentMemoryRule);
    SafeDeleteArray(m_pBootromSMS);
    SafeDeleteArray(m_pBootromGG);

    if (IsValidPointer(m_pDisassembledROMMap))
    {
        for (int i = 0; i < MAX_ROM_SIZE; i++)
        {
            SafeDelete(m_pDisassembledROMMap[i]);
        }
        SafeDeleteArray(m_pDisassembledROMMap);
    }

    if (IsValidPointer(m_pDisassembledMap))
    {
        for (int i = 0; i < 0x10000; i++)
        {
            SafeDelete(m_pDisassembledMap[i]);
        }
        SafeDeleteArray(m_pDisassembledMap);
    }
}

void Memory::SetProcessor(Processor* pProcessor)
{
    m_pProcessor = pProcessor;
}

void Memory::Init()
{
    m_pMap = new u8[0x10000];
#ifndef GS_DISABLE_DISASSEMBLER
    m_pDisassembledMap = new GS_Disassembler_Record*[0x10000];
    for (int i = 0; i < 0x10000; i++)
    {
        InitPointer(m_pDisassembledMap[i]);
    }

    m_pDisassembledROMMap = new GS_Disassembler_Record*[MAX_ROM_SIZE];
    for (int i = 0; i < MAX_ROM_SIZE; i++)
    {
        InitPointer(m_pDisassembledROMMap[i]);
    }
#endif
    Reset(false);
}

void Memory::Reset(bool bGameGear)
{
    m_bGameGear = bGameGear;
    m_MediaSlot = IsBootromEnabled() ? BiosSlot : CartridgeSlot;
    m_DesiredMediaSlot = IsBootromEnabled() ? m_StoredMediaSlot : CartridgeSlot;
    m_bIOEnabled = true;

    for (int i = 0; i < 0x10000; i++)
    {
        m_pMap[i] = 0x00;
    }

    if (IsBootromEnabled())
        ResetDisassemblerRecords();
}

void Memory::SetCurrentRule(MemoryRule* pRule)
{
    m_pCurrentMemoryRule = pRule;
}

void Memory::SetBootromRule(MemoryRule* pRule)
{
    m_pBootromMemoryRule = pRule;
}

MemoryRule* Memory::GetCurrentRule()
{
    return m_pCurrentMemoryRule;
}

u8* Memory::GetMemoryMap()
{
    return m_pMap;
}

void Memory::LoadSlotsFromROM(u8* pTheROM, int size)
{
    // loads the first 48KB only (bank 0, 1 and 2)
    int i;
    for (i = 0; ((i < 0xC000) && (i < size)); i++)
    {
        m_pMap[i] = pTheROM[i];
    }

    if (m_pCartridge->IsSG1000())
    {
        for (int j = i; j < 0xC000; j++)
            m_pMap[j] = 0xFF;
    }

    Debug("%d bytes copied from cartridge", i);
}

void Memory::MemoryDump(const char* szFilePath)
{
    if (!IsValidPointer(m_pDisassembledMap))
        return;

    using namespace std;

    ofstream myfile;
    open_ofstream_utf8(myfile, szFilePath, ios::out | ios::trunc);

    if (myfile.is_open())
    {
        for (int i = 0; i < 0x10000; i++)
        {
            if (IsValidPointer(m_pDisassembledMap[i]) && (m_pDisassembledMap[i]->name[0] != 0))
            {
                myfile << "0x" << hex << i << "\t " << m_pDisassembledMap[i]->name << "\n";
                i += (m_pDisassembledMap[i]->size - 1);
            }
            else
            {
                myfile << "0x" << hex << i << "\t [0x" << hex << (int) m_pMap[i] << "]\n";
            }
        }

        myfile.close();
    }
}

void Memory::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (m_pMap), 0x10000);
    stream.write(reinterpret_cast<const char*> (&m_bIOEnabled), sizeof (m_bIOEnabled));
}

void Memory::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (m_pMap), 0x10000);
    stream.read(reinterpret_cast<char*> (&m_bIOEnabled), sizeof (m_bIOEnabled));
}

void Memory::EnableBootromSMS(bool enable)
{
    m_bBootromSMSEnabled = enable;

    if (m_bBootromSMSEnabled)
    {
        Log("SMS Bootrom enabled");
    }
    else
    {
        Log("SMS Bootrom disabled");
    }
}

void Memory::EnableBootromGG(bool enable)
{
    m_bBootromGGEnabled = enable;

    if (m_bBootromGGEnabled)
    {
        Log("GG Bootrom enabled");
    }
    else
    {
        Log("GG Bootrom disabled");
    }
}

void Memory::LoadBootromSMS(const char* szFilePath)
{
    Log("Loading SMS Bootrom %s...", szFilePath);

    LoadBootroom(szFilePath, false);
}

void Memory::LoadBootromGG(const char* szFilePath)
{
    Log("Loading GG Bootrom %s...", szFilePath);

    LoadBootroom(szFilePath, true);
}

void Memory::LoadBootroom(const char* szFilePath, bool gg)
{
    using namespace std;

    if (gg)
    {
        SafeDeleteArray(m_pBootromGG);
    }
    else
    {
        SafeDeleteArray(m_pBootromSMS);
    }

    ifstream file;
    open_ifstream_utf8(file, szFilePath, ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
        int size = static_cast<int> (file.tellg());

        u8* bootrom = new u8[size];

        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(bootrom), size);
        file.close();

        if (gg)
        {
            m_bBootromGGLoaded = true;
            m_pBootromGG = bootrom;
            m_iBootromGGSize = size;
            m_iBootromBankCountGG = std::max(Pow2Ceil(size / 0x4000), 1u);
        }
        else
        {
            m_bBootromSMSLoaded = true;
            m_pBootromSMS = bootrom;
            m_iBootromSMSSize = size;
            m_iBootromBankCountSMS = std::max(Pow2Ceil(size / 0x4000), 1u);
        }

        Log("Bootrom %s loaded (%d bytes)", szFilePath, size);
    }
    else
    {
        if (gg)
        {
            m_bBootromGGLoaded = false;
            SafeDelete(m_pBootromGG);
        }
        else
        {
            m_bBootromSMSLoaded = false;
            SafeDelete(m_pBootromSMS);
        }
        Log("There was a problem opening the file %s", szFilePath);
    }
}

bool Memory::IsBootromEnabled()
{
    if (m_pCartridge->IsSG1000() || m_pCartridge->IsGameGearInSMSMode())
        return false;

    return (m_bBootromSMSEnabled && m_bBootromSMSLoaded && !m_bGameGear) || (m_bBootromGGEnabled && m_bBootromGGLoaded && m_bGameGear);
}

bool Memory::IsIOEnabled()
{
    return m_bIOEnabled;
}

void Memory::SetPort3E(u8 port3E)
{
    MediaSlots oldSlot = m_MediaSlot;

    bool expansion = !IsSetBit(port3E, 7);
    bool cartridge = !IsSetBit(port3E, 6);
    bool card = !IsSetBit(port3E, 5);
    bool bios = !IsSetBit(port3E, 3);

    if (!m_bGameGear)
    {
        bool io = !IsSetBit(port3E, 2);
        if (io != m_bIOEnabled)
        {
            Debug("Port 3E: IO %s", io ? "enabled" : "disabled");
            m_bIOEnabled = io;
        }
    }

    if (cartridge)
    {
        m_MediaSlot = CartridgeSlot;
        Debug("Port 3E: Cartridge");
    }
    else if (bios)
    {
        m_MediaSlot = BiosSlot;
        Debug("Port 3E: BIOS");
    }
    else if (expansion && !m_bGameGear)
    {
        m_MediaSlot = ExpansionSlot;
        Debug("Port 3E: Expansion");
    }
    else if (card && !m_bGameGear)
    {
        m_MediaSlot = CardSlot;
        Debug("Port 3E: Card");
    }

    if (oldSlot != m_MediaSlot)
    {
        ResetDisassemblerRecords();
    }
}

u8* Memory::GetBootrom()
{
    return m_bGameGear ? m_pBootromGG : m_pBootromSMS;
}

int Memory::GetBootromBankCount()
{
    return m_bGameGear ? m_iBootromBankCountGG : m_iBootromBankCountSMS;
}

int Memory::GetBootromSize()
{
    return m_bGameGear ? m_iBootromGGSize : m_iBootromSMSSize;
}

MemoryRule* Memory::GetBootromRule()
{
    return m_pBootromMemoryRule;
}

void Memory::SetMediaSlot(MediaSlots slot)
{
    m_StoredMediaSlot = slot;
}

Memory::MediaSlots Memory::GetCurrentSlot()
{
    return m_MediaSlot;
}

void Memory::ResetDisassemblerRecords()
{
#ifndef GS_DISABLE_DISASSEMBLER
    if (IsValidPointer(m_pDisassembledROMMap))
    {
        for (int i = 0; i < MAX_ROM_SIZE; i++)
        {
            SafeDelete(m_pDisassembledROMMap[i]);
        }
    }
    if (IsValidPointer(m_pDisassembledMap))
    {
        for (int i = 0; i < 0x10000; i++)
        {
            SafeDelete(m_pDisassembledMap[i]);
        }
    }
#endif
}

GS_Disassembler_Record* Memory::GetOrCreateDisassemblerRecord(u16 address)
{
    u32 physical_address = GetPhysicalAddress(address);
    bool rom = (address < 0xC000);

    GS_Disassembler_Record** map = rom ? m_pDisassembledROMMap : m_pDisassembledMap;
    u32 offset = rom ? physical_address : (u32)address;

    GS_Disassembler_Record* record = map[offset];

    if (!IsValidPointer(record))
    {
        record = new GS_Disassembler_Record();
        record->address = physical_address;
        record->bank = GetBank(address);
        record->segment[0] = 0;
        record->name[0] = 0;
        record->bytes[0] = 0;
        record->size = 0;
        for (int i = 0; i < 7; i++)
            record->opcodes[i] = 0;
        record->jump = false;
        record->jump_address = 0;
        record->jump_bank = 0;
        record->subroutine = false;
        record->irq = 0;
        record->has_operand_address = false;
        record->operand_address = 0;
        record->auto_symbol[0] = 0;
        map[offset] = record;
    }

    return record;
}