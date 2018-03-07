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
#include "../../../platforms/audio-shared/Sound_Queue.h"

class Emulator
{
public:
    Emulator();
    ~Emulator();
    void Init();
    void RunToVBlank(GS_Color* pFrameBuffer);
    void LoadRom(const char* szFilePath, bool saveInROMFolder);
    void KeyPressed(GS_Joypads joypad, GS_Keys key);
    void KeyReleased(GS_Joypads joypad, GS_Keys key);
    void Pause();
    void Resume();
    bool IsPaused();
    void Reset(bool saveInROMFolder);
    void MemoryDump();
    void SetSoundSettings(bool enabled, int rate);
    void SaveState(int index);
    void LoadState(int index);

private:
    void SaveRam();
    void LoadRam();

private:
    GearsystemCore* m_pGearsystemCore;
    Sound_Queue* m_pSoundQueue;
    QMutex m_Mutex;
    bool m_bAudioEnabled;
    bool m_bSaveInROMFolder;
};

#endif	/* EMULATOR_H */
