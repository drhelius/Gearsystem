/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef GUI_DEBUG_CONSTANTS_H
#define GUI_DEBUG_CONSTANTS_H

#include "imgui.h"
#include "gearsystem.h"

static const ImVec4 cyan =          ImVec4(0.10f, 0.90f, 0.90f, 1.0f);
static const ImVec4 dark_cyan =     ImVec4(0.00f, 0.30f, 0.30f, 1.0f);
static const ImVec4 magenta =       ImVec4(1.00f, 0.50f, 0.96f, 1.0f);
static const ImVec4 dark_magenta =  ImVec4(0.30f, 0.18f, 0.27f, 1.0f);
static const ImVec4 yellow =        ImVec4(1.00f, 0.90f, 0.05f, 1.0f);
static const ImVec4 dark_yellow =   ImVec4(0.30f, 0.25f, 0.00f, 1.0f);
static const ImVec4 orange =        ImVec4(1.00f, 0.50f, 0.00f, 1.0f);
static const ImVec4 dark_orange =   ImVec4(0.60f, 0.20f, 0.00f, 1.0f);
static const ImVec4 red =           ImVec4(0.98f, 0.15f, 0.45f, 1.0f);
static const ImVec4 dark_red =      ImVec4(0.30f, 0.04f, 0.16f, 1.0f);
static const ImVec4 green =         ImVec4(0.10f, 0.90f, 0.10f, 1.0f);
static const ImVec4 dim_green =     ImVec4(0.05f, 0.40f, 0.05f, 1.0f);
static const ImVec4 dark_green =    ImVec4(0.03f, 0.20f, 0.02f, 1.0f);
static const ImVec4 violet =        ImVec4(0.68f, 0.51f, 1.00f, 1.0f);
static const ImVec4 dark_violet =   ImVec4(0.24f, 0.15f, 0.30f, 1.0f);
static const ImVec4 blue =          ImVec4(0.20f, 0.40f, 1.00f, 1.0f);
static const ImVec4 dark_blue =     ImVec4(0.07f, 0.10f, 0.30f, 1.0f);
static const ImVec4 white =         ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
static const ImVec4 gray =          ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
static const ImVec4 mid_gray =      ImVec4(0.40f, 0.40f, 0.40f, 1.0f);
static const ImVec4 dark_gray =     ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
static const ImVec4 black =         ImVec4(0.00f, 0.00f, 0.00f, 1.0f);
static const ImVec4 brown =         ImVec4(0.68f, 0.50f, 0.36f, 1.0f);
static const ImVec4 dark_brown =    ImVec4(0.38f, 0.20f, 0.06f, 1.0f);

static const char* const c_cyan = "{19E6E6}";
static const char* const c_dark_cyan = "{004C4C}";
static const char* const c_magenta = "{FF80F5}";
static const char* const c_dark_magenta = "{4C2E45}";
static const char* const c_yellow = "{FFE60D}";
static const char* const c_dark_yellow = "{4C4000}";
static const char* const c_orange = "{FF8000}";
static const char* const c_dark_orange = "{993300}";
static const char* const c_red = "{FA2673}";
static const char* const c_dark_red = "{4C0A29}";
static const char* const c_green = "{19E619}";
static const char* const c_dim_green = "{0D660D}";
static const char* const c_dark_green = "{083305}";
static const char* const c_violet = "{AD82FF}";
static const char* const c_dark_violet = "{3D274D}";
static const char* const c_blue = "{3366FF}";
static const char* const c_dark_blue = "{12194D}";
static const char* const c_white = "{FFFFFF}";
static const char* const c_gray = "{808080}";
static const char* const c_mid_gray = "{666666}";
static const char* const c_dark_gray = "{1A1A1A}";
static const char* const c_black = "{000000}";
static const char* const c_brown = "{AD805C}";
static const char* const c_dark_brown = "{61330F}";

struct stDebugLabel
{
    u16 address;
    const char* label;
};

static const int k_debug_label_count = 43;
static const stDebugLabel k_debug_labels[k_debug_label_count] = 
{
    { 0x0000, "VDC_ADDRESS" },
    { 0x0002, "VDC_DATA_LO" },
    { 0x0003, "VDC_DATA_HI" },
    { 0x0400, "VCE_CONTROL" },
    { 0x0402, "VCE_ADDR_LO" },
    { 0x0403, "VCE_ADDR_HI" },
    { 0x0404, "VCE_DATA_LO" },
    { 0x0405, "VCE_DATA_HI" },
    { 0x0800, "PSG_CH_SELECT" },
    { 0x0801, "PSG_MAIN_VOL" },
    { 0x0802, "PSG_FREQ_LO" },
    { 0x0803, "PSG_FREQ_HI" },
    { 0x0804, "PSG_CH_CTRL" },
    { 0x0805, "PSG_CH_VOL" },
    { 0x0806, "PSG_CH_DATA" },
    { 0x0807, "PSG_NOISE" },
    { 0x0808, "PSG_LFO_FREQ" },
    { 0x0809, "PSG_LFO_CTRL" },
    { 0x0C00, "TIMER_COUNTER" },
    { 0x0C01, "TIMER_CONTROL" },
    { 0x1000, "JOYPAD" },
    { 0x1402, "IRQ_DISABLE" },
    { 0x1403, "IRQ_STATUS" },
    { 0x1800, "CD_STATUS" },
    { 0x1801, "CD_DATA_BUS" },
    { 0x1802, "CD_ENABLED_IRQS" },
    { 0x1803, "CD_ACTIVE_IRQS" },
    { 0x1804, "CD_RESET" },
    { 0x1805, "CD_PCM_LSB" },
    { 0x1806, "CD_PCM_MSB" },
    { 0x1807, "CD_BRAM_UNLOCK" },
    { 0x1808, "CD_DATA_ACK_ADPCM_LSB" },
    { 0x1809, "CD_ADPCM_MSB" },
    { 0x180A, "CD_ADPCM_DATA" },
    { 0x180B, "CD_ADPCM_DMA" },
    { 0x180C, "CD_ADPCM_STATUS" },
    { 0x180D, "CD_ADPCM_CONTROL" },
    { 0x180E, "CD_ADPCM_RATE" },
    { 0x180F, "CD_AUDIO_FADER" },
    { 0x18C0, "CD_SIGNATURE0" },
    { 0x18C1, "CD_SIGNATURE1" },
    { 0x18C2, "CD_SIGNATURE2" },
    { 0x18C3, "CD_SIGNATURE3" }
};

static const int k_cdrom_bios_symbol_count = 76;

static const stDebugLabel k_cdrom_bios_symbols[k_cdrom_bios_symbol_count] = 
{
    // CD commands
    { 0xE000, "CD_BOOT"     },
    { 0xE003, "CD_RESET"    },
    { 0xE006, "CD_BASE"     },
    { 0xE009, "CD_READ"     },
    { 0xE00C, "CD_SEEK"     },
    { 0xE00F, "CD_EXEC"     },
    { 0xE012, "CD_PLAY"     },
    { 0xE015, "CD_SEARCH"   },
    { 0xE018, "CD_PAUSE"    },
    { 0xE01B, "CD_STAT"     },
    { 0xE01E, "CD_SUBQ"     },
    { 0xE021, "CD_DINFO"    },
    { 0xE024, "CD_CONTENTS" },
    { 0xE027, "CD_SUBRD"    },
    { 0xE02A, "CD_PCMRD"    },
    { 0xE02D, "CD_FADE"     },

    // ADPCM commands
    { 0xE030, "AD_RESET"    },
    { 0xE033, "AD_TRANS"    },
    { 0xE036, "AD_READ"     },
    { 0xE039, "AD_WRITE"    },
    { 0xE03C, "AD_PLAY"     },
    { 0xE03F, "AD_CPLAY"    },
    { 0xE042, "AD_STOP"     },
    { 0xE045, "AD_STAT"     },

    // Block manager
    { 0xE048, "BM_FORMAT"   },
    { 0xE04B, "BM_FREE"     },
    { 0xE04E, "BM_READ"     },
    { 0xE051, "BM_WRITE"    },
    { 0xE054, "BM_DELETE"   },
    { 0xE057, "BM_FILES"    },

    // System extensions (I)
    { 0xE05A, "EX_GETVER"   },
    { 0xE05D, "EX_SETVEC"   },
    { 0xE060, "EX_GETFNT"   },
    { 0xE063, "EX_JOYSNS"   },
    { 0xE066, "EX_JOYREP"   },
    { 0xE069, "EX_SCRSIZ"   },

    // System extensions (II)
    { 0xE06C, "EX_DOTMOD"   },
    { 0xE06F, "EX_SCRMOD"   },
    { 0xE072, "EX_IMODE"    },
    { 0xE075, "EX_VMODE"    },
    { 0xE078, "EX_HMODE"    },
    { 0xE07B, "EX_VSYNC"    },
    { 0xE07E, "EX_RCRON"    },
    { 0xE081, "EX_RCROFF"   },
    { 0xE084, "EX_IRQON"    },
    { 0xE087, "EX_IRQOFF"   },
    { 0xE08A, "EX_BGON"     },
    { 0xE08D, "EX_BGOFF"    },
    { 0xE090, "EX_SPRON"    },
    { 0xE093, "EX_SPROFF"   },
    { 0xE096, "EX_DSPON"    },
    { 0xE099, "EX_DSPOFF"   },
    { 0xE09C, "EX_DMAMOD"   },
    { 0xE09F, "EX_SPRDMA"   },
    { 0xE0A2, "EX_SATCLR"   },
    { 0xE0A5, "EX_SPRPUT"   },
    { 0xE0A8, "EX_SETRCR"   },
    { 0xE0AB, "EX_SETRED"   },
    { 0xE0AE, "EX_SETWRT"   },
    { 0xE0B1, "EX_SETDMA"   },
    { 0xE0B4, "EX_COLORCMD" },
    { 0xE0B7, "EX_BINBCD"   },
    { 0xE0BA, "EX_BCDBIN"   },
    { 0xE0BD, "EX_RND"      },

    // Maths
    { 0xE0C0, "MA_MUL8U"    },
    { 0xE0C3, "MA_MUL8S"    },
    { 0xE0C6, "MA_MUL16U"   },
    { 0xE0C9, "MA_DIV16S"   },
    { 0xE0CC, "MA_DIV16U"   },
    { 0xE0CF, "MA_SQRT"     },
    { 0xE0D2, "MA_SIN"      },
    { 0xE0D5, "MA_COS"      },
    { 0xE0D8, "MA_ATNI"     },

    // PSG
    { 0xE0DB, "PSG_BIOS"    },
    { 0xE0DE, "GRP_BIOS"    },
    { 0xE0E1, "PSG_DRIVE"   }
};

#endif /* GUI_DEBUG_CONSTANTS_H */