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

#ifndef MEMORY_H
#define	MEMORY_H

#include "definitions.h"
#include "log.h"
#include "MemoryRule.h"
#include <vector>

class Processor;
class Cartridge;

class Memory
{
public:
    enum MediaSlots
    {
        CartridgeSlot,
        BiosSlot,
        ExpansionSlot,
        CardSlot,
        NoSlot
    };

public:
    Memory(Cartridge* pCartridge);
    ~Memory();
    void SetProcessor(Processor* pProcessor);
    void Init();
    void Reset(bool bGameGear);
    void SetCurrentRule(MemoryRule* pRule);
    void SetBootromRule(MemoryRule* pRule);
    MemoryRule* GetCurrentRule();
    u8* GetMemoryMap();
    u8 Read(u16 address);
    void Write(u16 address, u8 value);
    u8 Retrieve(u16 address);
    u8 DebugRetrieve(u16 address);
    void Load(u16 address, u8 value);
    GS_Disassembler_Record* GetDisassemblerRecord(u16 address);
    GS_Disassembler_Record* GetDisassemblerRecord(u16 address, u8 bank);
    GS_Disassembler_Record* GetOrCreateDisassemblerRecord(u16 address);
    void ResetDisassemblerRecords();
    GS_Disassembler_Record** GetAllDisassemblerRecords();
    void LoadSlotsFromROM(u8* pTheROM, int size);
    void MemoryDump(const char* szFilePath);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);
    void EnableBootromSMS(bool enable);
    void EnableBootromGG(bool enable);
    void LoadBootromSMS(const char* szFilePath);
    void LoadBootromGG(const char* szFilePath);
    bool IsBootromEnabled();
    bool IsIOEnabled();
    void SetPort3E(u8 port3E);
    u8* GetBootrom();
    int GetBootromBankCount();
    int GetBootromSize();
    MemoryRule* GetBootromRule();
    void SetMediaSlot(MediaSlots slot);
    MediaSlots GetCurrentSlot();
    u32 GetPhysicalAddress(u16 address);
    u8 GetBank(u16 address);

private:
    void LoadBootroom(const char* szFilePath, bool gg);

private:
    Processor* m_pProcessor;
    Cartridge* m_pCartridge;
    MemoryRule* m_pCurrentMemoryRule;
    MemoryRule* m_pBootromMemoryRule;
    u8* m_pMap;
    GS_Disassembler_Record** m_pDisassembledMap;
    GS_Disassembler_Record** m_pDisassembledROMMap;
    bool m_bBootromSMSEnabled;
    bool m_bBootromGGEnabled;
    bool m_bBootromSMSLoaded;
    bool m_bBootromGGLoaded;
    u8* m_pBootromSMS;
    u8* m_pBootromGG;
    MediaSlots m_MediaSlot;
    MediaSlots m_DesiredMediaSlot;
    MediaSlots m_StoredMediaSlot;
    bool m_bGameGear;
    int m_iBootromBankCountSMS;
    int m_iBootromBankCountGG;
    int m_iBootromSMSSize;
    int m_iBootromGGSize;
    bool m_bIOEnabled;
};

#include "Memory_inline.h"

#endif	/* MEMORY_H */
