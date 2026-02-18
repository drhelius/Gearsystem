# HuC6202 — Video Priority Controller (VPC)

## 1. Overview

The HuC6202 is the Video Priority Controller used in the SuperGrafx system.
It receives pixel output streams from two HuC6270 Video Display Controllers
(VDC #1 and VDC #2), combines them according to programmable priority rules,
and forwards the resulting pixel stream to the HuC6260 Video Color Encoder (VCE).

Primary functions:

- Selection and decoding of video register access in SuperGrafx mode
- Selection of target VDC for store-immediate CPU instructions
- Per-pixel priority control between VDC #1 and VDC #2 outputs
- Window-based control of layer visibility and priority

---

## 2. Video Data Interface

Each VDC outputs a 9-bit video code:

- Bits 3–0: pixel data
- Bits 7–4: palette data
- Bit 8: sprite/background indicator

These 9-bit codes are input to the VPC, processed, and a single 9-bit output
is forwarded to the VCE.

The VPC does not receive priority information internal to each VDC
(low/high sprite priority is resolved inside the VDCs).

---

## 3. Register Mapping

The HuC6202 registers are memory-mapped in the I/O page (physical block $FF),
within the video register area.

### Address range (bank $FF offsets)

| Offset | Register |
|------:|----------|
| $0008 | Priority Control Register A |
| $0009 | Priority Control Register B |
| $000A | Window 1 Width (LSB) |
| $000B | Window 1 Width (MSB) |
| $000C | Window 2 Width (LSB) |
| $000D | Window 2 Width (MSB) |
| $000E | Store Immediate Target Select |
| $000F | Unused |

Registers are mirrored throughout the video I/O region according to system mode.

---

## 4. Priority Control Registers

### 4.1 Priority Control Register A — $0008

Defines priority and visibility for two window regions.

| Bits | Region |
|-----:|--------|
| 7–4 | Window 2 region |
| 3–0 | Overlap of Window 1 and Window 2 |

Each 4-bit field has the following layout:

| Bit | Meaning |
|----:|---------|
| 3 | Priority mode bit 1 |
| 2 | Priority mode bit 0 |
| 1 | VDC #2 display enable |
| 0 | VDC #1 display enable |

---

### 4.2 Priority Control Register B — $0009

Defines priority and visibility for the remaining two regions.

| Bits | Region |
|-----:|--------|
| 7–4 | Region with no window |
| 3–0 | Window 1 region |

Each 4-bit field uses the same format as Register A.

Registers return the last value written.

---

## 5. Priority Modes

The priority mode is defined by a 2-bit field:

| Bits | Mode |
|-----:|------|
| 00 | Default ordering |
| 01 | Alternate ordering |
| 10 | Alternate ordering |
| 11 | Default ordering |

The priority mode affects the relative ordering of the four possible layers:
background and sprite output from both VDCs.

---

## 6. Window Registers

The VPC provides two horizontal windows, each defined by a 10-bit width.

Windows start from the leftmost pixel of the physical screen.

### 6.1 Window 1 Width

- LSB: $000A (bits 7–0)
- MSB: $000B (bits 1–0)

---

### 6.2 Window 2 Width

- LSB: $000C (bits 7–0)
- MSB: $000D (bits 1–0)

---

### Window Width Characteristics

- Width is specified in single-pixel units
- Values below $0040 disable the window
- A value of $03FF spans the full screen width

Only bits 1–0 of the MSB registers are stored.

---

## 7. Store-Immediate Target Select

### Store Immediate Control Register — $000E

| Bit | Meaning |
|----:|---------|
| 0 | Store-immediate target select |
| 7–1 | Unused |

When bit 0 is set, store-immediate CPU instructions (ST0, ST1, ST2)
target VDC #2 instead of VDC #1.

Reads always return $00.

---

## 8. Unused Register

### Register $000F

- Not used
- Reads return $00
- Writes have no defined effect

---

## 9. Power-On State

On power-up, the VPC registers are initialized as follows:

| Register | Value |
|--------:|-------|
| $0008 | $11 |
| $0009 | $11 |
| $000A | $00 |
| $000B | $00 |
| $000C | $00 |
| $000D | $00 |
| $000E | $00 |
| $000F | $00 |

This configuration enables only VDC #1 output with default priority and
disables both windows.
