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

#include "SmsIOPorts.h"
#include "Audio.h"
#include "Video.h"
#include "Input.h"

SmsIOPorts::SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput)
{
    m_pAudio = pAudio;
    m_pVideo = pVideo;
    m_pInput = pInput;
}

SmsIOPorts::~SmsIOPorts()
{
}

u8 SmsIOPorts::DoInput(u8 port)
{
    return 0x00;
}

void SmsIOPorts::DoOutput(u8 port, u8 value)
{
    if ((port >= 0x40) && (port < 0x80))
    {
        // Writes to any address go to the SN76489 PSG
        m_pAudio->WriteAudioRegister(value);
    }
}
