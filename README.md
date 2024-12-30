# Gearsystem

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/drhelius/Gearsystem/gearsystem.yml)](https://github.com/drhelius/Gearsystem/actions/workflows/gearsystem.yml)
[![GitHub Releases)](https://img.shields.io/github/v/tag/drhelius/Gearsystem?label=version)](https://github.com/drhelius/Gearsystem/releases)
[![commits)](https://img.shields.io/github/commit-activity/t/drhelius/Gearsystem)](https://github.com/drhelius/Gearsystem/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/drhelius/Gearsystem)](https://github.com/drhelius/Gearsystem/graphs/contributors)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/drhelius)](https://github.com/sponsors/drhelius)
[![License](https://img.shields.io/github/license/drhelius/Gearsystem)](https://github.com/drhelius/Gearsystem/blob/master/LICENSE)
[![Twitter Follow](https://img.shields.io/twitter/follow/drhelius)](https://x.com/drhelius)

Gearsystem is a very accurate, cross-platform Sega Master System / Game Gear / SG-1000 emulator written in C++ that runs on Windows, macOS, Linux, BSD and RetroArch.

This is an open source project with its ongoing development made possible thanks to the support by these awesome [backers](backers.md). If you find it useful, please, consider [sponsoring](https://github.com/sponsors/drhelius).

Don't hesitate to report bugs or ask for new features by [openning an issue](https://github.com/drhelius/Gearsystem/issues).

<img src="http://www.geardome.com/files/gearsystem/gearsystem_debug_01.png">

## Downloads

- **Windows**:
  - [Gearsystem-3.6.1-windows.zip](https://github.com/drhelius/Gearsystem/releases/download/3.6.1/Gearsystem-3.6.1-windows.zip)
  - NOTE: If you have errors you may need to install:
    - [Microsoft Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572)
    - [OpenGL Compatibility Pack](https://apps.microsoft.com/detail/9nqpsl29bfff)
- **macOS**:
  - [Gearsystem-3.6.1-macos-arm.zip](https://github.com/drhelius/Gearsystem/releases/download/3.6.1/Gearsystem-3.6.1-macos-arm.zip)
  - [Gearsystem-3.6.1-macos-intel.zip](https://github.com/drhelius/Gearsystem/releases/download/3.6.1/Gearsystem-3.6.1-macos-intel.zip)
- **Linux**:
  - [Gearsystem-3.6.1-ubuntu-22.04.zip](https://github.com/drhelius/Gearsystem/releases/download/3.6.1/Gearsystem-3.6.1-ubuntu-22.04.zip)
  - [Gearsystem-3.6.1-ubuntu-20.04.zip](https://github.com/drhelius/Gearsystem/releases/download/3.6.1/Gearsystem-3.6.1-ubuntu-20.04.zip) 
  - NOTE: You may need to install `libsdl2` and `libglew`
- **RetroArch**: [Libretro core documentation](https://docs.libretro.com/library/gearcoleco/).

## Supported Machines

- Sega Mark III
- Sega Master System
- Sega Game Gear
- Sega Game 1000 (SG-1000)
- Othello Multivision

## Features

- Accurate Z80 core, including undocumented opcodes and behavior like R and [MEMPTR](https://gist.github.com/drhelius/8497817) registers.
- Supported cartridges: ROM, ROM + RAM, SEGA, Codemasters, Korean, MSX + Nemesis, Janggun, SG-1000 and many Korean multi-carts.
- Automatic region detection: NTSC-JAP, NTSC-USA, PAL-EUR.
- Accurate VDP emulation, including timing and VDP specifics for SMS, SMS2, GG and TMS9918 modes.
- Support for YM2413 (OPLL) FM sound chip.
- Light Phaser and Paddle Control
- Internal database for rom detection.
- Battery powered RAM save support.
- Save states.
- Compressed rom support (ZIP).
- *Game Genie* and *Pro Action Replay* cheat support.
- Supported platforms (standalone): Windows, Linux, BSD and macOS.
- Supported platforms (libretro): Windows, Linux, macOS, Raspberry Pi, Android, iOS, tvOS, PlayStation Vita, PlayStation 3, Nintendo 3DS, Nintendo GameCube, Nintendo Wii, Nintendo WiiU, Nintendo Switch, Emscripten, Classic Mini systems (NES, SNES, C64, ...), OpenDingux, RetroFW and QNX.
- Full debugger with just-in-time disassembler, cpu breakpoints, memory access breakpoints, code navigation (goto address, JP JR and CALL double clicking), debug symbols, memory editor, IO inspector and VRAM viewer including tiles, sprites, backgrounds and palettes.
- Windows and Linux *Portable Mode*.
- Rom loading from the command line by adding the rom path as an argument.
- Support for modern game controllers through [gamecontrollerdb.txt](https://github.com/gabomdq/SDL_GameControllerDB) file located in the same directory as the application binary.

## Tips

- *BIOS*: Gearsystem can run with or without a BIOS. You can optionally load a BIOS and enable it. It is possible to run the title included in the BIOS by setting **Media Slot** to `None` in the **Emulator** menu. This will emulate a machine without any media plugged in.
- *Overscan*: For a precise representation of the original image select **Overscan** `Top+Bottom` and **Aspect Ratio** `Standard (4:3 DAR)` in the **Video** menu. Game Gear will ignore any overscan settings.
- *Mouse Cursor*: Automatically hides when hovering main output window or when Main Menu is disabled.
- *Portable Mode*: Create an empty file named `portable.ini` in the same directory as the application binary to enable portable mode.
- *Debug Symbols*: The emulator always tries to load a symbol file at the same time a rom is being loaded. For example, for ```path_to_rom_file.sms``` it tries to load ```path_to_rom_file.sym```. It is also possible to load a symbol file using the GUI or using the CLI.
- *Command Line Usage*: ```gearsystem [rom_file] [symbol_file]```

## Build Instructions

### Windows

- Install Microsoft Visual Studio Community 2022 or later.
- Open the Gearsystem Visual Studio solution `platforms/windows/Gearsystem.sln` and build.

### macOS

- Install Xcode and run `xcode-select --install` in the terminal for the compiler to be available on the command line.
- Run these commands to generate a Mac *app* bundle:

``` shell
brew install sdl2
cd platforms/macos
make dist
```

### Linux

- Ubuntu / Debian / Raspberry Pi (Raspbian):

``` shell
sudo apt install build-essential libsdl2-dev libglew-dev libgtk-3-dev
cd platforms/linux
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++ SDL2-devel glew-devel gtk3-devel
cd platforms/linux
make
```

### BSD

- FreeBSD:

``` shell
su root -c "pkg install -y git gmake pkgconf SDL2 glew lang/gcc gtk3"
cd platforms/bsd
gmake
```

- NetBSD:

``` shell
su root -c "pkgin install gmake pkgconf SDL2 glew lang/gcc gtk3"
cd platforms/bsd
gmake
```

### Libretro

- Ubuntu / Debian / Raspberry Pi (Raspbian):

``` shell
sudo apt install build-essential
cd platforms/libretro
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++
cd platforms/libretro
make
```

## Accuracy Tests

Zexall Z80 instruction exerciser ([from SMS Power!](http://www.smspower.org/Homebrew/ZEXALL-SMS))

Gearsystem passes all tests in Zexall, including undocumented instructions and behaviours.

![zexall.sms](http://www.geardome.com/files/gearsystem/zexall.png)

SMS VDP Test  ([from SMS Power!](http://www.smspower.org/Homebrew/SMSVDPTest-SMS))

![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest5.png)![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest4.png)
<img src="http://www.geardome.com/files/gearsystem/vdptest7.png" alt="vdptest.sms" width="368"/>

## Screenshots

![Screenshot](http://www.geardome.com/files/gearsystem/01.png)![Screenshot](http://www.geardome.com/files/gearsystem/02.png)
![Screenshot](http://www.geardome.com/files/gearsystem/03.png)![Screenshot](http://www.geardome.com/files/gearsystem/04.png)
![Screenshot](http://www.geardome.com/files/gearsystem/05.png)![Screenshot](http://www.geardome.com/files/gearsystem/06.png)
![Screenshot](http://www.geardome.com/files/gearsystem/07.png)![Screenshot](http://www.geardome.com/files/gearsystem/08.png)
![Screenshot](http://www.geardome.com/files/gearsystem/09.png)![Screenshot](http://www.geardome.com/files/gearsystem/10.png)
![Screenshot](http://www.geardome.com/files/gearsystem/11.png)![Screenshot](http://www.geardome.com/files/gearsystem/12.png)
![Screenshot](http://www.geardome.com/files/gearsystem/13.png)![Screenshot](http://www.geardome.com/files/gearsystem/14.png)
<img src="http://www.geardome.com/files/gearsystem/15.png" width="368" height="326"><img src="http://www.geardome.com/files/gearsystem/16.png" width="368" height="326">
<img src="http://www.geardome.com/files/gearsystem/17.png" width="368" height="326"><img src="http://www.geardome.com/files/gearsystem/18.png" width="368" height="326">

## Contributors

Thank you to all the people who have already contributed to Gearsystem!

[![Contributors](https://contrib.rocks/image?repo=drhelius/gearsystem)]("https://github.com/drhelius/gearsystem/graphs/contributors)

## License

Gearsystem is licensed under the GNU General Public License v3.0 License, see [LICENSE](LICENSE) for more information.
