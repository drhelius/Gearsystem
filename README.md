Gearsystem
=======
<b>Copyright &copy; 2013 by Ignacio Sanchez</b>

----------
[![Build Status](https://travis-ci.org/drhelius/Gearsystem.svg?branch=master)](https://travis-ci.org/drhelius/Gearsystem)

Gearsystem is a Sega Master System / Game Gear emulator written in C++ that runs on iOS, Raspberry Pi, Mac, Windows, Linux and RetroArch.

Follow me on Twitter for updates: http://twitter.com/drhelius

----------

Downloads
--------
- **iOS**: Build Gearsystem with Xcode and transfer it to your device. You can open rom files from other apps like Safari or Dropbox, or use [iTunes file sharing](http://support.apple.com/kb/ht4094).
- **Mac OS X**: <code>brew install gearsystem</code>
- **Windows**: [Gearsystem-2.5.1-Windows.zip](https://github.com/drhelius/Gearsystem/releases/download/gearsystem-2.5.1/Gearsystem-2.5.0-Windows.zip) (NOTE: You may need to install the [Microsoft Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572))
- **Linux**: [Gearsystem-2.5.1-Linux.tar.xz](https://github.com/drhelius/Gearsystem/releases/download/gearsystem-2.5.1/Gearsystem-2.5.0-Linux.tar.xz)
- **RetroArch**: [Libretro core documentation](https://docs.libretro.com/library/gearsystem/).
- **Raspberry Pi**: Build Gearsystem from sources. Optimized projects are provided for Raspberry Pi 1, 2 and 3.

Features
--------
- Highly accurate Z80 core, including undocumented opcodes and behaviour like R and [MEMPTR](https://gist.github.com/drhelius/8497817) registers.
- Multi-Mapper support: SEGA, Codemasters, and ROM only cartridges.
- External RAM support with save files.
- Automatic region detection: NTSC-JAP, NTSC-USA, PAL-EUR.
- Highly accurate VDP emulation including timing and SMS2 only 224 mode support.
- Internal database for rom detection.
- Audio emulation using SDL Audio and [Sms_Snd_Emu library](http://slack.net/%7Eant/libs/audio.html#Sms_Snd_Emu).
- Saves battery powered RAM cartridges to file.
- Save states.
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Compressed rom support (ZIP deflate).
- Game Genie and Pro Action Replay cheat support.
- Multi platform. Runs on Windows, Linux, Mac OS X, Raspberry Pi, iOS and as a libretro core (RetroArch).

Build Instructions
----------------------

### iOS
- Install Xcode for Mac OS X. You need iOS SDK 8 or later.
- Build the project <code>platforms/ios/Gearsystem.xcodeproj</code>
- Run it on real hardware using your iOS developer certificate. Make sure it builds on Release for better performance.

### Raspberry Pi 2 & 3 - Raspbian
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development:
``` shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libasound2-dev libjpeg-dev libtiff5-dev libwebp-dev automake
cd ~
wget https://www.libsdl.org/release/SDL2-2.0.9.tar.gz
tar zxvf SDL2-2.0.9.tar.gz
cd SDL2-2.0.9 && mkdir build && cd build
../configure --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl --host=armv7l-raspberry-linux-gnueabihf
make -j 4
sudo make install
```
- Install libconfig library dependencies for development: <code>sudo apt-get install libconfig++-dev</code>.
- Use <code>make -j 4</code> in the <code>platforms/raspberrypi3/x64/</code> folder to build the project.
- Use <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the best performance.
- Gearsystem generates a <code>gearsystem.cfg</code> configuration file where you can customize keyboard and gamepads. Key codes are from [SDL](https://wiki.libsdl.org/SDL_Keycode).

### Windows
- You need Visual Studio 2017 or later.
- Install the [Qt 5 Open Source SDK for Windows](https://www.qt.io/download/).
- Install the [QtVisualStudioTools Extension](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools-19123) and point it to the Qt SDK.
- Open the Gearsystem Visual Studio solution <code>platforms/windows/Gearsystem/Gearsystem.sln</code> and build.

### Mac OS X
- You need Qt Creator, included in the Qt 5 SDK.
- Install Xcode and run <code>xcode-select --install</code> in the terminal for the compiler to be available on the command line.
- Install the [Qt 5 SDK for Mac OS](https://www.qt.io/download/).
- Download [SDL 2](http://www.libsdl.org/download-2.0.php) source code. Then run this commands:
``` shell
./configure
make
sudo make install
```
- Open the <code>platforms/macosx/Gearsystem/Gearsystem.pro</code> project file with Qt Creator and build.

### Linux
- Ubuntu / Debian:
``` shell
sudo apt-get install build-essential qt5-default qttools5-dev-tools freeglut3-dev libsdl2-dev libglew-dev
cd platforms/linux/Gearsystem
qmake Gearsystem.pro && make
```
- Fedora:
``` shell
sudo dnf install @development-tools gcc-c++ qt5-devel freeglut-devel SDL2-devel glew-devel
cd platforms/linux/Gearsystem
qmake-qt5 Gearsystem.pro && make
```

Accuracy Tests
--------------

Zexall Z80 instruction exerciser ([from SMS Power!](http://www.smspower.org/Homebrew/ZEXALL-SMS))

Gearsystem passes all tests in Zexall, including undocumented instructions and behaviours.

![zexall.sms](http://www.geardome.com/files/gearsystem/zexall.png)

SMS VDP Test  ([from SMS Power!](http://www.smspower.org/Homebrew/SMSVDPTest-SMS))

![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest5.png)![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest4.png)![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest6.png)

Screenshots
-----------

![Screenshot](http://www.geardome.com/files/gearsystem/01.png)![Screenshot](http://www.geardome.com/files/gearsystem/02.png)
![Screenshot](http://www.geardome.com/files/gearsystem/03.png)![Screenshot](http://www.geardome.com/files/gearsystem/04.png)
![Screenshot](http://www.geardome.com/files/gearsystem/05.png)![Screenshot](http://www.geardome.com/files/gearsystem/06.png)
![Screenshot](http://www.geardome.com/files/gearsystem/07.png)![Screenshot](http://www.geardome.com/files/gearsystem/08.png)
![Screenshot](http://www.geardome.com/files/gearsystem/09.png)![Screenshot](http://www.geardome.com/files/gearsystem/10.png)
![Screenshot](http://www.geardome.com/files/gearsystem/11.png)![Screenshot](http://www.geardome.com/files/gearsystem/12.png)
![Screenshot](http://www.geardome.com/files/gearsystem/13.png)![Screenshot](http://www.geardome.com/files/gearsystem/14.png)

License
-------

<i>Gearsystem - Sega Master System / Game Gear Emulator</i>

<i>Copyright (C) 2013  Ignacio Sanchez</i>

<i>This program is free software: you can redistribute it and/or modify</i>
<i>it under the terms of the GNU General Public License as published by</i>
<i>the Free Software Foundation, either version 3 of the License, or</i>
<i>any later version.</i>

<i>This program is distributed in the hope that it will be useful,</i>
<i>but WITHOUT ANY WARRANTY; without even the implied warranty of</i>
<i>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the</i>
<i>GNU General Public License for more details.</i>

<i>You should have received a copy of the GNU General Public License</i>
<i>along with this program.  If not, see http://www.gnu.org/licenses/</i>
