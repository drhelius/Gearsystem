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

SmsIOPorts::SmsIOPorts(Audio* pAudio, Video* pVideo, Input* pInput, Cartridge* pCartridge, Memory* pMemory, Processor* pProcessor)
{
    m_pAudio = pAudio;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    m_Port3F = 0;
    m_Port3F_HC = 0;
}

SmsIOPorts::~SmsIOPorts()
{
}

void SmsIOPorts::Reset()
{
    m_Port3F = 0;
    m_Port3F_HC = 0;
}

void SmsIOPorts::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_Port3F), sizeof(m_Port3F));
    stream.write(reinterpret_cast<const char*> (&m_Port3F_HC), sizeof(m_Port3F_HC));
}

void SmsIOPorts::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_Port3F), sizeof(m_Port3F));
    stream.read(reinterpret_cast<char*> (&m_Port3F_HC), sizeof(m_Port3F_HC));
}
