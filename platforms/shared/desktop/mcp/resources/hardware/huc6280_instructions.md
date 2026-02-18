# HuC6280 — Instruction Set Reference (Software View)

## 1. General Notes

- CPU core: 65C02-compatible with HuC6280 extensions
- Registers affected: A, X, Y, S, P, PC
- Flags in P: N V T B D I Z C
- Cycle counts are base values; page-cross penalties are noted when applicable
- Bytes include opcode and operands

---

## 2. Arithmetic Instructions

### ADC — Add with Carry
- Bytes: 2–3
- Cycles: 2–6
- Modes: IMM, ZP, ZP,X, (ZP), (ZP,X), (ZP),Y, ABS, ABS,X, ABS,Y
- Operation: A ← A + M + C
- Flags: N V Z C

---

### SBC — Subtract with Carry
- Bytes: 2–3
- Cycles: 2–6
- Modes: IMM, ZP, ZP,X, (ZP), (ZP,X), (ZP),Y, ABS, ABS,X, ABS,Y
- Operation: A ← A − M − (1 − C)
- Flags: N V Z C

---

### INC — Increment Memory / Accumulator
- Bytes: 1–3
- Cycles: 2–7
- Modes: ACC, ZP, ZP,X, ABS, ABS,X
- Operation: M or A ← M or A + 1
- Flags: N Z

---

### DEC — Decrement Memory / Accumulator
- Bytes: 1–3
- Cycles: 2–7
- Modes: ACC, ZP, ZP,X, ABS, ABS,X
- Operation: M or A ← M or A − 1
- Flags: N Z

---

### INX / INY
- Bytes: 1
- Cycles: 2
- Operation: X or Y ← X or Y + 1
- Flags: N Z

---

### DEX / DEY
- Bytes: 1
- Cycles: 2
- Operation: X or Y ← X or Y − 1
- Flags: N Z

---

## 3. Logical Instructions

### AND / ORA / EOR
- Bytes: 2–3
- Cycles: 2–6
- Modes: IMM, ZP, ZP,X, (ZP), (ZP,X), (ZP),Y, ABS, ABS,X, ABS,Y
- Operation:
  - AND: A ← A ∧ M
  - ORA: A ← A ∨ M
  - EOR: A ← A ⊕ M
- Flags: N Z

---

### BIT
- Bytes: 2–3
- Cycles: 2–4
- Modes: IMM, ZP, ZP,X, ABS, ABS,X
- Operation: Test bits of M against A
- Flags: Z from A ∧ M, N and V from M

---

## 4. Shift and Rotate Instructions

### ASL / LSR
- Bytes: 1–3
- Cycles: 2–7
- Modes: ACC, ZP, ZP,X, ABS, ABS,X
- Operation:
  - ASL: shift left, bit 7 → C
  - LSR: shift right, bit 0 → C
- Flags: N Z C

---

### ROL / ROR
- Bytes: 1–3
- Cycles: 2–7
- Modes: ACC, ZP, ZP,X, ABS, ABS,X
- Operation:
  - Rotate through carry
- Flags: N Z C

---

## 5. Load and Store Instructions

### LDA / LDX / LDY
- Bytes: 2–3
- Cycles: 2–6
- Modes:
  - LDA: IMM, ZP, ZP,X, (ZP), (ZP,X), (ZP),Y, ABS, ABS,X, ABS,Y
  - LDX: IMM, ZP, ZP,Y, ABS, ABS,Y
  - LDY: IMM, ZP, ZP,X, ABS, ABS,X
- Operation: Load register from memory
- Flags: N Z

---

### STA / STX / STY / STZ
- Bytes: 2–3
- Cycles: 2–6
- Modes:
  - STA: ZP, ZP,X, (ZP), (ZP,X), (ZP),Y, ABS, ABS,X, ABS,Y
  - STX: ZP, ZP,Y, ABS
  - STY: ZP, ZP,X, ABS
  - STZ: ZP, ZP,X, ABS, ABS,X
- Operation: Store register or zero to memory
- Flags: none

---

## 6. Register Transfer Instructions

### TAX / TAY / TXA / TYA
- Bytes: 1
- Cycles: 2
- Operation: Transfer between A, X, Y
- Flags: N Z

---

### TSX / TXS
- Bytes: 1
- Cycles: 2
- Operation: Transfer between S and X
- Flags: TSX sets N Z; TXS affects no flags

---

### SAX / SAY / SXY
- Bytes: 1
- Cycles: 3
- Operation: Swap registers
- Flags: N Z

---

## 7. Stack Instructions

### PHA / PHP / PHX / PHY
- Bytes: 1
- Cycles: 3
- Operation: Push register onto stack
- Flags: none

---

### PLA / PLP / PLX / PLY
- Bytes: 1
- Cycles: 4
- Operation: Pull register from stack
- Flags: Loaded from value

---

## 8. Branch Instructions

### Conditional Branches
- Bytes: 2
- Cycles: 2 (+1 if branch taken, +1 if page crossed)
- Instructions:
  - BCC, BCS
  - BEQ, BNE
  - BMI, BPL
  - BVC, BVS
- Operation: PC ← PC + offset if condition true

---

### BRA — Branch Always
- Bytes: 2
- Cycles: 3
- Operation: PC ← PC + offset

---

### BBRi / BBSi
- Bytes: 3
- Cycles: 5
- Operation:
  - Test bit i of zero-page memory and branch on reset/set

---

## 9. Jump and Subroutine Instructions

### JMP
- Bytes: 3
- Cycles: 3
- Modes: ABS, (ABS), (ABS,X)
- Operation: PC ← address

---

### JSR
- Bytes: 3
- Cycles: 6
- Operation: Push return address; PC ← target

---

### RTS
- Bytes: 1
- Cycles: 6
- Operation: Pull return address; PC ← PC + 1

---

### BSR
- Bytes: 2
- Cycles: 8
- Operation: Relative subroutine call

---

### RTI
- Bytes: 1
- Cycles: 7
- Operation: Restore P and PC from stack

---

## 10. Flag Control Instructions

| Instruction | Bytes | Cycles | Effect |
|------------|-------|--------|--------|
| CLC | 1 | 2 | C ← 0 |
| SEC | 1 | 2 | C ← 1 |
| CLI | 1 | 2 | I ← 0 |
| SEI | 1 | 2 | I ← 1 |
| CLD | 1 | 2 | D ← 0 |
| SED | 1 | 2 | D ← 1 |
| CLV | 1 | 2 | V ← 0 |
| SET | 1 | 2 | T ← 1 |

---

## 11. Bit Manipulation Instructions

### RMBi / SMBi
- Bytes: 2
- Cycles: 5
- Operation: Reset / Set bit i in zero-page memory
- Flags: none

---

### TRB / TSB
- Bytes: 2–3
- Cycles: 5–6
- Modes: ZP, ABS
- Operation:
  - TRB: M ← M & ~A
  - TSB: M ← M | A
- Flags: Z from A ∧ M

---

### TST
- Bytes: 3–4
- Cycles: 5–7
- Modes: IMM ZP, IMM ZP,X, IMM ABS, IMM ABS,X
- Operation: Test memory against immediate
- Flags: Z, N, V

---

## 12. Block Transfer Instructions

### TII / TIA / TAI / TDD / TIN
- Bytes: 7
- Cycles: Variable (per transferred word)
- Operation:
  - Memory block transfer
  - Source, destination, length from registers
- Registers used: A, X, Y, SH, DH, LH
- Flags: none

---

#### TII — Transfer Increment / Increment
- Source increments
- Destination increments

---

#### TIA — Transfer Increment / Alternate
- Source increments
- Destination alternates between increment and fixed

---

#### TAI — Transfer Alternate / Increment
- Source alternates between increment and fixed
- Destination increments

---

#### TDD — Transfer Decrement / Decrement
- Source decrements
- Destination decrements

---

#### TIN — Transfer Fixed / Increment
- Source fixed
- Destination increments

---

## 13. Memory Mapping Instructions

### TAMi
- Bytes: 2
- Cycles: 4
- Operation: Transfer A to mapping register i. Each bit set in the immediate operand selects one MPR (0–7).

---

### TMAi
- Bytes: 2
- Cycles: 4
- Operation: Transfer mapping register i to A
- Flags: N Z

---

## 14. HuC6270 Interface Instructions

### ST0 / ST1 / ST2
- Bytes: 2
- Cycles: 4
- Operation: Store immediate value to HuC6270 control ports
- Target address determined by opcode

#### Operation

These instructions store an immediate value to fixed VDC I/O addresses:

| Instruction | Target |
|------------|--------|
| ST0 | VDC register select |
| ST1 | VDC data (low byte) |
| ST2 | VDC data (high byte) |

The target address is encoded in the opcode.

Flags affected:
- None

---

## 15. System Instructions

### BRK

- Bytes: 1
- Cycles: 7
- Addressing: Implied

#### Operation

1. Program Counter is incremented by 2
2. High byte of PC is pushed to stack
3. Low byte of PC is pushed to stack
4. Status register P is pushed to stack with:
   - B flag set
   - T flag cleared
5. Interrupt Disable flag (I) is set
6. Program Counter is loaded from vector at $FFF6 (IRQ2/BRK)

BRK shares the IRQ2 vector.

Flags affected:
- B set in stacked value
- I set in P

---

### NOP
- Bytes: 1
- Cycles: 2
- Operation: No operation

---

## 16. Clear Instructions

### CLA / CLX / CLY

#### CLA — Clear Accumulator
- Bytes: 1
- Cycles: 2
- Operation: A ← 0
- Flags: N Z

---

#### CLX — Clear X Register
- Bytes: 1
- Cycles: 2
- Operation: X ← 0
- Flags: N Z

---

#### CLY — Clear Y Register
- Bytes: 1
- Cycles: 2
- Operation: Y ← 0
- Flags: N Z