# HuC6270 — Video Display Controller (VDC)

## 1. VDC I/O Ports

The HuC6270 is accessed through two 16-bit I/O ports in the hardware page (bank $FF).

| Address | R/W | Function |
|--------:|:---:|----------|
| $0000   | R   | Status Register (SR) |
| $0000   | W   | Address/Index Register (AR) |
| $0002   | R/W | Data Register (DR) |

Byte access conventions:

- $0000 (read): returns SR low byte; high byte reads return a fixed value (see SR section).
- $0000 (write): writes AR (register index)
- $0002/$0003: data low/high byte for the selected internal register

---

## 2. Internal Register Access Model

The HuC6270 contains internal registers R00–R13 (20 registers total). The CPU selects a target register by writing its index to AR, then reads/writes the selected register through the Data Register.

### 2.1 Address Register (AR) — write via $0000

- Write-only register selecting the internal register number.
- Valid register numbers: 0x00–0x13.
- Register indices 0x03 and 0x04 are reserved; documentation warns not to use AR=0x04.

AR bit layout (as documented):

- Bits 15–5: unused
- Bits 4–0: register index (0–19 decimal)

### 2.2 Data Register — read/write via $0002/$0003

- 16-bit access to the internal register selected by AR.
- Low byte at $0002, high byte at $0003.

---

## 3. Status Register (SR) — read via $0000

SR reports VDC status and interrupt conditions. If an interrupt source occurs (and is enabled in the appropriate control bits), the corresponding SR flag is set and the IRQ output is asserted. Reading SR clears the interrupt status bits except BSY.

SR bit meanings (low byte, bit 7..0):

| Bit | Symbol | Meaning |
|----:|:------:|---------|
| 6 | BSY | Busy flag |
| 5 | VD | Vertical blanking detect flag |
| 4 | DV | VRAM-to-VRAM block transfer end detect |
| 3 | DS | VRAM-to-SATB block transfer end detect |
| 2 | RR | Scanline detect flag |
| 1 | OR | Sprite overflow detect flag |
| 0 | CR | Sprite collision detect flag |

Additional notes from documentation:

- BSY corresponds to VRAM access activity and is not cleared by reading SR.
- High byte reads of SR return 0x00.

---

## 4. Internal Register Map (R00–R13)

The VDC provides 20 internal 16-bit registers. Two indices (0x03 and 0x04) are reserved. 

| Index | Name (common) | Name (manual) | R/W |
|------:|----------------|---------------|:---:|
| 0x00 | MAWR | Memory Address Write Register | W |
| 0x01 | MAAR / MARR | Memory Address Read Register | W |
| 0x02 | VWR / VRR | VRAM Data Write / Read Register | R/W (shared index) |
| 0x03 | Reserved | Reserved | — |
| 0x04 | Reserved | Reserved (do not use) | — |
| 0x05 | CR | Control Register | W |
| 0x06 | RCR | Scanline Detection (Raster Compare) Register | W |
| 0x07 | BXR | BG X Scroll Register | W |
| 0x08 | BYR | BG Y Scroll Register | W |
| 0x09 | MWR | Memory Access Width Register | W |
| 0x0A | HSR | Horizontal Sync Register | W |
| 0x0B | HDR | Horizontal Display Register | W |
| 0x0C | VPR | Vertical Sync Register | W |
| 0x0D | VDR | Vertical Display Register | W |
| 0x0E | VCR | Vertical Display End Position Register | W |
| 0x0F | DCR / OCR | Block Transfer Control Register | W |
| 0x10 | SOUR | Block Transfer Source Address Register | W |
| 0x11 | DESR | Block Transfer Destination Address Register | W |
| 0x12 | LENR / LENA | Block Transfer Length Register | W |
| 0x13 | DVSSR | VRAM–SATB Transfer Source Address Register | W |

---

## 5. VRAM Access Registers

### 5.1 MAWR (R00) — Memory Address Write Register

- 16-bit VRAM write pointer. 

### 5.2 MAAR / MARR (R01) — Memory Address Read Register

- 16-bit VRAM read pointer. 
- Writing the high byte initiates a VRAM read to load the VRAM read data register, then the read address register is incremented.

### 5.3 VWR/VRR (R02) — VRAM Data Write / Read Register

R02 is used for both VRAM write and VRAM read operations. 

- VWR (write): writing the high byte initiates a VRAM write at MAWR, then MAWR is incremented.
- VRR (read): reads return the currently latched VRAM word. Reading the high byte triggers the next VRAM word read and increments MARR.

---

## 6. Control Register (CR) — R05

CR configures VDC operation modes including interrupts, external sync, background/sprite enable, DISP output selection, refresh, and VRAM address auto-increment. 

CR bit fields (16-bit):

- Bits 0–3: IE (Interrupt enable flags)
- Bits 4–5: EX (External sync control)
- Bit 6: SB (Sprite blanking control)
- Bit 7: BB (Background blanking control)
- Bits 8–9: TE (DISP output select)
- Bit 10: DR (Dynamic RAM refresh enable)
- Bits 11–12: IW (MAWR/MARR auto-increment select)
- Bits 13–15: unused/reserved

### 6.1 IE — Interrupt Enable (bits 0–3)

| Bit | Symbol | Interrupt source |
|----:|:------:|------------------|
| 0 | CC | Collision Detect |
| 1 | OC | Over Detect |
| 2 | RC | Scanline Detect |
| 3 | VC | Vertical Blanking Detect |

### 6.2 EX — External Sync (bits 4–5)

| EX5 EX4 | VSYNC | HSYNC |
|:-------:|:-----:|:-----:|
| 00 | input | input |
| 01 | input | output |
| 10 | invalid | invalid |
| 11 | output | output |

### 6.3 SB / BB — Sprite & Background Blanking (bits 6–7)

- SB: sprite display control
- BB: background display control

### 6.4 TE — DISP Output Select (bits 8–9)

| TE9 TE8 | DISP output |
|:-------:|-------------|
| 00 | Display period indicator (H level during display) |
| 01 | BURST position indicator (low active) |
| 10 | Internal horizontal SYNC |
| 11 | Invalid |

### 6.5 DR — Dynamic RAM Refresh (bit 10)

- Enables refresh addresses on MA0–MA15 under documented conditions related to VRAM access width.

### 6.6 IW — MAWR/MARR Auto-Increment Select (bits 11–12)

The MAWR/MARR increment is applied on access to the high byte of the VRAM data register.

| IW12 IW11 | Increment |
|:---------:|----------:|
| 00 | +1 |
| 01 | +32 |
| 10 | +64 |
| 11 | +128 |

(Manual describes increments in hexadecimal notation as +20h, +40h, +80h.) 

---

## 7. Raster Compare Register — R06 (RCR)

- Specifies a scanline value for scanline-detect interrupt generation. 
- Bit layout (as documented):
  - Bits 15–10: unused
  - Bits 9–0: scanline value

The manual describes the internal scanline counter initialization and the relationship to the display start line.

---

## 8. Background Scroll Registers

### 8.1 BXR — R07 (BG X Scroll)

- Bit layout:
  - Bits 15–10: unused
  - Bits 9–0: horizontal scroll position within the virtual BG map 

### 8.2 BYR — R08 (BG Y Scroll)

- Bit layout:
  - Bits 15–9: unused
  - Bits 8–0: vertical scroll position within the virtual BG map 

---

## 9. Memory Access Width Register — R09 (MWR)

MWR configures VRAM access width modes for background and sprites, virtual screen dimensions, and CG selection for 4-clock mode. 

MWR bit fields (8-bit used in low byte):

- Bits 0–1: VM (VRAM access width mode)
- Bits 2–3: SM (Sprite access width mode)
- Bits 4–6: SCREEN (virtual screen size)
- Bit 7: CM (CG mode selection for 4-clock mode)

### 9.1 VM — VRAM Access Width Mode (bits 0–1)

Selects the number of clocks used for VRAM/BAT/CG accesses and for block transfer.

### 9.2 SM — Sprite Access Width Mode (bits 2–3)

Selects the number of clocks used for sprite generator access during horizontal blanking.

### 9.3 SCREEN — Virtual Screen Size (bits 4–6)

Defines virtual screen dimensions in characters (X and Y). 

| SCREEN6 SCREEN5 SCREEN4 | X chars | Y chars |
|:------------------------:|--------:|--------:|
| 000 | 32 | 32 |
| 001 | 64 | 32 |
| 010 | 128 | 32 |
| 011 | 128 | 32 |
| 100 | 32 | 64 |
| 101 | 64 | 64 |
| 110 | 128 | 64 |
| 111 | 128 | 64 |

### 9.4 CM — CG Mode (bit 7)

Selects which character generator blocks are used in 4-clock mode.

---

## 10. Display Timing Registers (R0A–R0E)

These registers define horizontal and vertical timing and display geometry. 

### 10.1 HSR — R0A (Horizontal Sync Register)

Bit fields:

- Bits 0–4: HSW (horizontal sync pulse width) stored as N−1
- Bits 8–14: HDS (horizontal display start position) stored as N−1



### 10.2 HDR — R0B (Horizontal Display Register)

Bit fields:

- Bits 0–6: HDW (horizontal display width) stored as N−1
- Bits 8–14: HDE / HOE (horizontal display end position) stored as N−1



### 10.3 VPR — R0C (Vertical Sync Register)

Bit fields:

- Bits 0–4: VSW (vertical sync pulse width) stored as N−1
- Bits 8–15: VDS (vertical display start position) stored as N−2



### 10.4 VDR — R0D (Vertical Display Register)

- Bits 8–0: VDW (vertical display width) stored as N−1 

### 10.5 VCR — R0E (Vertical Display End Position Register)

- Bits 7–0: VCR (vertical display end/front-porch position) stored as N 

---

## 11. Block Transfer / DMA Registers (R0F–R13)

The HuC6270 supports:
- VRAM-to-VRAM block transfer
- VRAM-to-SATB block transfer

SATB (Sprite Attribute Table Buffer) is internal to the VDC; it is updated via VRAM-to-SATB block transfer. 

### 11.1 DCR / OCR — R0F (Block Transfer Control Register)

Bit fields:

- Bit 0: DSC (VRAM–SATB transfer complete interrupt enable)
- Bit 1: DVC (VRAM–VRAM transfer complete interrupt enable)
- Bit 2: SI/D (VRAM–VRAM source address inc/dec; 0=inc, 1=dec)
- Bit 3: DI/D (VRAM–VRAM destination address inc/dec; 0=inc, 1=dec)
- Bit 4: DSR (VRAM–SATB transfer auto-repeat each vertical blank)



### 11.2 SOUR — R10 (VRAM–VRAM Source Address)

- 16-bit source start address for VRAM–VRAM block transfer 

### 11.3 DESR — R11 (VRAM–VRAM Destination Address)

- 16-bit destination start address for VRAM–VRAM block transfer 

### 11.4 LENR / LENA — R12 (VRAM–VRAM Block Length)

- 16-bit block length in words
- Stored as M−1
- Writing the high byte triggers the block transfer start



### 11.5 DVSSR — R13 (VRAM–SATB Source Address)

- 16-bit source start address for VRAM–SATB block transfer
- Writing this register schedules a VRAM–SATB transfer at the beginning of the following vertical blanking period (per manual note)



---

## 12. Background Data Structures in VRAM

### 12.1 Background Attribute Table (BAT)

The BAT resides in VRAM starting at address 0 and provides, for each character position in the virtual screen:
- Character code
- 4-bit area color (CG COLOR)

BAT entry bit layout (16-bit):

- Bits 15–12: CG COLOR (4-bit area color code)
- Bits 11–0: Character code



### 12.2 Character Generator (CG)

Character patterns are defined in VRAM.
One character is 8×8 dots, with color specified by a 4-bit code per dot (16-color per character).
The manual describes the CH0–CH3 organization and alignment constraints for CG areas.

---

## 13. Sprite Data Structures

### 13.1 SATB (Sprite Attribute Table Buffer)

The HuC6270 contains an internal SATB for 64 sprites.
SATB cannot be written directly by the CPU; it is loaded via VRAM–SATB block transfer from a sprite attribute table stored in VRAM.

A VRAM sprite attribute table uses 4 words per sprite (256 words total for 64 sprites).

### 13.2 Sprite Attributes

The manual documents sprite fields including:
- X coordinate
- Y coordinate
- Pattern code
- Sprite color (4-bit area color)
- Priority control (SPBG)
- Horizontal/vertical flip (X, Y)
- Combined sprite modes (CGX, CGY)

These fields are defined in the SAT word layout in the manual sections for SATB.

### 13.3 Sprite Generator (SG)

Sprite patterns are stored in VRAM and described as a set of areas (SG0–SG3) with alignment requirements and pattern-code mapping rules.

---

## 14. CPU Programming Sequences (Register/VRAM Access)

### 14.1 Writing an Internal Register

1. Write target register index to AR ($0000 write)
2. Write low byte to data port ($0002)
3. Write high byte to data port ($0003)



### 14.2 Reading SR (Status)

- Read SR low byte from $0000.
- High byte reads return fixed documented value.

### 14.3 Writing VRAM (via VDC)

1. Select MAWR in AR and write VRAM address (low then high)
2. Select VWR in AR and write VRAM data (low then high)
3. Repeat writes to VWR as needed



### 14.4 Reading VRAM (via VDC)

1. Select MARR in AR and write VRAM address (low then high)
2. Select VRR in AR
3. Read VRAM data (low then high)
4. Repeat reads as needed

