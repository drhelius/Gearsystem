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

#ifndef OPCODE_NAMES_H
#define	OPCODE_NAMES_H

struct stOPCodeInfo
{
    const char* name;
    int size;
    int type;
};

#include "opcodexx_names.h"
#include "opcodecb_names.h"
#include "opcodeed_names.h"
#include "opcodedd_names.h"
#include "opcodefd_names.h"
#include "opcodeddcb_names.h"
#include "opcodefdcb_names.h"

#endif	/* OPCODE_NAMES_H */

