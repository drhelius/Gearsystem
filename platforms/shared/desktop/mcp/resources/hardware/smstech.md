
 SMS/GG hardware notes
 by Charles MacDonald
 WWW: http://cgfm2.emuviews.com

 Unpublished work Copyright 2000, 2001, 2002  Charles MacDonald

 This document describes the hardware of several Sega systems that are SMS
 compatible, including the SMS, SMS 2, GameGear, and Genesis.

 Table of Contents

 1.) Overview
 2.) Z80 memory map
 3.) Z80 I/O ports
 4.) I/O port registers
 5.) Interrupts
 6.) BIOS information
 7.) Sound hardware
 8.) Credits and Acknowledgements
 9.) Disclaimer

 What's new:

 (11/12/02)
 - Added description of SMS interrupt behavior.
 - Added description of bit 5 of port $DD for the SMS.
 - Added description of SMS Z80 port map.
 - Fixed reset button entries in the features table. (Thanks Bock)
 - Added mirrored addresses for the 3D glasses. (Thanks Bock)
 - Added description of SMS to memory control register.
 - Fixed description of ports $DE/$DF. (Thanks Richard)

 ----------------------------------------------------------------------------
 1.) Overview
 ----------------------------------------------------------------------------

 The main differences between various Sega consoles are what slots are
 available (cartridge, card, expansion, BIOS), if there is a YM2413 sound
 chip, and if there is a reset or pause button.

 Console        Cart    Card    Exp.    BIOS    YM2413  Reset   Pause

 Mark III       y       y       y       y       *1      n       y
 Japanese SMS   y       y       y       y       y       n       y
 SMS            y       y       y       y       n       y       y
 SMS 2          y       n       n       y       n       n       y
 Genesis        y       n       n       n       n       n       n
 Genesis+PBC    y       y       n       n       n       n       y
 GameGear       y       n       n       *2      n       n       y

 1.) The YM2413 is an optional add-on for the Mark III which uses the
     expansion port.

 2.) Some Sega Game Gears do not have a BIOS, others are reported to.
     All Majesco Game Gears have a BIOS.

 The Genesis is mostly compatible with the Sega 8-bit systems, but some
 additional hardware is needed to interface the cartridge connector with
 a standard SMS cartridge or card game.

 Also, while the Genesis has a expansion port, reset button, and some models
 have a BIOS ROM, they are used for different purposes and are not related
 to the expansion port, reset button, and BIOS of other Sega 8-bit systems.

 ----------------------------------------------------------------------------
 2.) Z80 memory map
 ----------------------------------------------------------------------------

 The Z80's address space is shared by several components.
 It has the following layout:

 $0000-$BFFF : Slot area
 $C000-$FFFF : Work RAM (8K, mirrored at $E000-$FFFF)

 The slot area can be assigned to the expansion connector, cartridge, card,
 or BIOS ROM. Any of these slots may have extra hardware such as a memory
 mapping chip to provide bank switching.

 3D glasses

 The 3D glasses adapter plugs into the card slot of a SMS or Genesis+PBC.
 It has a single register at address $FFF8 with mirrors at $FFF9,$FFFA, and
 $FFFB. Bit 0 toggles which shutter on the LCD glasses is closed, leaving
 the other one open.

 From looking at several 3D games, it seems bit 0 can be read as well
 as written, and that the card slot does not have to be enabled for this
 register to be accessible.

 Register mirroring

 The work RAM is only 8K, and is mirrored at $E000-$FFFF. The Sega memory
 mapping hardware and 3D glasses have write-only registers at $FFF8 and
 $FFFC-$FFFF, which cause data written to be sent to RAM as well.
 Consequently, RAM addresses $1FF8 and $1FFC-1FFF hold the values written,
 and this data can be read back from both the regular and mirrored addresses
 at $DFF8/$FFF8 and $DFFC-$DFFF/$FFFC-$FFFF.

 Codemasters mapper

 All SMS and GG games by Codemasters have a fixed 32K ROM area at $0000-$7FFF
 and a 16K bank at $8000-$BFFF. The bank can be selected by writing a byte
 to address $8000.

 I'll also point out that the Codemasters SMS games are only compatible
 for the most part with PAL SMS 2 machines, as they use or rely on features
 specific to the video hardware of those consoles.

 ----------------------------------------------------------------------------
 3.) Z80 I/O ports
 ----------------------------------------------------------------------------

 The Z80 has a 16-bit address bus that can be used to access 64K 8-bit
 ports. In the SMS, only the lower 8 bits of the address bus are used and
 the upper 8 bits are ignored.

 While this doesn't cover all of the exceptions, a general way of looking
 at the port layout is that A7, A6, and A0 are used to decode the addresses.
 This makes for eight unique addresses out of the entire 256 port range.

 The following addresses are most often used in commercial software:

 $3E : Memory control
 $3F : I/O port control
 $7E : V counter / PSG
 $7F : H counter / PSG
 $BE : VDP data
 $BF : VDP control
 $DC : I/O port A/B
 $DD : I/O port B/misc.

 And some mirrors are used too:

 $BD : VDP control
 $C0 : I/O port A/B
 $C1 : I/O port B/misc.

 Some software will write data to ports $DE and $DF. The SG-1000 and SC-3000
 had an 8255 PPI which used these ports to control a keyboard, but later
 consoles got rid of it. There is a keyboard peripheral for the Mark III,
 which perhaps some games try to detect and use.

 SMS port map:

 $00-$3F : Writes to even addresses go to memory control register.
           Writes to odd addresses go to I/O control register.
           Reads return the last byte of the instruction which read the port.
 $40-$7F : Writes to any address go to the SN76489 PSG.
           Reads from even addresses return the V counter.
           Reads from odd address return the H counter.
 $80-$BF : Writes to even addresses go to the VDP data port.
           Writes to odd addresses go to the VDP control port.
           Reads from even addresses return the VDP data port contents.
           Reads from odd address return the VDP status flags.
 $C0-$FF : Writes have no effect.
           Reads from even addresses return the I/O port A/B register.
           Reads from odd address return the I/O port B/misc. register.

 SMS 2 port map:

 $00-$3F : Writes to even addresses go to memory control register.
           Writes to odd addresses go to I/O control register.
           Reads return $FF.
 $40-$7F : Writes to any address go to the SN76489 PSG.
           Reads from even addresses return the V counter.
           Reads from odd address return the H counter.
 $80-$BF : Writes to even addresses go to the VDP data port.
           Writes to odd addresses go to the VDP control port.
           Reads from even addresses return the VDP data port contents.
           Reads from odd address return the VDP status flags.
 $C0-$FF : Writes have no effect.
           Reads from even addresses return the I/O port A/B register.
           Reads from odd address return the I/O port B/misc. register.

 Game Gear port map: (GG mode)

 $00-$06 : GG specific registers. Initial state is 'C0 7F FF 00 FF 00 FF'.
 $07-$3F : Writes to even addresses go to memory control register.
           Writes to odd addresses go to I/O control register.
           Reads return $FF.
 $40-$7F : Writes to any address go to the SN76489 PSG.
           Reads from even addresses return the V counter.
           Reads from odd address return the H counter.
 $80-$BF : Writes to even addresses go to the VDP data port.
           Writes to odd addresses go to the VDP control port.
           Reads from even addresses return the VDP data port contents.
           Reads from odd address return the VDP status flags.
 $C0-$FF : Writes have no effect.
           Reads from $C0 and $DC return the I/O port A/B register.
           Reads from $C1 and $DD return the I/O port B/misc. register.
           The remaining locations return $FF.

 Game Gear port map: (MS mode)

 $00-$3F : Writes to even addresses go to memory control register.
           Writes to odd addresses go to I/O control register.
           Reads return $FF.
 $40-$7F : Writes to any address go to the SN76489 PSG.
           Reads from even addresses return the V counter.
           Reads from odd address return the H counter.
 $80-$BF : Writes to even addresses go to the VDP data port.
           Writes to odd addresses go to the VDP control port.
           Reads from even addresses return the VDP data port contents.
           Reads from odd address return the VDP status flags.
 $C0-$FF : Writes have no effect.
           Reads from $C0 and $DC return the I/O port A/B register.
           Reads from $C1 and $DD return the I/O port B/misc. register.
           The remaining locations return $FF.

 Genesis port map:

 $00-$3F : Writes to even addresses go to memory control register.
           Writes to odd addresses go to I/O control register.
           Reads return $00 or random data.
 $40-$7F : Writes to any address go to the SN76489 PSG.
           Reads from even addresses return the V counter.
           Reads from odd address return the H counter.
 $80-$BF : Writes to even addresses go to the VDP data port.
           Writes to odd addresses go to the VDP control port.
           Reads from even addresses return the VDP data port contents.
           Reads from odd address return the VDP status flags.
 $C0-$FF : Writes have no effect.
           Reads from $C0 and $DC return the I/O port A/B register.
           Reads from $C1 and $DD return the I/O port B/misc. register.
           The remaining locations return $00 or random data.

 Genesis port map: (with PBC)

 $00-$3F : Writes to even addresses go to memory control register.
           Writes to odd addresses go to I/O control register.
           Reads return $FF.
 $40-$7F : Writes to any address go to the SN76489 PSG.
           Reads from even addresses return the V counter.
           Reads from odd address return the H counter.
 $80-$BF : Writes to even addresses go to the VDP data port.
           Writes to odd addresses go to the VDP control port.
           Reads from even addresses return the VDP data port contents.
           Reads from odd address return the VDP status flags.
 $C0-$FF : Writes have no effect.
           Reads from $C0 and $DC return the I/O port A/B register.
           Reads from $C1 and $DD return the I/O port B/misc. register.
           The remaining locations return $FF.

 Remember that ports $C0-$FF for some consoles will return different data
 when the I/O ports are disabled. See the following section for details.

 ----------------------------------------------------------------------------
 4.) I/O port registers
 ----------------------------------------------------------------------------

 Port $3E : Memory control

 D7 : Expansion slot enable (1= disabled, 0= enabled)
 D6 : Cartridge slot enable (1= disabled, 0= enabled)
 D5 : Card slot disabled (1= disabled, 0= enabled)
 D4 : Work RAM disabled (1= disabled, 0= enabled)
 D3 : BIOS ROM disabled (1= disabled, 0= enabled)
 D2 : I/O chip disabled (1= disabled, 0= enabled)
 D1 : Unknown
 D0 : Unknown

 A SMS with the snail maze BIOS sets this to $AB when running a cartridge.
 A SMS 2 with Alex Kidd sets this to $AB when running a cartridge.
 A Majesco Game Gear sets this to $A8 when running a cartridge.
 The Genesis does not initialize Z80 RAM prior to a game starting.

 For an SMS system with a cartridge inserted but no card, neither the
 card or expansion slots do anything when enabled. Reading these locations
 will return the last byte of the instruction which read memory. If the
 BIOS and cartridge are enabled at the same time, the cartridge has
 precedence and has it's data returned instead. No data bytes are corrupted
 in this case. I haven't checked to see what values are returned when work
 RAM is disabled.

 Bits 7 and 5 have no effect on a SMS 2. If the BIOS is enabled at the
 same time the cartridge slot is, the data from both sources are logically
 ANDed together when read. The BIOS only enables the cartridge slot after
 relocating itself to work RAM and disabling the BIOS beforehand.

 When bit 2 is set, all ports at $C0 through $FF return $FF on a SMS 2,
 Game Gear, and Genesis. For a SMS, these ports return the last byte of
 the instruction which read the port.

 Bits 7, 6, 5, 3, and 2 have no effect on a Genesis, or a Genesis with a PBC.
 I haven't checked to see what values are returned when work RAM is disabled.

 Even though a PBC includes a card slot, there is no BIOS to handle
 slot selection, so most likely PBC hardware enables the card slot or
 the cartridge slot, instead of it being software controlled. I couldn't
 get bit 5 to do anything, though I didn't have a card game to test with
 at the time.

 Bits 7, 6, 5, and 2 have no effect on a GameGear. I haven't checked to see
 what values are returned when work RAM is disabled. Bit 3 will enable a
 1K BIOS ROM at $0000-$03FF when clear, and map the cartridge slot to
 the same space when set. When the BIOS is enabled, the rest of the
 address space ($0400-$BFFF) is mapped to the cartridge slot as well, so
 the BIOS can write to the cartridge mapper registers (if any) and read
 the header data as well.

 Bits 1 and 0 have no effect on a SMS, SMS 2, Game Gear, and Genesis. They
 may be unused.

 Port $3F : I/O port control

 D7 : Port B TH pin output level (1=high, 0=low)
 D6 : Port B TR pin output level (1=high, 0=low)
 D5 : Port A TH pin output level (1=high, 0=low)
 D4 : Port A TR pin output level (1=high, 0=low)
 D3 : Port B TH pin direction (1=input, 0=output)
 D2 : Port B TR pin direction (1=input, 0=output)
 D1 : Port A TH pin direction (1=input, 0=output)
 D0 : Port A TR pin direction (1=input, 0=output)

 This port is used to detect if the machine is domestic (Japanese model)
 or export (US, European, Brazil, etc.) While the exact details of the
 detection scheme are unknown, it seems to be as follows:

 Set the TH pins for ports A and B as outputs. Set their output level
 to any value desired by writing to bits 7 and 5. Read the state of both
 TH pins back through bits 7 and 6 of port $DD. If the data returned is
 the same as the data written, it's an export machine, otherwise it's
 a domestic one.

 I ran a test on my Genesis which has an export/domestic mode switch. When
 in export mode reading the TH and TR pins for either port returns their
 current output level when they are set as outputs, like normal. When in
 domestic mode they always return zero, no matter what.

 A standard 2-button controller uses TR as an input and does not use TH.
 The light gun uses TH as an input to signal the VDP to latch the H counter,
 and Genesis style 3 and 6 button controllers can be used by setting TH as
 an output.

 Port $DC : I/O port A and B

 D7 : Port B DOWN pin input
 D6 : Port B UP pin input
 D5 : Port A TR pin input
 D4 : Port A TL pin input
 D3 : Port A RIGHT pin input
 D2 : Port A LEFT pin input
 D1 : Port A DOWN pin input
 D0 : Port A UP pin input

 In the case of a controller, a pressed button returns 0, otherwise 1.

 Port $DD : I/O port B and miscellaneous

 D7 : Port B TH pin input
 D6 : Port A TH pin input
 D5 : Unused
 D4 : RESET button (1= not pressed, 0= pressed)
 D3 : Port B TR pin input
 D2 : Port B TL pin input
 D1 : Port B RIGHT pin input
 D0 : Port B LEFT pin input

 Bit 5 returns 0 on a Genesis and 1 on an SMS, SMS 2 and GG.
 Bit 4 always returns 1 on a Genesis and GG which have no RESET button.

 In the case of a controller, a pressed button returns 0, otherwise 1.

 ----------------------------------------------------------------------------
 5.) Interrupts
 ----------------------------------------------------------------------------

 Interrupts:

 The Z80's NMI pin is connected to the PAUSE button. When this button is
 pressed an NMI is generated, causing the PC to change to $0066. Releasing
 the button does nothing.

 Interrupt mode 0

 The interrupting device can place a single or multi-byte opcode on the
 data bus for the Z80 to fetch and execute when an interrupt occurs.

 For the SMS 2, Game Gear, and Genesis, the value $FF is always read from
 the data bus, which corresponds to the instruction 'RST 38H'.

 For the SMS, a random value is returned which could correspond to any
 possible instruction.

 Interrupt mode 1

 When an interrupt occurs the Z80's PC register is set to $0038.

 Interrupt mode 2

 The interrupting device can place a single byte on the data bus which is
 used as the LSB of a 16-bit address, of which the MSB comes from the Z80's
 I register. The Z80 manual says the address must be even, but odd addresses
 work fine. The Z80 then jumps to that address.

 The SMS 2, Game Gear, and Genesis will return $FF. If the I register was
 set to $C0, this means the interrupt vector would be read from addresses
 $C0FF and $C100. For example:

                di
                ld      a, $C0
                ld      i, a            
                ld      hl, $xxxx       ; Address of your interrupt handler
                ld      ($C0FF), hl     
                im      2
                ei
        wait:   jr      wait

 The SMS returns a random value. The only way to use mode 2 in this case is
 to fill a 257-byte table with the same value, which corresponds to an
 address. No matter which set of bytes the Z80 ends up reading, it will
 always pick the same value:

                di
                ld      a, $C0
                ld      i, a            
                ld      b, $00
                ld      hl, $C000       ; Fill $C000-$C0FF with $70
                ld      a, $70
        fill:   ld      (hl), a
                inc     hl
                djnz    fill
                ld      (hl), a         ; Fill 257th byte ($C100)
                inc     hl
                im      2
                ei
        wait:   jr      wait

                .org    $7070
                ; Your interrupt handler goes here

 ----------------------------------------------------------------------------
 6.) BIOS information
 ----------------------------------------------------------------------------

 The SMS and SMS 2 consoles have a BIOS which is enabled on power-up.
 The BIOS copies a portion of code to RAM, and executes from there.
 It then checks the cartridge, card, and expansion slots to see if
 there is any valid software to run.

 This is done by examining the header, which is a 16-byte region that
 can start at addresses $1FF0, $3FF0, and $7FF0. I won't go into the
 details of the header, as this is documented elsewhere.

 Some SMS games rely on the BIOS to initialize certain parts of the
 system, such as memory or Z80 registers. They will not work in an emulator
 that doesn't support the BIOS or at least set up the system as how the
 BIOS would last leave it.

 The BIOS keeps a copy of the last value written to port $3E and stores this
 at memory address $C000. Game software uses this value when manipulating
 port $3E. One possible advantage of this method instead of using hard-coded
 values is that a game could also be run from a card or the expansion port
 as well as the cartridge slot. (Such as games like Hang-On, Transbot
 which were released on card and cartridge formats)

 ----------------------------------------------------------------------------
 7.) Sound hardware
 ----------------------------------------------------------------------------

 Game Gear:

 Port $06 is used to control the stereo output of the GG's PSG. Each channel
 can be sent to the left or right speaker, as follows:

 D7 : Channel #4 output from left speaker. (1= disabled, 0= enabled)
 D6 : Channel #3 output from left speaker. (1= disabled, 0= enabled)
 D5 : Channel #2 output from left speaker. (1= disabled, 0= enabled)
 D4 : Channel #1 output from left speaker. (1= disabled, 0= enabled)
 D3 : Channel #4 output from right speaker. (1= disabled, 0= enabled)
 D2 : Channel #3 output from right speaker. (1= disabled, 0= enabled)
 D1 : Channel #2 output from right speaker. (1= disabled, 0= enabled)
 D0 : Channel #1 output from right speaker. (1= disabled, 0= enabled)

 Writing $FF enables all channels through both speakers.
 Writing $00 disables all channels, and effectively mutes the sound.

 YM2413 sound:

 Mark III consoles had a FM sound unit which primarily consisted
 of a Yamaha YM2413 sound chip. The Japanese SMS had the YM2413 built in.
 Some games try to detect the YM2413 hardware as to be compatible with
 Mark III systems that do not have FM sound unit.

 The following ports are used by the FM add-on:

 Port $F0 : Output to YM2413 data port (with A0=0, select register latch)
 Port $F1 : Output to YM2413 data port (with A0=1, select register data)
 Port $F2 : Bit 0 can be read and written to detect if YM2413 is available.

 The SMS normally assigns reads from $C0-$FF to the I/O chip. You must
 disable the I/O chip by setting bit 2 of port $3E if you want to read
 port $F2 to perform YM2413 detection.

 The YM2413 has no readable registers. It isn't known exactly what data
 is returned from ports $C0-$FF if the YM2413 is present.

 ----------------------------------------------------------------------------
 8.) Credits and Acknowledgements
 ----------------------------------------------------------------------------

 In particular I'd like to thank Asynchronous and Mike G. for their many
 contributions to the S8-DEV forum. Quite a few things mentioned here are
 based on their posts.

 And I would like to thank everyone else:

 - Charles Doty
 - Chris MacDonald
 - Flavio Morsoletto
 - Jon
 - Marat Fayzullin
 - Maxim
 - Omar Cornut
 - Pascal Bosquet
 - Richard Atkinson
 - Richard Talbot-Watkins
 - Sean Young
 - Steve Snake
 - Everyone on the S8-DEV forum

 If I've missed anyone, please let me know.

 ----------------------------------------------------------------------------
 9.) Disclaimer
 ----------------------------------------------------------------------------

 If you use any information from this document, please credit me
 (Charles MacDonald) and optionally provide a link to my webpage
 (http://cgfm2.emuviews.com/) so interested parties can access it.

 The credit text should be present in the accompanying documentation of
 whatever project which used the information, or even in the program
 itself (e.g. an about box)

 Regarding distribution, you cannot put this document on another
 website, nor link directly to it.

 ----------------------------------------------------------------------------

 Unpublished work Copyright 2002 Charles MacDonald

