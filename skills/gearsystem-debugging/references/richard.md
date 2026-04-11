[[25/04/1999 2:15:24][]]
                  SEGA MASTER SYSTEM TECHNICAL INFORMATION
                  ========================================
                 Richard Talbot-Watkins (rich.tw@iname.com)
                               10th June 1998

Welcome to the Sega Master System technical document, a collation of all my
current knowledge of the SMS hardware and architecture.  The information
within comes from a number of sources - much of the credit is due to Marat
Fayzullin (http://www.komkon.org/fms) whose original, but sadly incomplete,
technical document was a very good starting point.  Some of this text file is
loosely based on this document which can be found at Marat's website, above.

The information represented within isn't based on any official Sega
information (if only it were...), and so I can't vouch for its correctness or
completeness - it just represents my own understanding of the SMS hardware so
far.  So far this model, as used by my emulator, Miracle, seems to emulate the
machine satisfactorily, but if anyone spots any mistakes in my documentation,
or has any useful additions to contribute, then *please* do get in touch!  Any
help will of course be appreciated.

As a note of explanation, I'll use '$' to denote hexadecimal numbers in this
document, i.e. $1000 means 1000 hex.  Occasionally, I might use '&amp;' by
accident - this also means 'hex' and is what's used on Acorn machines.


Contents
========

 1) Concise technical specifications of the SMS
 2) Memory map
    - Paging register summary
 3) I/O Ports
 4) Interrupt handling
 5) Screen display
    - VRAM Memory map
 6) Programming the VDP Ports
 7) VDP Register descriptions
    - Description of line interrupts
 8) Programming the SN76489 PSG
 9) Programming the Yamaha YM2413 FM Chip
10) And finally...


Technical specifications of the SMS
===================================

- Z80 CPU running at about 3.3MHz (53.203MHz/16).  [This value is from Marat's
  document, though I suspect the real value is closer to 4MHz]

- Custom video controller (VDP), derived from the TMS9918/9928 chip made by
  Texas Instruments, and providing:
    - 256x192 tile-based screen in 16 colours,
    - 64 8x8, 8x16 or 16x16 hardware sprites,
    - 32 colours on screen (16 for sprites, 16 for background) from a
      palette of 64,
    - Hardware up/down/left/right scrolling of all or part of screen.

- Video information held in 16k of VRAM, not in the Z80 memory map, but
  accessed via input/output ports.

- Generic SN76489 sound chip (PSG) made by Texas Instruments, providing:
    - 3 square-wave sound channels
    - 1 white noise/periodic noise channel
  It's actually the same sound chip as in the good ol' BBC Micro...

- Memory paging registers accessed through main RAM at $FFFC..$FFFF.
  Other input/output implemented via standard Z80 I/O ports.

- Yamaha YM2413 FM sound chip - this was present in some of the Japanese Mk 3
  machines and, apparently, also in *some* European machines.  It provides:
    - 9 channel synthesised sound (9 pure voices, or 6 pure + 3 percussion)
    - 15 instruments + 1 user definable
    - Hardware vibrato / amplitude modulation


Memory map
==========

    $10000 -----------------------------------------------------------
           Paging registers
     $FFFC -----------------------------------------------------------
           Mirror of RAM at $C000-$DFFF
     $E000 -----------------------------------------------------------
           8k of on-board RAM (mirrored at $E000-$FFFF)
     $C000 -----------------------------------------------------------
           16k ROM Page 2, or one of two pages of Cartridge RAM
     $8000 -----------------------------------------------------------
           16k ROM Page 1
     $4000 -----------------------------------------------------------
           15k ROM Page 0
     $0400 -----------------------------------------------------------
           First 1k of ROM Bank 0, never paged out with rest of Page 0
     $0000 -----------------------------------------------------------

SMS ROMs are either 32k, 64k, 128k, 256k or 512k in size.  The small 32k ROMs
do not require the paging hardware and simply map from $0000..$8000.  The
larger ROMs by default map their first 48k from $0000..$C000, but each 16k
page can be remapped to different internal ROM banks via the paging registers.

The paging registers at $FFFC-$FFFF are read/write, and allow different ROM
banks to be paged into Pages 0-2, as well as, optionally, RAM into Page 2.
Here is an overview of their use:

  $FFFC: RAM select register.
           bit 7: ??
               6: ??
               5: ??
               4: ??
               3: 0 = Page 2 mapped as ROM as from register $FFFF
                  1 = Page 2 mapped as on-board cartridge RAM
               2: 0 = Use first page of cartridge RAM
                  1 = Use second page of cartridge RAM
               1: ??
               0: ??

  $FFFD: Page 0 ROM bank.  Bank X lies at an offset of $4000*X within the
         cartridge ROM.
  $FFFE: Page 1 ROM bank.
  $FFFF: Page 2 ROM bank.  If bit 3 of $FFFC is set, Cartridge RAM will be
         mapped here instead.

Note that the first 1k of the memory map can never be paged out - this is
because the Z80 entry points for reset (at $0000), IRQs (at $0038) and NMIs
(at $0066) should always have valid handlers, and these should be guaranteed
to be in place on power-up.


I/O ports - Overview
====================

PORT $DC - Joypad port 1 (read only)
------------------------------------
(This port is also mirrored at $C0, as used by some games).
This is the first port providing input from the two joypads.  Each bit
corresponds to a button: 0 for pressed, 1 for released (note that this appears
to be in contradiction to Marat's documentation):

  bit 7: Joypad 2 Down
      6: Joypad 2 Up
      5: Joypad 1 Fire B
      4: Joypad 1 Fire A
      3: Joypad 1 Right
      2: Joypad 1 Left
      1: Joypad 1 Down
      0: Joypad 1 Up

PORT $DD - Joypad port 2 (read only)
------------------------------------
(This port is also mirrored at $C1, as used by some games).
This is the second port, which operates as above, its bits meaning:

  bit 7: Lightgun 2 + Nationalisation bit 2
      6: Lightgun 1 + Nationalisation bit 1
      5: unused
      4: Reset button
      3: Joypad 2 Fire B
      2: Joypad 2 Fire A
      1: Joypad 2 Right
      0: Joypad 2 Left

Bits 6 and 7 of this port, in conjunction with port $3F, are used to determine
the country of origin of the machine.

PORTS $DE/$DF
-------------
Some games appear to read and write from/to these ports, for reasons which are
as yet unknown.

PORT $3F - Automatic nationalisation (write only)
-------------------------------------------------
This port may have a number of functions, but the only one I am aware of is
that of determining the country of origin of the machine.  Even these details
are slightly sketchy!  The port is written to as follows:

  bit 7: Output to nationalisation bit 2
      6: ??
      5: Output to nationalisation bit 1
      4: ??
      3: 0
      2: 0 = reset nationalisation bits,
         1 = determine nationalisation
      1: 0
      0: 0 = reset nationalisation bits,
         1 = determine nationalisation

The value actually written to the nationalisation bits depends on the country
of origin.  British machines directly copy the values in bits 5 and 7 of port
$3F to the nationalisation bits.  Japanese machines appear to complement one
or more of these bits, although these details are uncertain.

Virtually every game which needs to determine nationalisation outputs $F5 to
this port and then reads bits 6 and 7 of port $DD.  Different machines will
return their own unique value in these bits.  The nationalisation bits of port
$DD are then restored to their function as lightgun bits by writing $F0 to
port $3F.

PORT $7F - SN76489 PSG output / Current scanline input (read/write)
-------------------------------------------------------------------
(This port is also mirrored at $7E).
When written, this port serves as the output to Texas Instruments SN76489
sound chip.  The programming format is described in a separate section, below.

When read, it returns the number of the line of the screen currently being
drawn by the video hardware, not including the top or bottom borders.  Hence
0..191 are returned for lines of the actual SMS screen.  Any other values
returned are not necessarily defined.

PORT $BE - VDP data (read/write)
--------------------------------
This is a VDP chip port used to read and write data from/to VRAM and the
palette registers.  Its operation is described in detail in the VDP section
below.

PORT $BF - VDP address / status register (read/write)
-----------------------------------------------------
(This port is also mirrored at $BD, as used by some games)
This port performs a dual role.  When read, is returns the VDP status bits as
follows:

  bit 7: VSync flag, set at the beginning of each VSync impulse
      6: Line interruot flag: set when a line interrupt is generated
         (see the VDP section for details on line interrupts)
      5: Sprite collision flag: set when two sprites overlap
      4: ?
      3: ?
      2: ?
      1: ?
      0: ?

Bits 6 and 7 are reset to 0 when this port is read.

When written, this port is used to present VRAM addresses or palette register
numbers to the VDP (this requires two writes to this port).  It is also used
to write to VDP registers.  See the VDP section for more details.

PORT $F0 - YM2413 address register (write only)
-----------------------------------------------
This port is used to specify which YM2413 register is to be written.  See the
FM sound section for more details.

PORT $F1 - YM2413 data register (write only)
--------------------------------------------
This port is used to write data to the currently selected YM2413 register.
See the FM sound section for more details.

PORT $F2 - YM2413 control register (read/write)
-----------------------------------------------
Very little known about this at present, except that it is somehow used by
many games to detect the presence of the YM2413 chip.


Interrupt handling
==================

The SMS hardware generates two types of interrupts: IRQs and NMIs.  An IRQ is
a maskable interrupt which may be generated by:
- the VSync impulse which occurs when a frame has been rasterised, or:
- a scanline counter falling below zero (see the VDP Register 10 description
  for details)
The SMS interrupt generating hardware does not place a value on the data bus
when it activates the IRQ line, which means that the Z80 interrupt modes 0 and
2 are not of any use.  Therefore, the SMS is always used in interrupt mode 1,
which causes a jump to location $0038 when an interrupt is generated.

The other type of interrupt, NMI, is generated when the Pause button is
pressed.  An NMI causes an unconditional processor jump to address $0066.  The
routine should be short and quick, probably just setting a flag in RAM to be
tested in the game main loop, and exiting via a RETN instruction as normal.


Screen display
==============

The video controller used in the SMS is based on a Texas Instruments chip, the
TMS9918/9928, used in machines such as the MSX.  It provides a single
tile-based screen mode, the internal size of which is 32x28 tiles, whilst the
displayed size is variable, although usually 32x24 or 31x24.  Each tile is
made up from 8x8 pixels in 16 colours, and can be used as either a "building
block" of the screen, or as a sprite.

Also provided are 64 hardware sprites, each of which may be defined by a
single 8x8 tile, or by 2 8x8 tiles, one above the other, and a programmable
palette for graphics and sprites, 16 colours out of 64 for each.  All video
information is held in 16k of VRAM, which is not mapped into Z80 address
space, but instead accessed via input/output ports provided by the VDP.

A typical memory map for the VRAM is as follows (noting that the screen
display and sprite information tables can be moved to any desired point):

   $4000 ---------------------------------------------------------------
         Sprite info table: contains x,y and tile number for each sprite
   $3F00 ---------------------------------------------------------------
         Screen display: 32x28 table of tile numbers/attributes
   $3800 ---------------------------------------------------------------
         Sprite/tile patterns, 256..447
   $2000 ---------------------------------------------------------------
         Sprite/tile patterns, 0..255
   $0000 ---------------------------------------------------------------

The sprite info table is a 256 byte table which stores the (x,y) coordinates
of the top-left corner of each sprite, and the tile number which is to be
used as the sprite.  For a given sprite n (where n=0..63), the entries in the
table are at the following offsets:

    +n       : sprite y position (for -16&lt;y&lt;0, store 256+y here)
    +64+n    : tantalisingly unknown!!  (perhaps sprite attributes of some
               sort?)
    +128+n*2 : sprite x position
    +129+n*2 : sprite tile number (0..255)

The order of sprite printing by the hardware is such that sprites with higher
numbers overprint those with lower numbers.  It appears that, for each
scanline, the hardware steps through the sprite y position table, checking for
sprites lying within the current scanline; when one is found, the (x, number)
byte pair is fetched and the appropriate sprite row displayed.  This process
is terminated when either:
  - the end of the sprite table is reached
  - an entry with a y position of 208 is encountered
  - eight sprites have been displayed on the current scanline

The screen display (or name table) is a 32x28 table, each entry comprising a
16-bit value which define the contents of each tile position on the screen.
The bits have the following meaning:

  bit 15: unused
      14: unused
      13: unused
      12: when set, tile is displayed in front of sprites
      11: when set, display tile using sprite palette rather than tile palette
      10: when set, flip tile vertically
       9: when set, flip tile horizontally
     0-8: tile definition number to use (0..511)

Alternatively, bits 0-7 can be thought of as the first byte of the pair, and
bits 8-15 as the second byte.

The tile number is effectively multiplied by 32 by the video hardware to
provide the VRAM address of the tile definition to be displayed.  Note that,
since the name table and sprite info table occupy the top section of VRAM by
the default memory mapping, it is not possible to use tile numbers above 447.
Any attempt to plot these will result in a graphical mess defined by the
current contents of the name/sprite tables!

Tile definitions can be viewed as consisting of 8 32-bit little endian words,
one for each row of the tile definition.  Then, the colour (a 4-bit value) of
each pixel in a tile row is defined by the following bits of the word:

  Leftmost pixel 0: bits 31 (msb of colour), 23, 15, 7 (lsb of colour)
                 1: bits 30, 22, 14, 6
                       .   .   .
                       .   .   .
 Rightmost pixel 7: bits 24, 16, 8, 0

e.g. in order to set the leftmost pixel of a tile row to colour 8, the
rightmost to colour 15, and leave all remaining pixels in colour 0, you would
write the following bytes to VRAM:

  bit 7      0     15      8      23      16     31      24
      00000001      00000001       00000001       10000001


Programming the VDP Ports
=========================

Recall that ports $BE and $BF provide the interface between VRAM and the
palette registers, and the processor.  In simple terms, port $BF is used to
set a pointer to an address in VRAM or palette RAM, the value at which can
then be read from or written to via port $BE.  Port $BF also has the secondary
functions of providing a means of writing to the internal write-only VDP
registers, and also returning a VDP status byte when read.

In order to read or write from/to VRAM, a VRAM address (a 14-bit value) needs
to be specified - this is achieved by two writes to port $BF:

   bit  7 6 5 4 3 2 1 0          bit  7 6 5 4 3 2 1 0
        b b b b b b b b               0 1 a a a a a a

where aaaaaabbbbbbbb is a 14-bit VRAM address.  Reading from or writing to
port $BE will then access the VRAM address just specified, and auto-increment
the latched address.  This is very useful, given the overheads of specifying
the address in the above way.

Note that the old latched address is still valid after only one write to port
$BF; the whole 14-bit address is only transferred to the internal latches
after the second write.  Any access to port $BE after the first write will use
the already stored address, and also "throw away" the new low byte just
written to $BF.  I believe that reading from port $BF after the first write
has the same effect, though feel free to correct me if I'm wrong!

The SMS has 32 bytes of RAM which hold the palette information.  Palette RAM
addresses 0..15 hold the physical colour of tile colours 0..15, and addresses
16..31 hold the physical colour of sprite colours 0..15.  To write to palette
RAM, first its address should be set up by writing two bytes to port $BF:

   bit  7 6 5 4 3 2 1 0          bit  7 6 5 4 3 2 1 0
        0 0 0 a a a a a               1 1 0 0 0 0 0 0

where aaaaa is the 5-bit address of palette RAM.  Then a byte in the following
format should be written to port $BE:

   bit  7 6 5 4 3 2 1 0
        0 0 b b g g r r

where rr, gg and bb are two bit values specifying a red, green and blue
intensity between 0 and 3, hence giving a 64 colour palette.  It is also
possible to read the contents of palette RAM by reading from port $BE.  After
writing or reading palette RAM, the internal palette RAM address pointer is
auto-incremented.

I mentioned earlier that port $BF is also used to write to the internal VDP
registers.  This is done with the usual double write, this time in the
following way:

   bit  7 6 5 4 3 2 1 0          bit  7 6 5 4 3 2 1 0
        d d d d d d d d               1 0 0 0 r r r r

This writes the 8-bit value dddddddd to the 4-bit VDP register number rrrr.
The VDP registers are described in detail in the next section.

The only remaining form of write to $BF is:

   bit  7 6 5 4 3 2 1 0          bit  7 6 5 4 3 2 1 0
        b b b b b b b b               0 0 a a a a a a

This appears to set the VRAM address pointer, as if bit 6 of the second write
were set.  I do not know if this operation is defined, or if its results are
even guaranteed.


VDP Registers
=============

The VDP has 16 internal registers, all of them are write-only.  Here is a
description of each of them, to my best knowledge:

VDP Register 0
--------------
This register contains miscellaneous flags which affect aspects of the screen
display.  Its bits, when set, have the following meaning:

  bit 7: Right hand 8 columns of screen not affected by vertical scrolling
      6: Top 2 rows of screen not affected by horizontal scrolling
      5: Do not display leftmost column of the screen
      4: Enable line interrupt IRQ's (see VDP Register 10)
      3: Shift all sprites left by 1 character
      2: ?
      1: Enable stretch screen (33 columns wide)
      0: Screen sync off

Bits 6 and 7 are widely used by games to provide stationary panels at the top
or right of the screen whilst the remainder of the screen is scrolled (using
VDP registers 8 and 9).  Bit 5 is frequently set to provide a 31 column screen
- this is so that horizontally scrolling games have a hidden part of the name
table which can be updated at any point, rather than having to wait for
flyback.  Bit 4 enables line interrupts, which are described later on.

VDP Register 1
--------------
More miscellaneous flags - its bits, when set, mean:

  bit 7: ?
      6: Enable display; no picture is shown when this bit is 0
      5: Enable VSync interrupt generation (IRQ)
      4: Extend screen by 4 rows (i.e. to 28 rows)
      3: Extend screen by 6 rows (i.e. to 30 rows)
      2: ?
      1: 8x16 sprites; when this bit is 0, the default 8x8 sprites are used
      0: Zoomed sprites; all pixels in all sprites are doubled in size

Bit 1 is very useful - each of the 64 sprites has its tile number rounded down
to an even number, and then is automatically plotted with the subsequent tile
number attached to it 8 pixels below.  Bit 5, when set, allows the VSync
signal (50Hz on PAL, 60Hz on NTSC) to generate IRQs (see section on interrupt
handling).  Most games use this to provide their most basic timing.  I have
not seen the zoomed sprites or the extend screen row options used in any SMS
game, although I may be wrong.

VDP Register 2
--------------
This register is used to set the base address of the name table in VRAM.
Recall that the name table is a $700 byte long table containing 32x28 16-bit
entries, one for each character position on the display.  It must be placed at
a $800 byte offset in VRAM, i.e. $0000, $0800, $1000, $1800 etc.  This shows
how the VDP register 2 input is mapped to a VRAM address:

   VDP     bit  7  6  5  4  3  2  1  0
  Reg 2:        x  x  x  x  d  d  d  x        (x = bit ignored)
                            :  :  :
                VRAM       13 12 11 10  9  8  7  6  5  4  3  2  1  0
               Address      d  d  d  0  0  0  0  0  0  0  0  0  0  0

At power-up, the three active lines of VDP(2), D1, D2 and D3, will be high.
Clearly this leads to the default name table address of $3800.

VDP Registers 3 & 4
-------------------
It is likely that these registers are relics of the original VDP chip from
which the SMS VDP chip is derived.  They do not appear to be used on the SMS.

VDP Register 5
--------------
This register is used to set the base address of the sprite information table
in VRAM.  Recall that this table is 256 bytes long and contains information on
the positions and tile numbers of the hardware sprites.  It must be placed on
a 256-byte boundary, and this is forced by the way in which VDP(5) is mapped
to the VRAM address bus, as shown:

   VDP     bit  7  6  5  4  3  2  1  0
  Reg 5:        x  d  d  d  d  d  d  x        (x = bit ignored)
                   :  :  :  :  :  :
        VRAM      13 12 11 10  9  8  7  6  5  4  3  2  1  0
       Address     d  d  d  d  d  d  0  0  0  0  0  0  0  0

On power-up, when D1-D6 of VDP(5) are high, this will result in a default
sprite table base of $3F00.

VDP Register 6
--------------
This register sets the base address of tile definitions used for sprites,
either $0000 or $2000.  This register is necessary because only an 8-bit value
is used to store the tile number of each sprite in the sprite information
table.  Hence, this register effectively provides a global ninth bit for the
tile number of ALL sprites.  The format of this register is:

   bit  7 6 5 4 3 2 1 0
        x x x x x d x x

where if d=0, all sprites use tiles defined from $0000 (i.e. tiles 0..255)
   or if d=1, all sprites use tiles defined from $2000 (i.e. tiles 256..511).
x represents unused bits.

I can't imagine why this register is mapped to bit 2 of the data bus, but
there you go.

VDP Register 7
--------------
This 4-bit register takes a value from 0 to 15, which is used as the colour
number from the sprite palette to use for the border.

VDP Register 8
--------------
This register controls the horizontal scroll offset.  Any 8-bit value may be
written and the screen display will be affected immediately - this means that
it can be rewritten during screen refresh to get raster splits.  The value
written is used as the pixel offset of column 0 from the left hand edge of the
screen.

VDP Register 9
--------------
This register controls the vertical scroll offset, and may contain values from
0 to 223.  I don't know what a real SMS would do with a value greater than
223!  This value is used as the internal row number which is displayed on the
first screen scanline.

VDP Register 10
---------------
The VDP is able to generate two kinds of interrupts: VSync, which we have
already met, and Line Interrupts.  There is an internal counter which is
initialised to the contents of VDP(10) before the video hardware starts to
rasterise the first line, which is then decremented each time another scanline
is rasterised.  A line interrupt is generated, if enabled, when this counter
falls below zero, and the counter is then re-initialised with the contents of
VDP(10).  Hence this register determines the number of scanlines to be
rasterised between subsequent line interrupts; a value of 0 means that line
interrupts are generated every scanline, a value greater than 192 (probably)
means that line interrupts will not be generated.

Suppose we wish to generate a line interrupt at line 32 of each frame (perhaps
        to split scrolling at this point).  This is how to go about it:

   - Initially, set VDP(10)=255, so no line interrupt on screen this frame
   - Ensure VSync and Line Interrupts are enabled
   - Wait for VSync interrupt
** VSync interrupt generated at end of frame refresh
   - In VSync interrupt, set VDP(10)=16.  Set a flag to zero.
** When rasterisation of the new frame begins, the hardware will set its
** scanline counter to VDP(10), i.e. 16
   - Wait for Line Interrupt.  This is guaranteed to be at line 16.
** Line interrupt generated at line 16.  Scanline counter immediately
** reset to VDP(10), i.e. 16
   - In line interrupt handler, first increment flag.  If flag=1 (it does!)
     then we're not interested in this interrupt, but we set VDP(10) to the
     value we want the hardware to latch next time, i.e. on our interrupt at
     line 32.  In this case, we set VDP(10)=255 as we don't want any other
     line interrupts on this frame.
** Line interrupt generated at line 32.  Scanline counter immediately reset
** to VDP(10), i.e. 255
   - Line interrupt: increment flag again.  As it equals 2, we know this is
     the interrupt we've been waiting for, and execute the relevant code for
     line 32, e.g. setting scroll offsets.
** Next IRQ generated is a VSync - back to the beginning again

The important point here is that there are usually twice as many line
interrupts generated as are required, due to the automatic re-loading of the
scanline counter upon line interrupt generation.

VDP Registers 11-15
-------------------
These are unused on the SMS to my knowledge.


Programming the SN76489 PSG sound chip
======================================

The SN76489 is a simple sound chip providing three square wave tone channels
and one noise channel.  These sounds are mixed on-chip.  Sixteen volume levels
are available per channel, and the tone channels can produce 1024 different
frequencies, ranging from 122Hz to 125kHz.  The noise channel is able to
produce (impure) white noise, or 'periodic' noise, both of which can be tuned
to three preset frequencies, or to match the frequency of the third tone
channel.

The three tone channels will be referred to as channels 0..2.  The noise
channel will be referred to as channel 3.

To set the volume of a channel, the SN76489 is programmed thus:

    bit  7 6 5 4 3 2 1 0
         1 r r 1 d d d d

where rr = 00: set channel 0 volume
           01: set channel 1 volume
           10: set channel 2 volume
           11: set channel 3 volume

and dddd is a 4-bit value representing the desired volume.  0000 is loudest
and 1111 is quietest (off).

Setting the frequency of a tone channel usually requires two writes and is
done in the following way:

    bit  7 6 5 4 3 2 1 0
         1 r r 0 f f f f

    bit  7 6 5 4 3 2 1 0
         0 x h h h h h h

where rr = 00: set channel 0 frequency
      rr = 01: set channel 1 frequency
      rr = 02: set channel 2 frequency

and hhhhhhffff is a 10-bit value representing the desired frequency, ffff
being the least significant 4 bits and hhhhhh being the most significant 6
bits.  x is an unused bit.

The actual frequency of the tone produced, in Hertz, is 125000/(10 bit value).
In the case where hhhhhhffff=0000000000, a 125000Hz tone is produced.

The second byte may be continually updated, without rewriting the first.

The noise channel, channel 3, is programmed in the following way:

    bit  7 6 5 4 3 2 1 0
         1 1 1 0 x f n n

where nn determines the frequency of noise produced:
  nn = 00: high pitch noise (clock/16 = 7800Hz)
       01: medium pitch noise (clock/32 = 3900Hz)
       10: low pitch noise (clock/64 = 1950Hz)
       11: frequency is taken from that of channel 2.

and f is the 'feedback' bit:
  f = 0: produce periodic (synchronous) noise [this sounds like a tuned tone,
         but reaching much lower pitches than the square wave generators
         and with a very different timbre]
      1: produce white noise.

x is an unused bit.


Programming the Yamaha YM2413 FM Sound chip
===========================================

The YM2413 is a chip capable of producing FM synthesized sound which was
present in the Japanese Mark 3 machines, and also, it would appear, some
European Master Systems.  It supports 9 channels, each of which may play any
of 15 pre-defined 'instruments', or a user-defined sound.  In addition, 3
channels can be set to 'rhythm mode' which allows them to reproduce percussion
sounds.

The basic idea surrounding FM synthesis is that there are at least two signal
generators per channel, each oscillating at a programmable frequency.  One of
these is the higher frequency carrier wave, and the other is a modulator wave.
The modulator wave 'modulates' the frequency of the carrier wave by a
'modulation index', providing many different timbres of sound.  If this all
sounds very complicated, well.. yes, it is - although presumably it is much
easier to produce from analogue components!  In simple terms, the waveform
produced by FM synthesis can be described by the following function:

   f(t) = A * sin (2 * pi * Fc * t + Mi * sin (2 * pi * Fm * t))

where: A is the overall amplitude of the waveform at time t
       Fc is the carrier frequency
       Fm is the base modulator frequency
       Mi is the maximum sinusoidal displacement of the modulating frequency
          (modulation index)
       pi is, well, pi (3.141....)

In terms which can be related to, A is effectively the volume of the waveform.
Fm is the pitch of the tone to be produced (provided that Mi is non-zero,
which it always should be - otherwise all that is heard is the carrier tone).
Fc and Mi are values which change the quality of the sound.  If Fc/Fm is an
integer, then pure tones are produced which contain the (Fc/Fm)th harmonic of
the fundamental; a non-integral value results in more percussive sounds -
bells, cymbals, gongs, drums etc.  Of course, A can be changed as the waveform
progresses - this is caled an amplitude envelope, and can be programmed by the
YM2413.  Other effects can be achieved by changing Mi with time, although I
don't know if the YM2413 supports this.

Anyway, that's the theory.  The YM2413 actually provides mostly preset
instruments, i.e. well-chosen values for Fc/Fm and Mi.  As I've not even
attempted to try emulating the chip yet, I haven't a clue what these values
are - I'm not helped by the fact that all my good documentation is in Dutch,
so I have to try and interpret such words as "vermenigvuldigingsfactor", not
being able to speak a word of Dutch (what I'm really hinting of course, is
that I'd welcome any Dutch speakers getting in touch if they think they could
help with a little interpretation!)

So details are a little sketchy at the moment, but here's a summary of the
YM2413 registers and their function.  It's probably not enough to be able to
emulate it particularly well, but it's a start....

On the SMS, the YM2413 is programmed by first outputting the desired register
number to port $F0.  The data to be written is then outputted to port $F1.

Registers $00-$07 are used for describing the parameters of the user
instrument, instrument 0.

Register $00 (Modulator) / $01 (Carrier)
----------------------------------------
  bit 7: amplitude modulation on/off
      6: frequency vibration on/off
      5: 0=sustaining envelope pattern, 1=decaying envelope pattern
      4: key scale rate
    0-3: multi sample wave selection
         (maybe declare ratio between carrier and modulator frequency)

Register $02 (Modulator)
------------------------
bit 6-7: key scale level
    0-5: total level (modulation control)

Register $03 (Carrier)
----------------------
bit 6-7: key scale level
      5: unused
      4: rectification to output of modulator wave on/off (distorted waveform)
      3: rectification to output of carrier wave on/off (distorted waveform)
    0-2: FM feedback constant

Register $04 (Modulator) / $05 (Carrier)
----------------------------------------
bit 4-7: attack rate
    0-3: decay rate

Register $06 (Modulator) / $07 (Carrier)
----------------------------------------
bit 4-7: sustain level
    0-3: release rate

Registers $08-$0D unused

Register $0E
------------
bit 6-7: unused
      5: select rhythm mode
      4: sound bass drum
      3: sound snare drum
      2: sound tom-tom
      1: sound cymbal
      0: sound hi-hat

Register $0F
------------
bit 4-7: unused
      3: 1=sound output enabled
    1-2: unused
      0: 1=sample mode on?

Registers $10..$18 (channel 0..channel 8)
-----------------------------------------
These registers contain the bottom 8 bits of the f number, which determines
the frequency of the channel.  The frequency is calculated by:

  frequency = 50000 * Fnumber * 2^(Onumber-19)

Registers $19-$1F unused.

Registers $20..$28 (channel 0..channel 8)
-----------------------------------------
bit 6-7: unused
      5: sustain on
      4: key on (I think bits 4/5 are to do with a newly sounded instrument
         interrupting the last one on that channel, but my Dutch translation
         isn't up to scratch!)
    1-3: o number (octave, for frequency calculation, above)
      0: 9th bit of f number

Registers $29-$2F unused.

Registers $30..$38 (channel 0..channel 8)
-----------------------------------------
bit 4-7: instrument number
bit 0-3: volume

The instrument numbers are:

  0 User          4 Flute         8 Organ        12 Vibraphone
  1 Violin        5 Clarinet      9 Tube         13 Synth bass
  2 Guitar        6 Oboe         10 Synthesizer  14 Wood bass
  3 Piano         7 Trumpet      11 Harpsichord  15 Electric bass

I do not know whether 0 represents most loud or most quiet volume.

If Rhythm mode is on, then registers $36-$38 take on different functions:

Register $36 in rhythm mode
---------------------------
bit 4-7: unused
    0-3: bass drum volume

Register $37 in rhythm mode
---------------------------
bit 4-7: hi-hat volume
    0-3: snare drum volume

Register $38 in rhythm mode
---------------------------
bit 4-7: tom-tom volume
    0-3: top cymbal volume

That's as good as I've got so far.  If anyone can explain anything in better
(any?) detail, then much appreciated!  The original Dutch documents are at:
http://ftp.castel.nl/msx/docs/fm-pac.txt and fm-pac2.txt.


And finally...
==============

Well that's it!  If anyone has any corrections or additions to make, then
please get in touch so the document can be updated.  If anyone thinks I've
documented any of this in a crap way that's unintelligible, then please get in
touch so it can be amended.  Anyway, hope someone finds it useful.

- Rich Talbot-Watkins (rich.tw@iname.com)
  10/6/98

