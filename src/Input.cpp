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

Input::Input()
{
    m_Joypad1 = 0;
    m_Joypad2 = 0;
    m_IOPortDC = 0;
    m_IOPortDD = 0;
    m_iInputCycles = 0;
}

void Input::Init()
{
    Reset();
}

void Input::Reset()
{
    m_Joypad1 = 0xFF;
    m_Joypad2 = 0xFF;
    m_IOPortDC = 0xFF;
    m_IOPortDD = 0xFF;
    m_iInputCycles = 0;
}

void Input::Tick(unsigned int clockCycles)
{
    m_iInputCycles += clockCycles;

    // Joypad Poll Speed (60 Hz)
    if (m_iInputCycles >= 71591)
    {
        m_iInputCycles -= 71591;
        Update();
    }
}

void Input::KeyPressed(GS_Joypads joypad, GS_Keys key)
{
    if (joypad == Joypad_1)
        m_Joypad1 = UnsetBit(m_Joypad1, key);
    else
        m_Joypad2 = UnsetBit(m_Joypad2, key);
}

void Input::KeyReleased(GS_Joypads joypad, GS_Keys key)
{
    if (joypad == Joypad_1)
        m_Joypad1 = SetBit(m_Joypad1, key);
    else
        m_Joypad2 = SetBit(m_Joypad2, key);
}

u8 Input::GetPortDC()
{
    return m_IOPortDC;
}

u8 Input::GetPortDD()
{
    return m_IOPortDD;
}

void Input::Update()
{
    m_IOPortDC = (m_Joypad1 & 0x3F) + ((m_Joypad2 << 6) & 0xC0);
    m_IOPortDD = ((m_Joypad2 >> 2) & 0x0F);       
}
