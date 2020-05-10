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

#ifndef ROMONLYMORYRULE_H
#define	ROMONLYMORYRULE_H

#include "MemoryRule.h"

class RomOnlyMemoryRule : public MemoryRule
{
public:
    RomOnlyMemoryRule(Memory* pMemory, Cartridge* pCartridge);
    virtual ~RomOnlyMemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset();
    virtual u8* GetPage(int index);
    virtual int GetBank(int index);
};

#endif	/* ROMONLYMORYRULE_H */
