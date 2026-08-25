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

#define GUI_DEBUG_GEARTOGEAR_IMPORT
#include "gui_debug_geartogear.h"

#include "imgui.h"
#include "GameGearIOPorts.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "gui_debug_widgets.h"
#include "config.h"
#include "emu.h"

static void serial_write_callback(u16 address, u8 value, void* user_data)
{
    GameGearIOPorts* ports = (GameGearIOPorts*)user_data;
    ports->DoOutput((u8)address, value);
}

static void draw_byte_value(const char* label, u8 value)
{
    ImGui::TextColored(violet, "%s", label);
    ImGui::SameLine();
    ImGui::Text("$%02X ", value);
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED ")", BYTE_TO_BINARY(value));
}

static void draw_metric(const char* label, u64 value)
{
    ImGui::TextColored(violet, "%s", label);
    ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)value);
}

static void draw_metric_pair(const char* label, u64 first, u64 second)
{
    ImGui::TextColored(violet, "%s", label);
    ImGui::SameLine();
    ImGui::TextColored(white, "%llu / %llu", (unsigned long long)first, (unsigned long long)second);
}

void gui_debug_window_geartogear_serial_registers(void)
{
    static const u32 baud_rates[4] = { 4800, 2400, 1200, 300 };

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(100, 90), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(242, 274), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game Gear Serial Registers", &config_debug.show_geartogear_serial_registers);

    GS_GearToGear_DebugState hardware =
        emu_geartogear_get_debug_state();
    GearToGearStatus link = emu_geartogear_get_status();
    GearsystemCore* core = emu_get_core();
    GameGearIOPorts* ports = core->GetGameGearIOPorts();
    u8 baud_index = (hardware.serial_control >> 6) & 0x03;

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(magenta, "REGISTERS:");

    EditableRegister8("PDR     ", " $01", 0x01, hardware.parallel_data, serial_write_callback, ports);
    EditableRegister8("DDR/NINT", " $02", 0x02, hardware.direction_nint, serial_write_callback, ports);
    EditableRegister8("TX DATA ", " $03", 0x03, hardware.tx_data, serial_write_callback, ports);
    draw_byte_value(" RX DATA        ", hardware.rx_data);
    EditableRegister8("SCTRL   ", " $05", 0x05, hardware.serial_control, serial_write_callback, ports);
    draw_byte_value(" SERIAL STATUS  ", hardware.serial_status);

    ImGui::TextColored(violet, " NATIVE GG      ");
    ImGui::SameLine();
    ImGui::TextColored(core->IsNativeGameGearMode() ? green : gray, "%s", core->IsNativeGameGearMode() ? "YES" : "NO");

    ImGui::TextColored(violet, " BAUD RATE      ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%u", baud_rates[baud_index]);
    ImGui::TextColored(violet, " SERIAL NMI     ");
    ImGui::SameLine();
    ImGui::TextColored((hardware.serial_control & 0x08) ? green : gray, "%s", (hardware.serial_control & 0x08) ? "ENABLED" : "DISABLED");
    ImGui::TextColored(violet, " TRANSMITTER    ");
    ImGui::SameLine();
    ImGui::TextColored((hardware.serial_control & 0x10) ? green : gray, "%s", (hardware.serial_control & 0x10) ? "ON" : "OFF");
    ImGui::TextColored(violet, " RECEIVER       ");
    ImGui::SameLine();
    ImGui::TextColored((hardware.serial_control & 0x20) ? green : gray, "%s", (hardware.serial_control & 0x20) ? "ON" : "OFF");
    ImGui::TextColored(violet, " TXFL/RXRD/FRER ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%d / %d / %d", hardware.tx_busy ? 1 : 0, hardware.rx_ready ? 1 : 0, hardware.frame_error ? 1 : 0);
    ImGui::TextColored(violet, " CABLE          ");
    ImGui::SameLine();
    ImGui::TextColored(link.cable_connected ? green : gray, "%s", link.cable_connected ? "CONNECTED" : "DISCONNECTED");

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_geartogear_serial_status(void)
{
    static const char* rx_states[4] =
    {
        "IDLE", "CONFIRM START", "DATA", "STOP"
    };

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(248, 478), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game Gear Serial Status", &config_debug.show_geartogear_serial_status);

    GS_GearToGear_DebugState hardware = emu_geartogear_get_debug_state();
    GearsystemCore* core = emu_get_core();
    const char* rx_state = hardware.rx_state < 4 ? rx_states[hardware.rx_state] : "UNKNOWN";

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(magenta, "TRANSMITTER:");

    ImGui::TextColored(violet, " STATE          ");
    ImGui::SameLine();
    ImGui::TextColored(hardware.tx_busy ? green : gray, "%s", hardware.tx_busy ? "ACTIVE" : "IDLE");
    draw_byte_value(" LATCH           ", hardware.tx_data);
    draw_byte_value(" FRAME DATA      ", hardware.tx_frame_data);
    ImGui::TextColored(violet, " LINE            ");
    ImGui::SameLine();
    ImGui::TextColored(hardware.tx_line ? green : yellow, "%d", hardware.tx_line ? 1 : 0);
    ImGui::TextColored(violet, " PHASE           ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%u / 10 bits", hardware.tx_phase);
    ImGui::TextColored(violet, " BIT CYCLES      ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%u", hardware.tx_bit_cycles);
    ImGui::TextColored(violet, " NEXT EDGE       ");
    ImGui::SameLine();

    if (hardware.tx_busy)
    {
        ImGui::TextColored(white, "%llu",
            (unsigned long long)hardware.tx_next_cycle);
    }
    else
        ImGui::TextColored(gray, "-");

    ImGui::Separator();
    ImGui::TextColored(magenta, "RECEIVER:");

    ImGui::TextColored(violet, " STATE           ");
    ImGui::SameLine();
    ImGui::TextColored(hardware.rx_state == 0 ? gray : green, "%s", rx_state);
    draw_byte_value(" SHIFT DATA      ", hardware.rx_shift);
    draw_byte_value(" DATA LATCH      ", hardware.rx_data);
    ImGui::TextColored(violet, " BIT INDEX       ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%u", hardware.rx_bit);
    ImGui::TextColored(violet, " BIT CYCLES      ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%u", hardware.rx_bit_cycles);
    ImGui::TextColored(violet, " NEXT SAMPLE     ");
    ImGui::SameLine();

    if (hardware.rx_state != 0)
    {
        ImGui::TextColored(white, "%llu",
            (unsigned long long)hardware.rx_next_cycle);
    }
    else
        ImGui::TextColored(gray, "-");

    ImGui::Separator();
    ImGui::TextColored(magenta, "PARALLEL / NMI:");

    ImGui::TextColored(violet, " PC6/SERIAL LATCH");
    ImGui::SameLine();
    ImGui::TextColored(white, "%d / %d", hardware.parallel_nmi ? 1 : 0, hardware.serial_nmi ? 1 : 0);
    ImGui::TextColored(violet, " NMI ASSERTED    ");
    ImGui::SameLine();
    ImGui::TextColored(hardware.nmi_asserted ? green : gray, "%s", hardware.nmi_asserted ? "YES" : "NO");
    ImGui::TextColored(violet, " NINT ARM/DELAY  ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%d / %u", hardware.nint_armed ? 1 : 0, hardware.nint_arm_delay);
    ImGui::TextColored(violet, " I/O CYCLE       ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)hardware.cycle);
    ImGui::TextColored(violet, " LINK CYCLE      ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)core->GetGearToGearCycles());

    ImGui::Separator();
    ImGui::TextColored(magenta, "PHYSICAL PINS:");

    ImGui::TextColored(violet, " LOCAL DRIVE/LEVEL");
    ImGui::SameLine();
    ImGui::TextColored(white, "%02X / %02X", hardware.local_state.drive_mask, hardware.local_state.levels);
    ImGui::TextColored(violet, " REMOTE MAPPED   ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%02X / %02X", hardware.remote_state.drive_mask, hardware.remote_state.levels);
    ImGui::TextColored(violet, " PINS/CONTENTION ");
    ImGui::SameLine();
    ImGui::TextColored(white, "%02X / %02X", hardware.resolved_pins, hardware.contention_mask);

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_geartogear_transport(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(300, 90), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 636), ImGuiCond_FirstUseEver);
    ImGui::Begin("Gear-to-Gear (Transport)", &config_debug.show_geartogear_transport);

    GearToGearStatus link = emu_geartogear_get_status();

    const char* mode = "DISABLED";
    if (link.mode == GearToGearModeConnected)
        mode = "JOINED";
    else if (link.mode == GearToGearModeFault)
        mode = "FAULT";

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(magenta, "SESSION:");

    ImGui::TextColored(violet, " CABLE           "); ImGui::SameLine();
    ImGui::TextColored(link.cable_connected ? green : red, "%s", link.cable_connected ? "CONNECTED" : "DISCONNECTED");
    ImGui::TextColored(violet, " STATUS          "); ImGui::SameLine();
    ImGui::TextColored(link.mode == GearToGearModeFault ? red : white, "%s", mode);
    ImGui::TextColored(violet, " SESSION         "); ImGui::SameLine();
    ImGui::TextColored(white, "%u", link.session);
    ImGui::TextColored(violet, " PEER            "); ImGui::SameLine();

    if (link.mode == GearToGearModeConnected)
    {
        ImGui::TextColored(white, "%d / %d", link.local_peer_id,
            link.peer_count);
    }
    else
        ImGui::TextColored(gray, "-");

    ImGui::TextColored(violet, " HARDWARE LOCAL  "); ImGui::SameLine();
    ImGui::TextColored(link.local_hardware_ready ? green : gray, "%s", link.local_hardware_ready ? "READY" : "INACTIVE");
    ImGui::TextColored(violet, " HARDWARE REMOTE "); ImGui::SameLine();
    ImGui::TextColored(link.remote_hardware_ready ? green : gray, "%s", link.remote_hardware_ready ? "READY" : "INACTIVE");
    ImGui::TextColored(violet, " PACING          "); ImGui::SameLine();

    if (link.mode == GearToGearModeConnected)
    {
        if (!link.cable_connected)
            ImGui::TextColored(white, "LOCAL AUDIO");
        else
        {
            ImGui::TextColored(link.pacing_peer ? green : cyan, "%s",
                link.pacing_peer ? "LEADER" : "FOLLOWER");
        }
    }
    else
        ImGui::TextColored(gray, "-");

    ImGui::TextColored(violet, " LOCAL ANCHOR    "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)link.local_anchor);
    ImGui::TextColored(violet, " BUS ANCHOR      "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)link.bus_anchor);
    ImGui::TextColored(violet, " BUS CYCLE       "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)link.bus_cycle);
    ImGui::TextColored(violet, " LINK CYCLE      "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu", (unsigned long long)emu_get_core()->GetGearToGearCycles());
    ImGui::TextColored(violet, " MAX LEAD        "); ImGui::SameLine();
    ImGui::TextColored(white, "%u cycles", GEARTOGEAR_MAX_LEAD_CYCLES);
    draw_metric_pair(" LOCAL PROG/PROM ", link.local_progress, link.local_promise);
    draw_metric_pair(" REMOTE PROG/PROM", link.remote_progress, link.remote_promise);
    ImGui::TextColored(violet, " REMOTE GEN      "); ImGui::SameLine();
    ImGui::TextColored(white, "%u", link.remote_generation);

    ImGui::Separator();
    ImGui::TextColored(magenta, "WIRE ACTIVITY:");

    draw_metric_pair(" EVENTS PUB/CON  ", link.events_published, link.events_consumed);
    draw_metric_pair(" BASELINE/OVERRUN", link.baseline_samples, link.state_ring_overruns);

    ImGui::Separator();
    ImGui::TextColored(magenta, "SYNCHRONIZATION:");

    draw_metric(" SYNC CALLS      ", link.sync_calls);
    draw_metric_pair(" FENCE CALL/WAIT ", link.fence_calls, link.fence_waits);
    draw_metric_pair(" FENCE TOTAL/MAX ", link.fence_wait_us, link.fence_wait_max_us);
    draw_metric_pair(" BARRIER # / us  ", link.barrier_waits, link.barrier_wait_us);
    draw_metric_pair(" MAX WAIT/GAP us ", link.barrier_wait_max_us, link.sync_gap_max_us);
    ImGui::TextColored(violet, " WAITS 1/10/50ms "); ImGui::SameLine();
    ImGui::TextColored(white, "%llu / %llu / %llu",
        (unsigned long long)link.barrier_wait_over_1ms,
        (unsigned long long)link.barrier_wait_over_10ms,
        (unsigned long long)link.barrier_wait_over_50ms);
    draw_metric_pair(" SPIN / SLEEP    ", link.spin_iterations, link.sleep_calls);

    ImGui::Separator();
    ImGui::TextColored(magenta, "RECOVERY:");

    draw_metric_pair(" DETACH / RECLAIM", link.peer_detaches, link.slot_reclaims);
    draw_metric(" ATTACHMENTS     ", link.attachments);
    draw_metric(" SEQ RETRIES     ", link.seqlock_retries);
    draw_metric(" MAX DETACH AGE us", link.peer_detach_max_age_us);
    draw_metric(" GAPS > 50ms     ", link.sync_gap_over_50ms);

    if (link.mode == GearToGearModeFault)
    {
        ImGui::Separator();
        ImGui::TextColored(red, "%s", link.last_error);
    }

    ImGui::Separator();

    if (ImGui::Button("RESET METRICS"))
        emu_geartogear_reset_metrics();

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}
