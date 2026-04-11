
 Sega Master System VDP documentation
 by Charles MacDonald
 E-mail: cgfm2@hotmail.com
 WWW: http://cgfm2.emuviews.com

 Unpublished work Copyright 2000, 2001, 2002  Charles MacDonald

 Table of contents:

  1.) Introduction
  2.) VDP ports
  3.) VDP programming
  4.) Status flags
  5.) Color RAM
  6.) Display modes
  7.) Register reference
  8.) Patterns
  9.) Background
 10.) Sprites
 11.) Display timing
 12.) Interrupts
 13.) Genesis VDP
 14.) Game Gear VDP
 15.) Miscellaneous
 16.) Credits and acknowledgements

 What's New

 (11/12/02)
 - Added information about SMS VDP status flags.
 - Fixed mode list for SMS VDP.
 - Updated description of the invalid text mode.
 - Added sprite zooming description for Genesis VDP.
 - Added information about table addressing for the SMS VDP.

 (05/15/02)
 - Fixed some errors in the register reference.

 (04/14/02)
 - Added note about fixed resolutions in GG mode.
 - Added note about screen blanking for TMS9918 modes on the Genesis.
 - Removed information about bit 7 of register #1. (it does nothing)
 - Added information about how SMS display is shown on a GameGear.
 - Added information about GameGear palette hardware.
 - Added information about TMS9918 CRAM use on a GG.

 ----------------------------------------------------------------------------
 1.) Introduction
 ----------------------------------------------------------------------------

 The Video Display Processor (VDP) is a graphics chip derived from the
 Texas Instruments TMS9918, used by Sega in their video game consoles and
 arcade hardware.

 There are four versions of the VDP:

 315-5124 - Used in the Mark III, Sega Master System .
 315-5246 - Used in the SMS 2 and later versions of the SMS. 
 315-5378 - Used in the Game Gear. (1)
 315-5313 - Used in the Genesis and Mega Drive. (2)

 Just for simplicity, I'll refer to these in order as the SMS, SMS 2, GG,
 and Genesis VDP. Though the real distinction is in the part number, not
 the console they were used in.

 All versions of the VDP also have a Texas Instruments SN76489 sound chip
 built in. It is identical to the stand-alone version of the same chip, so
 I won't discuss how it works beyond what VDP ports it is mapped to. (3)

 Notes:

 1. The Game Gear also has another chip, the 315-5377. It isn't known which
    one is the VDP, in this document, I'll assume it's the 315-5378.

 2. The Genesis video hardware is substantially different from other
    versions of the VDP. I'll only discuss it's implementation of mode 4.

 3. The Game Gear has a modified PSG, where each of the four channels
    (three tone and one noise) can be output through the left or right
    speakers by a stereo control register mapped to Z80 I/O port $06.
    Other than this feature, it works identically to the SN76489.

 ----------------------------------------------------------------------------
 2.) VDP ports
 ----------------------------------------------------------------------------

 In the Master System and Game Gear, the VDP is commonly accessed at the
 following Z80 I/O ports:

 $7E = V counter (read) / SN76489 data (write)
 $7F = H counter (read) / SN76489 data (write, mirror)
 $BE = Data port (r/w)
 $BF = Control port (r/w)

 The address decoding for the I/O ports is done with A7, A6, and A0 of
 the Z80 address bus, so the VDP locations are mirrored:

 $40-7F = Even locations are V counter/PSG, odd locations are H counter/PSG
 $80-BF = Even locations are data port, odd locations are control port.

 The H and V counters are described in the display timing section.
 The control and data ports are described in the VDP programming section.

 ----------------------------------------------------------------------------
 3.) VDP Programming
 ----------------------------------------------------------------------------

 Control port

 The VDP is programmed by sending a two-byte sequence to the control port,
 which I call a command word. The command word is used to define an offset
 into VRAM or CRAM for subsequent data port I/O, and also to write to the
 internal VDP registers.

 In order for the VDP to know if it is recieving the first or second byte
 of the command word, it has a flag which is set after the first one is sent,
 and cleared when the second byte is written. The flag is also cleared when
 the control port is read, and when the data port is read or written. This
 is primarily used to initialize the flag to zero after it has been modified
 unpredictably, such as after an interrupt routine has executed.

 The VDP has two components that are used for accessing CRAM and VRAM,
 the address register and the code register. The address register is 14
 bits and defines the address into VRAM for reads and writes, and the
 address into CRAM for writes. The code register is 2 bits and selects
 four different operations: VRAM write, VRAM read, CRAM write, and VDP
 register write.

 The command word has the following format:

 MSB                         LSB
 CD1 CD0 A13 A12 A11 A10 A09 A08    Second byte written
 A07 A06 A05 A04 A03 A02 A01 A00    First byte written

 CDx : Code register 
 Axx : Address register

 When the first byte is written, the lower 8 bits of the address register are
 updated. When the second byte is written, the upper 6 bits of the address
 register and the code register are updated, and the VDP may carry out
 additional processing based on the value of the code register:

 Code value         Actions taken

    0               A byte of VRAM is read from the location defined by the
                    address register and is stored in the read buffer. The
                    address register is incremented by one. Writes to the
                    data port go to VRAM.
    1               Writes to the data port go to VRAM.
    2               This value signifies a VDP register write, explained
                    below. Writes to the data port go to VRAM.
    3               Writes to the data port go to CRAM.

 When accessing CRAM, the upper bits of the address register are ignored as
 CRAM is smaller than 16K (either 32 or 64 bytes depending on the VDP).
 The address register will wrap when it exceeds $3FFF.
   
 VDP register write

 While the address and code register are updated like normal when a VDP
 register write is done, the command word sent can be viewed as having
 a different format to the programmer:

 MSB                         LSB
  1   0   ?   ?  R03 R02 R01 R00    Second byte written
 D07 D06 D05 D04 D03 D02 D01 D00    First byte written

 Rxx : VDP register number
 Dxx : VDP register data
  ?  : Ignored

 The VDP selects a register using bits 3-0 of the second byte, and writes
 the data from the first byte to the register in question. There are only
 10 registers, values 11 through 15 have no effect when written to.

 Data port

 Depending on the code register, data written to the data port is sent to
 either VRAM or CRAM. After each write, the address register is incremented
 by one, and will wrap past $3FFF.

 Reads from VRAM are buffered. Every time the data port is read (regardless
 of the code regsister) the contents of a buffer are returned. The VDP will
 then read a byte from VRAM at the current address, and increment the address
 register. In this way data for the next data port read is ready with no
 delay while the VDP reads VRAM. An additional quirk is that writing to the
 data port will also load the buffer with the value written.

 ----------------------------------------------------------------------------
 4.) Status flags 
 ----------------------------------------------------------------------------

 Reading the control port returns a byte containing status flags:

 MSB                         LSB
 INT OVR COL --- --- --- --- ---

 INT - Frame interrupt pending

 This flag is set on the first line after the end of the active display
 period. It is cleared when the control port is read. For more details,
 see the interrupts section.

 OVR - Sprite overflow

 This flag is set if there are more than eight sprites that are positioned
 on a single scanline. It is cleared when the control port is read. For more
 information see the sprites section.

 COL - Sprite collision

 This flag is set if an opaque pixel from any two sprites overlap. It is
 cleared when the control port is read. For more information see the
 sprites section.

 The remaining bits are not set by the VDP, and return garbage values for
 the SMS and SMS 2. The Genesis returns extended status flags in the lower
 bits that are specific to the Genesis VDP only.

 ----------------------------------------------------------------------------
 5.) Color RAM
 ----------------------------------------------------------------------------

 Master System:

 The color palette used by sprites and backgrounds is arranged as two
 16-color palettes. Background patterns can use either palette, while sprite
 patterns can only use the second one.

 The palette is defined by 32 bytes of color RAM (CRAM). This memory is
 internal to the VDP, and is write-only. Each byte has the following format:

 MSB  LSB
 --BBGGRR

 - = Unused
 R = Red component
 G = Green component
 B = Blue component

 Up to 64 possible colors can be used, and up to 32 can be displayed on
 screen at any given time.

 Game Gear:

 The Game Gear has improved palette hardware which is only available in GG
 mode. When the console is in SMS mode, the palette hardware works the same
 way as described above. Switching between modes does not change any colors
 that were previously set.

 In Game Gear mode, CRAM has been expanded to 32 words (64 bytes) Each word
 defines a single color, like so:

 MSB              LSB
 --------BBBBGGGGRRRR

 - = Unused
 R = Red component
 G = Green component
 B = Blue component

 Up to 4096 possible colors can be used, but only 32 can be shown at any
 given time.

 The address register now wraps past address $003F. Writing an even CRAM
 address causes the data written to be stored in a latch, and writing to
 an odd CRAM address makes the VDP write the contents of the latch as well
 as the new data from the CPU to the current CRAM entry. For example:

        ld      hl, $C000       ; CRAM address $0000
        rst     10h             ; Assume this function sets the VDP address
        ld      a, $FF          ; Color data
        out     ($BE), a        ; CRAM unchanged, latch = $FF
        ld      hl, $C021       ; CRAM address $0021
        rst     10h             ; Set the address again
        ld      a, $0F          ; Color data
        out     ($BE), a        ; CRAM word at $0020 is now $0FFF,
                                ;  and the data at $0000 is unchanged.

 So, writing single bytes to even CRAM addresses will not modify CRAM. 
 You could write multiple times to even addresses allows the same latched
 color data to be written to multiple palette entries.

 ----------------------------------------------------------------------------
 6.) Display modes
 ----------------------------------------------------------------------------

 The TMS9918 has three bits which select different display modes called M1,
 M2, and M3. However, only four combinations of these bits are documented
 in the TMS9918 manual:

 Mode 0 - Graphics I
 Mode 1 - Text
 Mode 2 - Graphics II
 Mode 3 - Multicolor

 The other four undocumented modes are simply variations of the above, they
 are not unique.

 The SMS VDP added another mode select bit that enabled mode 4, which is
 specific to the SMS. The SMS 2 and GG VDP changed the function of the
 TMS9918 mode select bits to pick different resolutions for a standard mode
 4 screen. The Genesis VDP added mode 5, which is specific to the Genesis.

 Here's a list of all possible display modes:

    M4 M3 M2 M1     SMS VDP                 SMS 2 / GG VDP
    -- -- -- --     ----------              -------------------------
     0  0  0  0     Graphic I               Graphic I
     0  0  0  1     Text                    Text
     0  0  1  0     Graphic 2               Graphic 2
     0  0  1  1     Mode 1+2                Mode 1+2
     0  1  0  0     Multicolor              Multicolor
     0  1  0  1     Mode 1+3                Mode 1+3
     0  1  1  0     Mode 2+3                Mode 2+3
     0  1  1  1     Mode 1+2+3              Mode 1+2+3
     1  0  0  0     Mode 4                  Mode 4
     1  0  0  1     Invalid text mode       Invalid text mode
     1  0  1  0     Mode 4                  Mode 4
     1  0  1  1     Invalid text mode       Mode 4 (224-line display)
     1  1  0  0     Mode 4                  Mode 4
     1  1  0  1     Invalid text mode       Invalid text mode
     1  1  1  0     Mode 4                  Mode 4 (240-line display)
     1  1  1  1     Invalid text mode       Mode 4

 For the undocumented TMS9918 modes, I used the same naming convention
 as in Sean Young's TMS9918 documentation.

 To summarize from the above chart, for the SMS 2 and GG VDP only, the M1 and
 M3 bits select the display resolution only when M2 is set. If both bits are
 set, the regular 192-line display is used. If M2 is cleared, then M3 has no
 effect and M1 will select the invalid text mode.

 The invalid text mode is identical to TMS9918 text mode. It uses CRAM for
 colors instead of using a fixed palette and register #7 for selecting the
 text color. Each tile is repeated four times on the display, which limits
 the usefulness of this mode. Scrolling only moves the tile graphics
 around within their 6x8 cells, the actual position in the name table is not
 changed. If bit 7 of register #0 is set, columns 0-23, 32-39 can scroll
 vertically while columns 24-31 do not. If bit 6 of register #1 is set,
 rows 0 and 1 will not scroll horizontally. Finally, setting the horizontal
 scroll to values of 6 or 7 will cause each tile to be filled with the
 backdrop color.

 If the GameGear is in GG mode, it's resolution is fixed and does not
 change when the extended resolutions are used. However, the differences
 in sprite and background processing are still used. (e.g. end of sprite
 list marker doesn't work, background starts at $0700 offsets, etc.)

 ----------------------------------------------------------------------------
 7.) Register reference
 ----------------------------------------------------------------------------

 The following information only applies to mode 4. When using the TMS9918
 modes, some registers cannot be used and others have different purposes.

 Register $00 - Mode Control No. 1

 D7 - 1= Disable vertical scrolling for columns 24-31
 D6 - 1= Disable horizontal scrolling for rows 0-1  
 D5 - 1= Mask column 0 with overscan color from register #7
 D4 - (IE1) 1= Line interrupt enable
 D3 - (EC) 1= Shift sprites left by 8 pixels
 D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
 D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4.
      Otherwise has no effect.
 D0 - 1= No sync, display is monochrome, 0= Normal display

 Bits 0 and 5 have no effect on the GameGear in either mode, while bit 6
 has no effect in GG mode but works normally in SMS mode.

 For the SMS VDP, setting bit 1 causes the sync information to be lost and
 gradually the color and picture brightness fade until the display is off.

 Register $01 - Mode Control No. 2

 D7 - No effect
 D6 - (BLK) 1= Display visible, 0= display blanked.
 D5 - (IE0) 1= Frame interrupt enable.
 D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
 D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
 D2 - No effect
 D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
 D0 - Sprite pixels are doubled in size.

 Even though some games set bit 7, it does nothing.

 Register $02 - Name Table Base Address

 D7 - No effect
 D6 - No effect
 D5 - No effect
 D4 - No effect
 D3 - Bit 13 of the table address
 D2 - Bit 12 of the table address
 D1 - Bit 11 of the table address
 D0 - No effect

 If the 224 or 240-line displays are being used, only bits 3 and 2 select
 the table address like so:

 D3 D2  Address
 -- --  -------
  0  0  $0700
  0  1  $1700
  1  0  $2700
  1  1  $3700

 For the SMS VDP only, the contents of this register are handled differently.
 In order to explain, here is a layout of the VRAM address bus when the VDP
 fetches name table data:

 MSB          LSB
 --bbbrrrrrcccccw   Address bus
 --xxxx----------   Contents of register #2 shifted to the left

 b = Bits 3-1 of register #2
 r = Name table row
 c = Name table column
 w = Low or high byte of name table word
 x = Bits 3-0 of register #2

 In all other versions of the VDP, bit 0 of register #2 is ignored.
 However, the SMS VDP will logically AND bit 0 with the VDP address, meaning
 if bit 0 is cleared, bit 4 of the name table row is forced to zero.

 When the screen is displayed, this causes the lower 8 rows to mirror the
 top 16 rows. The only game that utilizes this feature is the Japanese
 version of Y's.

 The original TMS9918 has a similar problem with other table registers,
 however this is not apparent in the TMS9918 modes of the SMS2 and GG VDP.

 Register $03 - Color Table Base Address

 D7 - No effect
 D6 - No effect
 D5 - No effect
 D4 - No effect
 D3 - No effect
 D2 - No effect
 D1 - No effect
 D0 - No effect

 For the SMS VDP only, all bits should be set. Otherwise the VDP will fetch
 pattern data and name table data incorrectly.

 Register $04 - Background Pattern Generator Base Address

 D7 - No effect
 D6 - No effect
 D5 - No effect
 D4 - No effect
 D3 - No effect
 D2 - No effect
 D1 - No effect
 D0 - No effect

 For the SMS VDP only, bits 2-0 should be set. Otherwise the VDP will fetch
 pattern data and name table data incorrectly.

 Register $05 - Sprite Attribute Table Base Address

 D7 - No effect
 D6 - Bit 13 of the table base address
 D5 - Bit 12 of the table base address
 D4 - Bit 11 of the table base address
 D3 - Bit 10 of the table base address
 D2 - Bit  9 of the table base address
 D1 - Bit  8 of the table base address
 D0 - No effect

 For the SMS VDP only, if bit 0 is cleared the sprite X position and tile
 index will be fetched from the lower 128 bytes of the sprite attribute
 table instead of the upper 128 bytes. It should be set for normal operation.

 Register $06 - Sprite Pattern Generator Base Address

 D7 - No effect
 D6 - No effect
 D5 - No effect
 D4 - No effect
 D3 - No effect
 D2 - Bit 13 of the table base address
 D1 - No effect
 D0 - No effect

 For the SMS VDP only, bits 1 and 0 act as an AND mask over bits 8 and 6 of
 the tile index if cleared. They should be set for normal operation.

 Register $07 - Overscan/Backdrop Color

 D7 - No effect
 D6 - No effect
 D5 - No effect
 D4 - No effect
 D3 - Bit 3 of the overscan color
 D2 - Bit 2 of the overscan color
 D1 - Bit 1 of the overscan color
 D0 - Bit 0 of the overscan color

 The backdrop color is taken from the sprite palette.

 Register $08 - Background X Scroll

 All eight bits define the horizontal scroll value.
 See the background section for more details.

 Register $09 - Background Y Scroll

 All eight bits define the vertical scroll value.
 See the background section for more details.

 Register $0A - Line counter

 All eight bits define the line counter value.
 See the interrupts section for more details.

 ----------------------------------------------------------------------------
 8.) Patterns
 ----------------------------------------------------------------------------

 The background and sprites are made out of 8x8-pixel patterns. Each
 pattern is composed of four bitplanes, so individual pixels can be one
 of 16 colors.

 Pattern data is stored in VRAM. There is enough memory for 512 patterns,
 though some of the available VRAM is used by the sprite attribute table
 and name table.

 Each pattern uses 32 bytes. The first four bytes are bitplanes 0 through 3
 for line 0, the next four bytes are bitplanes 0 through 3 for line 1, etc.,
 up to line 7.

 For more information about how patterns are used, see the background and
 sprite sections.

 ----------------------------------------------------------------------------
 9.) Background
 ----------------------------------------------------------------------------

 The background is made up of 8x8 tiles that can use 16 colors from either
 palette. The background is defined by the name table, which is a 32x28 
 matrix of words stored in VRAM. If using the 224 or 240-line displays,
 the size of the name table is 32x32 instead. Each word in the name table
 has the following layout:

 MSB          LSB
 ---pcvhnnnnnnnnn

 - = Unused. Some games use these bits as flags for collision and damage
     zones. (such as Wonderboy in Monster Land, Zillion 2)
 p = Priority flag. When set, sprites will be displayed underneath the
     background pattern in question.
 c = Palette select.
 v = Vertical flip flag.
 h = Horizontal flip flag.
 n = Pattern index, any one of 512 patterns in VRAM can be selected.

 Horizontal scrolling:

 Register $08 can be divided into two parts, the upper five bits are the
 starting column, and the lower three bits are the fine scroll value.

 On each scanline, the VDP has a column counter that goes from 0 to 31.
 The exact pixel on which the VDP starts rendering columns is offset to the
 right by the fine scroll value.

 For example, if the fine scroll value is set to 7, then 31 columns are
 fully visible and the first pixel of the 32nd column is shown at the far
 right edge of the display.

 The starting column value gives the first column in the name table to use,
 calculated by subtracting it from the value 32. So if the starting column
 value was $1D, the difference of it from 32 would be $02, hence the first
 column drawn is number 2 from the name table.

 After each column is drawn, the starting column value is incremented by
 one. It wraps at 32. When bit 7 of register #1 is set, it is the column
 counter, not the starting column value, that is checked to see when the
 vertical scroll value will be set to zero on columns 24 to 31.

 Most video display hardware will show part of the column that appears in
 the left 1-7 pixels of the display as the fine scroll value is increased
 (such as the NES, Genesis, TurboGrafx-16). However, the SMS VDP does not,
 and instead this area is filled with the backdrop color (and sometimes
 pattern data from sprite #0, but exactly how this is selected is unknown).

 For example, if the fine scroll value was set to $07, instead of normally
 seeing 7 pixels of column 32 in pixels 0-7, that area would be filled with
 the backdrop color and/or sprite pattern data.

 If bit #6 of VDP register $00 is set, horizontal scrolling will be fixed
 at zero for scanlines zero through 15. This is commonly used to create
 a fixed status bar at the top of the screen for horizontally scrolling
 games.

 Vertical scrolling

 Register $09 can be divided into two parts, the upper five bits are the
 starting row, and the lower three bits are the fine scroll value.

 In the regular 192-line display mode, the name table is 32x28 so the
 vertical scroll register wraps past 223. Values larger than 223 are
 treated as if the scroll range was between 0 and 31.

 In the 224 and 240-line display modes, the name table is 32x32 so the
 vertical scroll register wraps past 255.

 The vertical scroll value cannot be changed during the active display
 period, any changes made will be stored in a temporary location and
 used only when the active display period ends (prematurely blanking the
 screen with bit #6 of register #1 doesn't count).

 If bit 7 of register $00 is set, the vertical scroll value will be fixed
 to zero when columns 24 to 31 are rendered. This is commonly used to
 create a fixed status bar at the right edge of the screen for vertically
 scrolling games.

 ----------------------------------------------------------------------------
 10.) Sprites
 ----------------------------------------------------------------------------

 Each sprite is defined in the sprite attribute table (SAT), a 256-byte
 table located in VRAM. The SAT has the following layout:

    00: yyyyyyyyyyyyyyyy
    10: yyyyyyyyyyyyyyyy
    20: yyyyyyyyyyyyyyyy
    30: yyyyyyyyyyyyyyyy
    40: ????????????????
    50: ????????????????
    60: ????????????????
    70: ????????????????
    80: xnxnxnxnxnxnxnxn
    90: xnxnxnxnxnxnxnxn
    A0: xnxnxnxnxnxnxnxn
    B0: xnxnxnxnxnxnxnxn
    C0: xnxnxnxnxnxnxnxn
    D0: xnxnxnxnxnxnxnxn
    E0: xnxnxnxnxnxnxnxn
    F0: xnxnxnxnxnxnxnxn

 y = Y coordinate + 1
 x = X coordinate
 n = Pattern index
 ? = Unused

 The 64-byte unused area in the middle of the table can be used for anything,
 like pattern data.

 The Y coordinate is treated as being plus one, so a value of zero would
 place a sprite on scanline 1 and not scanline zero.

 If the Y coordinate is set to $D0, then the sprite in question and all
 remaining sprites of the 64 available will not be drawn. This only works
 in the 192-line display mode, in the 224 and 240-line modes a Y coordinate
 of $D0 has no special meaning.

 Sprites that are partially off-screen when the X coordinate is greater
 than 248 do not wrap around to the other side of the screen. If bit 3
 of register $00 is set, the X coordinate is treated as being minus eight.
 Sprites that are partially displayed on the left edge of the screen do
 not wrap, either.

 The pattern index selects one of 256 patterns to use. Bit 2 of register #6
 acts like a global bit 8 in addition to this value, allowing sprite patterns
 to be taken from the first 256 or last 256 of the 512 available patterns.

 When bit 1 of register #1 is set, bit 0 of the pattern index is ignored.
 The pattern index selects the pattern to be used for the top of the 8x16
 sprite, and the same pattern index plus one is used for the bottom half
 of the sprite.

 When bit 0 of register #1 is set, sprite pixels are zoomed to double
 their size. 8x8 sprites are 16x16, 8x16 sprites become 16x32. There is
 a bug in how the original SMS VDP processes zoomed sprites compared to
 the SMS 2 and GG VDP; it will only allow the first four sprites of the
 eight shown on a scanline to be zoomed horizontally and vertically, and
 the remaining four will be zoomed vertically. The SMS 2 and GG allow
 all eight sprites to be zoomed in both directions.

 In my opinion, this problem comes from the original TMS9918 which also
 supported zoomed sprites but only allowed four sprites to be shown per
 scanline. Perhaps the designers of the SMS VDP forgot to add 'horizontal
 zoom' flags to the extra four sprites they added, which is why only four of
 the eight sprites are affected.
 
 Table parsing:

 On each scanline, the VDP parses the SAT to find which sprites will be
 displayed on the next line. It goes through each Y coordinate and checks
 the position along with the sprite height (controlled by bit 1 of
 register #1) to see if the sprite falls on the next line. If it does, the
 sprite is added to an internal 8-entry buffer.

 The VDP stops parsing sprites under the following conditions:

 - All 64 sprites have been checked.
 - All eight buffer entries have been filled.
 - A sprite Y coordinate of 208 is found (in 192-line mode).

 If all eight buffer entries have been used and there are more sprites
 that fall on the same line, bit 6 of the status flags is set, indicating
 a sprite overflow condition occured. The bit remains set until the control
 port is read. Note that this is regardless of the sprite X coordinate
 or pattern data, so eight transprent sprites that were off-screen would
 count towards a sprite overflow.

 On the next scanline, the VDP uses the X coordinate as a counter that is
 decremented each time the H counter is incremented. (1) When the counter
 expires, the bitplanes for the current line of the sprite are shifted
 out one bit at a time to form a 4-bit pixel.

 The inter-sprite priority is defined by the order of the sprites in the
 internal buffer. An opaque pixel from a lower-entry sprite is displayed
 over any opaque pixel from a higher-entry sprite.

 For example: Out of the eight possible sprites, 5 are used.  Sprites 1,2,3
 have transparent pixels. Sprites 4 and 5 have opaque pixels. Only the
 pixel from sprite 4 will be shown since it comes before sprite 5.

 In the situation where any two sprites from any of the eight positions
 have opaque pixels that overlap, the VDP will set bit 6 of the status
 flags indicating a sprite collision has occured. This bit remains set
 until the status flags are read.

 The resulting sprite pixel is printed over any low priority background
 tile.  Or, for high priority background tiles, only where there is a
 transparent pixel.

 Note:

 1. This is how the TMS9918 describes it's sprite hardware. Though it's
    not actually important from an emulation standpoint, I assume the SMS
    also uses the X coordinate as a down-counter.

 ----------------------------------------------------------------------------
 11.) Display timing
 ----------------------------------------------------------------------------

 The two basic elements of the VDP are the H and V counters. The H counter
 is incremented with each dot clock. Different video-related events happen
 when the H counter reaches certain points. At the end of each scanline,
 the V counter is incremented by one, and this continues until an entire
 frame has been generated.

 The H and V counters can be read by the Z80 (see the VDP ports section
 for details). The V counter is free-running and can be read at any time.
 The H counter can only be read when the state of the TH pin of either
 joystick port changes, which is used for the lightgun peripheral. When
 this happens, the value of the H counter at the time TH changed is
 returned when the H counter is read, and remains frozen until TH is
 changed again.

 The V counter section shows which values will be read on each of the 262
 or 313 lines from the V counter. The V counter counts up linearly, but
 at certain points the value will change on the next scanline, and this
 change is indicated by a comma in the range of numbers. All V counter
 values are printed in hexadecimal.

 Display details:

 A NTSC machine displays 60 frames per second, each frame has 262 scanlines.
 A PAL machine displays 50 frames per second, each frame has 313 scanlines.

 The following is a list is a breakdown of the components for each type of
 display mode for PAL and NTSC machines. Most of this originally came from
 my "SMS Video" document which was based on data from the TMS9918 and a
 NTSC Sega Genesis, but now I've checked the results against a NTSC SMS 2
 fitted with a PAL/NTSC switch.

 NTSC, 256x192
 -------------

 Lines  Description

 192    Active display
 24     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 27     Top border

 V counter values
 00-DA, D5-FF

 NTSC, 256x224
 -------------

 Lines  Description

 224    Active display
 8      Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 11     Top border

 V counter values
 00-EA, E5-FF

 NTSC, 256x240
 -------------

 This mode does not work on NTSC machines. All 30 rows of the name table are
 displayed, there is no border, blanking, or retrace period, and the next
 frame starts after the 30th row. The display rolls continuously though it
 can be stabilized by adjusting the vertical hold.

 V counter values
 00-FF, 00-06

 PAL, 256x192
 ------------

 Lines  Description

 192    Active display
 48     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 54     Top border

 V counter values
 00-F2, BA-FF

 PAL, 256x224
 ------------

 Lines  Description

 224    Active display
 32     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 38     Top border

 V counter values
 00-FF, 00-02, CA-FF

 PAL, 256x240
 ------------

 Lines  Description

 240    Active display
 24     Bottom border
 3      Bottom blanking
 3      Vertical blanking
 13     Top blanking
 30     Top border

 V counter values
 00-FF, 00-0A, D2-FF

 Here are some details about what the different screen areas look like,
 useful if you are emulating overscan or if you want to have a 'virtual'
 vertical hold control in your emulator.

 Active display  - Where the display generated by the VDP goes.
 Bottom border   - Filled with border color from VDP register #7.
 Bottom blanking - Filled with a light black color. (like display was blanked)
 Vertical sync   - Filled with a pure black color. (like display was turned off)
 Top blanking    - Filled with a light black color. (like display was blanked)
 Top border      - Filled with the border color from VDP register #7.

 H counter

 Now for some additional information on the horizontal aspects of the
 display, taken from the TMS9918 manual and my tests on a NTSC Genesis.
 Be warned that a real SMS may have different results.

 The H counter is 9 bits, and reading it returns the upper 8 bits. This is
 because a scanline consists of 342 pixels, which couldn't be represented
 with an 8-bit counter. Each scanline is divided up as follows:
 
 Pixels H.Cnt   Description
  256 : 00-7F : Active display
   15 : 80-87 : Right border
    8 : 87-8B : Right blanking
   26 : 8B-ED : Horizontal sync
    2 : ED-EE : Left blanking
   14 : EE-F5 : Color burst
    8 : F5-F9 : Left blanking
   13 : F9-FF : Left border

 Here's a description of what these areas look like:

 Active display  - Where the display generated by the VDP goes.
 Right border    - Filled with border color from VDP register $07.
 Right blanking  - Filled with a light black color. (like display was blanked)
 Horizontal sync - Filled with a pure black color. (like display was turned off)
 Left blanking   - Filled with the border color from VDP register $07.
 Color burst     - Filled with a dark brown/orange color.
 Left blanking   - Filled with a light black color. (like display was blanked)
 Left border     - Filled with the border color from VDP register $07.

 Currently, I don't have information about where events occur within a
 single scanline. These events would include the following:

 - When a line interrupt is generated.
 - When a frame interrupt is generated.
 - When the V counter is incremented.

 ----------------------------------------------------------------------------
 12.) Interrupts
 ----------------------------------------------------------------------------

 The VDP can generate an interrupt relating to two conditions; when the
 vertical blanking period has started (frame interrupt), and when the line
 interrupt counter has expired (line interrupt).

 Frame interrupts

 Depending on the height of the display, the VDP will attempt to generate a
 frame interrupt on the following lines:

 Height  Line
 ------  ----
 192     $C1
 224     $E1
 240     $F1

 On the line in question, the VDP will set bit 7 of the status flags.
 This bit will remain set until the control port is read.

 Bit 5 of register $01 acts like a on/off switch for the VDP's IRQ line.
 As long as bit 7 of the status flags is set, the VDP will assert the IRQ
 line if bit 5 of register $01 is set, and it will de-assert the IRQ line
 if the same bit is cleared.

 Line interrupts

 The VDP has a counter that is loaded with the contents of register $0A on
 every line outside of the active display period excluding the line after
 the last line of the active display period. It is decremented on every
 line within the active display period including the line after the last
 line of the active display period.

 I'll admit that's a bad description, though I couldn't think of any other
 way to say it. To help you understand how this works, here is an example
 for a 192-line display on an NTSC machine that has 262 scanlines per frame:

 Out of lines 0-261:
 - The counter is decremented on lines 0-191 and 192.
 - The counter is reloaded on lines 193-261.

 When the counter underflows from $00 to $FF, it is reloaded with the last
 value written to register $0A. Writing to register $0A will not immediately
 change the contents of the counter, this only occurs when the counter is
 reloaded (meaning outside of the active display period as described above,
 or when the counter has underflows).

 When the counter underflows, the VDP sets an internal flag which I will
 call the line interrupt pending flag. This flag remains set until the
 control port is read.

 Bit 4 of register $00 acts like a on/off switch for the VDP's IRQ line.
 As long as the line interrupt pending flag is set, the VDP will assert the
 IRQ line if bit 4 of register $00 is set, and it will de-assert the IRQ line
 if the same bit is cleared.

 ----------------------------------------------------------------------------
 13.) Genesis VDP
 ----------------------------------------------------------------------------

 Here is a list of differences between the Genesis VDP and the other VDP
 types, concerning mode 4.

 - Bit 2 of register #1 enables mode 5 when set and mode 4 when clear.

 - When the TMS9918 modes are selected, the display is always blanked and
   is colored black. The M1, M2 and M3 bits have no effect.

 - When the first byte of the command word is written, it is latched and
   the LSB of the address register is not updated until the second byte
   is written.

 - When the data port is written to, the read buffer is not updated.
   It is only updated when the data port is read.

 - When writing to the data port, data will go to CRAM if bit 1 of the
   code register is set. For the other VDP types the code register must
   be set to $03 to access CRAM.

 - None of the TMS9918 modes are available, nor are the extended resolutions
   of the SMS 2 and GG VDP.

 - Sprites cannot be zoomed using bit 0 of register #1, this bit does
   nothing.

 Despite all of these differences, only a few games do not work properly
 on the Genesis. They are:

 F-15 Fighting Falcon   -  Uses a TMS9918 display mode which results in a
                           black screen on the Genesis.

 Bart vs. Space Mutants -  Minor palette problems as the game expects to
                           write to VRAM with bit 1 of the code register set.

 Ax Battler (GG)        -  Missing sprites and palette problems as the game
                           expects to write to VRAM with bit 1 of the code
                           register set. In addition the Game Gear palette
                           hardware is not compatible with the Genesis.

 Cosmic Spacehead       -  Uses a 224-line display which is not supported,
                           depends on being able to update the LSB of the
                           address register on the 1st control port write.

 Micro Machines         -  Uses a 224-line display which is not supported.

 Y's (Japanese)         -  Uses bit 0 of register #2 to cause name table
                           mirroring, a bug which is fixed in the Genesis VDP.

 ----------------------------------------------------------------------------
 14.) Game Gear VDP
 ----------------------------------------------------------------------------

 When using the TMS9918 display modes, all background and sprite graphics
 are shown using colors 16-31. No default palette is selected, so CRAM
 data has to be set up prior to using these modes. The format of the CRAM
 data written depends if the machine is in SMS or GG mode.

 This is opposed to the SMS2 which forces TMS9918 graphics to be shown
 using a hardwired palette that is independant of the colors defined in CRAM.

 The GG's LCD screen isn't big enough to display SMS graphics in their
 entirety when in SMS mode. Instead, the graphics are shrunk on the fly
 by the VDP. Normally the display shows roughly 192 lines with a 16 line
 overscan area at the top and bottom of the display. These two areas are
 not visible when using a 224 line display, which takes up the entire
 height of the screen. I don't know how the GG modifies the display to
 pack 224 lines of graphics into just 144 lines, but it seems like some
 rows are skipped.

 For each line of SMS graphics, only the middle 240 pixels are displayed.
 A single line is compressed into 160 pixels by using two RGB elements
 from each pixel of the LCD screen to represent a dot of color from the SMS
 display. This causes artefacting between pixels, for example a single bar
 of a solid white color will never be shown as white, but only as cyan,
 magenta, or yellow (as only the red+green, green+blue, or blue+red elements
 will be used to represent that pixel). The effect is also very noticable
 when the screen is scrolled horizontally.

 The GameGear I have behaves like a NTSC SMS2. The 240-line display mode
 is not supported in SMS or GG mode, and there are 262 scanlines per frame.
 It may be the case where PAL GameGear's exist, though it seems to be that
 GG machines are the same throughout the world (the only exception I can
 think of would be a developer's model that had TV output capability, this
 would need PAL support if the video wasn't RGB but was something else like
 RF or composite).

 ----------------------------------------------------------------------------
 15.) Miscellaneous
 ----------------------------------------------------------------------------

 When writing to CRAM rapidly during the active display, I've found that
 sometimes data is written to the wrong address or that the data isn't
 written altogether. This happened with an SMS 2, but the same program runs
 fine on a Genesis and Game Gear.

 I think the cause is that the VDP can only allow so many CPU accesses to
 VRAM and CRAM during a scanline, and if you write when the VDP is busy
 processing a previous write, the data is simply gone. The Genesis hardware
 has a write FIFO that takes care of this problem in mode 5, and I assume for
 mode 4 the VDP's faster clock rate allows it to give more time to the Z80.

 Bit 3 of register $00 (early clock) also affects sprites in the TMS9918
 modes, by the same amount of 8 pixels.

 In Sean Young's TMS9918 document, he describes a problem where the unused
 bits in some VDP registers that define a table address work like an AND
 mask over the high order address bits of a pattern index. This is the exact
 same problem that causes the name table mirroring as described in the
 register reference section. Note that this does not occur in the GG and
 SMS2 VDP, the unused bits of any register will not change the address the
 VDP forms, in mode 4 or any of the TMS9918 modes.

 ----------------------------------------------------------------------------
 16.) Credits and Acknowledgements
 ----------------------------------------------------------------------------

 I would like to thank the following people for their assistance, either
 by contributing information or running test programs for me:

 - Asynchronous
 - Charles Doty
 - Chris MacDonald
 - Flavio Morsoletto
 - Jon
 - Maxim
 - Marat Fayzullin
 - Mike G.
 - Omar Cornut
 - Pascal Bosquet
 - Richard Talbot-Watkins
 - Sean Young
 - Steve Snake
 - Everyone on the S8-DEV forum

 If I've missed anyone, please let me know.

 ----------------------------------------------------------------------------

 Unpublished work Copyright 2000, 2001, 2002  Charles MacDonald

