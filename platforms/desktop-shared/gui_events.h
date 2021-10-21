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

#ifndef GUI_EVENTS_H
#define	GUI_EVENTS_H

#include <SDL.h>

// Define the gui event enums.
enum gui_ShortCutEvent
{
	gui_ShortCutEventFirst = -1,
#define GUI_EVENT(name) gui_Shortcut##name,
#include "gui_events.def"
#undef GUI_EVENT
	gui_ShortCutEventMax
};

#ifdef __APPLE__
#define GUI_EVENT_PLATFORM_KEY_UPPERCASE "CMD"
#define GUI_EVENT_PLATFORM_KEY_CAMELCASE "Cmd"
#elif _WIN64 || defined(_WIN32)
#define GUI_EVENT_PLATFORM_KEY_UPPERCASE "WIN"
#define GUI_EVENT_PLATFORM_KEY_CAMELCASE "Win"
#else
#define GUI_EVENT_PLATFORM_KEY_UPPERCASE "META"
#define GUI_EVENT_PLATFORM_KEY_CAMELCASE "Meta"
#endif

const char* gui_event_get_name(gui_ShortCutEvent event);
void gui_event_get_shortcut_string(char* buffer, int bufferSize, gui_ShortCutEvent shortcutEvent);
const char* gui_event_get_modifier_name(SDL_Keymod modifier);

#endif	/* GUI_EVENTS_H */
