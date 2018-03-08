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

#ifndef IOPORTS_H
#define	IOPORTS_H

#include "definitions.h"

class IOPorts
{
public:
    IOPorts() { };
    virtual ~IOPorts() { };
    virtual void Reset() = 0;
    virtual u8 DoInput(u8 port) = 0;
    virtual void DoOutput(u8 port, u8 value) = 0;
    virtual void SaveState(std::ostream& stream) = 0;
    virtual void LoadState(std::istream& stream) = 0;
};

#endif	/* IOPORTS_H */
