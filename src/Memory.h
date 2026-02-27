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
    struct stDisassembleRecord
    {
        u16 address;
        char name[32];
        char bytes[16];
        int size;
        int bank;
        u8 opcodes[4];
        bool jump;
        u16 jump_address;
    };

    struct stMemoryBreakpoint
    {
        u16 address1;
        u16 address2;
        bool read;
        bool write;
        bool range;
    };

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
    void Load(u16 address, u8 value);
    stDisassembleRecord** GetDisassembledMemoryMap();
    stDisassembleRecord** GetDisassembledROMMemoryMap();
    void LoadSlotsFromROM(u8* pTheROM, int size);
    void MemoryDump(const char* szFilePath);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);
    std::vector<stDisassembleRecord*>* GetBreakpointsCPU();
    std::vector<stMemoryBreakpoint>* GetBreakpointsMem();
    stDisassembleRecord* GetRunToBreakpoint();
    void SetRunToBreakpoint(stDisassembleRecord* pBreakpoint);
    void EnableBootromSMS(bool enable);
    void EnableBootromGG(bool enable);
    void LoadBootromSMS(const char* szFilePath);
    void LoadBootromGG(const char* szFilePath);
    bool IsBootromEnabled();
    bool IsIOEnabled();
    void SetPort3E(u8 port3E);
    u8* GetBootrom();
    int GetBootromBankCount();
    void SetMediaSlot(MediaSlots slot);
    MediaSlots GetCurrentSlot();
    void ResetDisassembledMemory();
    void ResetRomDisassembledMemory();

private:
    void LoadBootroom(const char* szFilePath, bool gg);
    void CheckBreakpoints(u16 address, bool write);

private:
    Processor* m_pProcessor;
    Cartridge* m_pCartridge;
    MemoryRule* m_pCurrentMemoryRule;
    MemoryRule* m_pBootromMemoryRule;
    u8* m_pMap;
    stDisassembleRecord** m_pDisassembledMap;
    stDisassembleRecord** m_pDisassembledROMMap;
    std::vector<stDisassembleRecord*> m_BreakpointsCPU;
    std::vector<stMemoryBreakpoint> m_BreakpointsMem;
    stDisassembleRecord* m_pRunToBreakpoint;
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
    bool m_bIOEnabled;
};

#include "Memory_inline.h"

#endif	/* MEMORY_H */
