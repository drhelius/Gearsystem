# Gearsystem Debugger Migration Plan

## From Geargrafx (HuC6280 / PC Engine) → Gearsystem (Z80 / SMS-GG)

### Goal

Adapt Gearsystem's debugger to match Geargrafx's feature set. Maximize shared code (verbatim where possible). The GUI shell has already been copied over; this plan focuses on the **CPU-level infrastructure**, **disassembler**, **breakpoints**, **symbols/labels/bookmarks**, **call stack**, **stepping**, **trace logger**, and **debug settings persistence**.

---

## Table of Contents

1. [Disassembler Record Structure](#1-disassembler-record-structure)
2. [Disassembler Record Storage & Management](#2-disassembler-record-storage--management)
3. [Disassembly Routine](#3-disassembly-routine)
4. [Breakpoint System](#4-breakpoint-system)
5. [Call Stack Tracking](#5-call-stack-tracking)
6. [Main Loop Debug Integration (RunToVBlank)](#6-main-loop-debug-integration-runtovblank)
7. [Emu Debug Layer (Stepping)](#7-emu-debug-layer-stepping)
8. [Opcode Names & Color-Coded Format Strings](#8-opcode-names--color-coded-format-strings)
9. [Symbols, Labels & Bookmarks](#9-symbols-labels--bookmarks)
10. [Hardware Debug Labels](#10-hardware-debug-labels)
11. [CPU Register Window](#11-cpu-register-window)
12. [Disassembler GUI Window](#12-disassembler-gui-window)
13. [Breakpoints GUI Window](#13-breakpoints-gui-window)
14. [Call Stack GUI Window](#14-call-stack-gui-window)
15. [Trace Logger](#15-trace-logger)
16. [Debug Settings Persistence](#16-debug-settings-persistence)
17. [Compile-Time Disassembler Toggle](#17-compile-time-disassembler-toggle)
18. [Implementation Order](#18-implementation-order)

---

## 1. Disassembler Record Structure

### Current State

**Geargrafx** (`GG_Disassembler_Record` in `src/types.h` L142–L160): 17 fields.
**Gearsystem** (`stDisassembleRecord` in `src/Memory.h` L34–L44): 8 fields.

### Fields to Add to Gearsystem

| Field | Type | Purpose | Z80 Equivalent |
|-------|------|---------|----------------|
| `segment` | `char[8]` | Memory type label ("ROM", "RAM", etc.) | Needed: "ROM", "RAM", "SRAM", "BIOS", "IO" |
| `jump_bank` | `u8` | Bank of jump target | Yes, for cross-bank jumps |
| `subroutine` | `bool` | True for CALL/RST instructions | Yes: `CALL`, `CALL cc`, `RST xx` |
| `irq` | `int` | IRQ vector index for auto-symbols | Yes: INT handler at $0038, NMI at $0066 |
| `has_operand_address` | `bool` | Operand references an address | Yes: LD A,(nn), JP nn, etc. |
| `operand_address` | `u16` | The operand address value | Same |

### Fields to Widen

| Field | Geargrafx | Gearsystem (current) | Action |
|-------|-----------|---------------------|--------|
| `name` | `char[64]` | `char[32]` | Widen to `char[64]` (needed for color tags) |
| `bytes` | `char[25]` | `char[16]` | Widen to `char[25]` |
| `opcodes` | `u8[7]` | `u8[4]` | Widen to `u8[7]` (Z80 max = 4 bytes, but padding for uniformity) |
| `address` | `u32` | `u16` | Keep `u16` — Z80 64K address space fits. Can change to `u32` if ROM physical offset needed |
| `bank` | `u8` | `int` | Change to `u8` for consistency with geargrafx |

### Plan

Move `stDisassembleRecord` out of `Memory.h` into `definitions.h` (or a new `types.h`), matching geargrafx's approach. Rename it to keep the naming convention consistent. The struct must match geargrafx's `GG_Disassembler_Record` field layout so that all GUI code (which uses these fields) compiles identically.

```cpp
// In definitions.h (or a new gearsystem types.h)
struct GS_Disassembler_Record
{
    u32 address;
    u8 bank;
    char name[64];
    char bytes[25];
    char segment[8];
    u8 opcodes[7];
    int size;
    bool jump;
    u16 jump_address;
    u8 jump_bank;
    bool subroutine;
    int irq;
    bool has_operand_address;
    u16 operand_address;
};
```

**Reference**: `geargrafx/src/types.h` L142–L160

---

## 2. Disassembler Record Storage & Management

### Current State

**Geargrafx** (`src/memory.h` / `src/memory.cpp`):
- Single array `GG_Disassembler_Record*[0x200000]` indexed by physical address (bank << 13 | offset)
- Methods: `GetDisassemblerRecord(u16)`, `GetDisassemblerRecord(u16, u8)`, `GetOrCreateDisassemblerRecord(u16)`, `ResetDisassemblerRecords()`, `GetAllDisassemblerRecords()`

**Gearsystem** (`src/Memory.h` / `src/Memory.cpp`):
- Two arrays: `m_pDisassembledMap[0x10000]` (logical) + `m_pDisassembledROMMap[MAX_ROM_SIZE]` (ROM physical)
- Methods: `GetDisassembledMemoryMap()`, `GetDisassembledROMMemoryMap()`, `ResetDisassembledMemory()`, `ResetRomDisassembledMemory()`

### Plan

Rewrite `Memory` to use geargrafx-style record management API. Gearsystem can keep its dual-map approach internally (since Z80 banking works differently from HuC6280 MPR), but must expose the same public interface:

**New methods to add to `Memory`:**

```cpp
GS_Disassembler_Record* GetDisassemblerRecord(u16 address);
GS_Disassembler_Record* GetDisassemblerRecord(u16 address, u8 bank);
GS_Disassembler_Record* GetOrCreateDisassemblerRecord(u16 address);
void ResetDisassemblerRecords();
GS_Disassembler_Record** GetAllDisassemblerRecords();
```

The internal storage can remain dual-map, but `GetAllDisassemblerRecords()` should return the ROM map for GUI iteration (similar to geargrafx returning the full physical array).

**Key mapping difference**: Geargrafx uses MPR-based physical addressing (`bank << 13 | (addr & 0x1FFF)`). Gearsystem must translate using its memory rule's `GetBank(slot)`:
- For 8K banking: `physical = bank * 0x2000 + (addr & 0x1FFF)`
- For 16K banking: `physical = bank * 0x4000 + (addr & 0x3FFF)`
- For RAM (≥ 0xC000): use logical map

**Reference**: `geargrafx/src/memory.h` L67–L86, `geargrafx/src/memory.cpp` L258–L345, `geargrafx/src/memory_inline.h` L263–L360

---

## 3. Disassembly Routine

### Current State

**Geargrafx** (`src/huc6280_inline.h`):
- `DisassembleNextOPCode()` (L477–L519): calls `CheckBreakpoints()`, creates/updates record
- `PopulateDisassemblerRecord()` (L563–L725): fills all 17 fields with operand parsing, segment labeling, jump/subroutine detection
- `InvalidateOverlappingRecords()` (L521–L560): handles self-modifying code by clearing stale records
- `DisassembleAhead()` (L727–L797): proactive disassembly, follows jumps recursively (depth ≤ 3)

**Gearsystem** (`src/Processor.cpp`):
- `Disassemble(u16 address)` (L337–L563): monolithic function that creates records, parses Z80 prefixes (DD/FD/CB/ED/DDCB/FDCB), checks breakpoints. Only fills 8 fields.
- `DisassembleNextOpcode()` (L329–L335): simple wrapper

### Changes Required

#### 3.1 Split `Disassemble` into Multiple Methods

Match geargrafx's approach by splitting the monolithic `Disassemble()` into:

1. **`DisassembleNextOPCode()`** — calls `CheckBreakpoints()` + creates/updates record (verbatim from geargrafx structure)
2. **`PopulateDisassemblerRecord()`** — Z80-specific field population (replaces the inner body of current `Disassemble`)
3. **`InvalidateOverlappingRecords()`** — port verbatim from geargrafx (adapt byte count: Z80 max 4 bytes vs HuC6280 max 7)
4. **`DisassembleAhead(int count)`** and **`DisassembleAhead(u16 start, int count, int depth)`** — port from geargrafx, adapt stop opcodes for Z80 (RET=0xC9, JP nn=0xC3, JR=0x18, HALT=0x76, RETI=0xED4D, RETN=0xED45)

#### 3.2 Populate New Record Fields

`PopulateDisassemblerRecord` must fill all new fields for Z80:

| Field | Z80 Implementation |
|-------|-------------------|
| `segment` | Determine from address range: $0000–$BFFF→"ROM", $C000–$DFFF→"RAM", $E000–$FFFF→"RAM" (mirror). For SRAM pages→"SRAM", BIOS→"BIOS" |
| `bank` | From memory rule's `GetBank(slot)` |
| `subroutine` | True for: `CALL nn` (0xCD), `CALL cc,nn` (0xC4/CC/D4/DC/E4/EC/F4/FC), `RST xx` (0xC7/CF/D7/DF/E7/EF/F7/FF) |
| `irq` | Track IRQ type: 1=RESET, 2=NMI ($0066), 3=INT ($0038) — set from a new `m_debug_next_irq` tracking variable |
| `has_operand_address` | True for absolute addressing modes (type 3 in current opcode tables) and direct memory ops |
| `operand_address` | The 16-bit address from the operand |
| `jump` | Already implemented. Extend to cover all conditional jumps: JP cc (0xC2/CA/D2/DA/E2/EA/F2/FA), JR cc (0x20/28/30/38), DJNZ (0x10) |
| `jump_bank` | Compute from `jump_address` using same bank lookup |

#### 3.3 Add IRQ Tracking

Add `m_debug_next_irq` to `Processor`, set it in interrupt handling:
- NMI handler → `m_debug_next_irq = 2`
- INT mode 1 handler → `m_debug_next_irq = 3`
- On reset → `m_debug_next_irq = 1`

Then `PopulateDisassemblerRecord` reads and clears it (identical logic to geargrafx's `huc6280_inline.h` L508–L512).

**Reference**: `geargrafx/src/huc6280_inline.h` L477–L797

---

## 4. Breakpoint System

### Current State

**Geargrafx** (`src/huc6280.h` / `src/huc6280.cpp`):
- Unified `GG_Breakpoint` struct with `enabled`, `type`, `address1`, `address2`, `read`, `write`, `execute`, `range`
- `GG_Breakpoint_Type` enum (5 types: ROMRAM, VRAM, Palette, HuC6270, HuC6260)
- Methods all on `HuC6280` class: `AddBreakpoint()`, `RemoveBreakpoint()`, `IsBreakpoint()`, `EnableBreakpoints()`, `ResetBreakpoints()`, `CheckBreakpoints()`, `CheckMemoryBreakpoints()`, `AddRunToBreakpoint()`
- Per-breakpoint enable/disable toggle
- IRQ breakpoints (`m_breakpoints_irq_enabled`)
- Run-to breakpoint (struct-based, not pointer-based)

**Gearsystem** (`src/Memory.h` / `src/Processor.h`):
- Split system: `stDisassembleRecord*` vector for CPU breakpoints (pointer identity), `stMemoryBreakpoint` vector for memory R/W
- No per-breakpoint enable/disable
- No typed breakpoints
- No execute flag (CPU breakpoints always execute)
- Run-to breakpoint is a pointer to a disassemble record

### Changes Required

#### 4.1 Create Unified Breakpoint System

Add to `Processor` class (matching geargrafx's approach of putting breakpoints on the CPU class):

```cpp
// In Processor.h

enum GS_Breakpoint_Type {
    GS_BREAKPOINT_TYPE_ROMRAM = 0,
    GS_BREAKPOINT_TYPE_VRAM,
    GS_BREAKPOINT_TYPE_VDP_REGISTER,
    GS_BREAKPOINT_TYPE_COUNT
};

struct GS_Breakpoint {
    bool enabled;
    int type;
    u16 address1;
    u16 address2;
    bool read;
    bool write;
    bool execute;
    bool range;
};

struct GS_CallStackEntry {
    u16 src;
    u16 dest;
    u16 back;
};
```

**Breakpoint types for SMS/GG** replacement for PC Engine types:
- `GS_BREAKPOINT_TYPE_ROMRAM` → ROM/RAM address space (equivalent to `HuC6280_BREAKPOINT_TYPE_ROMRAM`)
- `GS_BREAKPOINT_TYPE_VRAM` → VDP VRAM (equivalent to `HuC6280_BREAKPOINT_TYPE_VRAM`)
- `GS_BREAKPOINT_TYPE_VDP_REGISTER` → VDP registers (combines HuC6270+HuC6260 equivalents)

#### 4.2 Add Breakpoint Methods to `Processor`

Port all methods from `HuC6280` to `Processor`, keeping identical signatures where possible:

| Method | Source | Notes |
|--------|--------|-------|
| `void EnableBreakpoints(bool enable, bool irqs)` | `huc6280.cpp` L123–L127 | Verbatim |
| `void ResetBreakpoints()` | `huc6280.cpp` L129–L132 | Verbatim |
| `bool AddBreakpoint(int type, char* text, bool read, bool write, bool execute)` | `huc6280.cpp` L134–L205 | Verbatim (hex parsing is CPU-agnostic) |
| `bool AddBreakpoint(u16 address)` | `huc6280.cpp` L209–L214 | Verbatim |
| `void AddRunToBreakpoint(u16 address)` | `huc6280.cpp` L216–L227 | Verbatim |
| `void RemoveBreakpoint(int type, u16 address)` | `huc6280.cpp` L229–L240 | Verbatim |
| `bool IsBreakpoint(int type, u16 address)` | `huc6280.cpp` L243–L254 | Verbatim |
| `std::vector<GS_Breakpoint>* GetBreakpoints()` | `huc6280_inline.h` L389–L391 | Verbatim |
| `bool BreakpointHit()` | `huc6280_inline.h` L374–L377 | Return `m_cpu_breakpoint_hit \|\| m_memory_breakpoint_hit` |
| `bool MemoryBreakpointHit()` | `huc6280_inline.h` L379–L381 | Verbatim |
| `bool RunToBreakpointHit()` | `huc6280_inline.h` L384–L386 | Verbatim |
| `void CheckBreakpoints()` | `huc6280_inline.h` L423–L472 | Verbatim logic (check PC against execute breakpoints) |
| `void CheckMemoryBreakpoints(int type, u16 address, bool read)` | `huc6280.cpp` L264–L299 | Verbatim logic |

#### 4.3 Remove Old Breakpoint Code from `Memory`

Remove `m_BreakpointsCPU`, `m_BreakpointsMem`, `m_pRunToBreakpoint` and associated methods from `Memory`. Memory's `CheckBreakpoints()` should now delegate to `Processor::CheckMemoryBreakpoints()`.

#### 4.4 Hook Memory Breakpoints into VDP

For VRAM and VDP register breakpoints, add `CheckMemoryBreakpoints()` calls in the VDP class (equivalent to geargrafx calling it from HuC6270/HuC6260):
- VDP VRAM reads/writes → `CheckMemoryBreakpoints(GS_BREAKPOINT_TYPE_VRAM, address, is_read)`
- VDP register writes → `CheckMemoryBreakpoints(GS_BREAKPOINT_TYPE_VDP_REGISTER, register, false)`

**Reference**: `geargrafx/src/huc6280.h` L69–L130, `geargrafx/src/huc6280.cpp` L123–L299, `geargrafx/src/huc6280_inline.h` L374–L472

---

## 5. Call Stack Tracking

### Current State

**Geargrafx**: Full implementation with `GG_CallStackEntry` struct, `std::stack` capped at 256 entries, push on JSR/BSR/IRQ, pop on RTS/RTI.

**Gearsystem**: **None**. No call stack tracking exists.

### Plan

Add to `Processor` class, matching geargrafx's API exactly:

```cpp
// In Processor.h
private:
    std::stack<GS_CallStackEntry> m_disassembler_call_stack;

public:
    void ClearDisassemblerCallStack();
    std::stack<GS_CallStackEntry>* GetDisassemblerCallStack();

private:
    void PushCallStack(u16 src, u16 dest, u16 back);
    void PopCallStack();
```

**Push locations** (in opcode handlers, guarded by `#ifndef GS_DISABLE_DISASSEMBLER`):
- `CALL nn` (0xCD) → `PushCallStack(pc_before, dest, pc_after)`
- `CALL cc,nn` (0xC4/CC/D4/DC/E4/EC/F4/FC) — only when condition is met
- `RST xx` (0xC7/CF/D7/DF/E7/EF/F7/FF) → `PushCallStack(pc_before, rst_vector, pc_after)`
- NMI handler → `PushCallStack(pc, 0x0066, pc)`
- INT mode 1 handler → `PushCallStack(pc, 0x0038, pc)`

**Pop locations:**
- `RET` (0xC9)
- `RET cc` (0xC0/C8/D0/D8/E0/E8/F0/F8) — only when condition is met
- `RETI` (0xED 0x4D)
- `RETN` (0xED 0x45)

**Implementation**: Port `PushCallStack`/`PopCallStack` verbatim from `geargrafx/src/huc6280_inline.h` L399–L423.

**Reference**: `geargrafx/src/huc6280.h` L91–L96, L128–L129, L171, `geargrafx/src/huc6280_inline.h` L399–L423, `geargrafx/src/huc6280_opcodes_inline.h` L178–L194, `geargrafx/src/huc6280_opcodes.cpp` L414–L415, L606–L607

---

## 6. Main Loop Debug Integration (RunToVBlank)

### Current State

**Geargrafx** (`src/geargrafx_core_inline.h` L71–L156):
- `GG_Debug_Run` struct with 4 booleans: `step_debugger`, `stop_on_breakpoint`, `stop_on_run_to_breakpoint`, `stop_on_irq`
- Template-dispatched (`<debugger, is_cdrom, is_sgx>`) for zero-overhead release builds
- `instruction_completed` tracking for multi-cycle block transfers
- Debug callback hook (`m_debug_callback`)
- Separate breakpoint hit types checked individually

**Gearsystem** (`src/GearsystemCore.cpp` L154–L195):
- Two loose `bool` parameters: `step`, `stopOnBreakpoints`
- No template dispatch, `#ifndef` inline
- No `instruction_completed`, no debug callback
- Simple combined `BreakpointHit()` check

### Changes Required

#### 6.1 Add `GS_Debug_Run` Struct

```cpp
// In GearsystemCore.h
struct GS_Debug_Run
{
    bool step_debugger;
    bool stop_on_breakpoint;
    bool stop_on_run_to_breakpoint;
    bool stop_on_irq;
};
```

#### 6.2 Add Debug Callback

```cpp
// In GearsystemCore.h
typedef void (*GS_Debug_Callback)(void);

// In GearsystemCore class
public:
    void SetDebugCallback(GS_Debug_Callback callback);
private:
    GS_Debug_Callback m_debug_callback;
```

#### 6.3 Refactor `RunToVBlank`

Change the signature to match geargrafx:

```cpp
bool RunToVBlank(u8* pFrameBuffer, s16* pSampleBuffer,
    int* pSampleCount, GS_Debug_Run* debug = NULL);
```

Port the debug stop conditions from geargrafx's `RunToVBlankTemplate`:
1. Call `m_debug_callback()` if set
2. Pass `instruction_completed` pointer to `RunFor` (not strictly needed for Z80 since there are no multi-cycle block transfers like TIA/TDD, but for API parity, consider Z80 block instructions like LDIR/LDDR/CPIR/CPDR which do repeat)
3. Check memory breakpoints, CPU breakpoints, run-to breakpoints separately
4. Return `BreakpointHit() || RunToBreakpointHit()`

**Note**: Z80 block instructions (LDIR, LDDR, CPIR, CPDR, OTIR, OTDR, INIR, INDR) repeat by decrementing BC and re-executing. Unlike HuC6280's multi-cycle block transfers, these are self-repeating opcodes. The `instruction_completed` concept can be simulated by checking if the last instruction was a repeating block instruction that hasn't finished.

**Reference**: `geargrafx/src/geargrafx_core.h` L45–L55, `geargrafx/src/geargrafx_core_inline.h` L71–L156

---

## 7. Emu Debug Layer (Stepping)

### Current State

**Geargrafx** (`platforms/shared/desktop/emu.cpp`):
- All stepping functions implemented: `step_into`, `step_over`, `step_out`, `step_frame`, `break`, `continue`
- `emu_update` constructs `GG_Debug_Run` and drives `RunToVBlank`

**Gearsystem** (`platforms/shared/desktop/emu.cpp`):
- `step_into`, `step_frame`, `break`, `continue` — **working**
- `step_over`, `step_out` — **stubbed** (commented out, references `GetHuC6280()`)
- `emu_update` references nonexistent `GS_Debug_Run` and `GetHuC6280()` — **won't compile**

### Changes Required

#### 7.1 Fix `emu_update()`

Replace references to `GS_Debug_Run` (which doesn't exist yet) with the new struct once created. Replace `GetHuC6280()` with `GetProcessor()`:
- `GetHuC6280()->EnableBreakpoints(...)` → `GetProcessor()->EnableBreakpoints(...)`
- `GetHuC6280()->DisassembleAhead(...)` → `GetProcessor()->DisassembleAhead(...)`

#### 7.2 Implement `emu_debug_step_over()`

Port from geargrafx (`emu.cpp` L432–L449), adapting for Z80:

```
1. Get the disassembler record at current PC
2. If record says the instruction is a subroutine (record->subroutine):
   - Set run-to-breakpoint at PC + record->size
   - Continue execution
3. Else: single-step
```

Z80 subroutine instructions: `CALL nn`, `CALL cc,nn`, `RST xx`. The `record->subroutine` field (once populated) handles this.

#### 7.3 Implement `emu_debug_step_out()`

Port from geargrafx (`emu.cpp` L458–L473):

```
1. Get the call stack: GetProcessor()->GetDisassemblerCallStack()
2. If non-empty, read top().back (return address)
3. Set run-to-breakpoint at return address
4. Continue execution
5. If stack empty, fall back to single step
```

This is CPU-agnostic logic — verbatim port once call stack is implemented.

#### 7.4 Implement `emu_debug_set_callback()`

```cpp
void emu_debug_set_callback(GS_Debug_Callback callback)
{
    gearsystem->SetDebugCallback(callback);
}
```

**Reference**: `geargrafx/platforms/shared/desktop/emu.cpp` L180–L226, L432–L498

---

## 8. Opcode Names & Color-Coded Format Strings

### Current State

**Geargrafx** (`src/huc6280_names.h`):
- Uses `GG_OPCode_Type` enum (9 types)
- Format strings include inline color tags: `{n}` (mnemonic), `{o}` (operand), `{e}` (extra/annotation)
- Example: `"{n}LDA {o}$%02X{e},X"` renders with mnemonic in cyan, operand in orange, indexing in gray

**Gearsystem** (`src/opcode_names.h`, `src/opcodexx_names.h`, etc.):
- Uses `stOPCodeInfo` with plain `name` (no color tags), `size`, `type` (0–6)
- 7 tables: `kOPCodeNames[256]`, `kOPCodeCBNames[256]`, `kOPCodeEDNames[256]`, `kOPCodeDDNames[256]`, `kOPCodeFDNames[256]`, `kOPCodeDDCBNames[256]`, `kOPCodeFDCBNames[256]`

### Changes Required

#### 8.1 Add Color Tags to Z80 Opcode Names

Update all 7 opcode tables to include `{n}`, `{o}`, `{e}` color tags. This is a large but mechanical change:

**Pattern for each entry:**
```cpp
// Before:
{ "LD A,$%02X", 2, 2 }
// After:
{ "{n}LD {o}A,{o}$%02X", 2, 2 }

// Before:
{ "JP $%04X", 3, 3 }
// After:
{ "{n}JP {o}$%04X", 3, 3 }

// Before:
{ "DJNZ $%04X  [%+d]", 2, 5 }
// After:
{ "{n}DJNZ {o}$%04X  {e}[%+d]", 2, 5 }
```

Color convention (matching geargrafx):
- `{n}` = instruction mnemonic (cyan)
- `{o}` = operands (orange/yellow)
- `{e}` = computed info like relative offsets, register annotations (gray)

This ensures the `TextColoredEx()` renderer (already copied to gearsystem in `gui_debug_text.h`) works identically.

#### 8.2 Add Operand Address Detection per Type

The `type` field in `stOPCodeInfo` already encodes the addressing mode. Extend the disassembler to set `has_operand_address` based on type:
- Type 3 (absolute address) → `has_operand_address = true`, `operand_address = nn`
- Type 5 (relative jump) → `has_operand_address = true` (target address)
- Other types with memory reference (type 1 for immediate is NOT an address, type 4 for displacement is relative)

**Reference**: `geargrafx/src/huc6280_names.h` L25–L316

---

## 9. Symbols, Labels & Bookmarks

### Current State

Both projects share the same GUI code structure in `gui_debug_disassembler.cpp`. Geargrafx is fully implemented; gearsystem has the same function signatures but bodies are commented out.

### Shared (Already Matching)

These elements are **CPU-agnostic** and can be ported verbatim:

| Feature | File | Status in Gearsystem |
|---------|------|---------------------|
| `DebugSymbol` struct | `gui_debug_disassembler.h` L33 | Already present |
| `DisassemblerLine` struct | `gui_debug_disassembler.cpp` L34 | Already present |
| `DisassemblerBookmark` struct | `gui_debug_disassembler.cpp` L44 | Already present |
| `SymbolEntry` struct | `gui_debug_disassembler.cpp` L49 | Already present |
| `fixed_symbols[256][65536]` storage | `gui_debug_disassembler.cpp` | Already present |
| `dynamic_symbols[256][65536]` storage | `gui_debug_disassembler.cpp` | Already present |
| Symbol file parsing (`add_symbol()`) | `gui_debug_disassembler.cpp` | **Working** — supports WLA/PCEAS/VASM formats |
| `gui_debug_load_symbols_file()` | `gui_debug_disassembler.cpp` | **Working** |
| `gui_debug_add_symbol()` popup | `gui_debug_disassembler.cpp` | **Working** |
| `gui_debug_remove_symbol()` | `gui_debug_disassembler.cpp` | **Working** |
| `gui_debug_get_symbols()` | `gui_debug_disassembler.cpp` | **Working** |
| `gui_debug_window_symbols()` | `gui_debug_disassembler.cpp` | **Working** |
| Bookmark management | `gui_debug_disassembler.cpp` | **Working** (add/remove/get/reset) |

### Stubbed (Require Uncommenting + Adaptation)

| Feature | Status | Depends On |
|---------|--------|------------|
| `gui_debug_reset_breakpoints()` | Stubbed — references `GetHuC6280()` | §4 (breakpoint system) |
| `gui_debug_toggle_breakpoint()` | Stubbed — references `GetHuC6280()` | §4 |
| `gui_debug_runto_address()` | Stubbed — references `GetHuC6280()` | §4, §7 |
| `gui_debug_resolve_symbol()` | Stubbed | §1 (record's `has_operand_address`, `operand_address`, `jump_address`) |
| `gui_debug_resolve_label()` | Stubbed | §10 (hardware labels) |
| `add_auto_symbol()` | Stubbed | §1, §3 (record's `irq`, `subroutine`, `jump`, `jump_bank`) |

### Auto-Symbol Generation for Z80

Port from geargrafx (`gui_debug_disassembler.cpp` L1037–L1090), adapting IRQ names for SMS/GG:

| IRQ Index | Geargrafx Symbol | Gearsystem Equivalent |
|-----------|------------------|-----------------------|
| 1 | `RESET_XX_XXXX` | `RESET_XX_XXXX` (same) |
| 2 | `NMI_XX_XXXX` | `NMI_XX_XXXX` (same) |
| 3 | `TIMER_IRQ_XX_XXXX` | N/A (no timer IRQ on Z80) |
| 4 | `IRQ1_XX_XXXX` | `INT_XX_XXXX` (Z80 maskable interrupt) |
| 5 | `IRQ2_BRK_XX_XXXX` | N/A |

Jump/subroutine auto-symbols are identical:
- `SUB_XX_XXXX` for subroutine targets
- `TAG_XX_XXXX` for jump targets

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_disassembler.cpp` L1037–L1090, L1237–L1288

---

## 10. Hardware Debug Labels

### Current State

**Geargrafx** (`platforms/shared/desktop/gui_debug_constants.h` L87–L214):
- 43 hardware I/O labels (VDC, VCE, PSG, Timer, Joypad, IRQ, CD-ROM)
- 76 CD-ROM BIOS symbols

**Gearsystem** (`platforms/shared/desktop/gui_debug_constants.h` L87–L214):
- **Contains geargrafx labels** — copy-pasted PC Engine registers, NOT SMS/GG registers!

### Plan

Replace `k_debug_labels` with SMS/GG hardware register labels. The Sega Master System uses port-mapped I/O (not memory-mapped like PC Engine), so labels apply to I/O port addresses:

```cpp
static const stDebugLabel k_debug_labels[] = {
    // VDP Ports
    { 0xBE, "VDP_DATA" },
    { 0xBF, "VDP_CTRL" },
    { 0x7E, "VDP_VCOUNTER" },
    { 0x7F, "VDP_HCOUNTER" },
    // PSG
    { 0x7E, "PSG_OUTPUT" },    // write to $7E/$7F
    { 0x7F, "PSG_OUTPUT" },
    // I/O Ports
    { 0x3F, "IO_CTRL" },
    { 0xDC, "IO_PORT_A" },
    { 0xDD, "IO_PORT_B" },
    { 0xDE, "IO_PORT_A" },     // mirror
    { 0xDF, "IO_PORT_B" },     // mirror
    // Memory Control
    { 0x3E, "MEM_CTRL" },
    // Game Gear specific
    { 0x00, "GG_START" },
    { 0x01, "GG_SERIAL_DATA" },
    { 0x02, "GG_SERIAL_DIR" },
    { 0x03, "GG_SERIAL_CTRL" },
    { 0x04, "GG_SERIAL_RECV" },
    { 0x05, "GG_SERIAL_XMIT" },
    { 0x06, "GG_STEREO" },
};
```

**Note**: SMS/GG uses port-mapped I/O (IN/OUT instructions), not memory-mapped I/O. The label system in geargrafx checks memory addresses. For gearsystem, label resolution must be adapted to check I/O port addresses used in `IN`/`OUT` instructions instead. This affects `gui_debug_resolve_label()` — it must detect `IN A,(n)`/`OUT (n),A` instructions and resolve the port number against `k_debug_labels`.

Remove `k_cdrom_bios_symbols` (no equivalent in SMS/GG). Can add ROM BIOS entry points if desired:
```cpp
static const stDebugLabel k_bios_symbols[] = {
    { 0x0000, "BIOS_ENTRY" },
    { 0x0038, "INT_HANDLER" },
    { 0x0066, "NMI_HANDLER" },
};
```

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_constants.h` L81–L214

---

## 11. CPU Register Window

### Current State

**Geargrafx** (`platforms/shared/desktop/gui_debug_huc6280.cpp`):
- Full window with editable registers (A, X, Y, S, P, PC, MPR0–7)
- Flag bits individually toggleable
- Timer and interrupt state display
- Uses `EditableRegister8/16/1` widgets from `gui_debug_widgets.h`
- Callbacks via `RegisterWriteCallback8/16/1`

**Gearsystem**: **No CPU register window exists**. `gui_debug_window_huc6280()` call is commented out in `gui_debug.cpp`.

### Plan

Create `gui_debug_processor.h` / `gui_debug_processor.cpp` (or name `gui_debug_z80.h/cpp` for clarity):

**Registers to display (from `ProcessorState`):**

| Register | Type | Widget |
|----------|------|--------|
| A, F | `u8` | `EditableRegister8` |
| B, C, D, E, H, L | `u8` | `EditableRegister8` (via split from 16-bit pairs) |
| IX, IY | `u16` | `EditableRegister16` |
| SP, PC | `u16` | `EditableRegister16` |
| I, R | `u8` | `EditableRegister8` |
| A', F', B', C', D', E', H', L' | `u8` | `EditableRegister8` (shadow set) |

**Flags to display (individual bit toggles):**

| Bit | Flag | Widget |
|-----|------|--------|
| 7 | S (Sign) | `EditableRegister1` |
| 6 | Z (Zero) | `EditableRegister1` |
| 5 | Y (undocumented) | `EditableRegister1` |
| 4 | H (Half carry) | `EditableRegister1` |
| 3 | X (undocumented) | `EditableRegister1` |
| 2 | P/V (Parity/Overflow) | `EditableRegister1` |
| 1 | N (Subtract) | `EditableRegister1` |
| 0 | C (Carry) | `EditableRegister1` |

**Additional state:**
- IFF1, IFF2 (interrupt flip-flops) → `EditableRegister1`
- Interrupt mode (0/1/2) → display only
- Halt state → display only
- WZ internal register → display only (advanced)

**Structure**: Follow geargrafx's pattern with callback ids:
```cpp
enum Z80RegId {
    Z80RegId_A = 0, Z80RegId_F, Z80RegId_B, Z80RegId_C,
    Z80RegId_D, Z80RegId_E, Z80RegId_H, Z80RegId_L,
    Z80RegId_IX, Z80RegId_IY, Z80RegId_SP, Z80RegId_PC,
    Z80RegId_I, Z80RegId_R,
    Z80RegId_A2, Z80RegId_F2, Z80RegId_B2, Z80RegId_C2,
    Z80RegId_D2, Z80RegId_E2, Z80RegId_H2, Z80RegId_L2,
    Z80RegId_IFF1, Z80RegId_IFF2, Z80RegId_IM
};
```

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_huc6280.h`, `geargrafx/platforms/shared/desktop/gui_debug_huc6280.cpp`, `geargrafx/platforms/shared/desktop/gui_debug_widgets.h`

---

## 12. Disassembler GUI Window

### Current State

**Geargrafx**: Fully functional.
**Gearsystem**: `draw_controls()` and `draw_context_menu()` are active. `prepare_drawable_lines()` and `draw_disassembly()` are **entirely commented out**.

### Functions to Uncomment and Adapt

#### 12.1 `prepare_drawable_lines()` (geargrafx L592–L680)

This function iterates all addresses, gets disassembler records, generates auto-symbols, inserts symbol/instruction lines. The logic is CPU-agnostic except for:

- `GetDisassemblerRecord(i)` → needs §2 (record management API)
- `add_auto_symbol()` → needs §9 (auto-symbol generation)
- `record->bank` / `record->segment` → needs §1 (new record fields)

**Action**: Uncomment, replace `emu_state->huc6280` with `emu_state->processor`, replace `GetHuC6280()` with `GetProcessor()`.

#### 12.2 `draw_disassembly()` (geargrafx L683–L861)

Renders the disassembly list. CPU-agnostic except for:
- `record->segment` display → needs §1
- `record->subroutine` for RET separator → adapt: Z80 RET = opcode 0xC9 (and conditional RETs)
- Breakpoint overlay → needs §4

**Action**: Uncomment, fix type references.

#### 12.3 `add_auto_symbol()` (geargrafx L1037–L1090)

See §9 for Z80 IRQ name adaptation.

#### 12.4 `add_breakpoint()` (geargrafx L1092–L1108)

**Action**: Uncomment, replace `GetHuC6280()->AddBreakpoint(...)` with `GetProcessor()->AddBreakpoint(...)`.

#### 12.5 Symbol/Label Resolution

- `replace_symbols()` / `gui_debug_resolve_symbol()` (geargrafx L1237–L1274): Uses `record->has_operand_address`, `record->operand_address`, `record->jump_address`. Once these fields exist in gearsystem records, this is verbatim.
- `replace_labels()` / `gui_debug_resolve_label()` (geargrafx L1276–L1288): Matches `record->operand_address` against `k_debug_labels`. Needs SMS/GG-specific labels from §10. Also needs I/O port awareness for `IN`/`OUT` instructions.

#### 12.6 Save Disassembly

- `save_full_disassembler()` / `save_current_disassembler()`: CPU-agnostic once record struct matches.

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_disassembler.cpp` L592–L2255

---

## 13. Breakpoints GUI Window

### Current State

**Geargrafx**: `draw_breakpoints_content()` (L430–L565) — full UI with type combo, address input, R/W/X checkboxes, per-breakpoint enable/disable.

**Gearsystem**: Entire body commented out. References `HuC6280::GG_Breakpoint` and `GetHuC6280()`.

### Plan

Uncomment `draw_breakpoints_content()` and replace:
- `HuC6280::GG_Breakpoint` → `Processor::GS_Breakpoint` (or whatever the struct is named)
- `HuC6280::HuC6280_BREAKPOINT_TYPE_ROMRAM` → `Processor::GS_BREAKPOINT_TYPE_ROMRAM`
- `GetHuC6280()` → `GetProcessor()`
- Breakpoint type names in the combo box: change from "ROM/RAM, VRAM, Palette, HuC6270, HuC6260" to "ROM/RAM, VRAM, VDP Register"

The breakpoint type combo box labels need a gearsystem-specific version:
```cpp
static const char* breakpoint_types[] = {
    "ROM/RAM", "VRAM", "VDP Register"
};
```

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_disassembler.cpp` L430–L565

---

## 14. Call Stack GUI Window

### Current State

**Geargrafx**: `gui_debug_window_call_stack()` renders a table with Source, Destination, Return columns.

**Gearsystem**: Stubbed (references `GetHuC6280()->GetDisassemblerCallStack()`).

### Plan

Uncomment and replace:
- `GetHuC6280()->GetDisassemblerCallStack()` → `GetProcessor()->GetDisassemblerCallStack()`
- `GG_CallStackEntry` → `GS_CallStackEntry`

The rendering code is CPU-agnostic — it just displays addresses.

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_disassembler.cpp` (search for `gui_debug_window_call_stack`)

---

## 15. Trace Logger

### Current State

**Geargrafx** (`platforms/shared/desktop/gui_debug_trace_logger.cpp`):
- `gui_debug_window_trace_logger()`: ImGui window with controls
- `gui_debug_trace_logger_update()`: per-instruction callback, logs bank, address, registers, flags, instruction
- `gui_debug_save_log()`: writes to file

**Gearsystem**: All stubbed (references `HuC6280_State`, `GetHuC6280()`).

### Plan

#### 15.1 Uncomment `gui_debug_window_trace_logger()`

The window layout is CPU-agnostic. Replace `GG_Debug_Callback` with `GS_Debug_Callback`.

#### 15.2 Adapt `gui_debug_trace_logger_update()`

Replace HuC6280 register reads with Z80 register reads:

```cpp
// Geargrafx:
// HuC6280::HuC6280_State* state = core->GetHuC6280()->GetState();
// u8 a = state->A->GetValue(), x = state->X->GetValue(), ...

// Gearsystem:
Processor::ProcessorState* state = core->GetProcessor()->GetState();
u16 af = state->AF->GetValue();
u16 bc = state->BC->GetValue();
u16 de = state->DE->GetValue();
u16 hl = state->HL->GetValue();
u16 sp = state->SP->GetValue();
u16 pc = state->PC->GetValue();
u8 a = (af >> 8) & 0xFF;
u8 f = af & 0xFF;
```

**Format string**: Adapt for Z80 registers:
```
[counter] [bank:]ADDR  [A:XX F:XX BC:XXXX DE:XXXX HL:XXXX SP:XXXX] [F:SZ-H-PNC] instruction [bytes]
```

The flag names change:
- HuC6280: `NVtBDiZC`
- Z80: `SZ_H_PNC` (Sign, Zero, _, Half, _, Parity/oVerflow, Negative, Carry)

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug_trace_logger.cpp` L41–L181

---

## 16. Debug Settings Persistence

### Current State

**Geargrafx** (`platforms/shared/desktop/gui_debug.cpp`):
- Binary format with `GGDEBUG1` magic
- Serializes: breakpoints (all fields), IRQ breakpoint flag, bookmarks (address + name), memory editor state
- Auto-save/load tied to ROM filename

**Gearsystem**: All stubbed. Magic `GSDEBUG1` is defined but unused.

### Plan

Uncomment `gui_debug_save_settings()` and `gui_debug_load_settings()`, replacing:
- `"GGDEBUG1"` → `"GSDEBUG1"` (already defined)
- `GetHuC6280()->GetBreakpoints()` → `GetProcessor()->GetBreakpoints()`
- `HuC6280::GG_Breakpoint` → `GS_Breakpoint`
- `GetHuC6280()->AddBreakpoint(...)` → `GetProcessor()->AddBreakpoint(...)`

The serialization format is type-agnostic (writes bools, ints, u16s) — once the breakpoint struct matches, it's verbatim.

**Reference**: `geargrafx/platforms/shared/desktop/gui_debug.cpp` L138–L310

---

## 17. Compile-Time Disassembler Toggle

### Current State

**Geargrafx**: `GG_DISABLE_DISASSEMBLER` guards 37 locations in 13 files.
**Gearsystem**: `GS_DISABLE_DISASSEMBLER` guards 12 locations in 8 files.

### Plan

As new debug features are added, wrap them in `#if !defined(GS_DISABLE_DISASSEMBLER)` / `#endif`:
- All new call stack push/pop code in opcode handlers
- All new IRQ tracking code
- `DisassembleAhead()` calls
- `PopulateDisassemblerRecord()` new field population
- `InvalidateOverlappingRecords()` calls
- Debug callback invocation in `RunToVBlank`

Follow geargrafx's pattern: `#if !defined(GS_DISABLE_DISASSEMBLER)` for positive guards (include code when disassembler enabled), `#if defined(GS_DISABLE_DISASSEMBLER)` for negative guards (skip code when disabled).

---

## 18. Implementation Order

The dependencies between sections dictate a bottom-up implementation order:

### Phase 1: Core Data Structures
1. **§1 — Disassembler Record Structure**: Extend `stDisassembleRecord` with all new fields. This is the foundation everything else depends on.
2. **§4.1 — Breakpoint Structs**: Add `GS_Breakpoint`, `GS_Breakpoint_Type`, `GS_CallStackEntry` to `Processor.h`.

### Phase 2: CPU Infrastructure
3. **§4.2–4.3 — Breakpoint Methods**: Implement all breakpoint management methods on `Processor`. Remove old breakpoint code from `Memory`.
4. **§5 — Call Stack**: Add `PushCallStack`/`PopCallStack` to `Processor`, hook into `CALL`/`RET`/`RST`/IRQ opcodes.
5. **§3 — Disassembly Routine**: Refactor `Processor::Disassemble` into `DisassembleNextOPCode`, `PopulateDisassemblerRecord`, `InvalidateOverlappingRecords`, `DisassembleAhead`. Populate all new record fields.
6. **§2 — Record Storage API**: Unify `Memory`'s record management to expose geargrafx-compatible API.

### Phase 3: Core Loop
7. **§6 — RunToVBlank Refactor**: Add `GS_Debug_Run`, `GS_Debug_Callback`, refactor `RunToVBlank` to match geargrafx.
8. **§7 — Emu Stepping**: Fix `emu_update`, implement `step_over`, `step_out`, `set_callback`.

### Phase 4: Opcode Tables & Labels
9. **§8 — Color-Coded Opcode Names**: Add `{n}`, `{o}`, `{e}` tags to all 7 Z80 opcode tables.
10. **§10 — Hardware Labels**: Replace PC Engine hardware labels with SMS/GG I/O port labels.

### Phase 5: GUI Activation
11. **§12 — Disassembler GUI**: Uncomment `prepare_drawable_lines`, `draw_disassembly`, `add_auto_symbol`, `add_breakpoint`, symbol/label resolution.
12. **§13 — Breakpoints GUI**: Uncomment `draw_breakpoints_content`, adapt type names.
13. **§14 — Call Stack GUI**: Uncomment, fix type references.
14. **§11 — CPU Register Window**: Create new `gui_debug_z80.cpp` with Z80 register layout.
15. **§15 — Trace Logger**: Uncomment and adapt for Z80 registers.
16. **§16 — Debug Settings**: Uncomment save/load, adapt type references.

### Phase 6: Polish
17. **§9 — Auto Symbols**: Adapt IRQ names for Z80.
18. **§17 — Disassembler Guards**: Add `#if !defined(GS_DISABLE_DISASSEMBLER)` to all new code.
19. **§4.4 — VDP Breakpoints**: Hook `CheckMemoryBreakpoints` into VDP for VRAM/register breakpoints.
20. **Validation**: Full build test, verify libretro builds with `GS_DISABLE_DISASSEMBLER` defined.

---

## Appendix A: File-by-File Change Summary

### Source Files (src/)

| File | Changes |
|------|---------|
| `definitions.h` | Add `GS_Disassembler_Record` struct (or add to a new `types.h`) |
| `Processor.h` | Add: `GS_Breakpoint`, `GS_Breakpoint_Type`, `GS_CallStackEntry` structs; breakpoint methods; call stack methods; `m_debug_next_irq`; `DisassembleAhead()` |
| `Processor.cpp` | Refactor `Disassemble()` into split methods; add `AddBreakpoint()`, `RemoveBreakpoint()`, `IsBreakpoint()`, `EnableBreakpoints()`, `ResetBreakpoints()`, `CheckMemoryBreakpoints()` |
| `Processor_inline.h` | Add: `CheckBreakpoints()`, `PushCallStack()`, `PopCallStack()`, `PopulateDisassemblerRecord()`, `InvalidateOverlappingRecords()`, `DisassembleAhead()` |
| `Processor_opcodes*.cpp/h` | Add call stack push/pop in CALL/RET/RST/RETI/RETN handlers |
| `Memory.h` | Replace old breakpoint vectors with new API; add `GetDisassemblerRecord()` / `GetOrCreateDisassemblerRecord()` / `GetAllDisassemblerRecords()` |
| `Memory.cpp` | Implement new record management; remove old breakpoint code; delegate `CheckBreakpoints` to Processor |
| `Memory_inline.h` | Update `Read`/`Write` to call `Processor::CheckMemoryBreakpoints()` |
| `GearsystemCore.h` | Add `GS_Debug_Run`, `GS_Debug_Callback`, `SetDebugCallback()`, update `RunToVBlank` signature |
| `GearsystemCore.cpp` | Refactor `RunToVBlank` to match geargrafx template pattern |
| `opcode_names.h` | No structural change |
| `opcodexx_names.h` | Add `{n}`, `{o}`, `{e}` color tags to all entries |
| `opcodecb_names.h` | Add color tags |
| `opcodedd_names.h` | Add color tags |
| `opcodeed_names.h` | Add color tags |
| `opcodefd_names.h` | Add color tags |
| `opcodeddcb_names.h` | Add color tags |
| `opcodefdcb_names.h` | Add color tags |
| `Video.h/cpp` (VDP) | Add `CheckMemoryBreakpoints` calls for VRAM/register access |

### GUI Files (platforms/shared/desktop/)

| File | Changes |
|------|---------|
| `gui_debug_constants.h` | Replace PC Engine labels with SMS/GG I/O port labels |
| `gui_debug_disassembler.cpp` | Uncomment all stubbed functions; replace `HuC6280`→`Processor`, `GetHuC6280()`→`GetProcessor()` |
| `gui_debug_z80.h` (NEW) | Z80 CPU register window header |
| `gui_debug_z80.cpp` (NEW) | Z80 CPU register window implementation |
| `gui_debug_trace_logger.cpp` | Uncomment; adapt register reads for Z80 |
| `gui_debug.cpp` | Uncomment save/load settings; fix type references; register processor window call |
| `emu.h` | Add `GS_Debug_Callback` typedef reference |
| `emu.cpp` | Fix `emu_update`; implement `step_over`/`step_out`/`set_callback` |

---

## Appendix B: Geargrafx Reference File Index

All source locations for porting reference:

| Feature | Geargrafx File | Key Lines |
|---------|---------------|-----------|
| Disassembler record | `src/types.h` | L142–L160 |
| Breakpoint struct/enum | `src/huc6280.h` | L69–L96 |
| Breakpoint methods | `src/huc6280.cpp` | L123–L299 |
| CheckBreakpoints | `src/huc6280_inline.h` | L423–L472 |
| Call stack impl | `src/huc6280_inline.h` | L399–L423 |
| DisassembleNextOPCode | `src/huc6280_inline.h` | L477–L519 |
| PopulateDisassemblerRecord | `src/huc6280_inline.h` | L563–L725 |
| InvalidateOverlappingRecords | `src/huc6280_inline.h` | L521–L560 |
| DisassembleAhead | `src/huc6280_inline.h` | L727–L797 |
| Record management | `src/memory.h` | L67–L86 |
| Record create/reset | `src/memory.cpp` | L258–L345 |
| Opcode names + color tags | `src/huc6280_names.h` | L25–L316 |
| Opcode sizes | `src/huc6280_timing.h` | (full file) |
| GG_Debug_Run struct | `src/geargrafx_core.h` | L45–L55 |
| RunToVBlankTemplate | `src/geargrafx_core_inline.h` | L71–L156 |
| Debug callback | `src/geargrafx_core.h` | L39 |
| CPU register window | `platforms/shared/desktop/gui_debug_huc6280.cpp` | (full file) |
| Disassembler GUI | `platforms/shared/desktop/gui_debug_disassembler.cpp` | L592–L2255 |
| Hardware labels | `platforms/shared/desktop/gui_debug_constants.h` | L87–L214 |
| Stepping functions | `platforms/shared/desktop/emu.cpp` | L432–L498 |
| emu_update debug path | `platforms/shared/desktop/emu.cpp` | L180–L226 |
| Trace logger | `platforms/shared/desktop/gui_debug_trace_logger.cpp` | L41–L181 |
| Debug save/load | `platforms/shared/desktop/gui_debug.cpp` | L138–L310 |
| Editable register widgets | `platforms/shared/desktop/gui_debug_widgets.h` | (full file) |
| Color text renderer | `platforms/shared/desktop/gui_debug_text.h` | (full file) |
