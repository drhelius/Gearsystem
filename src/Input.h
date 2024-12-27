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

public:
    Input(Processor* pProcessor, Video* pVideo);
    void Init();
    void Reset(bool bGameGear);
    void KeyPressed(GS_Joypads joypad, GS_Keys key);
    void KeyReleased(GS_Joypads joypad, GS_Keys key);
    void EnablePhaser(bool enable);
    void SetPhaser(int x, int y);
    bool IsPhaserEnabled();
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
};

#endif	/* INPUT_H */
