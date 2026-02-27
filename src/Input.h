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

#ifndef INPUT_H
#define	INPUT_H

#include "definitions.h"

class Memory;
class Processor;
class Video;

class Input
{
public:
    struct stPhaser
    {
        int x;
        int y;
    };

    struct stPaddle
    {
        float x;
        u8 reg;
        bool flip;
    };

public:
    Input(Processor* pProcessor, Video* pVideo);
    void Init();
    void Reset(bool bGameGear);
    void KeyPressed(GS_Joypads joypad, GS_Keys key);
    void KeyReleased(GS_Joypads joypad, GS_Keys key);
    void SetReset(bool pressed);
    void EnablePhaser(bool enable);
    void SetPhaser(int x, int y);
    void SetPhaserOffset(int x, int y);
    stPhaser* GetPhaser();
    bool IsPhaserEnabled();
    void EnablePaddle(bool enable);
    void SetPaddle(float x);
    bool IsPaddleEnabled();
    u8 GetPortDC();
    u8 GetPortDD();
    u8 GetPort00();
    u8 GetGlassesRegistry();
    void SetGlassesRegistry(u8 value);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream);

private:
    Processor* m_pProccesor;
    Video* m_pVideo;
    u8 m_Joypad1;
    u8 m_Joypad2;
    u8 m_GlassesRegistry;
    bool m_bGameGear;
    bool m_bPhaser;
    stPhaser m_Phaser;
    stPhaser m_PhaserOffset;
    bool m_bPaddle;
    stPaddle m_Paddle;
    bool m_bResetPressed;
};

#include "Video.h"

inline u8 Input::GetPortDC()
{
    if (m_bPhaser && !m_bGameGear)
    {
        return IsSetBit(m_Joypad1, Key_1) ? 0xFF : 0xEF;
    }
    else if (m_bPaddle && !m_bGameGear)
    {
        m_Paddle.flip ^= 0x01;
        u8 paddle_bits = (m_Paddle.flip == 0x00) ? m_Paddle.reg : (m_Paddle.reg >> 4);
        return (m_Joypad1 & 0x10) + (paddle_bits & 0x0F) + ((m_Joypad2 << 6) & 0xC0) + (m_Paddle.flip << 5);
    }
    else
    {
        return (m_Joypad1 & 0x3F) + ((m_Joypad2 << 6) & 0xC0);
    }
}

inline u8 Input::GetPortDD()
{
    u8 dd;
    if (m_bPhaser && !m_bGameGear)
    {
        dd = ((m_Joypad2 >> 2) & 0x0F) | 0xF0;

        if (m_pVideo->IsPhaserDetected())
            dd = UnsetBit(dd, 6);
    }
    else
    {
        dd = ((m_Joypad2 >> 2) & 0x0F) | 0xF0;
    }

    if (!m_bGameGear && m_bResetPressed)
        dd = UnsetBit(dd, 4);

    return dd;
}

inline u8 Input::GetPort00()
{
    return (IsSetBit(m_Joypad1, Key_Start) ? 0x80 : 0) & 0x80;
}

#endif	/* INPUT_H */
