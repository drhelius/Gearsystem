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

#include <math.h>
#include "Input.h"
#include "Memory.h"
#include "Processor.h"

Input::Input(Processor* pProcessor, Video* pVideo)
{
    m_pProccesor = pProcessor;
    m_pVideo = pVideo;
    m_bGameGear = false;
    m_Joypad1 = 0;
    m_Joypad2 = 0;
    m_GlassesRegistry = 0;
    m_bPhaser = false;
    m_Phaser.x = 0;
    m_Phaser.y = 0;
    m_PhaserOffset.x = 0;
    m_PhaserOffset.y = 0;
    m_bPaddle = false;
    m_Paddle.x = 128.0f;
    m_Paddle.reg = 128;
    m_Paddle.flip = 0;
    m_bResetPressed = false;
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
    m_GlassesRegistry = 0;
    m_Phaser.x = 0;
    m_Phaser.y = 0;
    m_Paddle.x = 128.0f;
    m_Paddle.reg = 128;
    m_Paddle.flip = 0;
    m_bResetPressed = false;
}

void Input::SetReset(bool pressed)
{
    m_bResetPressed = pressed;
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

    if (!m_bGameGear && m_bPhaser && (key == Key_1))
    {
        m_pVideo->SetPhaserCoordinates(m_Phaser.x + m_PhaserOffset.x, m_Phaser.y + m_PhaserOffset.y);
    }
}

void Input::KeyReleased(GS_Joypads joypad, GS_Keys key)
{
    if (joypad == Joypad_1)
        m_Joypad1 = SetBit(m_Joypad1, key);
    else
        m_Joypad2 = SetBit(m_Joypad2, key);
}

void Input::EnablePhaser(bool enable)
{
    Debug("Light Phaser %s", enable ? "enabled" : "disabled");
    m_bPhaser = enable;
}

void Input::SetPhaser(int x, int y)
{
    m_Phaser.x = x;
    m_Phaser.y = y;
}

void Input::SetPhaserOffset(int x, int y)
{
    m_PhaserOffset.x = x;
    m_PhaserOffset.y = y;
}

Input::stPhaser* Input::GetPhaser()
{
    return &m_Phaser;
}

bool Input::IsPhaserEnabled()
{
    return m_bPhaser;
}

void Input::EnablePaddle(bool enable)
{
    Debug("Paddle %s", enable ? "enabled" : "disabled");
    m_bPaddle = enable;
}

void Input::SetPaddle(float x)
{
    m_Paddle.x += x;
    if (m_Paddle.x < 0.0f)
        m_Paddle.x = 0.0f;
    else if (m_Paddle.x > 255.0f)
        m_Paddle.x = 255.0f;

    m_Paddle.reg = (u8)floor(m_Paddle.x + 0.5f);
}

bool Input::IsPaddleEnabled()
{
    return m_bPaddle;
}

u8 Input::GetGlassesRegistry()
{
    return m_GlassesRegistry;
}

void Input::SetGlassesRegistry(u8 value)
{
    m_GlassesRegistry = value;
}

void Input::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_Joypad1), sizeof(m_Joypad1));
    stream.write(reinterpret_cast<const char*> (&m_Joypad2), sizeof(m_Joypad2));
    stream.write(reinterpret_cast<const char*> (&m_GlassesRegistry), sizeof(m_GlassesRegistry));
    stream.write(reinterpret_cast<const char*> (&m_bPhaser), sizeof(m_bPhaser));
    stream.write(reinterpret_cast<const char*> (&m_Phaser), sizeof(m_Phaser));
    stream.write(reinterpret_cast<const char*> (&m_bPaddle), sizeof(m_bPaddle));
    stream.write(reinterpret_cast<const char*> (&m_Paddle), sizeof(m_Paddle));
}

void Input::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_Joypad1), sizeof(m_Joypad1));
    stream.read(reinterpret_cast<char*> (&m_Joypad2), sizeof(m_Joypad2));
    stream.read(reinterpret_cast<char*> (&m_GlassesRegistry), sizeof(m_GlassesRegistry));
    stream.read(reinterpret_cast<char*> (&m_bPhaser), sizeof(m_bPhaser));
    stream.read(reinterpret_cast<char*> (&m_Phaser), sizeof(m_Phaser));
    stream.read(reinterpret_cast<char*> (&m_bPaddle), sizeof(m_bPaddle));
    stream.read(reinterpret_cast<char*> (&m_Paddle), sizeof(m_Paddle));
}
