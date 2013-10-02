Gearsystem
=======
<b>Copyright &copy; 2013 by Ignacio Sanchez</b>

----------

Gearsystem is a Sega Master System / Game Gear emulator written in C++ that runs on iOS, Raspberry Pi, Mac, Windows and Linux.

The main focus of this emulator is readability of source code with very high compatibility.

Follow me on Twitter for updates: http://twitter.com/drhelius

If you want new features ask for them but don't forget donating, thanks :)

[![PayPal - The safer, easier way to pay online!](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=28YUTJVAH7JH8 "PayPal - The safer, easier way to pay online!")

----------

Downloads
--------
- Gearsystem 1.0 for Jailbroken iOS: Not yet.
- Gearsystem 1.0 for Non-Jailbroken iOS: Not yet. 
- Gearsystem 1.0 for Windows: Not yet.
- Gearsystem 1.0 for Linux: Not yet.

Features
--------
- Highly accurate Z80 core.
- Multi-Mapper support: SEGA, Codemasters, and ROM only cartridges.
- External RAM support with save files.
- Automatic region detection: NTSC-JAP, NTSC-USA, PAL-EUR.
- SMS2 only 224 mode support.
- Internal database for rom detection.
- Sound emulation using SDL Audio and [Sms_Snd_Emu library](http://slack.net/%7Eant/libs/audio.html#Sms_Snd_Emu).
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Compressed rom support (ZIP deflate).
- Multi platform. Runs on Windows, Linux, Mac OS X, Raspberry Pi and iOS.

Compiling Instructions
----------------------

The best way of compiling Gearsystem is by using one of the IDE projects provided for each platform.

For all desktop platforms you will need SDL and Qt Framework SDKs installed and configured. SDL is provided as a framework for iOS project.

There is a nice Netbeans + Qt tutorial [here](http://netbeans.org/kb/docs/cnd/qt-applications.html).

### iOS
- Install Xcode for Mac OS X. You need iOS SDK 5.1 or later.  
- Open the Gearsystem Xcode project and build.
- Run it on real hardware using your iOS developer certificate. For jailbroken devices use the jailbreak branch.

### Raspberry Pi - Raspbian
- Install SDL development dependencies (<code>sudo apt-get install libsdl1.2-dev</code>).
- Use <code>make</code> to build the project.
 
### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the [Qt SDK for Windows](http://qt-project.org/downloads).
- Install the [Qt Visual Studio Add-in](http://qt-project.org/downloads) and point it to the Qt SDK.
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- In order to use OpenGL extensions I used [GLEW](http://glew.sourceforge.net/). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions. Make sure the GLEW headers and libs are configured within VC++.
- Open the Gearsystem Visual Studio project and build.

### Mac OS X
- You need Netbeans 7.3 or later.
- Install Xcode for the compiler to be available on the command line.
- Install the [Qt SDK for Mac OS](http://qt-project.org/downloads).
- Add <code>qmake</code> to the PATH (You can find qmake in the bin directory where you have Qt SDK installed).
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- Open the Gearsystem Netbeans project and build. The project will use <code>clang</code>.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.

### Linux
- You need Netbeans 7.3 or later.
- Install Qt development dependencies (Ubuntu: <code>sudo apt-get install qt4-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL development dependencies (Ubuntu: <code>sudo apt-get install libsdl1.2-dev</code>).
- In order to use OpenGL extensions I used GLEW dependencies (Ubuntu: <code>sudo apt-get install libglew1.6-dev</code>). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions.
- Open the Gearsystem Netbeans project and build.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.
- In Ubuntu 12.04 I had to <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the sound to work properly.

Accuracy Tests
--------------

Zexall Z80 instruction exerciser ([from SMS Power!](http://www.smspower.org/Homebrew/ZEXALL-SMS))

Gearsystem passes all tests in Zexdoc. It passes all tests in Zexall except for BIT n,(HL) due to the undocumented behaviour of the XY flags on this instructions.

![zexall.sms](http://www.geardome.com/files/gearsystem/zexall.png)

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
