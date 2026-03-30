---
name: gearsystem-debugging
description: >-
  Debug and trace Sega Master System, Game Gear, and SG-1000 games using the
  Gearsystem emulator MCP server. Provides workflows for Z80 CPU debugging,
  breakpoint management, VDP/PSG/YM2413 hardware inspection, disassembly
  analysis, and execution tracing. Use when the user wants to debug an SMS/GG/SG-1000
  game, trace code execution, inspect Z80 CPU registers or hardware state, set
  breakpoints, analyze interrupts, step through Z80 instructions, reverse
  engineer game code, examine VDP registers, view sprites, inspect PSG or
  YM2413 FM audio, view the call stack, or diagnose rendering, audio, or timing
  issues. Also use when the user mentions Master System development, Game Gear
  homebrew testing, or Z80 debugging with Gearsystem.
compatibility: >-
  Requires the Gearsystem MCP server. Before installing or configuring, call
  debug_get_status to check if the server is already connected. If it responds,
  the server is ready — skip setup entirely.
metadata:
  author: drhelius
  version: "1.0"
---

# Sega Master System / Game Gear / SG-1000 Debugging with Gearsystem

## Overview

Debug Sega Master System, Game Gear, and SG-1000 games using the Gearsystem emulator as an MCP server. Control execution (pause, step, breakpoints), inspect the Z80 CPU and hardware (VDP, PSG, YM2413), read/write memory across multiple areas (ROM+RAM, VRAM, CRAM), disassemble code, trace instructions, view sprites, and capture screenshots — all through MCP tool calls.

## MCP Server Prerequisite

**IMPORTANT — Check before installing:** Before attempting any installation or configuration, you MUST first verify if the Gearsystem MCP server is already connected in your current session. Call `debug_get_status` — if it returns a valid response, the server is active and ready.

Only if the tool is not available or the call fails, you need to help install and configure the Gearsystem MCP server:

### Installing Gearsystem

Run the bundled install script (macOS/Linux):

```bash
bash scripts/install.sh
```

This installs Gearsystem via Homebrew on macOS or downloads the latest release on Linux. It prints the binary path on completion. You can also set `INSTALL_DIR` to control where the binary goes (default: `~/.local/bin`).

Alternatively, download from [GitHub Releases](https://github.com/drhelius/Gearsystem/releases/latest) or install with `brew install --cask drhelius/geardome/gearsystem` on macOS.

### Connecting as MCP Server

Configure your AI client to run Gearsystem as an MCP server via STDIO transport. Example for Claude Desktop (`~/Library/Application Support/Claude/claude_desktop_config.json`):
```json
{
  "mcpServers": {
    "gearsystem": {
      "command": "/path/to/gearsystem",
      "args": ["--mcp-stdio"]
    }
  }
}
```
Replace `/path/to/gearsystem` with the actual binary path from the install script. Add `--headless` before `--mcp-stdio` on headless machines.

---

## Debugging Workflow

### 1. Load and Orient

```
load_media → get_media_info → get_z80_status → get_screenshot
```

Start every session by loading the ROM, confirming it loaded correctly (file path, type, size, mapper, zone, system), then checking Z80 CPU state and taking a screenshot to understand the current game state. If a `.sym` file exists alongside the ROM, symbols are loaded automatically. Gearsystem supports sjasmplus/Pasmo (EQU), SDCC/NoICE (.noi), wla-dx, and vasm/generic symbol formats.

Load additional symbols with `load_symbols` or add individual labels with `add_symbol`.

### 2. Pause and Inspect

Always call `debug_pause` before inspecting state. While paused:

- **CPU state**: `get_z80_status` — registers AF, BC, DE, HL, AF', BC', DE', HL', IX, IY, SP, PC, WZ, I, R, flags (S, Z, H, P/V, N, C), interrupt state, halt status, interrupt mode (0/1/2)
- **Disassembly**: `get_disassembly` with a start/end address range
- **Call stack**: `get_call_stack` — current subroutine hierarchy
- **Memory**: `read_memory` with area name and address/length. Use `list_memory_areas` to see all available areas
- **Sprites**: `list_sprites` — all 64 sprites with position, size, pattern index

### 3. Set Breakpoints

Use breakpoints to stop execution at points of interest:

| Breakpoint Type | Tool | Use Case |
|---|---|---|
| Execution | `set_breakpoint` (type: exec) | Stop when PC reaches address |
| Read | `set_breakpoint` (type: read) | Stop when memory address is read |
| Write | `set_breakpoint` (type: write) | Stop when memory address is written |
| Range | `set_breakpoint_range` | Cover an address range (exec/read/write) |
| IRQ | `toggle_irq_breakpoints` | Break on RESET, NMI, or INT interrupts |

Breakpoints support 4 memory areas: `rom_ram`, `vram`, `cram`, `vdp_reg`. Use the appropriate area depending on what you're debugging.

**Important**: Read/write breakpoints stop with PC at the instruction *after* the memory access.

Manage breakpoints with `list_breakpoints` and `remove_breakpoint`.

### 4. Step Through Code

After hitting a breakpoint or pausing:

| Action | Tool | Behavior |
|---|---|---|
| Step Into | `debug_step_into` | Execute one Z80 instruction, enter subroutines |
| Step Over | `debug_step_over` | Execute one instruction, skip CALL subroutines |
| Step Out | `debug_step_out` | Run until RET returns from current subroutine |
| Step Frame | `debug_step_frame` | Execute until next frame |
| Run To | `debug_run_to_cursor` | Continue until PC reaches target address |
| Continue | `debug_continue` | Resume normal execution |

After each step, call `get_z80_status` and `get_disassembly` to see where you are.

### 5. Trace Execution

The trace logger records Z80 instructions interleaved with hardware events (VDP, PSG, YM2413, I/O, bank switching).

1. `set_trace_log` with `enabled: true` to start recording (optionally filter event types)
2. Let the game run or step through code
3. `set_trace_log` with `enabled: false` to stop (entries are preserved)
4. `get_trace_log` to read recorded entries

Tracing is essential for understanding timing-sensitive code, interrupt handlers, VDP timing, and hardware interaction sequences.

---

## Hardware Inspection

### VDP (Video Display Processor)

- `get_vdp_registers` — all 11 VDP registers (R0-R10) with hex values and descriptions
- `get_vdp_status` — status flags, counters, mode, SG-1000 mode, extended mode 224
- `list_sprites` — all 64 sprites with position, size, pattern index
- `get_sprite_image` — get a specific sprite as base64 PNG

### PSG (SN76489 Sound)

- `get_psg_status` — all 4 channels (3 tone + 1 noise): volume, period, frequency, GG stereo panning

### YM2413 FM Synthesis (Master System only)

- `get_ym2413_status` — 9 FM channels with instruments, key-on state, f-number, block, envelope, rhythm mode, user instrument definition

### Screen Capture

- `get_screenshot` — current rendered frame as PNG

Use screenshots after stepping or continuing to see the visual impact of changes.

---

## Memory Areas

Use `list_memory_areas` to discover all available areas. Breakpoints and memory operations support these key areas:

| Area | Description |
|---|---|
| `rom_ram` | Full Z80 64K address space (ROM + RAM) |
| `vram` | Video RAM — tile patterns, nametable, sprite attribute table |
| `cram` | Color RAM — palette entries |
| `vdp_reg` | VDP register file |

Additional areas for `read_memory`/`write_memory` include individual ROM banks, external RAM, BIOS, and more — use `list_memory_areas` for the complete list.

---

## Common Debugging Scenarios

### Finding an Interrupt Handler

1. `toggle_irq_breakpoints` to enable breaking on NMI (VBlank) or INT (scanline)
2. `debug_continue` to run until the interrupt fires
3. `get_z80_status` + `get_disassembly` to see the handler code
4. `get_call_stack` to see the call hierarchy
5. `add_symbol` to label the handler address and any subroutines it calls

Note: The Z80 NMI vector is fixed at $0066. The INT handler depends on interrupt mode (IM 0/1/2). In IM 1 (most common for SMS), INT jumps to $0038.

### Diagnosing Graphics Corruption

1. `debug_pause` → `get_vdp_registers` — check mode, nametable, sprite table, and pattern addresses
2. `get_vdp_status` — verify status flags, line counter, interrupt state
3. `read_memory` (area: vram) — inspect tile patterns, nametable, and sprite attribute table
4. `read_memory` (area: cram) — check palette data
5. `list_sprites` — verify sprite positions, patterns, and ordering
6. Set read/write breakpoints (area: vram) on display data addresses to catch corruption source

### Analyzing a Subroutine

1. `set_breakpoint` at the subroutine entry point
2. `debug_continue` → when hit, `get_z80_status`
3. Step through with `debug_step_into` / `debug_step_over`
4. After each step: check registers, read relevant memory
5. `add_symbol` for the routine and any called subroutines
6. `add_disassembler_bookmark` to mark interesting locations

### Tracking a Variable

1. `add_memory_watch` on the variable's address — watches are visible in the emulator GUI
2. Set a write breakpoint with `set_breakpoint` (type: write) on that address
3. When hit, `get_disassembly` reveals what code is modifying it
4. `get_call_stack` shows the call chain leading to the write

### Inspecting VDP Timing

1. `toggle_irq_breakpoints` to break on INT (scanline interrupt)
2. `get_vdp_status` to check the line counter and flags
3. `get_trace_log` to see interleaved Z80 + VDP events
4. Check VDP register writes in the trace for mid-frame register changes (raster effects)

### Debugging Sound

1. `get_psg_status` — check all 4 PSG channels (tone frequencies, volumes, noise mode)
2. `get_ym2413_status` — check FM synthesis state (instruments, key-on, channels)
3. Set write breakpoints on PSG I/O port ($7E-$7F) or YM2413 ports ($F0-$F1) to catch sound writes
4. Step through the sound driver code and correlate register writes with audio output

---

## Organizing Your Debug Session

- **Symbols**: Use `add_symbol` liberally to label addresses — makes disassembly readable
- **Bookmarks**: Use `add_disassembler_bookmark` for code locations and `add_memory_bookmark` for data regions
- **Watches**: Use `add_memory_watch` for variables you're tracking across steps
- **Save states**: Use `save_state` / `load_state` to snapshot and restore emulator state at interesting points
- **Screenshots**: Capture visual state with `get_screenshot` after significant changes
- **Modify registers**: Use `write_z80_register` to change register values live (AF, BC, DE, HL, IX, IY, SP, PC, etc.)
