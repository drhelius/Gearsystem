# Gearsystem Agent Skills

[Agent Skills](https://agentskills.io/) for the Gearsystem Sega Master System / Game Gear / SG-1000 emulator MCP server. These skills teach AI agents how to effectively use Gearsystem's MCP tools for debugging and ROM hacking tasks.

## Prerequisites

All skills require the **Gearsystem emulator** running as an MCP server. The emulator must be configured in your AI client (VS Code, Claude Desktop, Claude Code, etc.) so the agent can access the MCP tools.

See [MCP_README.md](../MCP_README.md) for complete setup instructions (STDIO, HTTP, VS Code, Claude Desktop, Claude Code).

## Installation

The recommended way to install the skills is using the [```skills```](https://skills.sh/docs) CLI, which requires no prior installation:

```bash
npx skills add drhelius/gearsystem
```

Or install a specific skill:

```bash
npx skills add drhelius/gearsystem --skill gearsystem-debugging
npx skills add drhelius/gearsystem --skill gearsystem-romhacking
```

This downloads and configures the skills for use with your AI agent. See the [skills CLI reference](https://skills.sh/docs/cli) for more details.

## Available Skills

### gearsystem-debugging

**Purpose**: Game development, debugging, and tracing of Sega Master System, Game Gear, and SG-1000 games.

**What it covers**:
- Loading ROMs and debug symbols (sjasmplus, SDCC/NoICE, wla-dx, vasm)
- Z80 CPU register and flag inspection (AF, BC, DE, HL, IX, IY, SP, PC, interrupt mode)
- Setting execution, read, write, and range breakpoints across memory areas (rom_ram, vram, cram, vdp_reg)
- IRQ breakpoints (RESET, NMI, INT)
- Stepping through code (into, over, out, frame, run-to)
- Execution tracing with interleaved hardware events (VDP, PSG, YM2413, I/O, bank switching)
- Hardware inspection: VDP (registers, status, modes), PSG (4 channels), YM2413 FM (9 channels)
- Sprite viewer (64 sprites with images)
- Screenshot capture
- Call stack analysis
- Organizing debug sessions with symbols, bookmarks, and watches

**Key MCP tools used**: `debug_pause`, `debug_step_into`, `debug_step_over`, `debug_step_out`, `set_breakpoint`, `toggle_irq_breakpoints`, `get_z80_status`, `get_disassembly`, `get_call_stack`, `get_trace_log`, `get_vdp_registers`, `get_vdp_status`, `get_psg_status`, `get_ym2413_status`, `list_sprites`, `add_symbol`, `get_screenshot`

**Example prompts**:
- "Find the VBlank interrupt handler and analyze what it does"
- "Set a breakpoint at $0066 and step through the NMI handler"
- "The game has corrupted graphics — diagnose the issue"
- "Trace the sprite update routine and explain the algorithm"

### gearsystem-romhacking

**Purpose**: Creating modifications, cheats, translations, and ROM hacks for Sega Master System, Game Gear, and SG-1000 games.

**What it covers**:
- Memory search workflows (capture → change → compare cycle)
- Finding game variables (lives, health, score, position)
- Creating cheats (infinite lives, score modification, etc.)
- Text and string discovery for translations
- Sprite and graphics data location via VRAM inspection
- Data table and structure reverse engineering
- Save state management for safe experimentation
- Fast forwarding to reach specific game states

**Key MCP tools used**: `memory_search_capture`, `memory_search`, `memory_find_bytes`, `read_memory`, `write_memory`, `set_breakpoint` (write type), `add_memory_watch`, `add_memory_bookmark`, `save_state`, `load_state`, `toggle_fast_forward`, `get_screenshot`, `list_sprites`, `get_sprite_image`, `controller_button`

**Example prompts**:
- "Find the lives counter and give me infinite lives"
- "Search for the score variable in memory"
- "Find all text strings in the ROM for translation"
- "Locate the sprite data for the player character"
