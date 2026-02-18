   ____                               _                  
  / ___| ___  __ _ _ __ ___ _   _ ___| |_ ___ _ __ ___   
 | |  _ / _ \/ _` | '__/ __| | | / __| __/ _ \ '_ ` _ \  
 | |_| |  __/ (_| | |  \__ \ |_| \__ \ ||  __/ | | | | | 
  \____|\___|\__,_|_|  |___/\__, |___/\__\___|_| |_| |_| 
                            |___/                        
-----------------------------------------------------
Instructions and tips at:
https://github.com/drhelius/Gearsystem
-----------------------------------------------------
Gearsystem is a very accurate, cross-platform Sega Master System / Game Gear / SG-1000 emulator written in C++ that runs on Windows, macOS, Linux, BSD and RetroArch.

This is an open source project with its ongoing development made possible thanks to the support by these awesome backers. If you find it useful, please consider sponsoring: https://github.com/sponsors/drhelius

Don't hesitate to report bugs or ask for new features by opening an issue: https://github.com/drhelius/Gearsystem/issues

Follow me on Twitter for updates: https://x.com/drhelius
-----------------------------------------------------
Supported Machines:
    - Sega Mark III
    - Sega Master System
    - Sega Game Gear
    - Sega Game 1000 (SG-1000)
    - Othello Multivision
-----------------------------------------------------
Features:
    - Accurate Z80 core, including undocumented opcodes and behaviors like R and MEMPTR registers (https://gist.github.com/drhelius/8497817).
    - Supported cartridges: ROM, ROM + RAM, SEGA, Codemasters, Korean, MSX + Nemesis, Janggun, SG-1000, and many Korean multi-carts.
    - Automatic region detection: NTSC-JAP, NTSC-USA, PAL-EUR.
    - Accurate VDP emulation, including timing and VDP specifics for SMS, SMS2, GG, and TMS9918 modes.
    - Support for YM2413 (OPLL) FM sound chip.
    - Light Phaser and Paddle Control support.
    - Internal database for ROM detection.
    - Battery powered RAM save support.
    - Save states.
    - Compressed ROM support (ZIP).
    - Game Genie and Pro Action Replay cheat support.
    - VGM recorder.
    - Supported platforms (standalone): Windows, Linux, BSD and macOS.
    - Supported platforms (libretro): Windows, Linux, macOS, Raspberry Pi, Android, iOS, tvOS, PlayStation Vita, PlayStation 3, Nintendo 3DS, Nintendo GameCube, Nintendo Wii, Nintendo WiiU, Nintendo Switch, Emscripten, Classic Mini systems (NES, SNES, C64, ...), OpenDingux, RetroFW and QNX.
    - Full debugger with just-in-time disassembler, CPU breakpoints, memory access breakpoints, code navigation (goto address, JP JR and CALL double clicking), debug symbols, memory editor, IO inspector and VRAM viewer including tiles, sprites, backgrounds and palettes.
    - Windows and Linux Portable Mode.
    - ROM loading from the command line by adding the ROM path as an argument.
    - ROM loading using drag & drop.
    - Support for modern game controllers through gamecontrollerdb.txt file (https://github.com/mdqinc/SDL_GameControllerDB) located in the same directory as the application binary.
-----------------------------------------------------
Tips:

Basic Usage:
    - BIOS: Gearsystem can run with or without a BIOS. You can optionally load a BIOS and enable it. It's possible to run the title included in the BIOS by setting Media Slot to "None" in the Emulator menu. This emulates a machine without any media plugged in.
    - Overscan: For a precise representation of the original image, select Overscan "Top+Bottom" and Aspect Ratio "Standard (4:3 DAR)" in the Video menu. Game Gear will ignore any overscan settings.
    - Mouse Cursor: Automatically hides when hovering over the main output window or when Main Menu is disabled.
    - Portable Mode: Create an empty file named "portable.ini" in the same directory as the application binary to enable portable mode.

Debugging Features:
    - Docking Windows: In debug mode, you can dock windows together by pressing SHIFT and dragging a window onto another.
    - Multi-viewport: In Windows or macOS, you can enable "multi-viewport" in the debug menu. You must restart the emulator for the change to take effect. Once enabled, you can drag debugger windows outside the main window.
    - Debug Symbols: The emulator automatically tries to load a symbol file when loading a ROM. For example, for "path_to_rom_file.sms" it tries to load "path_to_rom_file.sym". You can also load a symbol file using the GUI or the CLI.

Command Line Usage:
    gearsystem [options] [rom_file] [symbol_file]

    Options:
      -f, --fullscreen    Start in fullscreen mode
      -w, --windowed      Start in windowed mode with menu visible
      -v, --version       Display version information
      -h, --help          Display this help message
-----------------------------------------------------
Gearsystem is licensed under the GNU General Public License v3.0 License.

Copyright (C) 2013 Ignacio Sanchez

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/
