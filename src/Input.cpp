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

Input::Input(Processor* pProcessor)
{
    m_pProccesor = pProcessor;
    m_Joypad1 = 0;
    m_Joypad2 = 0;
    m_IOPortDC = 0;
    m_IOPortDD = 0;
    m_IOPort00 = 0;
    m_iInputCycles = 0;
    m_bGameGear = false;
}

void Input::Init()
{
    Reset(false);
}

void Input::Reset(bool bGameGear)
{
    m_bGameGear = bGameGear;
    m_Joypad1 = 0xFF;
    m_Joypad2 = 0xFF;
    m_IOPortDC = 0xFF;
    m_IOPortDD = 0xFF;
    m_IOPort00 = 0xFF;
    m_GlassesRegistry = 0;
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
    {
        if (!m_bGameGear && (key == Key_Start) && IsSetBit(m_Joypad1, Key_Start))
            m_pProccesor->RequestNMI();
        m_Joypad1 = UnsetBit(m_Joypad1, key);
    }
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

u8 Input::GetPort00()
{
    return m_IOPort00;
}

u8 Input::GetGlassesRegistry()
{
    return m_GlassesRegistry;
}

void Input::SetGlassesRegistry(u8 value)
{
    m_GlassesRegistry = value;
}

void Input::Update()
{
    m_IOPortDC = (m_Joypad1 & 0x3F) + ((m_Joypad2 << 6) & 0xC0);
    m_IOPortDD = ((m_Joypad2 >> 2) & 0x0F) | 0xF0;
    m_IOPort00 = (IsSetBit(m_Joypad1, Key_Start) ? 0x80 : 0) & 0x80;
}

void Input::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_Joypad1), sizeof(m_Joypad1));
    stream.write(reinterpret_cast<const char*> (&m_Joypad2), sizeof(m_Joypad2));
    stream.write(reinterpret_cast<const char*> (&m_IOPortDC), sizeof(m_IOPortDC));
    stream.write(reinterpret_cast<const char*> (&m_IOPortDD), sizeof(m_IOPortDD));
    stream.write(reinterpret_cast<const char*> (&m_IOPort00), sizeof(m_IOPort00));
    stream.write(reinterpret_cast<const char*> (&m_iInputCycles), sizeof(m_iInputCycles));
}

void Input::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_Joypad1), sizeof(m_Joypad1));
    stream.read(reinterpret_cast<char*> (&m_Joypad2), sizeof(m_Joypad2));
    stream.read(reinterpret_cast<char*> (&m_IOPortDC), sizeof(m_IOPortDC));
    stream.read(reinterpret_cast<char*> (&m_IOPortDD), sizeof(m_IOPortDD));
    stream.read(reinterpret_cast<char*> (&m_IOPort00), sizeof(m_IOPort00));
    stream.read(reinterpret_cast<char*> (&m_iInputCycles), sizeof(m_iInputCycles));
}
