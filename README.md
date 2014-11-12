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
- Gearsystem 2.0 for Jailbroken iOS: [Cydia](http://modmyi.com/info/gearsystem.d.php). You can open rom files from other apps like Safari or Dropbox. They can be placed in <code>/var/mobile/Media/ROMs/Gearsystem</code> too. Save files are placed in <code>/var/mobile/Library/Gearsystem</code>
- Gearsystem 2.0 for Non-Jailbroken iOS: You can open rom files from other apps like Safari or Dropbox, or use [iTunes file sharing](http://support.apple.com/kb/ht4094). 
- Gearsystem 1.2 for Windows: [Gearsystem-1.2-Windows.zip](http://www.geardome.com/files/gearsystem/Gearsystem-1.2-Windows.zip) (NOTE: You may need to install the [Microsoft Visual C++ Redistributable](http://www.microsoft.com/en-us/download/details.aspx?id=40784))
- Gearsystem 1.2 for Linux: [Gearsystem-1.2-Linux.tar.gz](http://www.geardome.com/files/gearsystem/Gearsystem-1.0-Linux.tar.gz)

Features
--------
- Highly accurate Z80 core, including undocumented functionality.
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

The best way of compiling Gearboy is by using one of the IDE projects provided for each platform.

For all desktop platforms you will need SDL 2 and Qt 5 SDKs installed and configured.

### iOS
- Install Xcode for Mac OS X. You need iOS SDK 8 or later. 
- Build the project. 
- Run it on real hardware using your iOS developer certificate. Make sure it compiles on Release for extra optimizations.
- For jailbroken devices use the jailbreak branch.

### Raspberry Pi - Raspbian
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development.
- Use <code>make</code> to build the project.
- Sound emulation in the Pi is awfully slow. Use <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator and over clock your Raspberry as much as you can for the best performance.
 
### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the [Qt 5 SDK for Windows](http://qt-project.org/downloads).
- Install the [Qt 5 Visual Studio Add-in](http://qt-project.org/downloads) and point it to the Qt SDK.
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development.
- In order to use OpenGL extensions I used [GLEW](http://glew.sourceforge.net/). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions. Make sure the GLEW headers and libs are configured within VC++.
- Open the Gearsystem Visual Studio project and build.

### Mac OS X
- You need Qt Creator, included in the Qt 5 SDK.
- Install Xcode and run <code>xcode-select --install</code> in the terminal for the compiler to be available on the command line.
- Install the [Qt 5 SDK for Mac OS](http://qt-project.org/downloads).
- Download [SDL 2](http://www.libsdl.org/download-2.0.php) source code. Then run this three commands <code>.configure</code> <code>make</code> <code>sudo make install</code> on the terminal.
- Open the Gearboy Qt project and build.

### Linux
- You need Netbeans 7.3 or later.
- Install Qt 5 development dependencies (Ubuntu: <code>sudo apt-get install qt5-default qttools5-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL 2 development dependencies (Ubuntu: <code>sudo apt-get install libsdl2-dev</code>).
- In order to use OpenGL extensions I used GLEW dependencies (Ubuntu: <code>sudo apt-get install libglew1.8-dev</code>). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions.
- Open the Gearsystem Netbeans project and build.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.
- In Ubuntu 13.10 I had to <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the sound to work properly.

Accuracy Tests
--------------

Zexall Z80 instruction exerciser ([from SMS Power!](http://www.smspower.org/Homebrew/ZEXALL-SMS))

Gearsystem passes all tests in Zexall, including undocumented instructions and behaviours. 

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
