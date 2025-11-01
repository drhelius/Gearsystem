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

#ifndef EEPROM93C46MEMORYRULE_H
#define EEPROM93C46MEMORYRULE_H

#include "MemoryRule.h"

class Eeprom93C46MemoryRule : public MemoryRule
{
public:
    Eeprom93C46MemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput);
    virtual ~Eeprom93C46MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
    virtual void SaveRam(std::ostream &file);
    virtual bool LoadRam(std::istream &file, s32 fileSize);
    virtual bool PersistedRAM();
    virtual size_t GetRamSize();
    virtual u8* GetRamBanks();
    virtual u8* GetPage(int index);
    virtual int GetBank(int index);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);

private:
    // EEPROM 93C46 definitions
    static const int EEPROM_93C46_LINE_DATA_IN = 0x01;
    static const int EEPROM_93C46_LINE_CLOCK = 0x02;
    static const int EEPROM_93C46_LINE_CS = 0x04;
    static const int EEPROM_93C46_LINE_DATA_OUT = 0x08;
    static const int EEPROM_93C46_LINE_DATA_OUT_POS = 3;
    
    static const int EEPROM_93C46_INIT_NORMAL = 0;
    static const int EEPROM_93C46_INIT_ALL = 1;
    
    static const int EEPROM_93C46_DATA_SIZE = 128;
    
    static const int EEPROM_93C46_STATUS_START = 0;
    static const int EEPROM_93C46_STATUS_OPCODE = 1;
    static const int EEPROM_93C46_STATUS_READING = 2;
    static const int EEPROM_93C46_STATUS_WRITING = 3;

    struct EEPROM93C46
    {
        u8 Enabled;
        u8 Lines;
        u8 Status;
        u8 ReadOnly;
        u8 Position;
        u16 Opcode;
        u16 Latch;
        u16 Data[EEPROM_93C46_DATA_SIZE / 2];
    };

    void EEPROM_93c46_Init(int init);
    void EEPROM_93c46_Clear();
    void EEPROM_93c46_Control(u8 value);
    void EEPROM_93c46_SetLines(u8 lines);
    u8 EEPROM_93c46_Read();
    void EEPROM_93c46_DirectWrite(int addr, u8 data);
    u8 EEPROM_93c46_DirectRead(int addr);

private:
    int m_iMapperSlot[3];
    int m_iMapperSlotAddress[3];
    EEPROM93C46 m_EEPROM;
};

#endif	/* EEPROM93C46MEMORYRULE_H */
