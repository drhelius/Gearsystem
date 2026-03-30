# Gearsystem MCP Server

A [Model Context Protocol](https://modelcontextprotocol.io/introduction) server for the Gearsystem emulator, enabling AI-assisted debugging and development of Sega Master System / Game Gear / SG-1000 games.

This server provides tools for game development, rom hacking, reverse engineering, and debugging through standardized MCP protocols compatible with AI agents like GitHub Copilot, Claude, ChatGPT and others.

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
      <td>x64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.3/Gearsystem-3.9.3-mcpb-windows-x64.mcpb">Gearsystem-3.9.3-mcpb-windows-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>ARM64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.3/Gearsystem-3.9.3-mcpb-windows-arm64.mcpb">Gearsystem-3.9.3-mcpb-windows-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td rowspan="2"><strong>macOS</strong></td>
      <td>x64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.3/Gearsystem-3.9.3-mcpb-macos-x64.mcpb">Gearsystem-3.9.3-mcpb-macos-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>ARM64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.3/Gearsystem-3.9.3-mcpb-macos-arm64.mcpb">Gearsystem-3.9.3-mcpb-macos-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td rowspan="2"><strong>Linux</strong></td>
      <td>x64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.3/Gearsystem-3.9.3-mcpb-linux-x64.mcpb">Gearsystem-3.9.3-mcpb-linux-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>ARM64</td>
      <td><a href="https://github.com/drhelius/Gearsystem/releases/download/3.9.3/Gearsystem-3.9.3-mcpb-linux-arm64.mcpb">Gearsystem-3.9.3-mcpb-linux-arm64.mcpb</a></td>
    </tr>
  </tbody>
</table>

## Features

- **Full Debugger Access**: CPU registers, memory inspection, breakpoints, and execution control
- **Multiple Memory Areas**: Access RAM, VRAM, CRAM, ROM banks, external RAM, BIOS, and more
- **Disassembly**: View disassembled Z80 code around PC or any address
- **Hardware Inspection**: Z80 CPU, VDP, PSG, YM2413 FM synthesis
- **Sprite Viewer**: List and inspect all 64 sprites with images
- **Symbol Support**: Add, remove, and list debug symbols
- **Bookmarks**: Memory and disassembler bookmarks for navigation
- **Call Stack**: View function call hierarchy
- **Trace Logger**: CPU instruction trace with interleaved hardware events (VDP, PSG, YM2413, I/O, bank switching)
- **Screenshot Capture**: Get current frame as PNG image
- **GUI Integration**: MCP server runs alongside the emulator GUI, sharing the same state

## Transport Modes

The Gearsystem MCP server supports two transport modes:

### STDIO Transport (Recommended)

The default mode uses standard input/output for communication. The emulator is launched by the AI client and communicates through stdin/stdout pipes.

### HTTP Transport

The HTTP transport mode runs the emulator with an embedded web server on `localhost:7777/mcp`. The emulator stays running independently while the AI client connects via HTTP.

### Headless Mode

Add `--headless` to run without a GUI window. This is useful for servers, CLI agents, or any machine without a display. All MCP tools work identically in headless mode. Requires `--mcp-stdio` or `--mcp-http`.

## Quick Start

### STDIO Mode with VS Code

1. **Install [GitHub Copilot extension](https://code.visualstudio.com/docs/copilot/overview)** in VS Code

2. **Configure VS Code settings**:

   Add to your workspace folder a file named `.vscode/mcp.json` with:

   ```json
   {
     "servers": {
       "gearsystem": {
         "command": "/path/to/gearsystem",
         "args": ["--mcp-stdio"]
       }
     }
   }
   ```

   **Important:** Update the `command` path to match your build location:
   - **macOS:** `/path/to/gearsystem`
   - **Linux:** `/path/to/gearsystem`
   - **Windows:** `C:/path/to/gearsystem.exe`

3. **Restart VS Code** may be necessary for settings to take effect

4. **Open GitHub Copilot Chat** and start debugging:
   - The emulator will auto-start with MCP server enabled
   - Load a game ROM
   - Start chatting with Copilot about the game state

### STDIO Mode with Claude Desktop

#### Option 1: Desktop Extension (Recommended)

The easiest way to install Gearsystem MCP server on Claude Desktop is using the MCPB package:

1. **Download the latest MCPB package** for your platform from the [releases page](https://github.com/drhelius/gearsystem/releases).

2. **Install the extension**:
   - Open Claude Desktop
   - Navigate to **Settings > Extensions**
   - Click **Advanced settings**
   - In the Extension Developer section, click **Install Extension…**
   - Select the downloaded `.mcpb` file

3. **Start debugging**: The extension is now available in your conversations. The emulator will automatically launch when the tool is enabled.

#### Option 2: Manual Configuration

If you prefer to build from source or configure manually:

1. **Edit Claude Desktop config file**:

   Follow [these instructions](https://modelcontextprotocol.io/quickstart/user#for-claude-desktop-users) to access Claude's config file, then edit it to include:

   ```json
   {
     "mcpServers": {
       "gearsystem": {
         "command": "/path/to/gearsystem/platforms/macos/gearsystem",
         "args": ["--mcp-stdio"]
       }
     }
   }
   ```

   **Config file locations:**
   - **macOS:** `~/Library/Application Support/Claude/claude_desktop_config.json`
   - **Windows:** `%APPDATA%\Claude\claude_desktop_config.json`
   - **Linux:** `~/.config/Claude/claude_desktop_config.json`

   **Important:** Update the `command` path to match your build location.

2. **Restart Claude Desktop**

### STDIO Mode with Claude Code

1. **Add the Gearsystem MCP server** using the CLI:
   ```bash
   claude mcp add --transport stdio gearsystem -- /path/to/gearsystem --mcp-stdio
   ```

   **Important:** Update the path to match your build location.

2. **Verify the server was added**:
   ```bash
   claude mcp list
   ```

3. **Start debugging**: Open Claude Code and start chatting about the game state. The emulator will auto-start when tools are invoked.

### HTTP Mode

1. **Start the emulator manually** with HTTP transport:
   ```bash
   ./gearsystem --mcp-http
   # Server will start on http://localhost:7777/mcp

   # Or specify a custom port:
   ./gearsystem --mcp-http --mcp-http-port 3000
   # Server will start on http://localhost:3000/mcp
   ```

   You can optionally start the server using the "MCP" menu in the GUI.

2. **Configure VS Code** `.vscode/mcp.json`:
   ```json
   {
     "servers": {
       "gearsystem": {
         "type": "http",
         "url": "http://localhost:7777/mcp",
         "headers": {}
       }
     }
   }
   ```

3. **Or configure Claude Desktop**:
   ```json
   {
     "mcpServers": {
       "gearsystem": {
         "type": "http",
         "url": "http://localhost:7777/mcp"
       }
     }
   }
   ```

4. **Or configure Claude Code**:
   ```bash
   claude mcp add --transport http gearsystem http://localhost:7777/mcp
   ```

5. **Restart your AI client** and start debugging

> **Note:** The MCP HTTP Server must be running standalone before connecting the AI client.

## Usage Examples

Once configured, you can ask your AI assistant:

### Basic Commands

- "What game is currently loaded?"
- "Load the ROM at /path/to/game.sms"
- "Show me the current CPU registers"
- "Read 16 bytes from RAM starting at 0xC000"
- "Set a breakpoint at address 0x0066"
- "Pause execution and show me all sprites"
- "Step through the next 5 instructions"
- "Capture a screenshot of the current frame"
- "Tap the up button on player 1 controller"

### Advanced Debugging Workflows

- "Find the VBlank interrupt handler, analyze what it does, and add symbols for all the subroutines it calls"

- "Locate the sprite update routine. Study how this game manages its sprite system, explain the algorithm, and add bookmarks to key sections. Also add watches for any sprite-related variables you find"

- "There's a data decompression routine around address 0x8000. Step through it instruction by instruction, reverse engineer the compression algorithm, and explain how it works with examples"

- "Find where the game stores its level data in ROM. Analyze the data structure format, create a memory map showing each section, and add symbols for the data tables"

- "The game is rendering corrupted graphics. Examine the VDP registers, check the VRAM contents, inspect the sprite attribute table, and diagnose what's causing the corruption. Set up watches on relevant memory addresses"

## Available MCP Tools

The server exposes tools organized in the following categories:

### Execution Control
- `debug_pause` - Pause emulation
- `debug_continue` - Resume emulation
- `debug_step_into` - Step one Z80 instruction
- `debug_step_over` - Step over subroutine calls
- `debug_step_out` - Step out of current subroutine
- `debug_step_frame` - Step one frame
- `debug_run_to_cursor` - Continue execution until reaching specified address
- `debug_reset` - Reset emulation
- `debug_get_status` - Get debug status (paused, at_breakpoint, pc address)

### CPU & Registers
- `write_z80_register` - Set register value (AF, BC, DE, HL, AF', BC', DE', HL', IX, IY, SP, PC, WZ, A, F, B, C, D, E, H, L, I, R)
- `get_z80_status` - Get complete Z80 CPU status (registers, flags, interrupts, halt, interrupt mode)

### Memory Operations
- `list_memory_areas` - List all available memory areas
- `read_memory` - Read from specific memory area
- `write_memory` - Write to specific memory area
- `get_memory_selection` - Get current memory selection range
- `select_memory_range` - Select a range of memory addresses
- `set_memory_selection_value` - Set all bytes in selection to specified value
- `add_memory_bookmark` - Add bookmark in memory area
- `remove_memory_bookmark` - Remove memory bookmark
- `list_memory_bookmarks` - List all bookmarks in memory area
- `add_memory_watch` - Add watch (tracked memory location)
- `remove_memory_watch` - Remove memory watch
- `list_memory_watches` - List all watches in memory area
- `memory_search_capture` - Capture memory snapshot for search comparison
- `memory_search` - Search memory with operators (<, >, ==, !=, <=, >=), compare types (previous, value, address), and data types (hex, signed, unsigned)

### Disassembly & Debugging
- `get_disassembly` - Get Z80 disassembly for specified address range
- `add_symbol` - Add symbol (label) at specified address
- `remove_symbol` - Remove symbol
- `list_symbols` - List all defined symbols
- `add_disassembler_bookmark` - Add bookmark in disassembler
- `remove_disassembler_bookmark` - Remove disassembler bookmark
- `list_disassembler_bookmarks` - List all disassembler bookmarks
- `get_call_stack` - View function call hierarchy
- `get_trace_log` - Read trace logger entries (CPU + hardware events). Start the trace logger from the debugger window first

### Breakpoints
- `set_breakpoint` - Set execution, read, or write breakpoint (supports 4 memory areas: rom_ram, vram, cram, vdp_reg)
- `set_breakpoint_range` - Set breakpoint for an address range (supports 4 memory areas)
- `remove_breakpoint` - Remove breakpoint
- `list_breakpoints` - List all breakpoints
- `toggle_irq_breakpoints` - Enable or disable breaking on IRQs (RESET, NMI, INT)

### Hardware Status
- `get_vdp_registers` - Get all 11 VDP registers (R0-R10) with hex values and descriptions
- `get_vdp_status` - Get VDP status (flags, counters, mode, SG-1000 mode, extended mode 224)
- `get_psg_status` - Get SN76489 PSG status for all 4 channels (3 tone + 1 noise): volume, period, frequency, GG stereo
- `get_ym2413_status` - Get YM2413 FM synth status: 9 channels, instruments, key-on, f-number, block, envelope, rhythm mode, user instrument

### Sprites
- `list_sprites` - List all 64 sprites with position, size, pattern index
- `get_sprite_image` - Get sprite image as base64 PNG

### Screen Capture
- `get_screenshot` - Capture current screen frame as base64 PNG

### Media & State Management
- `get_media_info` - Get loaded ROM info (file path, type, size, mapper, zone, system)
- `load_media` - Load ROM file (.sms, .gg, .sg, .zip). Automatically loads .sym symbol file if present
- `load_symbols` - Load debug symbols from file (.sym format with 'BANK:ADDRESS LABEL' entries)
- `list_save_state_slots` - List all 5 save state slots with information (rom name, timestamp, validity)
- `select_save_state_slot` - Select active save state slot (1-5) for save/load operations
- `save_state` - Save emulator state to currently selected slot
- `load_state` - Load emulator state from currently selected slot
- `set_fast_forward_speed` - Set fast forward speed multiplier (0: 1.5x, 1: 2x, 2: 2.5x, 3: 3x, 4: Unlimited)
- `toggle_fast_forward` - Toggle fast forward mode on/off

### Controller Input
- `controller_button` - Control a button on a controller (player 1-2). Use action 'press' to hold the button, 'release' to let it go, or 'press_and_release' to simulate a quick tap. Buttons: up, down, left, right, 1, 2, start

## How MCP Works in Gearsystem

- The MCP server runs **alongside** the GUI in a background thread
- The emulator GUI remains fully functional (you can play/debug normally while using MCP)
- Commands from the AI are queued and executed on the GUI thread
- Both GUI and MCP share the same emulator state
- Changes made through MCP are instantly reflected in the GUI and vice versa

## Architecture

### STDIO Transport
```
┌─────────────────┐                    ┌──────────────────┐
│   VS Code /     │       stdio        │   Gearsystem     │
│ Claude Desktop  │◄──────────────────►│    MCP Server    │
│   (AI Client)   │       pipes        │  (background)    │
└─────────────────┘                    └──────────────────┘
        │                                       │
        └───► Launches ►────────────────────────┘
                                                │
                                                │ Shared State
                                                ▼
                                       ┌──────────────────┐
                                       │   Emulator Core  │
                                       │   + GUI Window   │
                                       └──────────────────┘
```

### HTTP Transport
```
┌─────────────────┐                    ┌──────────────────┐
│   VS Code /     │  HTTP (port 7777)  │   Gearsystem     │
│ Claude Desktop  │◄──────────────────►│ MCP HTTP Server  │
│   (AI Client)   │                    │    (listener)    │
└─────────────────┘                    └──────────────────┘
                                                │
                                                │ Shared State
                                                ▼
                                       ┌──────────────────┐
                                       │   Emulator Core  │
                                       │   + GUI Window   │
                                       └──────────────────┘
```
