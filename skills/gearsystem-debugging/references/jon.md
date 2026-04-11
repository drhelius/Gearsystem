
Last update:23/11/93 by Jon

Introduction to the hardware

        The Sega Master System and Game Gear are both 8-bit games consoles based
on the Zilog Z80 microprocessor. The Master System is the older of the two
machines which are identical in most respects, with the Game Gear having both a
Master System mode, for backward compatibility, and a true Game Gear mode,
offering advantages over the original machine. A summary of the features of each
machine follows:

Master System & Master System II console.

Cartridge port
Two joypad ports
Pause button
RF output
Reset button                    }
RGB/Composite video output      } Not on Master System II
ROM card slot                   }
Edge connector                  }

Game Gear hand held

Colour LCD screen
Cartridge port
Built in joypad
Start button
Two player port


Introduction to the video

        Both consoles feature character-mapped screens, with the internal size
of the screen being 32x28 characters. The displayed size is variable on the
Master System, and Game Gear in Master System mode, but fixed in true Game Gear
mode. The characters are 8x8 pixels in size, and are used both on screen, and as
sprites. The characters are made up of 4 bit planes, allowing for 16 colours.
There are two user definable palettes of 16 colours each, one for the characters
displayed on screen, and the other for the sprites. The range of colours
available for use in these palettes is 64 on the Master System, and 512 on the
Game Gear.

        Up to 448 characters can be defined, arranged as one set of 256, and
another of 192. Both consoles have 64 hardware sprites, and the programmer can
define which of the two sets of characters to use as sprites. Generally the
sprites are just 8x8 characters, but there are other possibilities detailed in
the section on video registers.

        Hardware scrolling is supported on both the x, and the y axis.
Characters printed on screen may be flipped in either or both directions, and
additionally can be forced to be infront or behind sprites. Sadly sprites cannot
be flipped, which is a major omission, and one for which I would personally like
to see someone severely chastised with a root vegetable.

        All access to the screen, sprites, character set, and assorted video
registers is via ports. Access to certain parts of video RAM is only reliable
either on frame flyback, or when the screen is switched off. This is annoying in
the extreme, but not surprising.

Introduction to the sound chip

        The Master System has a 3 channel tone generator, along with a white
noise channel, and is mono only. The Game Gear can emulate this chip, in both
Master System mode and real Game Gear mode. Additionally it is mooted to have a
stereo chip of some sort, but as yet we have been unable to access it. The
standard mono chip is unimpressive, and very much in the same mould as the AY
chips used in Spectrum's and Atari ST's. Nothing spectacular by any means, but
with effort nice results can be achieved.

Joypad(s)

        The Master System has two joypad ports, with capability for a lightgun,
and 3D glasses. To retain credibility we shall not be dealing with either the
lightgun or the 3D glasses. The joypad is a standard 4 direction affair, with
two fire buttons. Additionally the original Master System had a RESET button.
The PAUSE button is nothing to do with the joypads, and will be dealt with
later.

        Reading the joypads is simplicity itself, although you must bear in mind
that Sega favour active low logic, so a 0 means button pressed, 1 means button
released.

Port &DC
        Bit 0   Joypad 1 UP
        Bit 1   Joypad 1 DOWN
        Bit 2   Joypad 1 LEFT
        Bit 3   Joypad 1 RIGHT
        Bit 4   Joypad 1 FIRE 1/START (not START button on Game Gear!)
        Bit 5   Joypad 1 FIRE 2
        Bit 6   Joypad 2 UP
        Bit 7   Joypad 2 DOWN

Port &DD
        Bit 0   Joypad 2 LEFT
        Bit 1   Joypad 2 RIGHT
        Bit 2   Joypad 2 FIRE 1/START
        Bit 3   Joypad 2 FIRE 2
        Bit 4   RESET button (original Master System only)
        Bit 5   Unconnected??
        Bit 6   Lightgun 1 Sync
        Bit 7   Lightgun 2 Sync

        On the Game Gear life is a little simpler, with only 1 joypad it is only
necessary to read port &DC. Bits 6 & 7 may or may not read as 1, and so you
should make sure that any Game Gear code ignores them. In true Game Gear mode
you cannot read the status of the blue START button through these ports.

        Writing to the joypad ports seems to have no effect, but of course you
can have hours of endless fun trying to prove me wrong...


Pause Button

        The pause button on the Master System is not readable. When pressed, it
generates an NMI, forcing the processor to stack it's PC register, and jump to
&0066. This is really quite nasty, but at least it is hardware debounced for
you.

        Please note that if you run the Game Gear in Master System mode then
its blue START button will act exactly as above.

Start Button

        On the Master System, fire button 1 is generally regarded, and indeed
labeled, as START. This is little more than a recommendation of how programmers
should interpret the user pressing fire button 1.

        On the Game Gear life is different. That delightful blue START button is
readable, and so you have the luxury of 3 buttons. Irritatingly the START button
is not readable from the same port as the joypad! Again active low logic
applies, and the port is detailed as below.

Port &00
        Bit 0   Unknown
        Bit 1   Unknown
        Bit 2   Unknown
        Bit 3   Unknown
        Bit 4   Unknown
        Bit 5   Unknown
        Bit 6   Unknown
        Bit 7   Start button (Game Gear only)



Two player port

        The Game Gear is capable of two player games when you connect two Game
Gears with the Gear-to-Gear cable. Since we last met, dear readers, I have
sussed that there are two possible ways to use the Gear-to-Gear cable, well,
three if you include murder of project managers.

        The original method, which I used remarkably well to talk to an Amiga
of all things, is now very much out of favour, with the main reason being that
it's crap. It allows you to use the link-up as a 7-bit bi-directional cable,
with hardware cross-over. It is useful for talking to other machines, such as
the Amiga, but turned out to be less than satisfactory for all but the most
trivial games. (Incidentally, it is still used in SQUINKY TENNIS, which should
be included in every Game Gear title Codemasters publish.) The ports used by
this method are as follows:

Port &02        Data direction bit-pattern (0=output, 1=input)
                Use bits 0..6 only

Port &01        Linkup data read/write (Use bits 0..6 only)
                Cross-over is as follows:
        WRITE           READ
        Bit 0   ----->  Bit 2
        Bit 1   ----->  Bit 3
        Bit 2   ----->  Bit 0
        Bit 3   ----->  Bit 1
        Bit 4   ----->  Bit 5
        Bit 5   ----->  Bit 4
        Bit 6   ----->  Bit 6
        Bit 7   Unused

        The second, much more useful, and presumably "official" way of using the
link-up is as follows. There are three ports used, a write port, a read port,
and the inevitable status port. Data is sent and received in bytes, with full
hardware handshaking (thank you!). Once the status port is set, then an NMI will
be generated when data has been received from the other Game Gear. Reading this
byte acts as an acknowledge, and allows further bytes to be received. Writing a
byte out is simple, and assuming the other Game Gear has set up its status port
correctly, will generate an NMI on the other machine. The ports used are as
follows:

Port &03        Write with a byte to be Tx'd

Port &04        Read during NMI to get Rx'd byte

Port &05        Status port
        Bit 0   Reads as 1 if just Tx'd a byte
        Bit 1   Reads as 1 if just Rx'd a byte
        Bit 2   Reads as 1 if other GG powered off, 0 if other GG powered on
        Bit 3   1 }
        Bit 4   1 }-Set these three bits to allow NMI generation on Rx'd bytes
        Bit 5   1 }
        Bit 6   0
        Bit 7   0

        The general rule would appear to be that you must read and act upon the
Rx'd byte as soon as you get it. Reading port &04 when a byte hasn't been Rx'd
has a habit of crashing the machine, and so should be avoided. The three known
status bits in the status port can be used to detect if the other player powers
off their GG or whatever, and act accordingly.

        If you have the link-up cable plugged in to your machine, and are
powered on, but the connected GG is powered off, then you will receive a barage
of NMI's. Obviously this could be a problem during a one-player game, throwing
all your timings out quite drastically. So, once a one-player game has been
selected, I advise writing a 0 to port &05 to stop NMI generation. This will
give you total control of the console as normal. Once you get back to your
selection screen or whatever, you can re-enable NMI generation and make sense of
any incoming bytes as you wish.

        One final point to note, is that neither of these two methods of
communication are available in Master System mode on the Game Gear. Once more
the message is clear kids, let's not write crappy Master System ports, coz the
chances are Codies will get miffed and stop publishing them.


The Sound Chip

        The sound chip is capable of generating 3 tones simultaneously, each
with individually selectable frequency and volume. Additionally it can generate
noise, with user definable "type" and volume. The sound chip can be accessed via
port &7E or port &7F, with no discernable difference.

        To change the frequency of one of the tone channels write the following
bytes to the port:

Byte 1
        Bit 0   }
        Bit 1   } 4 least significant bits
        Bit 2   } of frequency
        Bit 3   }

        Bit 4   --}000=channel 1
        Bit 5   --}010=channel 2
        Bit 6   --}100=channel 3

        Bit 7   1

Byte 2
        Bit 0   }
        Bit 1   }
        Bit 2   } 6 most significant bits
        Bit 3   } of frequency
        Bit 4   }
        Bit 5   }

        Bit 6   Unused
        Bit 7   Unused


        To change the volume of one of the tone channels write the following
byte to the port:

Byte 1
        Bit 0   }
        Bit 1   } 4 bits to define volume
        Bit 2   } 0000=loudest  1111=off
        Bit 3   }

        Bit 4   --}001=channel 1
        Bit 5   --}011=channel 2
        Bit 6   --}101=channel 3

        Bit 7   1


        To change the type of noise, write the following byte to the port:

Byte 1
        Bit 0   } 3 bits to define
        Bit 1   } noise type 0-7
        Bit 2   }

        Bit 3   Unused

        Bit 4   0
        Bit 5   1
        Bit 6   1
        Bit 7   1


        To change the volume of the noise channels write the following byte to
the port:

Byte 1
        Bit 0   }
        Bit 1   } 4 bits to define volume
        Bit 2   } 0000=loudest  1111=off
        Bit 3   }

        Bit 4   1
        Bit 5   1
        Bit 6   1
        Bit 7   1

The video chip

        The video chip is controlled via two ports, one is an address port &BF,
and the other is a data port &BE. Before writing to, or reading from video RAM
it is necessary to set the video address. This can be accomplished by writing
the low byte, and then the high byte of the address to port &BF.

Video RAM Memory Map

&4000-&77FF     -       448 character definitions
&7800-&7DFF     -       Screen memory
&7E00-&7EFF     -       Alternate area for sprite tables (apparently)
&7F00-&7F3F     -       64 sprite Y coords
&7F40-&7F7F     -       64 tantalisingly unused bytes (if only it could flip!)
&7F80-&7FFF     -       64 [sprite x,sprite character] pairs

&C000-&C00F     -       Master System character palette
&C010-&C01F     -       Master System sprite palette

&C000-&C01F     -       Game Gear character palette
&C020-&C03F     -       Game Gear sprite palette

        Writing to the data port, causes the video address to auto-increment, so
large blocks of contiguous data may be written once the video address is set.
Reading from video RAM is possible, although after setting the video address you
should ignore the first byte read from the data port. Again the video address
will auto-increment after each read.

        Accessing any video RAM should be restricted to either the vertical
retrace period, or when the screen is switched off. There is a limited amount of
access granted during the vertical flyback, after which the integrity of data
read or written cannot be guaranteed. I estimate this to be equivalent to the
amount of writes required to redefine 12 characters, ie 384 bytes. Using this
figure as a rough guide it is not too difficult to rationalise, and time-share,
your access to video RAM.

        In addition to the video RAM already discussed, there are a number of
registers in the video chip, which when altered have an effect on the display.
These are sadly write-only, which quite frankly spoilt my weekend. They are
accessed by writing to the video address port &BF. Firstly output the byte to be
sent to a particular register, and then the register number. Writes to video
registers can be done at practically any time, and so raster split effects are
possible.

Register &80
        Bit 0   Screen sync off
        Bit 1   Normal or enable stretch screen (Master System)
        Bit 2   Causes graphics change (related to screen addr?)
        Bit 3   Shift sprites left by 1 character
        Bit 4   0?
        Bit 5   Display extra column on LHS of screen
        Bit 6   Top 2 rows of screen horizontal non-scrolling (Master System)
        Bit 7   Right side of screen vertical non-scrolling (Master System)

        The stretch screen option is really only useful on a Master System, or a
Game Gear in Master System mode. Make of bit 2 what you will, but you'll
probably find that you should set it anyway. Likewise with bit 5. Bits 6 will
give you a two character high panel at the top of the screen that is unaffected
by the horizontal scroll. Bit 7 will give you a wide panel on the right hand
side of the screen that is unaffected by the vertical scroll.

Register &81
        Bit 0   Double sized pixels in sprites
        Bit 1   8x16 sprites
        Bit 2   0?
        Bit 3   Stretch screen by 6 rows (Master System)
        Bit 4   Stretch screen by 4 rows (Master System)
        Bit 5   Screen interrupts enable
        Bit 6   Screen enable
        Bit 7   0?

        Bit 0 causes all pixels in sprites to double in size, so that each 8x8
character sprite takes up 16x16 pixels. Alas this applies to ALL sprites, so
its usefulness it doubtful. Bit 1 allows you to use 8x16 sprites, which IS
useful. Instead of just one 8x8 appearing at each sprite coordinate, the
character immediately after the desired one will appear directly below the
original sprite. Using this mode does limit you to using even character numbers
only. Stretching the screen by 6 rows is not recommended, as it does not appear
to be a supported mode on the convertor on the Mega-Drive, one can only assume
that this mode is not officially acknowledged by Sega. I am given to understand
that stretching the screen by 4 rows will work on a convertor, but with any
stretched screen mode you have less characters.

        You must set bit 5 and put the Z80 in interrupt mode 1 with interrupts
enabled in order to get interrupts on the vertical retrace. If you disable the
screen by clearing bit 6 you can have unlimited access to video RAM.

Register &82
        Bit 0   0?
        Bit 1   --}
        Bit 2   --} Address of screen in video RAM
        Bit 3   --}
        Bit 4   0?
        Bit 5   0?
        Bit 6   0?
        Bit 7   0?

        Bits 1,2,3 control the address of the screen in video memory. The screen
is 1.5k in size, and can apparently be moved. Setting this register to &0E gives
us the screen at &7800, which seems pretty sensible to me. I have no idea what
the other bits do, if anything.

Register &83
        Bit 0   1?
        Bit 1   1?
        Bit 2   1?
        Bit 3   1?
        Bit 4   1?
        Bit 5   1?
        Bit 6   1?
        Bit 7   1?

        This register is a complete mystery. However, setting it to &FF seems to
have no adverse side effects at the moment.

Register &84
        Bit 0   1?
        Bit 1   1?
        Bit 2   1?
        Bit 3   1?
        Bit 4   1?
        Bit 5   1?
        Bit 6   1?
        Bit 7   1?

        I'm sure this register is related to &83. Again for safety I set it to
&FF. By messing with registers &83 and &84 and setting or clearing unknown bits
in other registers I have had a sort-of bitmap mode, although I have had no luck
in finding any sensible way to control it. Oh, for the official spec!! What
interesting and enlightening reading that would be....

Register &85
        Bit 0   --}
        Bit 1   --}
        Bit 2   --}
        Bit 3   --} Address of sprite table in video RAM
        Bit 4   --}
        Bit 5   --}
        Bit 6   --}
        Bit 7   0?

        This register controls the address of the sprite table in video memory.
Usually this is set to &7F, so that the table is at &7F00, but writing alternate
values will alter the address of the sprite table accordingly.

Register &86
        Bit 0   0?
        Bit 1   0?
        Bit 2   Controls which set of characters is used for sprites
        Bit 3   0?
        Bit 4   0?
        Bit 5   0?
        Bit 6   0?
        Bit 7   0?

        Writing 0 to bit 2 allows you to use the set of 256 characters as
sprites, writing 1 allows you to use the set of 192 characters as sprites. The
other bits don't seem to have an effect, although I could be wrong..

Register &87
        Bit 0   --}
        Bit 1   --} Border colour [0..15] from sprite palette
        Bit 2   --}
        Bit 3   --}
        Bit 4   0?
        Bit 5   0?
        Bit 6   0?
        Bit 7   0?

        Bits 0..3 control the border colour, which is taken from the sprite
palette. Bits 4..7 seemingly have no use.

Register &88
        Bit 0   --}
        Bit 1   --}
        Bit 2   --}
        Bit 3   --} Horizontal scroll
        Bit 4   --}
        Bit 5   --}
        Bit 6   --}
        Bit 7   --}

        This register controls the horizontal scrolling. You may write a value
from 0..255, and the screen display will be affected immediately. You can write
to this at different times during the display period to get raster splits.

Register &89
        Bit 0   --}
        Bit 1   --}
        Bit 2   --}
        Bit 3   --} Vertical scroll
        Bit 4   --}
        Bit 5   --}
        Bit 6   --}
        Bit 7   --}

        This register controls the vertical scrolling. Values 0..223 are valid.
Again you can write to it any time you fancy.

Register &8A
        Bit 0   --}
        Bit 1   --}
        Bit 2   --}
        Bit 3   --} No of pixels apart you want interrupts
        Bit 4   --}
        Bit 5   --}
        Bit 6   --}
        Bit 7   --}

        You can generate interrupts on any given raster line with a bit of
effort. If you just want an interrupt on the vertical blank period then write
&FF to this register and we'll say no more about it. However, the adventurous
amongst you may like to write a number between 0 and 127 to this port. This will
generate an interrupt that number of pixels apart. So, it's then quite easy to
get a rock solid raster-split. Firstly write &FF to the register, when you get
the interrupt on the vertical blank write a different number to the register.
When you get that interrupt alter the x-scroll or whatever, and then write &FF
to the register to make sure you get the next interrupt on the vertical blank.
Yes, it's a little bit of a hassle, but it will work!

Writing to the screen

        Writing to the screen is very simple, if a little slow. Firstly set the
video address to point to the part of the screen you wish to write to. Then
write the number of the character you want to print to the data port (0..255).
Next write a status byte as follows:

        Bit 0   Write 0 to use low 255 chars, 1 to use top 192
        Bit 1   X-flip character
        Bit 2   Y-flip character
        Bit 3   Printed on foreground colour
        Bit 4   0=print behind sprites, 1=print infront of sprites
        Bit 5   ? }
        Bit 6   ? }- I have no idea what these bits do.
        Bit 7   ? }

Palette details

        The Master System has a palette of 64 colours, from which you may choose
16 for the characters, and a further 16 for the sprites. Each colour is defined
as a byte thus:

        Bit 0   } Red value (0..3)
        Bit 1   }
        Bit 2   -} Green value (0..3)
        Bit 3   -}
        Bit 4   } Blue value (0..3)
        Bit 5   }
        Bit 6   ?
        Bit 7   ?

        The Game Gear has a better range of colours, 512 to choose from. This
requires writing two bytes to define a colour, hence the difference in the
address of the sprite palette on Game Gear and Master System. The format of the
bytes is thus:

Byte 0
        Bit 0   Unused
        Bit 1   }
        Bit 2   }- Red value (0..7)
        Bit 3   }
        Bit 4   Unused
        Bit 5   }
        Bit 6   }- Green value (0..7)
        Bit 7   }

Byte 1
        Bit 0   Unused
        Bit 1   }
        Bit 2   }- Blue value (0..7)
        Bit 3   }
        Bit 4   Unused
        Bit 5   Unused
        Bit 6   Unused
        Bit 7   Unused

Memory map

&0000-&BFFF     -       Cartridge memory.
&C000-&DFFF     -       8k user RAM.
&E000-&FFFF     -       Nothing. Appears to echo some parts of user RAM.


Jon's Handy Disclaimer(tm)

 Some of this may be correct, some of it may not be. Either way I don't care as
long as you don't hold me to any of it. Thanks. Jon.
