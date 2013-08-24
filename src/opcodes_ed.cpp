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

#include "Processor.h"

void Processor::OPCodeED0x40()
{
    // BIT 0 B
    OPCodes_BIT(BC.GetHighRegister(), 0);
}

void Processor::OPCodeED0x41()
{
    // BIT 0 C
    OPCodes_BIT(BC.GetLowRegister(), 0);
}

void Processor::OPCodeED0x42()
{
    // BIT 0 D
    OPCodes_BIT(DE.GetHighRegister(), 0);
}

void Processor::OPCodeED0x43()
{
    // BIT 0 E
    OPCodes_BIT(DE.GetLowRegister(), 0);
}

void Processor::OPCodeED0x44()
{
    // BIT 0 H
    OPCodes_BIT(HL.GetHighRegister(), 0);
}

void Processor::OPCodeED0x45()
{
    // BIT 0 L
    OPCodes_BIT(HL.GetLowRegister(), 0);
}

void Processor::OPCodeED0x46()
{
    // BIT 0 (HL)
    OPCodes_BIT_HL(0);
}

void Processor::OPCodeED0x47()
{
    // BIT 0 A
    OPCodes_BIT(AF.GetHighRegister(), 0);
}

void Processor::OPCodeED0x48()
{
    // BIT 1 B
    OPCodes_BIT(BC.GetHighRegister(), 1);
}

void Processor::OPCodeED0x49()
{
    // BIT 1 C
    OPCodes_BIT(BC.GetLowRegister(), 1);
}

void Processor::OPCodeED0x4A()
{
    // BIT 1 D
    OPCodes_BIT(DE.GetHighRegister(), 1);
}

void Processor::OPCodeED0x4B()
{
    // BIT 1 E
    OPCodes_BIT(DE.GetLowRegister(), 1);
}

void Processor::OPCodeED0x4C()
{
    // BIT 1 H
    OPCodes_BIT(HL.GetHighRegister(), 1);
}

void Processor::OPCodeED0x4D()
{
    // BIT 1 L
    OPCodes_BIT(HL.GetLowRegister(), 1);
}

void Processor::OPCodeED0x4E()
{
    // BIT 1 (HL)
    OPCodes_BIT_HL(1);
}

void Processor::OPCodeED0x4F()
{
    // BIT 1 A
    OPCodes_BIT(AF.GetHighRegister(), 1);
}

void Processor::OPCodeED0x50()
{
    // BIT 2 B
    OPCodes_BIT(BC.GetHighRegister(), 2);
}

void Processor::OPCodeED0x51()
{
    // BIT 2 C
    OPCodes_BIT(BC.GetLowRegister(), 2);
}

void Processor::OPCodeED0x52()
{
    // BIT 2 D
    OPCodes_BIT(DE.GetHighRegister(), 2);
}

void Processor::OPCodeED0x53()
{
    // BIT 2 E
    OPCodes_BIT(DE.GetLowRegister(), 2);
}

void Processor::OPCodeED0x54()
{
    // BIT 2 H
    OPCodes_BIT(HL.GetHighRegister(), 2);
}

void Processor::OPCodeED0x55()
{
    // BIT 2 L
    OPCodes_BIT(HL.GetLowRegister(), 2);
}

void Processor::OPCodeED0x56()
{
    // BIT 2 (HL)
    OPCodes_BIT_HL(2);
}

void Processor::OPCodeED0x57()
{
    // BIT 2 A
    OPCodes_BIT(AF.GetHighRegister(), 2);
}

void Processor::OPCodeED0x58()
{
    // BIT 3 B
    OPCodes_BIT(BC.GetHighRegister(), 3);
}

void Processor::OPCodeED0x59()
{
    // BIT 3 C
    OPCodes_BIT(BC.GetLowRegister(), 3);
}

void Processor::OPCodeED0x5A()
{
    // BIT 3 D
    OPCodes_BIT(DE.GetHighRegister(), 3);
}

void Processor::OPCodeED0x5B()
{
    // BIT 3 E
    OPCodes_BIT(DE.GetLowRegister(), 3);
}

void Processor::OPCodeED0x5C()
{
    // BIT 3 H
    OPCodes_BIT(HL.GetHighRegister(), 3);
}

void Processor::OPCodeED0x5D()
{
    // BIT 3 L
    OPCodes_BIT(HL.GetLowRegister(), 3);
}

void Processor::OPCodeED0x5E()
{
    // BIT 3 (HL)
    OPCodes_BIT_HL(3);
}

void Processor::OPCodeED0x5F()
{
    // BIT 3 A
    OPCodes_BIT(AF.GetHighRegister(), 3);
}

void Processor::OPCodeED0x60()
{
    // BIT 4 B
    OPCodes_BIT(BC.GetHighRegister(), 4);
}

void Processor::OPCodeED0x61()
{
    // BIT 4 C
    OPCodes_BIT(BC.GetLowRegister(), 4);
}

void Processor::OPCodeED0x62()
{
    // BIT 4 D
    OPCodes_BIT(DE.GetHighRegister(), 4);
}

void Processor::OPCodeED0x63()
{
    // BIT 4 E
    OPCodes_BIT(DE.GetLowRegister(), 4);
}

void Processor::OPCodeED0x64()
{
    // BIT 4 H
    OPCodes_BIT(HL.GetHighRegister(), 4);
}

void Processor::OPCodeED0x65()
{
    // BIT 4 L
    OPCodes_BIT(HL.GetLowRegister(), 4);
}

void Processor::OPCodeED0x66()
{
    // BIT 4 (HL)
    OPCodes_BIT_HL(4);
}

void Processor::OPCodeED0x67()
{
    // BIT 4 A
    OPCodes_BIT(AF.GetHighRegister(), 4);
}

void Processor::OPCodeED0x68()
{
    // BIT 5 B
    OPCodes_BIT(BC.GetHighRegister(), 5);
}

void Processor::OPCodeED0x69()
{
    // BIT 5 C
    OPCodes_BIT(BC.GetLowRegister(), 5);
}

void Processor::OPCodeED0x6A()
{
    // BIT 5 D
    OPCodes_BIT(DE.GetHighRegister(), 5);
}

void Processor::OPCodeED0x6B()
{
    // BIT 5 E
    OPCodes_BIT(DE.GetLowRegister(), 5);
}

void Processor::OPCodeED0x6C()
{
    // BIT 5 H
    OPCodes_BIT(HL.GetHighRegister(), 5);
}

void Processor::OPCodeED0x6D()
{
    // BIT 5 L
    OPCodes_BIT(HL.GetLowRegister(), 5);
}

void Processor::OPCodeED0x6E()
{
    // BIT 5 (HL)
    OPCodes_BIT_HL(5);
}

void Processor::OPCodeED0x6F()
{
    // BIT 5 A
    OPCodes_BIT(AF.GetHighRegister(), 5);
}

void Processor::OPCodeED0x70()
{
    // BIT 6 B
    OPCodes_BIT(BC.GetHighRegister(), 6);
}

void Processor::OPCodeED0x71()
{
    // BIT 6 C
    OPCodes_BIT(BC.GetLowRegister(), 6);
}

void Processor::OPCodeED0x72()
{
    // BIT 6 D
    OPCodes_BIT(DE.GetHighRegister(), 6);
}

void Processor::OPCodeED0x73()
{
    // BIT 6 E
    OPCodes_BIT(DE.GetLowRegister(), 6);
}

void Processor::OPCodeED0x74()
{
    // BIT 6 H
    OPCodes_BIT(HL.GetHighRegister(), 6);
}

void Processor::OPCodeED0x75()
{
    // BIT 6 L
    OPCodes_BIT(HL.GetLowRegister(), 6);
}

void Processor::OPCodeED0x76()
{
    // BIT 6 (HL)
    OPCodes_BIT_HL(6);
}

void Processor::OPCodeED0x77()
{
    // BIT 6 A
    OPCodes_BIT(AF.GetHighRegister(), 6);
}

void Processor::OPCodeED0x78()
{
    // BIT 7 B
    OPCodes_BIT(BC.GetHighRegister(), 7);
}

void Processor::OPCodeED0x79()
{
    // BIT 7 C
    OPCodes_BIT(BC.GetLowRegister(), 7);
}

void Processor::OPCodeED0x7A()
{
    // BIT 7 D
    OPCodes_BIT(DE.GetHighRegister(), 7);
}

void Processor::OPCodeED0x7B()
{
    // BIT 7 E
    OPCodes_BIT(DE.GetLowRegister(), 7);
}

void Processor::OPCodeED0x7C()
{
    // BIT 7 H
    OPCodes_BIT(HL.GetHighRegister(), 7);
}

void Processor::OPCodeED0x7D()
{
    // BIT 7 L
    OPCodes_BIT(HL.GetLowRegister(), 7);
}

void Processor::OPCodeED0x7E()
{
    // BIT 7 (HL)
    OPCodes_BIT_HL(7);
}

void Processor::OPCodeED0x7F()
{
    // BIT 7 A
    OPCodes_BIT(AF.GetHighRegister(), 7);
}

void Processor::OPCodeED0x80()
{
    // RES 0 B
    OPCodes_RES(BC.GetHighRegister(), 0);
}

void Processor::OPCodeED0x81()
{
    // RES 0 C
    OPCodes_RES(BC.GetLowRegister(), 0);
}

void Processor::OPCodeED0x82()
{
    // RES 0 D
    OPCodes_RES(DE.GetHighRegister(), 0);
}

void Processor::OPCodeED0x83()
{
    // RES 0 E
    OPCodes_RES(DE.GetLowRegister(), 0);
}

void Processor::OPCodeED0x84()
{
    // RES 0 H
    OPCodes_RES(HL.GetHighRegister(), 0);
}

void Processor::OPCodeED0x85()
{
    // RES 0 L
    OPCodes_RES(HL.GetLowRegister(), 0);
}

void Processor::OPCodeED0x86()
{
    // RES 0 (HL)
    OPCodes_RES_HL(0);
}

void Processor::OPCodeED0x87()
{
    // RES 0 A
    OPCodes_RES(AF.GetHighRegister(), 0);
}

void Processor::OPCodeED0x88()
{
    // RES 1 B
    OPCodes_RES(BC.GetHighRegister(), 1);
}

void Processor::OPCodeED0x89()
{
    // RES 1 C
    OPCodes_RES(BC.GetLowRegister(), 1);
}

void Processor::OPCodeED0x8A()
{
    // RES 1 D
    OPCodes_RES(DE.GetHighRegister(), 1);
}

void Processor::OPCodeED0x8B()
{
    // RES 1 E
    OPCodes_RES(DE.GetLowRegister(), 1);
}

void Processor::OPCodeED0x8C()
{
    // RES 1 H
    OPCodes_RES(HL.GetHighRegister(), 1);
}

void Processor::OPCodeED0x8D()
{
    // RES 1 L
    OPCodes_RES(HL.GetLowRegister(), 1);
}

void Processor::OPCodeED0x8E()
{
    // RES 1 (HL)
    OPCodes_RES_HL(1);
}

void Processor::OPCodeED0x8F()
{
    // RES 1 A
    OPCodes_RES(AF.GetHighRegister(), 1);
}

void Processor::OPCodeED0x90()
{
    // RES 2 B
    OPCodes_RES(BC.GetHighRegister(), 2);
}

void Processor::OPCodeED0x91()
{
    // RES 2 C
    OPCodes_RES(BC.GetLowRegister(), 2);
}

void Processor::OPCodeED0x92()
{
    // RES 2 D
    OPCodes_RES(DE.GetHighRegister(), 2);
}

void Processor::OPCodeED0x93()
{
    // RES 2 E
    OPCodes_RES(DE.GetLowRegister(), 2);
}

void Processor::OPCodeED0x94()
{
    // RES 2 H
    OPCodes_RES(HL.GetHighRegister(), 2);
}

void Processor::OPCodeED0x95()
{
    // RES 2 L
    OPCodes_RES(HL.GetLowRegister(), 2);
}

void Processor::OPCodeED0x96()
{
    // RES 2 (HL)
    OPCodes_RES_HL(2);
}

void Processor::OPCodeED0x97()
{
    // RES 2 A
    OPCodes_RES(AF.GetHighRegister(), 2);
}

void Processor::OPCodeED0x98()
{
    // RES 3 B
    OPCodes_RES(BC.GetHighRegister(), 3);
}

void Processor::OPCodeED0x99()
{
    // RES 3 C
    OPCodes_RES(BC.GetLowRegister(), 3);
}

void Processor::OPCodeED0x9A()
{
    // RES 3 D
    OPCodes_RES(DE.GetHighRegister(), 3);
}

void Processor::OPCodeED0x9B()
{
    // RES 3 E
    OPCodes_RES(DE.GetLowRegister(), 3);
}

void Processor::OPCodeED0x9C()
{
    // RES 3 H
    OPCodes_RES(HL.GetHighRegister(), 3);
}

void Processor::OPCodeED0x9D()
{
    // RES 3 L
    OPCodes_RES(HL.GetLowRegister(), 3);
}

void Processor::OPCodeED0x9E()
{
    // RES 3 (HL)
    OPCodes_RES_HL(3);
}

void Processor::OPCodeED0x9F()
{
    // RES 3 A
    OPCodes_RES(AF.GetHighRegister(), 3);
}

void Processor::OPCodeED0xA0()
{
    // RES 4 B
    OPCodes_RES(BC.GetHighRegister(), 4);
}

void Processor::OPCodeED0xA1()
{
    // RES 4 C
    OPCodes_RES(BC.GetLowRegister(), 4);
}

void Processor::OPCodeED0xA2()
{
    // RES 4 D
    OPCodes_RES(DE.GetHighRegister(), 4);
}

void Processor::OPCodeED0xA3()
{
    // RES 4 E
    OPCodes_RES(DE.GetLowRegister(), 4);
}

void Processor::OPCodeED0xA4()
{
    // RES 4 H
    OPCodes_RES(HL.GetHighRegister(), 4);
}

void Processor::OPCodeED0xA5()
{
    // RES 4 L
    OPCodes_RES(HL.GetLowRegister(), 4);
}

void Processor::OPCodeED0xA6()
{
    // RES 4 (HL)
    OPCodes_RES_HL(4);
}

void Processor::OPCodeED0xA7()
{
    // RES 4 A
    OPCodes_RES(AF.GetHighRegister(), 4);
}

void Processor::OPCodeED0xA8()
{
    // RES 5 B
    OPCodes_RES(BC.GetHighRegister(), 5);
}

void Processor::OPCodeED0xA9()
{
    // RES 5 C
    OPCodes_RES(BC.GetLowRegister(), 5);
}

void Processor::OPCodeED0xAA()
{
    // RES 5 D
    OPCodes_RES(DE.GetHighRegister(), 5);
}

void Processor::OPCodeED0xAB()
{
    // RES 5 E
    OPCodes_RES(DE.GetLowRegister(), 5);
}

void Processor::OPCodeED0xAC()
{
    // RES 5 H
    OPCodes_RES(HL.GetHighRegister(), 5);
}

void Processor::OPCodeED0xAD()
{
    // RES 5 L
    OPCodes_RES(HL.GetLowRegister(), 5);
}

void Processor::OPCodeED0xAE()
{
    // RES 5 (HL)
    OPCodes_RES_HL(5);
}

void Processor::OPCodeED0xAF()
{
    // RES 5 A
    OPCodes_RES(AF.GetHighRegister(), 5);
}

void Processor::OPCodeED0xB0()
{
    // RES 6 B
    OPCodes_RES(BC.GetHighRegister(), 6);
}

void Processor::OPCodeED0xB1()
{
    // RES 6 C
    OPCodes_RES(BC.GetLowRegister(), 6);
}

void Processor::OPCodeED0xB2()
{
    // RES 6 D
    OPCodes_RES(DE.GetHighRegister(), 6);
}

void Processor::OPCodeED0xB3()
{
    // RES 6 E
    OPCodes_RES(DE.GetLowRegister(), 6);
}

void Processor::OPCodeED0xB4()
{
    // RES 6 H
    OPCodes_RES(HL.GetHighRegister(), 6);
}

void Processor::OPCodeED0xB5()
{
    // RES 6 L
    OPCodes_RES(HL.GetLowRegister(), 6);
}

void Processor::OPCodeED0xB6()
{
    // RES 6 (HL)
    OPCodes_RES_HL(6);
}

void Processor::OPCodeED0xB7()
{
    // RES 6 A
    OPCodes_RES(AF.GetHighRegister(), 6);
}

void Processor::OPCodeED0xB8()
{
    // RES 7 B
    OPCodes_RES(BC.GetHighRegister(), 7);
}

void Processor::OPCodeED0xB9()
{
    // RES 7 C
    OPCodes_RES(BC.GetLowRegister(), 7);
}

void Processor::OPCodeED0xBA()
{
    // RES 7 D
    OPCodes_RES(DE.GetHighRegister(), 7);
}

void Processor::OPCodeED0xBB()
{
    // RES 7 E
    OPCodes_RES(DE.GetLowRegister(), 7);
}
