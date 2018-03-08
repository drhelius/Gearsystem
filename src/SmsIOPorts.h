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

#ifndef SMSIOPORTS_H
#define	SMSIOPORTS_H

#include "IOPorts.h"

class Audio;
class Video;
class Input;
class Cartridge;

class SmsIOPorts : public IOPorts
{
public:
    SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge);
    virtual ~SmsIOPorts();
    void Reset();
    virtual u8 DoInput(u8 port);
    virtual void DoOutput(u8 port, u8 value);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream);
private:
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    u8 m_Port3F;
    u8 m_Port3F_HC;
};

#endif	/* SMSIOPORTS_H */
