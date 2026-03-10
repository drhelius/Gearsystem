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

#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include "libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
 */

struct retro_core_option_v2_category option_cats_us[] = {
    {
        "system",
        "System",
        "Configure system type, region, mapper, refresh rate and BIOS settings."
    },
    {
        "video",
        "Video",
        "Configure aspect ratio, overscan, left bar visibility and 3D glasses settings."
    },
    {
        "audio",
        "Audio",
        "Configure FM sound chip settings."
    },
    {
        "input",
        "Input",
        "Configure controller behavior, light gun and paddle settings."
    },
    { NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] = {

    /* System */

    {
        "gearsystem_system",
        "System (restart)",
        NULL,
        "Select the console type to emulate. 'Auto' automatically detects the appropriate system based on the loaded content.",
        NULL,
        "system",
        {
            { "Auto",                           NULL },
            { "Master System / Mark III",       NULL },
            { "Game Gear (2 ASIC)",             NULL },
            { "Game Gear (2 ASIC) SMS Mode",    NULL },
            { "Game Gear (1 ASIC)",             NULL },
            { "Game Gear (1 ASIC) SMS Mode",    NULL },
            { "SG-1000 / Multivision",          NULL },
            { "SG-1000 II",                     NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearsystem_region",
        "Region (restart)",
        NULL,
        "Select which region is emulated. 'Auto' automatically detects the appropriate region based on the loaded content.",
        NULL,
        "system",
        {
            { "Auto",                    NULL },
            { "Master System Japan",     NULL },
            { "Master System Export",    NULL },
            { "Game Gear Japan",         NULL },
            { "Game Gear Export",        NULL },
            { "Game Gear International", NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearsystem_mapper",
        "Mapper (restart)",
        NULL,
        "Select which mapper (memory bank controller) is emulated. 'Auto' automatically detects the appropriate mapper based on the loaded content. Only change this if a game does not work correctly with the default setting.",
        NULL,
        "system",
        {
            { "Auto",                 NULL },
            { "ROM",                  NULL },
            { "SEGA",                 NULL },
            { "Codemasters",          NULL },
            { "Korean",               NULL },
            { "SG-1000",              NULL },
            { "MSX",                  NULL },
            { "Janggun",              NULL },
            { "Korean 2000 XOR 1F",   NULL },
            { "Korean MSX 32KB 2000", NULL },
            { "Korean MSX SMS 8000",  NULL },
            { "Korean SMS 32KB 2000", NULL },
            { "Korean MSX 8KB 0300",  NULL },
            { "Korean 0000 XOR FF",   NULL },
            { "Korean FFFF HiCom",    NULL },
            { "Korean FFFE",          NULL },
            { "Korean BFFC",          NULL },
            { "Korean FFF3 FFFC",     NULL },
            { "Korean MD FFF5",       NULL },
            { "Korean MD FFF0",       NULL },
            { "Jumbo Dahjee",         NULL },
            { "EEPROM 93C46",         NULL },
            { "Multi 4PAK All Action", NULL },
            { "Iratahack",            NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearsystem_timing",
        "Refresh Rate (restart)",
        NULL,
        "Select which refresh rate will be used in emulation. 'Auto' selects the best refresh rate based on the loaded content.",
        NULL,
        "system",
        {
            { "Auto",        NULL },
            { "NTSC (60 Hz)", NULL },
            { "PAL (50 Hz)", NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearsystem_bios_sms",
        "Master System BIOS (restart)",
        NULL,
        "Enable or disable the Master System BIOS. For this to work, the 'bios.sms' file must exist in the frontend's system directory. When enabled it will execute as in original hardware, which may cause invalid ROMs to lock or fail to boot.",
        NULL,
        "system",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearsystem_bios_gg",
        "Game Gear BIOS (restart)",
        NULL,
        "Enable or disable the Game Gear BIOS. For this to work, the 'bios.gg' file must exist in the frontend's system directory. When enabled it will execute as in original hardware, which may cause invalid ROMs to lock or fail to boot.",
        NULL,
        "system",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },

    /* Video */

    {
        "gearsystem_aspect_ratio",
        "Aspect Ratio",
        NULL,
        "Select which aspect ratio will be presented by the core. '1:1 PAR' selects an aspect ratio that produces square pixels.",
        NULL,
        "video",
        {
            { "1:1 PAR",  NULL },
            { "4:3 DAR",  NULL },
            { "16:9 DAR", NULL },
            { "16:10 DAR", NULL },
            { NULL, NULL },
        },
        "1:1 PAR"
    },
    {
        "gearsystem_overscan",
        "Overscan",
        NULL,
        "Select which overscan (borders) will be used in emulation. 'Top+Bottom' enables overscan for top and bottom only. 'Full' enables overscan for all sides with the specified width.",
        NULL,
        "video",
        {
            { "Disabled",         NULL },
            { "Top+Bottom",       NULL },
            { "Full (284 width)", NULL },
            { "Full (320 width)", NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearsystem_hide_left_bar",
        "Hide Left Bar (SMS only)",
        NULL,
        "Select when to hide the left column in Master System games. Some games use this column for scrolling artifacts. 'Auto' hides it only when a left bar is detected. 'Always' hides it unconditionally.",
        NULL,
        "video",
        {
            { "No",     NULL },
            { "Auto",   NULL },
            { "Always", NULL },
            { NULL, NULL },
        },
        "No"
    },
    {
        "gearsystem_glasses",
        "3D Glasses",
        NULL,
        "For games with 3D glasses support, select which eye to display. 'Both Eyes / OFF' is required for games without 3D support or to display both eyes simultaneously.",
        NULL,
        "video",
        {
            { "Both Eyes / OFF", NULL },
            { "Left Eye",        NULL },
            { "Right Eye",       NULL },
            { NULL, NULL },
        },
        "Both Eyes / OFF"
    },

    /* Audio */

    {
        "gearsystem_ym2413",
        "YM2413 (restart)",
        NULL,
        "Enable or disable the YM2413 (OPLL) FM sound chip. 'Auto' enables the chip based on the loaded content. Some Master System games use this chip for enhanced music.",
        NULL,
        "audio",
        {
            { "Auto",     NULL },
            { "Disabled",  NULL },
            { NULL, NULL },
        },
        "Auto"
    },
    {
        "gearsystem_psg_volume",
        "PSG Volume",
        NULL,
        "Set the volume of the PSG (SN76489). The value is a percentage from 0 to 200, where 100 is the default volume.",
        NULL,
        "audio",
        {
            { "0",   NULL },
            { "10",  NULL },
            { "20",  NULL },
            { "30",  NULL },
            { "40",  NULL },
            { "50",  NULL },
            { "60",  NULL },
            { "70",  NULL },
            { "80",  NULL },
            { "90",  NULL },
            { "100", NULL },
            { "110", NULL },
            { "120", NULL },
            { "130", NULL },
            { "140", NULL },
            { "150", NULL },
            { "160", NULL },
            { "170", NULL },
            { "180", NULL },
            { "190", NULL },
            { "200", NULL },
            { NULL, NULL },
        },
        "100"
    },
    {
        "gearsystem_fm_volume",
        "FM Volume",
        NULL,
        "Set the volume of the YM2413 (OPLL) FM sound chip. The value is a percentage from 0 to 200, where 100 is the default volume.",
        NULL,
        "audio",
        {
            { "0",   NULL },
            { "10",  NULL },
            { "20",  NULL },
            { "30",  NULL },
            { "40",  NULL },
            { "50",  NULL },
            { "60",  NULL },
            { "70",  NULL },
            { "80",  NULL },
            { "90",  NULL },
            { "100", NULL },
            { "110", NULL },
            { "120", NULL },
            { "130", NULL },
            { "140", NULL },
            { "150", NULL },
            { "160", NULL },
            { "170", NULL },
            { "180", NULL },
            { "190", NULL },
            { "200", NULL },
            { NULL, NULL },
        },
        "100"
    },

    /* Input */

    {
        "gearsystem_up_down_allowed",
        "Allow Up+Down / Left+Right",
        NULL,
        "Allow pressing, quickly alternating, or holding both left and right (or up and down) directions at the same time. This may cause movement based glitches in certain games.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearsystem_lightgun_input",
        "Light Gun Input",
        NULL,
        "Select which input method will be used for Light Phaser games. 'Light Gun' uses a mouse-controlled light gun input. 'Touchscreen' uses pointer-based touchscreen input.",
        NULL,
        "input",
        {
            { "Light Gun",   NULL },
            { "Touchscreen", NULL },
            { NULL, NULL },
        },
        "Light Gun"
    },
    {
        "gearsystem_lightgun_crosshair",
        "Light Gun Crosshair",
        NULL,
        "Enable or disable the crosshair overlay for Light Phaser games.",
        NULL,
        "input",
        {
            { "Disabled", NULL },
            { "Enabled",  NULL },
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        "gearsystem_lightgun_shape",
        "Light Gun Crosshair Shape",
        NULL,
        "Select the shape of the crosshair overlay for Light Phaser games.",
        NULL,
        "input",
        {
            { "Cross",  NULL },
            { "Square", NULL },
            { NULL, NULL },
        },
        "Cross"
    },
    {
        "gearsystem_lightgun_color",
        "Light Gun Crosshair Color",
        NULL,
        "Select the color of the crosshair overlay for Light Phaser games.",
        NULL,
        "input",
        {
            { "White",   NULL },
            { "Black",   NULL },
            { "Red",     NULL },
            { "Green",   NULL },
            { "Blue",    NULL },
            { "Yellow",  NULL },
            { "Magenta", NULL },
            { "Cyan",    NULL },
            { NULL, NULL },
        },
        "White"
    },
    {
        "gearsystem_lightgun_crosshair_offset_x",
        "Light Gun Crosshair Offset X",
        NULL,
        "Set the horizontal pixel offset of the crosshair for calibration purposes.",
        NULL,
        "input",
        {
            { "-10", NULL },
            { "-9",  NULL },
            { "-8",  NULL },
            { "-7",  NULL },
            { "-6",  NULL },
            { "-5",  NULL },
            { "-4",  NULL },
            { "-3",  NULL },
            { "-2",  NULL },
            { "-1",  NULL },
            { "0",   NULL },
            { "1",   NULL },
            { "2",   NULL },
            { "3",   NULL },
            { "4",   NULL },
            { "5",   NULL },
            { "6",   NULL },
            { "7",   NULL },
            { "8",   NULL },
            { "9",   NULL },
            { "10",  NULL },
            { NULL, NULL },
        },
        "0"
    },
    {
        "gearsystem_lightgun_crosshair_offset_y",
        "Light Gun Crosshair Offset Y",
        NULL,
        "Set the vertical pixel offset of the crosshair for calibration purposes.",
        NULL,
        "input",
        {
            { "-10", NULL },
            { "-9",  NULL },
            { "-8",  NULL },
            { "-7",  NULL },
            { "-6",  NULL },
            { "-5",  NULL },
            { "-4",  NULL },
            { "-3",  NULL },
            { "-2",  NULL },
            { "-1",  NULL },
            { "0",   NULL },
            { "1",   NULL },
            { "2",   NULL },
            { "3",   NULL },
            { "4",   NULL },
            { "5",   NULL },
            { "6",   NULL },
            { "7",   NULL },
            { "8",   NULL },
            { "9",   NULL },
            { "10",  NULL },
            { NULL, NULL },
        },
        "0"
    },
    {
        "gearsystem_paddle_sensitivity",
        "Paddle Sensitivity",
        NULL,
        "Set the sensitivity of the Paddle Control. Higher values result in faster paddle movement.",
        NULL,
        "input",
        {
            { "1",  NULL },
            { "2",  NULL },
            { "3",  NULL },
            { "4",  NULL },
            { "5",  NULL },
            { "6",  NULL },
            { "7",  NULL },
            { "8",  NULL },
            { "9",  NULL },
            { "10", NULL },
            { "11", NULL },
            { "12", NULL },
            { "13", NULL },
            { "14", NULL },
            { "15", NULL },
            { NULL, NULL },
        },
        "1"
    },

    { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us = {
    option_cats_us,
    option_defs_us
};

/*
 ********************************
 * Functions
 ********************************
 */

static void libretro_set_core_options(retro_environment_t environ_cb,
        bool *categories_supported)
{
    unsigned version = 0;

    if (!environ_cb || !categories_supported)
        return;

    *categories_supported = false;

    if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
        version = 0;

    if (version >= 2)
    {
        *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
                &options_us);
    }
    else
    {
        size_t i, j;
        size_t option_index  = 0;
        size_t num_options   = 0;
        struct retro_core_option_definition *option_v1_defs_us = NULL;
        struct retro_variable *variables   = NULL;
        char **values_buf                  = NULL;

        while (true)
        {
            if (option_defs_us[num_options].key)
                num_options++;
            else
                break;
        }

        if (version >= 1)
        {
            option_v1_defs_us = (struct retro_core_option_definition *)
                    calloc(num_options + 1, sizeof(struct retro_core_option_definition));

            for (i = 0; i < num_options; i++)
            {
                struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
                struct retro_core_option_value *option_values         = option_def_us->values;
                struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
                struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

                option_v1_def_us->key           = option_def_us->key;
                option_v1_def_us->desc          = option_def_us->desc;
                option_v1_def_us->info          = option_def_us->info;
                option_v1_def_us->default_value = option_def_us->default_value;

                while (option_values->value)
                {
                    option_v1_values->value = option_values->value;
                    option_v1_values->label = option_values->label;

                    option_values++;
                    option_v1_values++;
                }
            }

            environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
        }
        else
        {
            variables  = (struct retro_variable *)calloc(num_options + 1,
                    sizeof(struct retro_variable));
            values_buf = (char **)calloc(num_options, sizeof(char *));

            if (!variables || !values_buf)
                goto error;

            for (i = 0; i < num_options; i++)
            {
                const char *key                        = option_defs_us[i].key;
                const char *desc                       = option_defs_us[i].desc;
                const char *default_value              = option_defs_us[i].default_value;
                struct retro_core_option_value *values  = option_defs_us[i].values;
                size_t buf_len                         = 3;
                size_t default_index                   = 0;

                values_buf[i] = NULL;

                if (desc)
                {
                    size_t num_values = 0;

                    while (true)
                    {
                        if (values[num_values].value)
                        {
                            if (default_value)
                                if (strcmp(values[num_values].value, default_value) == 0)
                                    default_index = num_values;

                            buf_len += strlen(values[num_values].value);
                            num_values++;
                        }
                        else
                            break;
                    }

                    if (num_values > 0)
                    {
                        buf_len += num_values - 1;
                        buf_len += strlen(desc);

                        values_buf[i] = (char *)calloc(buf_len, sizeof(char));
                        if (!values_buf[i])
                            goto error;

                        strcpy(values_buf[i], desc);
                        strcat(values_buf[i], "; ");

                        strcat(values_buf[i], values[default_index].value);

                        for (j = 0; j < num_values; j++)
                        {
                            if (j != default_index)
                            {
                                strcat(values_buf[i], "|");
                                strcat(values_buf[i], values[j].value);
                            }
                        }
                    }
                }

                variables[option_index].key   = key;
                variables[option_index].value = values_buf[i];
                option_index++;
            }

            environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
        }

error:
        if (option_v1_defs_us)
        {
            free(option_v1_defs_us);
            option_v1_defs_us = NULL;
        }

        if (values_buf)
        {
            for (i = 0; i < num_options; i++)
            {
                if (values_buf[i])
                {
                    free(values_buf[i]);
                    values_buf[i] = NULL;
                }
            }

            free(values_buf);
            values_buf = NULL;
        }

        if (variables)
        {
            free(variables);
            variables = NULL;
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif
