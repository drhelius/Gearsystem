# HuC6280 — Programmable Sound Generator (PSG)

## 1. Overview

The HuC6280 includes an integrated Programmable Sound Generator (PSG) block.

General characteristics:

- Waveform memory sound generation
- Six sound channels
- Two noise generators (channels 5 and 6)
- Low Frequency Oscillator (LFO)
- Stereo output
- Direct D/A (DDA) mode
- Single master clock source

Waveform generation is based on waveform memory, with one cycle defined by 32 samples.

---

## 2. PSG Channel Configuration

### 2.1 Channel Count and Capabilities

| Channel | Waveform | Noise | LFO role |
|--------:|----------|-------|----------|
| 1 | Yes | No | Modulated by LFO |
| 2 | Yes | No | LFO waveform source |
| 3 | Yes | No | — |
| 4 | Yes | No | — |
| 5 | Yes | Yes | — |
| 6 | Yes | Yes | — |

All six channels share the same register structure except for the noise register (R7),
which exists only for channels 5 and 6.

---

## 3. Register Addressing Model

The PSG registers are selected using:

- Address lines A0–A3
- Channel Select Register (R0)

Registers R0, R1, R8 and R9 are global (one instance per PSG).
Registers R2–R7 are per-channel (except R7, only for channels 5 and 6).

Channel selection applies only to registers R2–R7.

---

## 4. PSG Register Summary

| Register | Name | Scope |
|---------|------|-------|
| R0 | Channel Select | Global |
| R1 | Main Amplitude Level (LMAL, RMAL) | Global |
| R2 | Frequency Low | Per-channel |
| R3 | Frequency High | Per-channel |
| R4 | Channel On / DDA / Amplitude Level | Per-channel |
| R5 | Left / Right Amplitude Level | Per-channel |
| R6 | Waveform Data | Per-channel |
| R7 | Noise Enable / Noise Frequency | Channel 5, 6 |
| R8 | LFO Frequency | Global |
| R9 | LFO Control | Global |

---

## 5. Channel Select Register — R0

Selects the active channel for access to per-channel registers.

### Bit layout

- Bits 7–5: unused
- Bits 4–0: channel select value

The value written determines which channel is affected by subsequent accesses
to registers R2–R7.

---

## 6. Main Amplitude Level Register — R1

Controls the overall output amplitude of the mixed PSG output.

### Bit layout

| Bits | Function |
|-----:|----------|
| 7–4 | LMAL (Left main amplitude level) |
| 3–0 | RMAL (Right main amplitude level) |

This register applies globally to all channels.

---

## 7. Frequency Registers — R2, R3

### R2 — Frequency Low

- Bits 7–0: lower 8 bits of frequency value

### R3 — Frequency High

- Bits 7–4: unused
- Bits 3–0: upper 4 bits of frequency value

R2 and R3 together form a 12-bit frequency value `F`.

---

## 8. Channel Control Register — R4

Controls channel enable, DDA mode, and channel amplitude level.

### Bit layout

| Bit | Name | Description |
|----:|------|-------------|
| 7 | chON | Channel enable |
| 6 | DDA | Direct D/A mode |
| 5–0 | AL | Channel amplitude level |

The interpretation of R6 (waveform data) depends on the state of `chON` and `DDA`.

---

## 9. Left / Right Amplitude Register — R5

Controls stereo balance per channel.

### Bit layout

| Bits | Function |
|-----:|----------|
| 7–4 | LAL (Left amplitude level) |
| 3–0 | RAL (Right amplitude level) |

---

## 10. Waveform Register — R6

Stores waveform data for the selected channel.

### Characteristics

- 32 words per waveform cycle
- Each word is 5 bits wide
- Internal waveform address counter selects the active word

### Bit layout

- Bits 7–5: unused
- Bits 4–0: waveform amplitude data

The internal waveform address counter advances according to the channel mode.

---

## 11. Noise Register — R7 (Channels 5 and 6)

Controls noise generation.

### Bit layout

| Bit | Name | Description |
|----:|------|-------------|
| 7 | NE | Noise enable |
| 6–5 | unused |
| 4–0 | NF | Noise frequency |

Noise replaces waveform output when enabled.

---

## 12. Low Frequency Oscillator (LFO)

The LFO modulates the frequency of channel 1 using waveform data from channel 2.

---

### 12.1 LFO Frequency Register — R8

Defines the LFO frequency in combination with channel 2 frequency registers.

### Bit layout

- Bits 7–0: LFO frequency value

---

### 12.2 LFO Control Register — R9

Controls LFO triggering and modulation depth.

### Bit layout

| Bits | Name | Description |
|-----:|------|-------------|
| 7 | LF TRG | LFO trigger/reset |
| 6–2 | unused |
| 1–0 | LF CTL | Modulation depth |

---

## 13. Frequency Generation

### 13.1 Waveform Output Frequency

Let:

- `F` = 12-bit value from R3:R2
- `f_master` = 7.16 MHz

The waveform output frequency is:

```
f_noise = f_master / (2 × 32 × 2 × NF)
```

`NF = 0` is undefined.

---

## 14. Amplitude Control Hierarchy

Amplitude control is applied at three levels:

1. Global level: R1 (LMAL / RMAL)
2. Channel level: R4 (AL)
3. Stereo balance: R5 (LAL / RAL)

Each register contributes independently to the final amplitude.

---

## 15. Waveform Memory Organization

- One waveform cycle consists of 32 samples
- Each sample is 5 bits wide
- Samples are addressed sequentially by an internal counter
- The counter behavior depends on channel mode configuration

---

## 16. Scale Table

The PSG frequency register values correspond to musical pitches
based on equal temperament, using `f_master = 7.16 MHz`.

The scale table defines the relation between:

- Musical note
- Register values (R3:R2)
- Resulting output frequency