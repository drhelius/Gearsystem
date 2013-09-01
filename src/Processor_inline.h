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

#ifndef PROCESSOR_INLINE_H
#define	PROCESSOR_INLINE_H

#include "definitions.h"
#include "SixteenBitRegister.h"
#include "Processor.h"
#include "IOPorts.h"

const bool kZ80ParityTable[256] = {
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true,
    true, false, false, true, false, true, true, false,
    true, false, false, true, false, true, true, false,
    false, true, true, false, true, false, false, true
};

inline u8 Processor::FetchOPCode()
{
    u8 opcode = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    return opcode;
}

inline void Processor::ClearAllFlags()
{
    SetFlag(FLAG_NONE);
}

inline void Processor::ToggleZeroFlagFromResult(u16 result)
{
    if (result == 0)
        ToggleFlag(FLAG_ZERO);
    else
        UntoggleFlag(FLAG_ZERO);
}

inline void Processor::ToggleSignFlagFromResult(u8 result)
{
    if ((result & 0x80) != 0)
        ToggleFlag(FLAG_SIGN);
    else
        UntoggleFlag(FLAG_SIGN);
}

inline void Processor::ToggleXYFlagsFromResult(u8 result)
{
    if ((result & 0x08) != 0)
        ToggleFlag(FLAG_X);
    else
        UntoggleFlag(FLAG_X);
    if ((result & 0x20) != 0)
        ToggleFlag(FLAG_Y);
    else
        UntoggleFlag(FLAG_Y);
}

inline void Processor::ToggleParityFlagFromResult(u8 result)
{
    if (kZ80ParityTable[result])
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
}

inline void Processor::SetFlag(u8 flag)
{
    AF.SetLow(flag);
}

inline void Processor::FlipFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() ^ flag);
}

inline void Processor::ToggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() | flag);
}

inline void Processor::UntoggleFlag(u8 flag)
{
    AF.SetLow(AF.GetLow() & (~flag));
}

inline bool Processor::IsSetFlag(u8 flag)
{
    return (AF.GetLow() & flag) != 0;
}

inline void Processor::StackPush(SixteenBitRegister* reg)
{
    SP.Decrement();
    m_pMemory->Write(SP.GetValue(), reg->GetHigh());
    SP.Decrement();
    m_pMemory->Write(SP.GetValue(), reg->GetLow());
}

inline void Processor::StackPop(SixteenBitRegister* reg)
{
    reg->SetLow(m_pMemory->Read(SP.GetValue()));
    SP.Increment();
    reg->SetHigh(m_pMemory->Read(SP.GetValue()));
    SP.Increment();
}

inline void Processor::SetInterruptMode(int mode)
{
    Log("--> ** Attempting to set interrupt mode %d", mode);
    m_iInterruptMode = mode;
}

inline SixteenBitRegister* Processor::GetPrefixedRegister()
{
    if (m_CurrentPrefix == 0xDD)
        return &IX;
    else if (m_CurrentPrefix == 0xFD)
        return &IY;
    else 
        return &HL;
}

inline u16 Processor::GetPrefixedDisplacementValue()
{
    u16 address = HL.GetValue();
    if (m_CurrentPrefix == 0xDD)
    {
        address = IX.GetValue() + static_cast<s8> (m_pMemory->Read(PC.GetValue()));
        PC.Increment();
    }
    else if (m_CurrentPrefix == 0xFD)
    {
        address = IY.GetValue() + static_cast<s8> (m_pMemory->Read(PC.GetValue()));
        PC.Increment();
    }
    return address;
}

inline bool Processor::IsPrefixedIntruction()
{
    return (m_CurrentPrefix == 0xDD) || (m_CurrentPrefix == 0xFD);
}

inline void Processor::OPCodes_LD(EightBitRegister* reg1, u8 value)
{
    reg1->SetValue(value);
}

inline void Processor::OPCodes_LD(EightBitRegister* reg, u16 address)
{
    reg->SetValue(m_pMemory->Read(address));
}

inline void Processor::OPCodes_LD(u16 address, u8 reg)
{
    m_pMemory->Write(address, reg);
}

inline void Processor::OPCodes_LD_dd_nn(SixteenBitRegister* reg)
{
    u8 l = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u8 h = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u16 address = (h << 8) + l;
    reg->SetLow(m_pMemory->Read(address));
    reg->SetHigh(m_pMemory->Read(address + 1));
}

inline void Processor::OPCodes_LD_nn_dd(SixteenBitRegister* reg)
{
    u8 l = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u8 h = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u16 address = (h << 8) + l;
    m_pMemory->Write(address, reg->GetLow());
    m_pMemory->Write(address + 1, reg->GetHigh());
}

inline void Processor::OPCodes_LDI()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    m_pMemory->Write(DE.GetValue(), result);
    DE.Increment();
    HL.Increment();
    BC.Decrement();
    UntoggleFlag(FLAG_NEGATIVE);
    UntoggleFlag(FLAG_HALF);
    if (BC.GetValue() != 0)
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
    u16 n = AF.GetHigh() + result;
    if ((n & 0x08) != 0)
        ToggleFlag(FLAG_X);
    else
        UntoggleFlag(FLAG_X);
    if ((n & 0x02) != 0)
        ToggleFlag(FLAG_Y);
    else
        UntoggleFlag(FLAG_Y);  
}

inline void Processor::OPCodes_LDD()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    m_pMemory->Write(DE.GetValue(), result);
    DE.Decrement();
    HL.Decrement();
    BC.Decrement();
    UntoggleFlag(FLAG_NEGATIVE);
    UntoggleFlag(FLAG_HALF);
    if (BC.GetValue() != 0)
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
    u16 n = AF.GetHigh() + result;
    if ((n & 0x08) != 0)
        ToggleFlag(FLAG_X);
    else
        UntoggleFlag(FLAG_X);
    if ((n & 0x02) != 0)
        ToggleFlag(FLAG_Y);
    else
        UntoggleFlag(FLAG_Y);
}

inline void Processor::OPCodes_CALL_nn()
{
    u8 l = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    u8 h = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    StackPush(&PC);
    PC.SetHigh(h);
    PC.SetLow(l);
}

inline void Processor::OPCodes_JP_nn()
{
    u8 l = m_pMemory->Read(PC.GetValue());
    u8 h = m_pMemory->Read(PC.GetValue() + 1);
    PC.SetHigh(h);
    PC.SetLow(l);
}

inline void Processor::OPCodes_JR_n()
{
    u16 pc = PC.GetValue();
    PC.SetValue(pc + 1 + (static_cast<s8> (m_pMemory->Read(pc))));
}

inline void Processor::OPCodes_IN_n(EightBitRegister* reg)
{
    u8 port = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    reg->SetValue(m_pIOPorts->DoInput(port));
}

inline void Processor::OPCodes_IN_C(EightBitRegister* reg)
{
    u8 result = m_pIOPorts->DoInput(BC.GetLow());
    if (IsValidPointer(reg))
        reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_INI()
{
    u8 result = m_pIOPorts->DoInput(BC.GetLow());
    m_pMemory->Write(HL.GetValue(), result);
    OPCodes_DEC(BC.GetHighRegister());
    HL.Increment();
    if ((result & 0x80) != 0)
        ToggleFlag(FLAG_NEGATIVE);
	else
        UntoggleFlag(FLAG_NEGATIVE);
    if ((result + ((BC.GetLow() + 1) & 0xFF)) > 0xFF)
    {
        ToggleFlag(FLAG_CARRY);
        ToggleFlag(FLAG_HALF);
    }
    else
    {
        UntoggleFlag(FLAG_CARRY);
        UntoggleFlag(FLAG_HALF);
    }  
    if (((result + ((BC.GetLow() + 1) & 0xFF)) & 0x07) ^ BC.GetHigh())
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_IND()
{
    u8 result = m_pIOPorts->DoInput(BC.GetLow());
    m_pMemory->Write(HL.GetValue(), result);
    OPCodes_DEC(BC.GetHighRegister());
    HL.Decrement();
    if ((result & 0x80) != 0)
        ToggleFlag(FLAG_NEGATIVE);
	else
        UntoggleFlag(FLAG_NEGATIVE);
    if ((result + ((BC.GetLow() - 1) & 0xFF)) > 0xFF)
    {
        ToggleFlag(FLAG_CARRY);
        ToggleFlag(FLAG_HALF);
    }
    else
    {
        UntoggleFlag(FLAG_CARRY);
        UntoggleFlag(FLAG_HALF);
    }  
    if (((result + ((BC.GetLow() + 1) & 0xFF)) & 0x07) ^ BC.GetHigh())
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_OUT_n(EightBitRegister* reg)
{
    u8 port = m_pMemory->Read(PC.GetValue());
    PC.Increment();
    m_pIOPorts->DoOutput(port, reg->GetValue());
}

inline void Processor::OPCodes_OUT_C(EightBitRegister* reg)
{
    m_pIOPorts->DoOutput(BC.GetLow(), reg->GetValue());
}

inline void Processor::OPCodes_OUTI()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    m_pIOPorts->DoOutput(BC.GetLow(), result);
    OPCodes_DEC(BC.GetHighRegister());
    HL.Increment();
    if ((result & 0x80) != 0)
        ToggleFlag(FLAG_NEGATIVE);
	else
        UntoggleFlag(FLAG_NEGATIVE);
    if ((HL.GetLow() + result) > 0xFF)
	{
        ToggleFlag(FLAG_CARRY);
        ToggleFlag(FLAG_HALF);
    }
    else
    {
        UntoggleFlag(FLAG_CARRY);
        UntoggleFlag(FLAG_HALF);
    }
    if (((HL.GetLow() + result) & 0x07) ^ BC.GetHigh())
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_OUTD()
{
    u8 result = m_pMemory->Read(HL.GetValue());
    m_pIOPorts->DoOutput(BC.GetLow(), result);
    BC.GetHighRegister()->Decrement();
    HL.Decrement();
    if ((result & 0x80) != 0)
        ToggleFlag(FLAG_NEGATIVE);
	else
        UntoggleFlag(FLAG_NEGATIVE);
    if ((HL.GetLow() + result) > 0xFF)
	{
        ToggleFlag(FLAG_CARRY);
        ToggleFlag(FLAG_HALF);
    }
    else
    {
        UntoggleFlag(FLAG_CARRY);
        UntoggleFlag(FLAG_HALF);
    }
    if (((HL.GetLow() + result) & 0x07) ^ BC.GetHigh())
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_EX(SixteenBitRegister* reg1, SixteenBitRegister* reg2)
{
    u16 tmp = reg1->GetValue();
    reg1->SetValue(reg2->GetValue());
    reg2->SetValue(tmp);
}

inline void Processor::OPCodes_OR(u8 number)
{
    u8 result = AF.GetHigh() | number;
    AF.SetHigh(result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    ToggleParityFlagFromResult(result);
}

inline void Processor::OPCodes_XOR(u8 number)
{
    u8 result = AF.GetHigh() ^ number;
    AF.SetHigh(result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    ToggleParityFlagFromResult(result);
}

inline void Processor::OPCodes_AND(u8 number)
{
    u8 result = AF.GetHigh() & number;
    AF.SetHigh(result);
    SetFlag(FLAG_HALF);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    ToggleParityFlagFromResult(result);
}

inline void Processor::OPCodes_CP(u8 number)
{
    int result = AF.GetHigh() - number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    SetFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    ToggleXYFlagsFromResult(number);
    if ((carrybits & 0x100) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    if ((((carrybits << 1) ^ carrybits) & 0x100) != 0)
       ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_CPI()
{
    u8 number = m_pMemory->Read(HL.GetValue());
    int result = AF.GetHigh() - number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    ToggleFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    else
        UntoggleFlag(FLAG_HALF);
    HL.Increment();
    BC.Decrement();
    if (BC.GetValue() != 0)
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
    int n = AF.GetHigh() - number - (IsSetFlag(FLAG_HALF) ? 1 : 0);
    if ((n & 0x08) != 0)
        ToggleFlag(FLAG_X);
    else
        UntoggleFlag(FLAG_X);
    if ((n & 0x02) != 0)
        ToggleFlag(FLAG_Y);
    else
        UntoggleFlag(FLAG_Y);
}

inline void Processor::OPCodes_CPD()
{
    u8 number = m_pMemory->Read(HL.GetValue());
    int result = AF.GetHigh() - number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    ToggleFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    else
        UntoggleFlag(FLAG_HALF);
    HL.Decrement();
    BC.Decrement();
    if (BC.GetValue() != 0)
        ToggleFlag(FLAG_PARITY);
    else
        UntoggleFlag(FLAG_PARITY);
    int n = AF.GetHigh() - number - (IsSetFlag(FLAG_HALF) ? 1 : 0);
    if ((n & 0x08) != 0)
        ToggleFlag(FLAG_X);
    else
        UntoggleFlag(FLAG_X);
    if ((n & 0x02) != 0)
        ToggleFlag(FLAG_Y);
    else
        UntoggleFlag(FLAG_Y);
}

inline void Processor::OPCodes_INC(EightBitRegister* reg)
{
    u8 result = reg->GetValue() + 1;
    reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    if ((result & 0x0F) == 0x00)
        ToggleFlag(FLAG_HALF);
    if (result == 0x80)
        ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_INC_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address) + 1;
    m_pMemory->Write(address, result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    if ((result & 0x0F) == 0x00)
        ToggleFlag(FLAG_HALF);
    if (result == 0x80)
        ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_DEC(EightBitRegister* reg)
{
    u8 result = reg->GetValue() - 1;
    reg->SetValue(result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    if ((result & 0x0F) == 0x00)
        ToggleFlag(FLAG_HALF);
    if (result == 0x7F)
        ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_DEC_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address) - 1;
    m_pMemory->Write(address, result);
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    ToggleFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
    if ((result & 0x0F) == 0x00)
        ToggleFlag(FLAG_HALF);
    if (result == 0x7F)
        ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_ADD(u8 number)
{
    int result = AF.GetHigh() + number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    AF.SetHigh(final_result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    ToggleXYFlagsFromResult(final_result);
    if ((carrybits & 0x100) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    if ((((carrybits << 1) ^ carrybits) & 0x100) != 0)
       ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_ADC(u8 number)
{
    int result = AF.GetHigh() + number + (IsSetFlag(FLAG_CARRY) ? 1 : 0);
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    AF.SetHigh(final_result);
    ClearAllFlags();
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    ToggleXYFlagsFromResult(final_result);
    if ((carrybits & 0x100) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    if ((((carrybits << 1) ^ carrybits) & 0x100) != 0)
       ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_SUB(u8 number)
{
    int result = AF.GetHigh() - number;
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    AF.SetHigh(final_result);
    SetFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    ToggleXYFlagsFromResult(final_result);
    if ((carrybits & 0x100) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    if ((((carrybits << 1) ^ carrybits) & 0x100) != 0)
       ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_SBC(u8 number)
{
    int result = AF.GetHigh() - number - (IsSetFlag(FLAG_CARRY) ? 1 : 0);
    int carrybits = AF.GetHigh() ^ number ^ result;
    u8 final_result = static_cast<u8> (result);
    AF.SetHigh(final_result);
    SetFlag(FLAG_NEGATIVE);
    ToggleZeroFlagFromResult(final_result);
    ToggleSignFlagFromResult(final_result);
    ToggleXYFlagsFromResult(final_result);
    if ((carrybits & 0x100) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x10) != 0)
        ToggleFlag(FLAG_HALF);
    if ((((carrybits << 1) ^ carrybits) & 0x100) != 0)
       ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_ADD_HL(u16 number)
{
    SixteenBitRegister* reg = GetPrefixedRegister();
    int result = reg->GetValue() + number;
    int carrybits = reg->GetValue() ^ number ^ result;
    reg->SetValue(static_cast<u16> (result));
    UntoggleFlag(FLAG_NEGATIVE);
    ToggleXYFlagsFromResult(reg->GetHigh());
    if ((carrybits & 0x10000) != 0)
        ToggleFlag(FLAG_CARRY);
    else
        UntoggleFlag(FLAG_CARRY);
    if ((carrybits & 0x1000) != 0)
        ToggleFlag(FLAG_HALF);
    else
        UntoggleFlag(FLAG_HALF);   
}

inline void Processor::OPCodes_ADC_HL(u16 number)
{
    int result = HL.GetValue() + number + (IsSetFlag(FLAG_CARRY) ? 1 : 0);
    int carrybits = HL.GetValue() ^ number ^ result;
    u16 final_result = static_cast<u16> (result);
    HL.SetValue(final_result);
    ClearAllFlags();
    ToggleXYFlagsFromResult(HL.GetHigh());
    ToggleSignFlagFromResult(HL.GetHigh());
    ToggleZeroFlagFromResult(final_result);
    if ((carrybits & 0x10000) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x1000) != 0)
        ToggleFlag(FLAG_HALF);
	if ((((carrybits << 1) ^ carrybits) & 0x10000) != 0)
        ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_SBC_HL(u16 number)
{
    int result = HL.GetValue() - number - (IsSetFlag(FLAG_CARRY) ? 1 : 0);
    int carrybits = HL.GetValue() ^ number ^ result;
    u16 final_result = static_cast<u16> (result);
    HL.SetValue(final_result);
    SetFlag(FLAG_NEGATIVE);
    ToggleXYFlagsFromResult(HL.GetHigh());
    ToggleSignFlagFromResult(HL.GetHigh());
    ToggleZeroFlagFromResult(final_result);
    if ((carrybits & 0x10000) != 0)
        ToggleFlag(FLAG_CARRY);
    if ((carrybits & 0x1000) != 0)
        ToggleFlag(FLAG_HALF);
	if ((((carrybits << 1) ^ carrybits) & 0x10000) != 0)
        ToggleFlag(FLAG_PARITY);
}

inline void Processor::OPCodes_SLL(EightBitRegister* reg)
{
    u16 address = 0x0000;
    if (IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    (reg->GetValue() & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u8 result = (reg->GetValue() << 1) | 0x01;
    reg->SetValue(result);
    if (IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SLL_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    (result & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result = (result << 1) | 0x01;
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SLA(EightBitRegister* reg)
{
    u16 address = 0x0000;
    if (IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    (reg->GetValue() & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u8 result = reg->GetValue() << 1;
    reg->SetValue(result);
    if (IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SLA_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    (result & 0x80) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SRA(EightBitRegister* reg)
{
    u16 address = 0x0000;
    if (IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    u8 result = reg->GetValue();
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((result & 0x80) != 0)
    {
        result >>= 1;
        result |= 0x80;
    }
    else
        result >>= 1;
    reg->SetValue(result);
    if (IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SRA_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    if ((result & 0x80) != 0)
    {
        result >>= 1;
        result |= 0x80;
    }
    else
        result >>= 1;
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SRL(EightBitRegister* reg)
{
    u16 address = 0x0000;
    if (IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    u8 result = reg->GetValue();
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    reg->SetValue(result);
    if (IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_SRL_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    (result & 0x01) != 0 ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_RLC(EightBitRegister* reg, bool isRegisterA)
{
    u16 address = 0x0000;
    if (!isRegisterA && IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    u8 result = reg->GetValue();
    if ((result & 0x80) != 0)
    {
        ToggleFlag(FLAG_CARRY);
        result <<= 1;
        result |= 0x1;
    }
    else
    {
        UntoggleFlag(FLAG_CARRY);
        result <<= 1;
    }
    reg->SetValue(result);
    if (!isRegisterA && IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    UntoggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_NEGATIVE);
    ToggleXYFlagsFromResult(result);
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
        ToggleSignFlagFromResult(result);
        ToggleParityFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RLC_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    if ((result & 0x80) != 0)
    {
        SetFlag(FLAG_CARRY);
        result <<= 1;
        result |= 0x1;
    }
    else
    {
        ClearAllFlags();
        result <<= 1;
    }
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_RL(EightBitRegister* reg, bool isRegisterA)
{
    u16 address = 0x0000;
    if (!isRegisterA && IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = reg->GetValue();
    ((result & 0x80) != 0) ? ToggleFlag(FLAG_CARRY) : UntoggleFlag(FLAG_CARRY);
    result <<= 1;
    result |= carry;
    reg->SetValue(result);
    if (!isRegisterA && IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    UntoggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_NEGATIVE);
    ToggleXYFlagsFromResult(result);
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
        ToggleSignFlagFromResult(result);
        ToggleParityFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RL_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 carry = IsSetFlag(FLAG_CARRY) ? 1 : 0;
    u8 result = m_pMemory->Read(address);
    ((result & 0x80) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result <<= 1;
    result |= carry;
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_RRC(EightBitRegister* reg, bool isRegisterA)
{
    u16 address = 0x0000;
    if (!isRegisterA && IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    u8 result = reg->GetValue();
    if ((result & 0x01) != 0)
    {
        ToggleFlag(FLAG_CARRY);
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        UntoggleFlag(FLAG_CARRY);
        result >>= 1;
    }
    reg->SetValue(result);
    if (!isRegisterA && IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    UntoggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_NEGATIVE);
    ToggleXYFlagsFromResult(result);
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
        ToggleSignFlagFromResult(result);
        ToggleParityFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RRC_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    if ((result & 0x01) != 0)
    {
        SetFlag(FLAG_CARRY);
        result >>= 1;
        result |= 0x80;
    }
    else
    {
        ClearAllFlags();
        result >>= 1;
    }
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_RR(EightBitRegister* reg, bool isRegisterA)
{
    u16 address = 0x0000;
    if (!isRegisterA && IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = reg->GetValue();
    ((result & 0x01) != 0) ? ToggleFlag(FLAG_CARRY) : UntoggleFlag(FLAG_CARRY);
    result >>= 1;
    result |= carry;
    reg->SetValue(result);
    if (!isRegisterA && IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
    UntoggleFlag(FLAG_HALF);
    UntoggleFlag(FLAG_NEGATIVE);
    ToggleXYFlagsFromResult(result);
    if (!isRegisterA)
    {
        ToggleZeroFlagFromResult(result);
        ToggleSignFlagFromResult(result);
        ToggleParityFlagFromResult(result);
    }
}

inline void Processor::OPCodes_RR_HL()
{
    u16 address = GetPrefixedDisplacementValue();
    u8 carry = IsSetFlag(FLAG_CARRY) ? 0x80 : 0x00;
    u8 result = m_pMemory->Read(address);
    ((result & 0x01) != 0) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    result >>= 1;
    result |= carry;
    m_pMemory->Write(address, result);
    ToggleZeroFlagFromResult(result);
    ToggleSignFlagFromResult(result);
    ToggleParityFlagFromResult(result);
    ToggleXYFlagsFromResult(result);
}

inline void Processor::OPCodes_BIT(EightBitRegister* reg, int bit)
{
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u8 value = reg->GetValue();
    if (IsPrefixedIntruction())
        value = m_pMemory->Read(GetPrefixedDisplacementValue());
    if (((value >> bit) & 0x01) == 0)
    {
        ToggleFlag(FLAG_ZERO);
        ToggleFlag(FLAG_PARITY);
    }
    else
    {     
        switch (bit)
        {
            case 3: ToggleFlag(FLAG_X); break;
            case 5: ToggleFlag(FLAG_Y); break;
            case 7: ToggleFlag(FLAG_SIGN); break;
        }          
    }
    ToggleFlag(FLAG_HALF);
}

inline void Processor::OPCodes_BIT_HL(int bit)
{
    IsSetFlag(FLAG_CARRY) ? SetFlag(FLAG_CARRY) : ClearAllFlags();
    u16 address = GetPrefixedDisplacementValue();
    if (((m_pMemory->Read(address) >> bit) & 0x01) == 0)
    {
        ToggleFlag(FLAG_ZERO);
        ToggleFlag(FLAG_PARITY);
    }
    else
    {     
        if (IsPrefixedIntruction())
        {
            u8 xy = (address >> 8) & 0xFF;
            if ((xy & 0x08) != 0)
                ToggleFlag(FLAG_X);
            if ((xy & 0x20) != 0)
                ToggleFlag(FLAG_Y);
            if (bit == 7)
                ToggleFlag(FLAG_SIGN);
        }
        else
        {
            switch (bit)
            {
                case 3: ToggleFlag(FLAG_X); break;
                case 5: ToggleFlag(FLAG_Y); break;
                case 7: ToggleFlag(FLAG_SIGN); break;
            }    
        }
    }
    ToggleFlag(FLAG_HALF);
}

inline void Processor::OPCodes_SET(EightBitRegister* reg, int bit)
{
    u16 address = 0x0000;
    if (IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    reg->SetValue(reg->GetValue() | (0x1 << bit));
    if (IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
}

inline void Processor::OPCodes_SET_HL(int bit)
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    result |= (0x1 << bit);
    m_pMemory->Write(address, result);
}

inline void Processor::OPCodes_RES(EightBitRegister* reg, int bit)
{
    u16 address = 0x0000;
    if (IsPrefixedIntruction())
    {
        address = GetPrefixedDisplacementValue();
        reg->SetValue(m_pMemory->Read(address));
    }
    reg->SetValue(reg->GetValue() & (~(0x1 << bit)));
    if (IsPrefixedIntruction())
        m_pMemory->Write(address, reg->GetValue());
}

inline void Processor::OPCodes_RES_HL(int bit)
{
    u16 address = GetPrefixedDisplacementValue();
    u8 result = m_pMemory->Read(address);
    result &= ~(0x1 << bit);
    m_pMemory->Write(address, result);
}

#endif	/* PROCESSOR_INLINE_H */

