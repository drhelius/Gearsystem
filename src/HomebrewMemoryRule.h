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

#ifndef HOMEBREWMORYRULE_H
#define	HOMEBREWMORYRULE_H

#include "MemoryRule.h"

#define FLASH_ID_SEQUENCE_LENGTH 6
#define FLASH_ERASE_SEQUENCE_LENGTH 10
#define FLASH_WRITE_SEQUENCE_LENGTH 6

class HomebrewMemoryRule : public MemoryRule
{
public:
    HomebrewMemoryRule(Memory* pMemory, Cartridge* pCartridge, Input* pInput);
    void ProcessFlashAccess(u16 address, u8 value);
    virtual ~HomebrewMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
    virtual u8* GetPage(int index);
    virtual int GetBank(int index);
private:
    int m_iGameSlot;
    int m_iMapperSlot;
    int m_iMapperSlotAddress;
    bool m_bFlashIDMode;
    bool m_bFlashEraseMode;
    bool m_bFlashWriteMode;
    int m_iFlashIDStep;
    int m_iFlashEraseStep;
    int m_iFlashWriteStep;
    const int m_iFlashIDSequence[FLASH_ID_SEQUENCE_LENGTH] = { 0x5555, 0xAA, 0x2AAA, 0x55, 0x5555, 0x90 };
    const int m_iFlashEraseSequence[FLASH_ERASE_SEQUENCE_LENGTH] = { 0x5555, 0xAA, 0x2AAA, 0x55, 0x5555, 0x80, 0x5555, 0xaa, 0x2aaa, 0x55 };
    const int m_iFlashWriteSequence[FLASH_WRITE_SEQUENCE_LENGTH] = { 0x5555, 0xAA, 0x2AAA, 0x55, 0x5555, 0xA0 };
    // Helper to advance address/value sequences used for flash commands
    bool AdvanceSequence(const int seq[], int len, int &step, u16 address, u8 value);
};

#endif	/* HOMEBREWMORYRULE_H */
