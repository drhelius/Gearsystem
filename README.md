# Gearsystem

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/drhelius/Gearsystem/gearsystem.yml)](https://github.com/drhelius/Gearsystem/actions/workflows/gearsystem.yml)
[![GitHub Releases)](https://img.shields.io/github/v/tag/drhelius/Gearsystem?label=version)](https://github.com/drhelius/Gearsystem/releases)
[![commits)](https://img.shields.io/github/commit-activity/t/drhelius/Gearsystem)](https://github.com/drhelius/Gearsystem/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/drhelius/Gearsystem)](https://github.com/drhelius/Gearsystem/graphs/contributors)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/drhelius)](https://github.com/sponsors/drhelius)
[![License](https://img.shields.io/github/license/drhelius/Gearsystem)](https://github.com/drhelius/Gearsystem/blob/master/LICENSE)
[![Twitter Follow](https://img.shields.io/twitter/follow/drhelius)](https://x.com/drhelius)

Gearsystem is a very accurate, cross-platform Sega Master System / Game Gear / SG-1000 emulator written in C++ that runs on Windows, macOS, Linux, BSD and RetroArch, with an embedded MCP server for debugging and tooling.

This is an open source project with its ongoing development made possible thanks to the support by these awesome [backers](backers.md). If you find it useful, please consider [sponsoring](https://github.com/sponsors/drhelius).

Don't hesitate to report bugs or ask for new features by [opening an issue](https://github.com/drhelius/Gearsystem/issues).

<img src="http://www.geardome.com/files/gearsystem/gearsystem_debug_03.png">

## Downloads

<table>
  <thead>
    <tr>
      <th>Platform</th>
      <th>Architecture</th>
      <th>Download Link</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="2"><strong>Windows</strong></td>
      <td>Desktop x64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-windows-x64.zip">Gearsystem-3.9.5-desktop-windows-x64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop ARM64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-windows-arm64.zip">Gearsystem-3.9.5-desktop-windows-arm64.zip</a></td>
    </tr>
    <tr>
      <td rowspan="3"><strong>macOS</strong></td>
      <td>Homebrew</td>
      <td><code>brew install --cask drhelius/geardome/gearsystem</code></td>
    </tr>
    <tr>
      <td>Desktop Apple Silicon</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-macos-arm64.zip">Gearsystem-3.9.5-desktop-macos-arm64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop Intel</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-macos-intel.zip">Gearsystem-3.9.5-desktop-macos-intel.zip</a></td>
    </tr>
    <tr>
      <td rowspan="4"><strong>Linux</strong></td>
      <td>Ubuntu PPA</td>
      <td><a href="https://github.com/drhelius/ppa-geardome">drhelius/ppa-geardome</a></td>
    </tr>
    <tr>
      <td>Desktop Ubuntu 24.04 x64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-ubuntu24.04-x64.zip">Gearsystem-3.9.5-desktop-ubuntu24.04-x64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop Ubuntu 22.04 x64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-ubuntu22.04-x64.zip">Gearsystem-3.9.5-desktop-ubuntu22.04-x64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop Ubuntu 24.04 ARM64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.5/Gearsystem-3.9.5-desktop-ubuntu24.04-arm64.zip">Gearsystem-3.9.5-desktop-ubuntu24.04-arm64.zip</a></td>
    </tr>
    <tr>
      <td><strong>MCPB</strong></td>
      <td>All platforms</td>
      <td><a href="MCP_README.md">MCP Readme</a></td>
    </tr>
    <tr>
      <td><strong>RetroArch</strong></td>
      <td>All platforms</td>
      <td><a href="https://docs.libretro.com/library/gearsystem/">Libretro core documentation</a></td>
    </tr>
    <tr>
      <td><strong>Dev Builds</strong></td>
      <td>All platforms</td>
      <td><a href="https://github.com/drhelius/Gearsystem/actions/workflows/gearsystem.yml">GitHub Actions</a></td>
    </tr>
  </tbody>
</table>

**Notes:**
- **Windows**: May need [Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572) and [OpenGL Compatibility Pack](https://apps.microsoft.com/detail/9nqpsl29bfff)
- **Linux**: May need `libsdl3`

## Supported Machines

- Sega Mark III
- Sega Master System
- Sega Game Gear
- Sega Game 1000 (SG-1000)
- Othello Multivision

## Features

- Very accurate Z80, VDP, PSG and FM emulation.
- Supported cartridges: ROM, ROM + RAM, SEGA, Codemasters, Korean, MSX + Nemesis, Janggun, SG-1000, and many Korean multi-carts.
- Automatic region detection: NTSC-JAP, NTSC-USA, PAL-EUR.
- Support for YM2413 (OPLL) FM sound chip.
- Light Phaser and Paddle Control support.
- Internal database for ROM detection.
- Battery powered RAM save support.
- Save states with preview.
- Compressed ROM support (ZIP).
- *Game Genie* and *Pro Action Replay* cheat support.
- VGM recorder.
- Supported platforms (standalone): Windows, Linux, BSD and macOS.
- Supported platforms (libretro): Windows, Linux, macOS, Raspberry Pi, Android, iOS, tvOS, webOS, PlayStation Vita, PlayStation 3, Nintendo 3DS, Nintendo GameCube, Nintendo Wii, Nintendo WiiU, Nintendo Switch, Emscripten, Classic Mini systems (NES, SNES, C64, ...), OpenDingux, RetroFW and QNX.
- Full debugger with just-in-time disassembler, CPU breakpoints, memory access breakpoints, code navigation (goto address, JP JR and CALL double clicking), debug symbols, automatic labels, memory editor, trace logger, IO inspector and VRAM viewer including tiles, sprites, backgrounds and palettes.
- MCP server for AI-assisted debugging with GitHub Copilot, Claude, ChatGPT and similar, exposing tools for execution control, memory inspection, hardware status, and more.
- Windows and Linux *Portable Mode*.
- ROM loading from the command line by adding the ROM path as an argument.
- ROM loading using drag & drop.
- Support for modern game controllers through [gamecontrollerdb.txt](https://github.com/mdqinc/SDL_GameControllerDB) file located in the same directory as the application binary.

## Tips

### Basic Usage
- **BIOS**: Gearsystem can run with or without a BIOS. You can optionally load a BIOS and enable it. It's possible to run the title included in the BIOS by setting **Media Slot** to `None` in the **Emulator** menu. This emulates a machine without any media plugged in.
- **Overscan**: For a precise representation of the original image, select **Overscan** `Top+Bottom` and **Aspect Ratio** `Standard (4:3 DAR)` in the **Video** menu. Game Gear will ignore any overscan settings.
- **Mouse Cursor**: Automatically hides when hovering over the main output window or when Main Menu is disabled.
- **Portable Mode**: Create an empty file named `portable.ini` in the same directory as the application binary to enable portable mode.

### Debugging Features
- **Docking Windows**: In debug mode, you can dock windows together by pressing SHIFT and dragging a window onto another.
- **Multi-viewport**: In Windows or macOS, you can enable "multi-viewport" in the debug menu. You must restart the emulator for the change to take effect. Once enabled, you can drag debugger windows outside the main window.
- **Single Instance**: You can enable "Single Instance" in the ```Emulator``` menu. When enabled, opening a ROM while another instance is running will send the ROM to the running instance instead of starting a new one.
- **Debug Symbols**: The emulator automatically tries to load a symbol file when loading a ROM (.sym, .noi). For example, for ```path_to_rom_file.sms``` it tries to load ```path_to_rom_file.sym```. You can also load a symbol file using the GUI or the CLI. It supports sjasmplus/Pasmo (EQU), SDCC/NoICE (.noi), wla-dx and vasm/generic file formats.

### Command Line Usage
```
gearsystem [options] [rom_file] [symbol_file]

Arguments:
  [rom_file]               ROM file: accepts ROMs (.sms, .gg, .sg, .mv) or ZIP (.zip)
  [symbol_file]            Optional symbol file for debugging

Options:
  -f, --fullscreen         Start in fullscreen mode
  -w, --windowed           Start in windowed mode with menu visible
      --mcp-stdio          Auto-start MCP server with stdio transport
      --mcp-http           Auto-start MCP server with HTTP transport
      --mcp-http-port N    HTTP port for MCP server (default: 7777)
      --headless           Run without GUI (requires --mcp-stdio or --mcp-http)
  -v, --version            Display version information
  -h, --help               Display this help message
```

### MCP Server

Gearsystem includes a [Model Context Protocol](https://modelcontextprotocol.io/introduction) (MCP) server that enables AI-assisted debugging through AI agents like GitHub Copilot, Claude, ChatGPT and similar. The server provides tools for execution control, memory inspection, breakpoints, disassembly, hardware status, and more.

For complete setup instructions and tool documentation, see [MCP_README.md](MCP_README.md).

### Agent Skills

Gearsystem provides [Agent Skills](https://agentskills.io/) that teach AI assistants how to effectively use the emulator for specific tasks:

- **[gearsystem-debugging](skills/gearsystem-debugging/SKILL.md)** — Game debugging, code tracing, breakpoint management, hardware inspection, and reverse engineering.
- **[gearsystem-romhacking](skills/gearsystem-romhacking/SKILL.md)** — Cheat creation, memory searching, ROM data modification, text translation, and game patching.

Install with `npx skills add drhelius/gearsystem`. See the [skills README](skills/README.md) for details.

## Build Instructions

### Windows

- Install Microsoft Visual Studio Community 2022 or later.
- Download the latest SDL3 VC development libraries from [SDL3 Releases](https://github.com/libsdl-org/SDL/releases) (the file named `SDL3-devel-x.y.z-VC.zip`).
- Extract the archive and rename the resulting folder (e.g. `SDL3-x.y.z`) to `SDL3`.
- Place the `SDL3` folder inside `platforms/windows/dependencies/` so that the include path is `platforms/windows/dependencies/SDL3/include/SDL3/`.
- Open the Gearsystem Visual Studio solution `platforms/windows/Gearsystem.sln` and build.

### macOS

- Install Xcode and run `xcode-select --install` in the terminal for the compiler to be available on the command line.
- Run these commands to generate a Mac *app* bundle:

``` shell
brew install sdl3
cd platforms/macos
make dist
```

### Linux

- Ubuntu / Debian / Raspberry Pi (Raspbian):

If you are using Ubuntu 25.04 or later, you can install SDL3 directly. Use the following commands to build:

``` shell
sudo apt install build-essential libsdl3-dev
cd platforms/linux
make
```

For older Ubuntu versions (22.04, 24.04), you need to build SDL3 from source first. Use the following commands to build both SDL3 and Gearlynx:

``` shell
sudo apt install build-essential cmake \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev \
  libxi-dev libxss-dev libxkbcommon-dev libwayland-dev libdecor-0-dev \
  libdrm-dev libgbm-dev libgl1-mesa-dev libegl1-mesa-dev libdbus-1-dev libudev-dev libxtst-dev
SDL3_TAG=$(curl -s https://api.github.com/repos/libsdl-org/SDL/releases/latest | jq -r '.tag_name')
git clone --depth 1 --branch "$SDL3_TAG" https://github.com/libsdl-org/SDL.git /tmp/SDL3
cmake -S /tmp/SDL3 -B /tmp/SDL3/build -DCMAKE_INSTALL_PREFIX=/usr -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
cmake --build /tmp/SDL3/build -j$(nproc)
sudo cmake --install /tmp/SDL3/build
cd platforms/linux
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++ SDL3-devel
cd platforms/linux
make
```

- Arch Linux:

``` shell
sudo pacman -S base-devel sdl3
cd platforms/linux
make
```

### BSD

- FreeBSD:

``` shell
su root -c "pkg install -y git gmake pkgconf SDL3 lang/gcc"
cd platforms/bsd
gmake
```

- NetBSD:

``` shell
su root -c "pkgin install gmake pkgconf SDL3 lang/gcc"
cd platforms/bsd
gmake
```

- OpenBSD

``` shell
doas pkg_add gmake sld3
cd platforms/bsd
LDFLAGS=-L/usr/X11R6/lib/ USE_CLANG=1 gmake
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

<img src="http://www.geardome.com/files/gearsystem/zexall.png" alt="zexall.png" width="256"/>

SMS VDP Test  ([from SMS Power!](http://www.smspower.org/Homebrew/SMSVDPTest-SMS))

<img src="http://www.geardome.com/files/gearsystem/vdptest10.png" alt="vdptest1.sms" width="256"/><img src="http://www.geardome.com/files/gearsystem/vdptest11.png" alt="vdptest2.sms" width="256"/><img src="http://www.geardome.com/files/gearsystem/vdptest12.png" alt="vdptest3.sms" width="256"/>

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

[![Contributors](https://contrib.rocks/image?repo=drhelius/gearsystem)](https://github.com/drhelius/gearsystem/graphs/contributors)

## License

Gearsystem is licensed under the GNU General Public License v3.0 License, see [LICENSE](LICENSE) for more information.
