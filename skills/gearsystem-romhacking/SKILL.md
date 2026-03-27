---
name: gearsystem-romhacking
description: >-
  Hack, modify, and translate Sega Master System, Game Gear, and SG-1000 ROMs
  using the Gearsystem emulator MCP server. Provides workflows for memory
  searching, value discovery, cheat creation, data modification, sprite/text
  finding, and translation patching. Use when the user wants to create cheats,
  find game values in memory, modify ROM data, translate an SMS/GG game, patch
  game behavior, create ROM hacks, discover hidden content, change sprites or
  graphics, find text strings, apply Game Genie or Pro Action Replay codes, do
  infinite lives or health hacks, search for score or item counters, or reverse
  engineer data structures in Sega Master System, Game Gear, or SG-1000 games.
  Also use for any ROM hacking, memory poking, or game modification task
  involving Gearsystem.
compatibility: >-
  Requires the Gearsystem MCP server. Before installing or configuring, call
  debug_get_status to check if the server is already connected. If it responds,
  the server is ready — skip setup entirely.
metadata:
  author: drhelius
  version: "1.0"
---

# Sega Master System / Game Gear / SG-1000 ROM Hacking with Gearsystem

## Overview

Hack, modify, and translate Sega Master System, Game Gear, and SG-1000 ROMs using the Gearsystem emulator as an MCP server. Search memory for game variables, create cheats, find text strings for translation, locate sprite data, inspect VRAM/CRAM, and reverse engineer data structures — all through MCP tool calls. Use save states as checkpoints and fast forward to reach specific game states. The emulator also supports Game Genie and Pro Action Replay codes natively.

## MCP Server Prerequisite

**IMPORTANT — Check before installing:** Before attempting any installation or configuration, you MUST first verify if the Gearsystem MCP server is already connected in your current session. Call `debug_get_status` — if it returns a valid response, the server is active and ready.

Only if the tool is not available or the call fails, you need to help install and configure the Gearsystem MCP server:

### Installing Gearsystem

Run the bundled install script (macOS/Linux):

```bash
bash scripts/install.sh
```

This installs Gearsystem via Homebrew on macOS or downloads the latest release on Linux. It prints the binary path on completion. You can also set `GEARSYSTEM_INSTALL_DIR` to control where the binary goes (default: `~/.local/bin`).

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

## Core Technique: Memory Search

Memory search is the primary tool for ROM hacking. It uses a capture → change → compare cycle to isolate memory addresses holding game values.

### The Search Loop

```
1. memory_search_capture    → snapshot current memory state
2. (change the value in-game using controller_button, fast forward, etc.)
3. memory_search            → compare against snapshot to find changed addresses
4. Repeat 2-3 until only a few candidates remain
5. read_memory / write_memory → verify and modify the found addresses
```

### Search Operators and Types

`memory_search` supports these **operators**: `<`, `>`, `==`, `!=`, `<=`, `>=`

**Compare types**:
- `previous` — compare current value to last captured snapshot (most common)
- `value` — compare current value to a specific number
- `address` — compare current value to value at another address

**Data types**: `hex`, `signed`, `unsigned`

### Example: Finding the Lives Counter

```
1. memory_search_capture                         → snapshot with 3 lives
2. Lose a life in-game (play or use controller_button)
3. memory_search (operator: <, compare: previous) → values that decreased
4. memory_search_capture                         → snapshot with 2 lives
5. Lose another life
6. memory_search (operator: <, compare: previous) → narrow further
7. Or use: memory_search (operator: ==, compare: value, value: 1)
   → find addresses holding exactly 1
8. write_memory on the candidate address to set lives to 99
9. get_screenshot to verify the change took effect
```

### Example: Finding a Score Counter

Score values are often stored as multi-byte (16-bit little-endian on Z80):

```
1. memory_search_capture                                → snapshot at score 0
2. Score some points in-game
3. memory_search (operator: >, compare: previous)       → values that increased
4. memory_search_capture
5. Score more points
6. memory_search (operator: >, compare: previous)       → narrow down
7. read_memory on candidates — look for values matching current score
8. write_memory to set a custom score
```

For 16-bit values: the low byte is at address N, high byte at N+1 (Z80 is little-endian). Many SMS games use BCD (Binary Coded Decimal) for score display — each nibble holds a single digit (e.g., score 1234 stored as $12 $34).

---

## Fast Forward for Efficiency

Use fast forward to speed through gameplay when you need to trigger in-game changes:

```
set_fast_forward_speed (4 = unlimited)
toggle_fast_forward              → enable
(play through the game section)
toggle_fast_forward              → disable
```

This is essential when you need to reach specific game states without waiting in real-time.

---

## Save States as Checkpoints

Save states are critical for ROM hacking — they let you save your position and retry modifications:

```
select_save_state_slot (1-5)     → pick a slot
save_state                       → save current state
(try modifications)
load_state                       → revert if something breaks
```

Use different slots for different game states (e.g., slot 1 = start, slot 2 = boss fight, slot 3 = specific level).

`list_save_state_slots` shows all slots with ROM name, timestamp, and validity.

---

## Finding and Modifying Game Data

### Text and String Discovery

To find text strings for translation or modification:

1. Determine the character encoding — SMS/GG games often use custom character maps stored as tile patterns in VRAM, not ASCII
2. `read_memory` across ROM areas scanning for known byte patterns
3. Use `memory_find_bytes` to search for specific byte sequences
4. Set read breakpoints on suspected text addresses with `set_breakpoint` (type: read) to confirm they're read by the text rendering routine
5. `get_screenshot` to correlate displayed text with memory contents
6. `read_memory` (area: vram) to examine the tile patterns used for font rendering

### Sprite and Graphics Data

1. `list_sprites` — view all 64 sprites with positions, sizes, and pattern indices
2. `get_sprite_image` — capture individual sprite images as PNG
3. `read_memory` (area: vram) — examine tile pattern data and sprite attribute table
4. `read_memory` (area: cram) — inspect palette colors
5. `get_vdp_registers` — check pattern generator and sprite attribute table base addresses
6. Set read breakpoints (area: vram) on sprite data to find the rendering code
7. `get_screenshot` before/after modifications to see visual changes

### VRAM and Tile Data

The VDP uses VRAM for all graphics:

- **Tile patterns**: 8x8 pixel tiles, 4 bits per pixel (16 colors per palette)
- **Nametable**: Background tile map referencing tile patterns
- **Sprite Attribute Table (SAT)**: Y position, X position, tile index for each sprite

Use `read_memory` (area: vram) with the base addresses from `get_vdp_registers` to access these.

### Data Tables and Structures

1. `debug_pause` → `get_disassembly` around code that loads data
2. Look for LD instructions with indexed or indirect addressing — these point to data tables
3. `read_memory` at the target addresses to dump the table contents
4. `add_memory_bookmark` to mark discovered data regions
5. `add_symbol` to label data table entry points for future reference

---

## Creating Cheats

### Infinite Lives / Health

```
1. Find the address using the search loop (above)
2. Set a write breakpoint: set_breakpoint (type: write) on the address
3. debug_continue → when it hits, get_disassembly to see the decrement code
4. Note the instruction (e.g., DEC (HL) or LD (addr),A)
5. Option A: Periodically write_memory to reset the value (simple poke cheat)
6. Option B: Identify the decrement routine for a NOP patch
```

### Watching Values in Real-Time

Use `add_memory_watch` on discovered addresses. Watches appear in the emulator's GUI memory editor, letting you monitor values as the game runs — useful for verifying cheats work across different game situations.

### Write Breakpoint Technique

The most powerful cheat-finding technique:

1. Find the variable address via memory search
2. `set_breakpoint` (type: write) on that address
3. `debug_continue` — the emulator stops when the game writes to that address
4. `get_z80_status` + `get_disassembly` reveals the exact code modifying the value
5. `get_call_stack` shows what triggered the write
6. You now know exactly where and how the game manages that variable

---

## Translation Workflow

### 1. Identify the Font System

1. `get_screenshot` of a screen with text
2. `read_memory` (area: vram) — find tile patterns used for font characters
3. Set read breakpoints on tile data addresses to trace back to the rendering code
4. `get_disassembly` to find the character mapping table (byte value → tile index)
5. `add_symbol` to label the font table and rendering routine

### 2. Find String Data

1. Look for sequential text bytes in ROM using `read_memory` with large ranges
2. Use `memory_find_bytes` to search for known byte patterns
3. Cross-reference with the character table to decode strings
4. `add_memory_bookmark` to mark each string location

### 3. Measure Space Constraints

ROM hacking translations must fit within existing space:

1. `read_memory` to determine how much space each string occupies
2. Check for string terminators (commonly $00, $FF, or length-prefixed)
3. If the translation is longer, look for unused ROM space or abbreviate

### 4. Apply and Test

1. `write_memory` to patch translated strings into memory
2. `get_screenshot` to verify rendering
3. `save_state` before each change so you can `load_state` if it breaks
4. Test all screens that display modified text

---

## Memory Map Quick Reference

Use `list_memory_areas` to get the full list. Key areas:

| Area | Description | Typical Use |
|---|---|---|
| `rom_ram` | Full Z80 64K address space | General game code and variables |
| `vram` | Video RAM (16KB) | Tile patterns, nametable, sprite table |
| `cram` | Color RAM | Palette entries (32 bytes SMS, 64 bytes GG) |
| `vdp_reg` | VDP registers | Display configuration |

### SMS Memory Layout (rom_ram)

| Address | Content |
|---|---|
| $0000-$03FF | ROM (first KB, always mapped) |
| $0038 | INT handler (IM 1) |
| $0066 | NMI handler (pause button) |
| $0400-$3FFF | ROM bank 0 (16K, slot 0) |
| $4000-$7FFF | ROM bank (slot 1, switchable) |
| $8000-$BFFF | ROM bank (slot 2, switchable) or cartridge RAM |
| $C000-$DFFF | System RAM (8K) |
| $E000-$FFFF | RAM mirror |
| $FFFC | RAM/ROM select register |
| $FFFD | Bank slot 0 register |
| $FFFE | Bank slot 1 register |
| $FFFF | Bank slot 2 register |

RAM ($C000-$DFFF) is the most common location for game variables (lives, health, score, position).

---

## Bookmarks and Organization

Keep your hacking session organized:

- `add_memory_bookmark` — mark discovered data regions, variable locations, string tables
- `add_memory_watch` — track values that change during gameplay
- `add_symbol` — label addresses in disassembly for readability
- `add_disassembler_bookmark` — mark code routines you've identified

Use `list_memory_bookmarks`, `list_memory_watches`, `list_symbols`, `list_disassembler_bookmarks` to review.

---

## Persisting Changes

Changes made via `write_memory` to ROM areas are applied to the emulator's in-memory copy only — they are **not** persisted to the ROM file on disk. To create a permanent patch, use command-line tools (e.g., a binary patch script) to apply the discovered modifications to the actual ROM file.
