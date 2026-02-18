# PC Engine — Memory Map (HuC6280 System)

## 1. Overview

The PC Engine memory system is based on the HuC6280 CPU and its internal
Memory Management Unit (MMU).

General characteristics:

- Logical address space: 64 KB
- Physical address space: 2 MB
- Address translation via 8 KB pages
- All memory and I/O accesses occur through the logical address space

Optional hardware extensions expand the effective physical memory usage.

---

## 2. Logical Address Space

The 64 KB logical address space is divided into eight fixed 8 KB pages.

| Page | Logical Address Range |
|----:|------------------------|
| 0 | $0000–$1FFF |
| 1 | $2000–$3FFF |
| 2 | $4000–$5FFF |
| 3 | $6000–$7FFF |
| 4 | $8000–$9FFF |
| 5 | $A000–$BFFF |
| 6 | $C000–$DFFF |
| 7 | $E000–$FFFF |

Each page is mapped independently using one MMU register.

---

## 3. Memory Management Unit (MMU)

### 3.1 Mapping Registers

The HuC6280 provides eight mapping registers (MPR0–MPR7).

- Width: 8 bits
- One register per logical page
- Each register selects one 8 KB physical block

| Logical Page | Mapping Register |
|-------------:|------------------|
| Page 0 | MPR0 |
| Page 1 | MPR1 |
| Page 2 | MPR2 |
| Page 3 | MPR3 |
| Page 4 | MPR4 |
| Page 5 | MPR5 |
| Page 6 | MPR6 |
| Page 7 | MPR7 |

---

### 3.2 Physical Address Construction

A physical address is formed by:

- Lower 13 bits: offset inside the page (A0–A12)
- Upper bits: MPR value (A13–A20)

This yields a 21-bit physical address.

---

## 4. Physical Address Space (8 KB Blocks)

The 2 MB physical space is divided into 256 blocks.

| Block Range | Designation |
|------------:|-------------|
| $00–$7F | Cartridge ROM |
| $80–$9F | CD-ROM system ROM / extension |
| $A0–$BF | CD-ROM work RAM |
| $C0–$DF | SuperGrafx / expansion |
| $E0–$EF | Expansion / unused |
| $F0–$F6 | Expansion / unused |
| $F7 | Backup RAM (BRAM) |
| $F8 | Work RAM |
| $F9–$FE | Expansion / unused |
| $FF | I/O page |

Exact usage depends on detected hardware.

---

## 5. Reset Mapping State

After RESET:

- MPR7 = $00
- MPR0–MPR6 undefined
- Logical page 7 maps to physical block $00

---

## 6. I/O Page (Physical Block $FF)

Physical block $FF contains all internal system devices.

The I/O page is sparse; unmapped regions are reserved.

---

## 7. I/O Address Map

### 7.1 Video Display Controller — HuC6270

| Address | Function |
|--------:|----------|
| $0000 | Status (R) / Register Select (W) |
| $0002 | Data Register (R/W) |
| $0001, $0003 | High byte access |

---

### 7.2 Video Color Encoder — HuC6260

| Address | Function |
|--------:|----------|
| $0400 | Control Register |
| $0402 | Color Table Address |
| $0404 | Color Table Data |
| $0401, $0403, $0405 | High byte access |

---

### 7.3 Programmable Sound Generator (PSG)

| Address Range | Function |
|---------------|----------|
| $0800–$080F | PSG registers |

---

### 7.4 Timer

| Address | Function |
|--------:|----------|
| $0C00 | Counter / Reload |
| $0C01 | Control |

---

### 7.5 Gamepad / I/O Port

| Address | Function |
|--------:|----------|
| $1000 | Gamepad read / control write |

---

### 7.6 Interrupt Controller

| Address | Function |
|--------:|----------|
| $1402 | Interrupt disable |
| $1403 | Interrupt status / acknowledge |

---

## 8. Optional Hardware Extensions

### 8.1 CD-ROM² / Super CD-ROM²

When CD-ROM hardware is present:

- Additional ROM and RAM blocks are exposed in physical space
- CD system ROM occupies part of $80–$9F
- CD work RAM occupies part of $A0–$BF
- Backup RAM ($F7) may be shared with CD system

Additional CD-specific I/O registers are accessed through the I/O page.

---

### 8.2 Arcade Card

When Arcade Card hardware is present:

- Large external RAM is exposed through bank-switching
- Physical block usage extends into expansion ranges
- Access is mediated through Arcade Card control registers

#### $FF:1A00–$FF:1AFF — Arcade Card interface registers

Arcade Card hardware exposes a register window in the **hardware bank ($FF)** at:

- **$FF:1A00–$FF:1AFF** — Arcade Card interface register block

This window provides access to the Arcade Card’s internal memory via multiple independent access “ports” (channels).

---

### 8.3 SuperGrafx

On SuperGrafx systems:

- A second VDC is present
- Additional video-related registers are exposed in the I/O page
- Extra VRAM is accessible through extended video hardware

The primary CPU and MMU remain unchanged.

#### $FF:0000–$FF:03FF — Video I/O block

- **VDC2 registers** (second HuC6270)
- **VPC registers** (Video Priority Controller)

#### Address map (Bank $FF offsets)

| Offset range | Device |
|------------:|--------|
| $0000–$0007 | VDC1 registers |
| $0008–$000F | VPC registers |
| $0010–$0017 | VDC2 registers |
| $0018–$001F | Unmapped |
| $0020–$03FF | Mirrors of $0000–$001F |

---

## 9. RAM Areas

### 9.1 Work RAM

- Physical block: $F8
- Size: 8 KB

---

### 9.2 Backup RAM (BRAM)

- Physical block: $F7
- Size: 8 KB
- Presence depends on cartridge or CD hardware

---

### 9.3 CD-ROM RAM

- Physical blocks: $A0–$BF
- Presence depends on CD-ROM hardware

---

## 10. Cartridge ROM

- Physical blocks: typically $00–$7F
- ROM size and layout are cartridge-defined

---

## 11. Summary

- 64 KB logical address space mapped via MMU
- 2 MB physical address space in 8 KB blocks
- Internal devices located in block $FF
- Optional hardware extends physical memory usage
- Mapping behavior is fully software-controlled