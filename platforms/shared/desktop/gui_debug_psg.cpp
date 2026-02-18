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

#define GUI_DEBUG_PSG_IMPORT
#include "gui_debug_psg.h"

#include "imgui.h"
#include "implot.h"
#include "fonts/IconsMaterialDesign.h"
#include "gearsystem.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "gui_debug_memeditor.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

static MemEditor mem_edit[6];
static float plot_x[32];
static float plot_y[32];
static bool exclusive_channel[6] = { false, false, false, false, false, false };
static float* wave_buffer_left = NULL;
static float* wave_buffer_right = NULL;

void gui_debug_psg_init(void)
{ /*
    wave_buffer_left = new float[GS_AUDIO_BUFFER_SIZE];
    wave_buffer_right = new float[GS_AUDIO_BUFFER_SIZE];

    GeargrafxCore* core = emu_get_core();
    HuC6280PSG* psg = core->GetAudio()->GetPSG();
    HuC6280PSG::HuC6280PSG_State* psg_state = psg->GetState();

    for (int channel = 0; channel < 6; channel++)
    {
        HuC6280PSG::HuC6280PSG_Channel* psg_channel = &psg_state->CHANNELS[channel];
        mem_edit[channel].Reset("WAVEFORM", psg_channel->wave_data, 32, 0, 1);
    }
        */
}

void gui_debug_psg_destroy(void)
{
    SafeDeleteArray(wave_buffer_left);
    SafeDeleteArray(wave_buffer_right);
}

void gui_debug_window_psg(void)
{
/*
    for (int i = 0; i < 6; i++)
    {
        mem_edit[i].SetGuiFont(gui_roboto_font);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(180, 45), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(444, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("PSG", &config_debug.show_psg);

    GeargrafxCore* core = emu_get_core();
    HuC6280PSG* psg = core->GetAudio()->GetPSG();
    HuC6280PSG::HuC6280PSG_State* psg_state = psg->GetState();

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "psg", true);

    ImGui::TextColored(cyan, "R00 "); ImGui::SameLine();
    ImGui::TextColored(violet, "CHANNEL SEL "); ImGui::SameLine();
    ImGui::TextColored(white, "%d", *psg_state->CHANNEL_SELECT);

    ImGui::TextColored(cyan, "R01 "); ImGui::SameLine();
    ImGui::TextColored(violet, "MAIN AMPL   "); ImGui::SameLine();
    ImGui::TextColored(white, "%02X", *psg_state->MAIN_AMPLITUDE);

    ImGui::NextColumn();

    ImGui::TextColored(cyan, "R08 "); ImGui::SameLine();
    ImGui::TextColored(violet, "LFO FREQ    "); ImGui::SameLine();
    ImGui::TextColored(white, "%02X", *psg_state->LFO_FREQUENCY);

    ImGui::TextColored(cyan, "R09 "); ImGui::SameLine();
    ImGui::TextColored(violet, "LFO CTRL    "); ImGui::SameLine();
    ImGui::TextColored(white, "%02X", *psg_state->LFO_CONTROL);

    ImGui::Columns(1);
    ImGui::NewLine();

    ImGui::PopFont();

    if (ImGui::BeginTabBar("##memory_tabs", ImGuiTabBarFlags_None))
    {
        for (int channel = 0; channel < 6; channel++)
        {
            char tab_name[32];
            snprintf(tab_name, 32, "CH %d", channel);

            if (ImGui::BeginTabItem(tab_name))
            {
                ImGui::PushFont(gui_default_font);

                HuC6280PSG::HuC6280PSG_Channel* psg_channel = &psg_state->CHANNELS[channel];

                ImGui::Columns(2, "channels", false);

                ImGui::BeginTable("##table", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX);

                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Text, psg_state->CHANNELS[channel].mute ? mid_gray : white);
                ImGui::PushFont(gui_material_icons_font);

                char label[32];
                snprintf(label, 32, "%s##mute%d", psg_state->CHANNELS[channel].mute ? ICON_MD_MUSIC_OFF : ICON_MD_MUSIC_NOTE, channel);
                if (ImGui::Button(label))
                {
                    for (int i = 0; i < 6; i++)
                        exclusive_channel[i] = false;
                    psg_state->CHANNELS[channel].mute = !psg_state->CHANNELS[channel].mute;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Mute Channel");

                snprintf(label, 32, "%s##exc%d", ICON_MD_STAR, channel);

                ImGui::PushStyleColor(ImGuiCol_Text, exclusive_channel[channel] ? yellow : white);
                if (ImGui::Button(label))
                {
                    exclusive_channel[channel] = !exclusive_channel[channel];
                    psg_state->CHANNELS[channel].mute = false;
                    for (int i = 0; i < 6; i++)
                    {
                        if (i != channel)
                        {
                            exclusive_channel[i] = false;
                            psg_state->CHANNELS[i].mute = exclusive_channel[channel] ? true : false;
                        }
                    }
                }
                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    ImGui::SetTooltip("Solo Channel");
                }
                ImGui::PopFont();
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();

                ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(1, 1));

                int trigger_left = 0;
                int trigger_right = 0;
                int data_size = (*psg_state->FRAME_SAMPLES) / 2;

                for (int i = 0; i < data_size; i++)
                {
                    wave_buffer_left[i] = (float)(psg_channel->output[i * 2]) / 32768.0f * 8.0f;
                    wave_buffer_right[i] = (float)(psg_channel->output[(i * 2) + 1]) / 32768.0f * 8.0f;
                }

                for (int i = 100; i < data_size; ++i)
                {
                    if (wave_buffer_left[i - 1] < 0.0f && wave_buffer_left[i] >= 0.0f)
                    {
                        trigger_left = i;
                        break;
                    }
                }

                for (int i = 100; i < data_size; ++i)
                {
                    if (wave_buffer_right[i - 1] < 0.0f && wave_buffer_right[i] >= 0.0f)
                    {
                        trigger_right = i;
                        break;
                    }
                }

                int half_window_size = 100;
                int x_min_left = MAX(0, trigger_left - half_window_size);
                int x_max_left = MIN(data_size, trigger_left + half_window_size);

                ImPlotAxisFlags flags = ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoTickMarks;

                if (ImPlot::BeginPlot("Left wave", ImVec2(80, 50), ImPlotFlags_CanvasOnly))
                {
                    ImPlot::SetupAxes("x", "y", flags, flags);
                    ImPlot::SetupAxesLimits(x_min_left, x_max_left, -1.0f, 1.0f, ImPlotCond_Always);
                    ImPlot::SetNextLineStyle(white, 1.0f);
                    ImPlot::PlotLine("Wave", wave_buffer_left, data_size);
                    ImPlot::EndPlot();
                }

                ImGui::SameLine();

                int x_min_right = MAX(0, trigger_right - half_window_size);
                int x_max_right = MIN(data_size, trigger_right + half_window_size);

                if (ImPlot::BeginPlot("Right wave", ImVec2(80, 50), ImPlotFlags_CanvasOnly))
                {
                    ImPlot::SetupAxes("x", "y", flags, flags);
                    ImPlot::SetupAxesLimits(x_min_right, x_max_right, -1.0f, 1.0f, ImPlotCond_Always);
                    ImPlot::SetNextLineStyle(white, 1.0f);
                    ImPlot::PlotLine("Wave", wave_buffer_right, data_size);
                    ImPlot::EndPlot();
                }

                ImGui::EndTable();

                ImGui::NewLine();

                ImGui::TextColored(cyan, "DDA "); ImGui::SameLine();
                ImGui::TextColored(violet, "DIRECT D/A  "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->dda);

                ImGui::TextColored(cyan, "R02 "); ImGui::SameLine();
                ImGui::TextColored(violet, "FREQ LOW    "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->frequency & 0xFF);

                ImGui::TextColored(cyan, "R03 "); ImGui::SameLine();
                ImGui::TextColored(violet, "FREQ HI     "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->frequency >> 8);

                ImGui::TextColored(cyan, "R04 "); ImGui::SameLine();
                ImGui::TextColored(violet, "CONTROL     "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->control);

                ImGui::TextColored(cyan, "R05 "); ImGui::SameLine();
                ImGui::TextColored(violet, "AMPLITUDE   "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->amplitude);

                ImGui::TextColored(cyan, "R06 "); ImGui::SameLine();
                ImGui::TextColored(violet, "WAVE        "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->wave);
                ImGui::TextColored(cyan, "    "); ImGui::SameLine();
                ImGui::TextColored(violet, "WAVE INDEX  "); ImGui::SameLine();
                ImGui::TextColored(white, "%02X", psg_channel->wave_index);

                if (channel > 3)
                {
                    ImGui::TextColored(cyan, "R07 "); ImGui::SameLine();
                    ImGui::TextColored(violet, "NOISE CTRL  "); ImGui::SameLine();
                    ImGui::TextColored(white, "%02X", psg_channel->noise_control);
                }

                ImGui::NextColumn();

                for (int i = 0; i < 32; i++)
                {
                    plot_x[i] = (float)i;
                    plot_y[i] = (float)psg_channel->wave_data[i];
                }

                if (ImPlot::BeginPlot("Wave data", ImVec2(200, 200), ImPlotFlags_CanvasOnly))
                {
                    ImPlotAxisFlags flags_waveform = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoTickMarks;
                    ImPlot::SetupAxes("x", "y", flags_waveform, flags_waveform);
                    ImPlot::SetupAxesLimits(-1,32, -1,32);
                    ImPlot::SetupAxisTicks(ImAxis_X1, 0, 32, 33, NULL, false);
                    ImPlot::SetupAxisTicks(ImAxis_Y1, 0, 32, 33, NULL, false);
                    ImPlot::SetNextLineStyle(orange, 3.0f);
                    ImPlot::PlotLine("waveform", plot_x, plot_y, 32);
                    ImPlot::EndPlot();
                }

                ImPlot::PopStyleVar();

                ImGui::NewLine();

                ImGui::Columns(1);

                ImGui::BeginChild("##waveform", ImVec2(ImGui::GetWindowWidth() - 20, 60), true);

                mem_edit[channel].Draw(false, false, false, false);

                ImGui::EndChild();

                ImGui::PopFont();

                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
    */
}