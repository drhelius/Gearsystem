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

#include "Input.h"
#include "Memory.h"
#include "Processor.h"

Input::Input(Memory* pMemory, Processor* pProcessor)
{
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    m_JoypadState = 0xFF;
    m_P1 = 0xFF;
    m_iInputCycles = 0;
}

void Input::Init()
{
    Reset();
}

void Input::Reset()
{
    m_JoypadState = 0xFF;
    m_P1 = 0xFF;
    m_iInputCycles = 0;
}

void Input::Tick(unsigned int clockCycles)
{
    m_iInputCycles += clockCycles;

    // Joypad Poll Speed (64 Hz)
    if (m_iInputCycles >= 65536)
    {
        m_iInputCycles -= 65536;
        Update();
    }
}

void Input::KeyPressed(GS_Keys key)
{
    m_JoypadState = UnsetBit(m_JoypadState, key);
}

void Input::KeyReleased(GS_Keys key)
{
    m_JoypadState = SetBit(m_JoypadState, key);
}

void Input::Write(u8 value)
{
    m_P1 = (m_P1 & 0xCF) | (value & 0x30);
    Update();
}

u8 Input::Read()
{
    return m_P1;
}

void Input::Update()
{

}
