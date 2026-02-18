# HuC6260 — Video Color Encoder (VCE)

## 1. Overview

The HuC6260 is the Video Color Encoder used in the PC Engine / TurboGrafx-16.
It receives digital color data from the Video Display Controller (VDC) and
outputs analog video signals.

The HuC6260 integrates:

- Color table (palette) RAM
- Digital-to-analog color conversion
- Color clock generation
- Video synchronization signal generation

General characteristics:

- 512 color entries
- 512 × 9-bit color table RAM
- Color format: 3-bit Red, 3-bit Green, 3-bit Blue
- Single 5V power supply

---

## 2. VCE I/O Ports

The HuC6260 is accessed through three 16-bit I/O ports located in hardware bank $FF.

| Address | R/W | Register |
|--------|-----|----------|
| $0400  | W   | Control Register (CR) |
| $0402  | W   | Color Table Address Register (CTA) |
| $0404  | R/W | Color Table Data Register (CTW / CTR) |

Low and high bytes are accessed as follows:

- Even address: low byte
- Even address + 1: high byte

---

## 3. Control Register (CR) — $0400

Write-only register.

### Bit layout (16-bit write)

- Bits 15–2: unused
- Bits 1–0: Color clock select

### Color Clock Select (Bits 1–0)

| Bits | Color clock frequency |
|-----|-----------------------|
| 00  | ~5 MHz |
| 01  | ~7 MHz |
| 10  | ~10 MHz |
| 11  | Reserved |

---

## 4. Color Table RAM

### 4.1 Organization

The color table RAM consists of:

- 512 addresses
- 9 bits per address
- One address corresponds to one color entry

Color bit allocation:

- Bits 8–6: Green
- Bits 5–3: Red
- Bits 2–0: Blue

Address ranges:

- 0–255   : Background color table
- 256–511 : Sprite color table

Each range is divided into:

- 16 color blocks
- 16 pattern colors per block

---

## 5. Relation with VDC Color Signals

The HuC6260 receives 9 digital color signals from the VDC (VD0–VD8).

| Signal | Description |
|------|-------------|
| VD8 | Background / Sprite selection |
| VD7–VD4 | Color block number (0–15) |
| VD3–VD0 | Pattern color number (0–15) |

These signals form the color table address.

---

## 6. Color Table Address Register (CTA) — $0402

Holds the current color table RAM address.

### Bit layout

- Bits 15–9: unused
- Bits 8–0: Color table address (0–511)

### Address update rule

- The address is incremented after a high-byte access to the color table data register
- The same rule applies to read and write operations

---

## 7. Color Table Data Register — $0404 / $0405

Used to read or write color table RAM data.

### Bit layout (16-bit access)

- Bits 15–9: unused
- Bits 8–6: Green
- Bits 5–3: Red
- Bits 2–0: Blue

Only the lower 9 bits are stored in color table RAM.

---

## 8. CPU Access Sequences

### 8.1 Writing Color Table RAM

1. Write low byte of address to $0402
2. Write high byte of address to $0403
3. Write low byte of data to $0404
4. Write high byte of data to $0405

---

### 8.2 Reading Color Table RAM

1. Write low byte of address to $0402
2. Write high byte of address to $0403
3. Read low byte from $0404
4. Read high byte from $0405

---

## 9. Notes

- Only bits explicitly documented are defined
- Unused bits have no specified function
- Background and sprite color tables share the same RAM