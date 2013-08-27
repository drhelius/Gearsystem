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
    // IN B,(C)
    OPCOdes_IN_C(BC.GetHighRegister());
}

void Processor::OPCodeED0x41()
{
    // OUT (C),B
    OPCOdes_OUT_C(BC.GetHighRegister());
}

void Processor::OPCodeED0x42()
{
    // SBC HL,BC
    OPCodes_SBC_HL(BC.GetValue());
}

void Processor::OPCodeED0x43()
{
    // LD (nn),BC
    OPCodes_LD_nn_dd(&BC);
}

void Processor::OPCodeED0x44()
{
    // NEG
    u8 value = AF.GetHigh();
    AF.SetHigh(0x00);
    OPCodes_SUB(value);
}

void Processor::OPCodeED0x45()
{
    // RETN
    // todo: unfinished 
    StackPop(&PC);
}

void Processor::OPCodeED0x46()
{
    // IM 0
    SetInterruptMode(0);
}

void Processor::OPCodeED0x47()
{
    // LD I,A
    InvalidOPCode();
}

void Processor::OPCodeED0x48()
{
    // IN C,(C)
    OPCOdes_IN_C(BC.GetLowRegister());
}

void Processor::OPCodeED0x49()
{
    // OUT (C),C
    OPCOdes_OUT_C(BC.GetLowRegister());
}

void Processor::OPCodeED0x4A()
{
    // ADC HL,BC
    OPCodes_ADC_HL(BC.GetValue());
}

void Processor::OPCodeED0x4B()
{
    // LD BC,(nn)
    OPCodes_LD_dd_nn(&BC);
}

void Processor::OPCodeED0x4C()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x4D()
{
    // RETI
    // todo: unfinished
    StackPop(&PC);
    m_bIME = true;
}

void Processor::OPCodeED0x4E()
{
    // IM 0*
    UndocumentedOPCode();
    OPCodeED0x46();
}

void Processor::OPCodeED0x4F()
{
    // LD R,A
    InvalidOPCode();
}

void Processor::OPCodeED0x50()
{
    // IN D,(C)
    OPCOdes_IN_C(DE.GetHighRegister());
}

void Processor::OPCodeED0x51()
{
    // OUT (C),D
    OPCOdes_OUT_C(DE.GetHighRegister());
}

void Processor::OPCodeED0x52()
{
    // SBC HL,DE
    OPCodes_SBC_HL(DE.GetValue());
}

void Processor::OPCodeED0x53()
{
    // LD (nn),DE
    OPCodes_LD_nn_dd(&DE);
}

void Processor::OPCodeED0x54()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x55()
{
    // RETN*
    UndocumentedOPCode();
    OPCodeED0x45();
}

void Processor::OPCodeED0x56()
{
    // IM 1
    SetInterruptMode(1);
}

void Processor::OPCodeED0x57()
{
    // LD A,I
    InvalidOPCode();
}

void Processor::OPCodeED0x58()
{
    // IN E,(C)
    OPCOdes_IN_C(DE.GetLowRegister());
}

void Processor::OPCodeED0x59()
{
    // OUT (C),E
    OPCOdes_OUT_C(DE.GetLowRegister());
}

void Processor::OPCodeED0x5A()
{
    // ADC HL,DE
    OPCodes_ADC_HL(DE.GetValue());
}

void Processor::OPCodeED0x5B()
{
    // LD DE,(nn)
    OPCodes_LD_dd_nn(&DE);
}

void Processor::OPCodeED0x5C()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x5D()
{
    // RETN*
    UndocumentedOPCode();
    OPCodeED0x45();
}

void Processor::OPCodeED0x5E()
{
    // IM 2
    SetInterruptMode(2);
}

void Processor::OPCodeED0x5F()
{
    // LD A,R
    InvalidOPCode();
}

void Processor::OPCodeED0x60()
{
    // IN H,(C)
    OPCOdes_IN_C(HL.GetHighRegister());
}

void Processor::OPCodeED0x61()
{
    // OUT (C),H
    OPCOdes_OUT_C(HL.GetHighRegister());
}

void Processor::OPCodeED0x62()
{
    // SBC HL,HL
    OPCodes_SBC_HL(HL.GetValue());
}

void Processor::OPCodeED0x63()
{
    // LD (nn),HL
    OPCodes_LD_nn_dd(&HL);
}

void Processor::OPCodeED0x64()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x65()
{
    // RETN*
    UndocumentedOPCode();
    OPCodeED0x45();
}

void Processor::OPCodeED0x66()
{
    // IM 0*
    UndocumentedOPCode();
    OPCodeED0x46();
}

void Processor::OPCodeED0x67()
{
    // RRD
    u8 value = m_pMemory->Read(HL.GetValue());
    u8 a = AF.GetHigh();
    u8 result = (a & 0xF0) | (value & 0x0F);
    AF.SetHigh(result);
    m_pMemory->Write(HL.GetValue(), ((a << 4) & 0xF0) | ((value >> 4) & 0x0F));
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodeED0x68()
{
    // IN L,(C)
    OPCOdes_IN_C(BC.GetLowRegister());
}

void Processor::OPCodeED0x69()
{
    // OUT (C),L
    OPCOdes_OUT_C(BC.GetLowRegister());
}

void Processor::OPCodeED0x6A()
{
    // ADC HL,HL
    OPCodes_ADC_HL(HL.GetValue());
}

void Processor::OPCodeED0x6B()
{
    // LD HL,(nn)
    OPCodes_LD_dd_nn(&HL);
}

void Processor::OPCodeED0x6C()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x6D()
{
    // RETN*
    UndocumentedOPCode();
    OPCodeED0x45();
}

void Processor::OPCodeED0x6E()
{
    // IM 0*
    UndocumentedOPCode();
    OPCodeED0x46();
}

void Processor::OPCodeED0x6F()
{
    // RLD
    u8 value = m_pMemory->Read(HL.GetValue());
    u8 a = AF.GetHigh();
    u8 result = (a & 0xF0) | ((value >> 4) & 0x0F);
    AF.SetHigh(result);
    m_pMemory->Write(HL.GetValue(), ((value << 4) & 0xF0) | (a & 0x0F));
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
}

void Processor::OPCodeED0x70()
{
    // IN F,(C)*
    UndocumentedOPCode();
    OPCOdes_IN_C(AF.GetLowRegister());
}

void Processor::OPCodeED0x71()
{
    // OUT (C),0*
    UndocumentedOPCode();
    m_pIOPorts->Output(BC.GetLow(), 0);
}

void Processor::OPCodeED0x72()
{
    // SBC HL,SP
    OPCodes_SBC_HL(SP.GetValue());
}

void Processor::OPCodeED0x73()
{
    // LD (nn),SP
    OPCodes_LD_nn_dd(&SP);
}

void Processor::OPCodeED0x74()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x75()
{
    // RETN*
    UndocumentedOPCode();
    OPCodeED0x45();
}

void Processor::OPCodeED0x76()
{
    // IM 1*
    UndocumentedOPCode();
    OPCodeED0x56();
}

void Processor::OPCodeED0x78()
{
    // IN A,(C)
    OPCOdes_IN_C(AF.GetHighRegister());
}

void Processor::OPCodeED0x79()
{
    // OUT (C),A
    OPCOdes_OUT_C(AF.GetHighRegister());
}

void Processor::OPCodeED0x7A()
{
    // ADC HL,SP
    OPCodes_ADC_HL(SP.GetValue());
}

void Processor::OPCodeED0x7B()
{
    // LD SP,(nn)
    OPCodes_LD_dd_nn(&SP);
}

void Processor::OPCodeED0x7C()
{
    // NEG*
    UndocumentedOPCode();
    OPCodeED0x44();
}

void Processor::OPCodeED0x7D()
{
    // RETN*
    UndocumentedOPCode();
    OPCodeED0x45();
}

void Processor::OPCodeED0x7E()
{
    // IM 2*
    UndocumentedOPCode();
    OPCodeED0x5E();
}

void Processor::OPCodeED0xA0()
{
    // LDI
    InvalidOPCode();
}

void Processor::OPCodeED0xA1()
{
    // CPI
    InvalidOPCode();
}

void Processor::OPCodeED0xA2()
{
    // INI
    InvalidOPCode();
}

void Processor::OPCodeED0xA3()
{
    // OUTI
    InvalidOPCode();
}

void Processor::OPCodeED0xA8()
{
    // LDD
    InvalidOPCode();
}

void Processor::OPCodeED0xA9()
{
    // CPD
    InvalidOPCode();
}

void Processor::OPCodeED0xAA()
{
    // IND
    InvalidOPCode();
}

void Processor::OPCodeED0xAB()
{
    // OUTD
    InvalidOPCode();
}

void Processor::OPCodeED0xB0()
{
    // LDIR
    InvalidOPCode();
}

void Processor::OPCodeED0xB1()
{
    // CPIR
    InvalidOPCode();
}

void Processor::OPCodeED0xB2()
{
    // INIR
    InvalidOPCode();
}

void Processor::OPCodeED0xB3()
{
    // OTIR
    InvalidOPCode();
}

void Processor::OPCodeED0xB8()
{
    // LDDR
    InvalidOPCode();
}

void Processor::OPCodeED0xB9()
{
    // CPDR
    InvalidOPCode();
}

void Processor::OPCodeED0xBA()
{
    // INDR
    InvalidOPCode();
}

void Processor::OPCodeED0xBB()
{
    // OTDR
    InvalidOPCode();
}
