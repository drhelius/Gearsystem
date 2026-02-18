# HuC6280 — CMOS 8-bit Microprocessor

## 1. Overview

The HuC6280 is an 8-bit CMOS microprocessor used as the main CPU in the PC Engine / TurboGrafx-16.
It is based on a 65C02-compatible core with additional instructions and integrated peripheral
hardware.

Integrated functional blocks:

- 65C02-compatible CPU core
- Memory Management Unit (MMU) with 8 mapping registers
- Interrupt controller
- 7-bit interval timer
- 8-bit input port
- 8-bit output port
- Programmable Sound Generator (PSG)

Addressing characteristics:

- 64 KB logical address space
- 2 MB physical address space
- Address translation via 8 KB pages

---

## 2. CPU Core Registers

The HuC6280 contains the following CPU-visible registers.

### 2.1 Accumulator (ACC)

- 8-bit general-purpose register
- Used as the primary ALU operand and result register
- Used as the low byte of the block transfer length counter

---

### 2.2 Index Registers (X, Y)

- 8-bit general-purpose registers
- Used for indexed addressing modes
- Used as low bytes of source and destination addresses during block transfer operations

---

### 2.3 Program Counter (PCL, PCH)

- 16-bit program counter composed of:
  - PCL: low byte
  - PCH: high byte
- Automatically incremented during instruction fetch
- Loaded from reset or interrupt vectors on control transfer

---

### 2.4 Stack Pointer (S)

- 8-bit register
- Holds the low byte of the stack address
- Stack is fixed to logical addresses $2100–$21FF
- Stack grows downward
- Initial value is undefined after reset

---

### 2.5 Status Register (P)

8-bit register containing processor status flags.

Bit layout:

| Bit | Name | Description |
|----:|------|-------------|
| 7 | N | Negative |
| 6 | V | Overflow |
| 5 | T | Memory Operation flag |
| 4 | B | Break Command |
| 3 | D | Decimal Mode |
| 2 | I | Interrupt Disable |
| 1 | Z | Zero |
| 0 | C | Carry |

---

### 2.6 Block Transfer Registers (SH, DH, LH)

Special-purpose internal registers used only during block transfer instructions.

- SH: Source address high byte
- DH: Destination address high byte
- LH: Length counter high byte

These registers are not directly accessible by software.

---

## 3. Memory Management Unit (MMU)

### 3.1 Logical Address Space

The logical address space is 64 KB, divided into eight 8 KB pages:

| Page | Logical Range |
|----:|----------------|
| 0 | $0000–$1FFF |
| 1 | $2000–$3FFF |
| 2 | $4000–$5FFF |
| 3 | $6000–$7FFF |
| 4 | $8000–$9FFF |
| 5 | $A000–$BFFF |
| 6 | $C000–$DFFF |
| 7 | $E000–$FFFF |

---

### 3.2 Mapping Registers (MPR0–MPR7)

- Eight 8-bit mapping registers
- Each register selects one 8 KB block in physical memory
- The selected block is mapped into the corresponding logical page

Physical address formation:

- Logical address bits A0–A12 select offset within page
- Mapping register value supplies physical address bits A13–A20

---

### 3.3 Mapping Register Selection

Mapping register selected by logical address bits:

| H7 H6 H5 | Mapping Register |
|--------:|------------------|
| 000 | MPR0 |
| 001 | MPR1 |
| 010 | MPR2 |
| 011 | MPR3 |
| 100 | MPR4 |
| 101 | MPR5 |
| 110 | MPR6 |
| 111 | MPR7 |

---

### 3.4 Accessing Mapping Registers

Two instructions are provided:

- TAMi: Transfer ACC to mapping register
- TMAi: Transfer mapping register to ACC

The register index is selected via a bitmask in the instruction operand.

---

### 3.5 Reset State of Mapping Registers

- MPR7 is set to $00 after reset
- Other mapping registers are undefined and must be initialized by software

---

## 4. Physical Address Space

- Total size: 2 MB
- Organized as 256 blocks of 8 KB
- Block numbers range from $00 to $FF

Commonly used blocks:

| Block | Typical Use |
|------:|-------------|
| $00–$7F | ROM |
| $F7 | Battery-backed RAM |
| $F8 | Work RAM |
| $FF | I/O page |

---

## 5. Interrupt System

### 5.1 Interrupt Sources

The HuC6280 supports the following interrupts:

- RESET
- NMI (Non-maskable interrupt)
- IRQ1 (external)
- IRQ2 (external / BRK)
- TIMER (internal)
- BRK (software interrupt)

---

### 5.2 Interrupt Priority

From highest to lowest priority:

1. RESET
2. NMI
3. BRK
4. TIMER
5. IRQ1
6. IRQ2

---

### 5.3 Interrupt Vectors (Logical Addresses)

| Vector | Address |
|------|---------|
| IRQ2 / BRK | $FFF6 |
| IRQ1 | $FFF8 |
| TIMER | $FFFA |
| NMI | $FFFC |
| RESET | $FFFE |

---

### 5.4 Interrupt Disable Register

Logical address: $1402 (I/O page)

Bit layout:

| Bit | Name | Description |
|----:|------|-------------|
| 2 | TIQD | Timer interrupt disable |
| 1 | IRQ1D | IRQ1 disable |
| 0 | IRQ2D | IRQ2 disable |

---

### 5.5 Interrupt Request Register

Logical address: $1403 (I/O page)

Bit layout:

| Bit | Name | Description |
|----:|------|-------------|
| 2 | TIQ | Timer interrupt request |
| 1 | IRQ1 | IRQ1 request |
| 0 | IRQ2 | IRQ2 request |

Writing to this register clears the timer interrupt request.

---

## 6. Timer

### 6.1 General Description

The HuC6280 contains a 7-bit interval timer.

Components:

- 7-bit downcounter
- 7-bit reload register
- Timer control register
- Prescaler

---

### 6.2 Timer Clock

- Input clock derived from OSC1
- OSC1 divided by 3, then by 1024
- Resulting timer clock ≈ 6.992 kHz when OSC1 = 21.48 MHz

---

### 6.3 Timer Registers

Timer registers are mapped in the I/O page.

| Address | Access | Function |
|--------:|:------:|----------|
| $0C00 | R/W | Reload register (bits 6–0) / Downcounter |
| $0C01 | W | Timer control register |

---

### 6.4 Timer Control Register

Bit layout:

| Bit | Name | Description |
|----:|------|-------------|
| 0 | START | Timer start/stop |
| 7–1 | — | Unused |

---

## 7. I/O Ports

### 7.1 Port K (Input Port)

- 8-bit input port
- TTL-level inputs with pull-up resistors
- Mapped to physical addresses $1FF000–$1FF3FF

---

### 7.2 Port O (Output Port)

- 8-bit output port with latch
- Complementary outputs
- Mapped to physical addresses $1FF000–$1FF3FF

---

## 8. Programmable Sound Generator (PSG)

The HuC6280 integrates a PSG block.

Interface signals include:

- Data bus D0–D7
- Address lines A0–A3
- Read/Write control
- Chip enable
- Clock input derived from OSC1
- Reset signal

Detailed PSG operation is described in a separate manual.

---

## 9. System Reset

### 9.1 Reset Input

- RESET is active low
- Minimum pulse width: 28 clock cycles with stabilized OSC1

---

### 9.2 Internal State After Reset

After reset:

- Interrupt Disable flag is set
- Decimal flag is cleared
- Timer is stopped
- Timer interrupt request is cleared
- Interrupt disable register is cleared
- MPR7 is set to $00
- Other MPRs are undefined
- Output port is set to high level
- System clock is output on SX pin

---

## 10. Instruction Extensions

The HuC6280 extends the 65C02 instruction set with:

- Block transfer instructions (TII, TIA, TAI, TIN, TDD)
- Bit manipulation instructions (SMB, RMB, BBS, BBR)
- Mapping register access instructions (TAM, TMA)
- Immediate I/O store instructions (ST0, ST1, ST2)

Instruction semantics are defined in the instruction set documentation.