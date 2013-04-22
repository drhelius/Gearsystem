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

#ifndef EMULATOR_H
#define	EMULATOR_H

#include <QMutex>
#include "../../../src/gearsystem.h"

class Emulator
{
public:
    Emulator();
    ~Emulator();
    void Init();
    void RunToVBlank(GS_Color* pFrameBuffer);
    void LoadRom(const char* szFilePath, bool forceDMG);
    void KeyPressed(GS_Keys key);
    void KeyReleased(GS_Keys key);
    void Pause();
    void Resume();
    bool IsPaused();
    void Reset(bool forceDMG);
    void MemoryDump();
    void SetSoundSettings(bool enabled, int rate);
    void SaveRam();

private:
    GearsystemCore* m_pGearsystemCore;
    QMutex m_Mutex;
};

#endif	/* EMULATOR_H */

