# Gearsystem

[![Build Status](https://travis-ci.org/drhelius/Gearsystem.svg?branch=master)](https://travis-ci.org/drhelius/Gearsystem)

Gearsystem is a cross-platform Sega Master System / Game Gear / SG-1000 emulator written in C++ that runs on iOS, Raspberry Pi, macOS, Windows, Linux and RetroArch.

Please, consider [sponsoring](https://github.com/sponsors/drhelius) and following me on [Twitter](https://twitter.com/drhelius) for updates.

----------

## Downloads

- **Windows**: [Gearsystem-3.0.0-Windows.zip](https://github.com/drhelius/Gearsystem/releases/download/gearsystem-3.0.0/Gearsystem-3.0.0-Windows.zip)
  - NOTE: You may need to install the [Microsoft Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572)
- **macOS**:
  - `brew cask install gearsystem`
  - Or install manually: [Gearsystem-3.0.0-macOS.zip](https://github.com/drhelius/Gearsystem/releases/download/gearsystem-3.0.0/Gearsystem-3.0.0-macOS.zip)
- **Linux**: [Gearsystem-3.0.0-Linux.tar.xz](https://github.com/drhelius/Gearsystem/releases/download/gearsystem-3.0.0/Gearsystem-3.0.0-Linux.tar.xz)
- **iOS**: Build Gearsystem with Xcode and transfer it to your device. You can open rom files from other apps like Safari or Dropbox, or use your iCloud Drive.
- **RetroArch**: [Libretro core documentation](https://docs.libretro.com/library/gearsystem/).
- **Raspberry Pi**: Build Gearsystem from sources. Optimized projects are provided for Raspberry Pi 1, 2 and 3.

## Supported Machines

- Sega Mark III
- Sega Master System
- Sega Game Gear
- Sega Game 1000 (SG-1000)
- Othello Multivision

## Features

- Accurate Z80 core, including undocumented opcodes and behaviour like R and [MEMPTR](https://gist.github.com/drhelius/8497817) registers.
- Multi-Mapper support: SEGA, Codemasters, Korean, SG-1000, and ROM only cartridges.
- External RAM support with save files.
- Automatic region detection: NTSC-JAP, NTSC-USA, PAL-EUR.
- Accurate VDP emulation including timing and SMS2 only 224 mode support.
- Internal database for rom detection.
- Audio emulation using SDL Audio and [Sms_Snd_Emu library](http://blargg.8bitalley.com/libs/audio.html#Sms_Snd_Emu).
- Saves battery powered RAM cartridges to file.
- Save states.
- Compressed rom support (ZIP deflate).
- Game Genie and Pro Action Replay cheat support.
- Supported platforms: Windows, Linux, macOS, Raspberry Pi, iOS and RetroArch (libretro).

## Build Instructions

### Windows

- You need Microsoft Visual Studio Community 2019 or later.
- Open the Gearsystem Visual Studio solution `platforms/windows/Gearsystem.sln` and build.
- You may want to use the `platforms/windows/Makefile` to build the application using MinGW.

### macOS

- Install Xcode and run `xcode-select --install` in the terminal for the compiler to be available on the command line.
- Run this commands to generate a Mac *app* bundle:

``` shell
brew install sdl2
cd platforms/macos
make dist
```

### Linux

- Ubuntu / Debian:

``` shell
sudo apt-get install build-essential libsdl2-dev libglew-dev
cd platforms/linux
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++ SDL2-devel glew-devel
cd platforms/linux
make
```

### iOS

- Install Xcode for macOS. You need iOS 13 SDK or later.
- Build the project `platforms/ios/Gearsystem.xcodeproj`
- Run it on real hardware using your iOS developer certificate. Make sure it builds on *Release* for better performance.

### Libretro

- Ubuntu / Debian:

``` shell
sudo apt-get install build-essential
cd platforms/libretro
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++
cd platforms/libretro
make
```

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

- Install libconfig library dependencies for development: `sudo apt-get install libconfig++-dev`
- Use `make -j 4` in the `platforms/raspberrypi3/x64/` folder to build the project.
- Use `export SDL_AUDIODRIVER=ALSA` before running the emulator for the best performance.
- Gearsystem generates a `gearsystem.cfg` configuration file where you can customize keyboard and gamepads. Key codes are from [SDL](https://wiki.libsdl.org/SDL_Keycode).


## Accuracy Tests

Zexall Z80 instruction exerciser ([from SMS Power!](http://www.smspower.org/Homebrew/ZEXALL-SMS))

Gearsystem passes all tests in Zexall, including undocumented instructions and behaviours.

![zexall.sms](http://www.geardome.com/files/gearsystem/zexall.png)

SMS VDP Test  ([from SMS Power!](http://www.smspower.org/Homebrew/SMSVDPTest-SMS))

![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest5.png)![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest4.png)![vdptest.sms](http://www.geardome.com/files/gearsystem/vdptest6.png)

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

## License

Gearsystem is licensed under the GNU General Public License v3.0 License, see [LICENSE](LICENSE) for more information.
