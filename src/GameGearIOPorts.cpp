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

#include "GameGearIOPorts.h"

static const u32 kGearToGearBitCycles[4] =
{
    746, 1492, 2983, 11932
};

static const u32 kGearToGearHalfBitCycles[4] =
{
    373, 746, 1492, 5966
};

GameGearIOPorts::GameGearIOPorts(Audio* pAudio, Video* pVideo, Input* pInput,
    Cartridge* pCartridge, Memory* pMemory, Processor* pProcessor)
{
    m_pAudio = pAudio;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_pMemory = pMemory;
    m_pProcessor = pProcessor;
    InitPointer(m_pTraceLogger);
    m_geartogear_publish_callback = NULL;
    m_geartogear_sample_callback = NULL;
    m_geartogear_poll_callback = NULL;
    m_geartogear_fence_callback = NULL;
    m_geartogear_sync_callback = NULL;
    m_geartogear_user_data = NULL;
    m_geartogear_transport_active = false;
    m_geartogear_cable_connected = false;
    m_geartogear_cycle = 0;
    m_geartogear_next_sync_cycle = 0;
    m_geartogear_has_published_state = false;
    m_geartogear_has_pending_remote_event = false;
    Reset();
}

GameGearIOPorts::~GameGearIOPorts()
{
}

void GameGearIOPorts::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void GameGearIOPorts::LogInputReadEvent(u8 port, u8 raw, u8 effective, u8 player)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    GS_Trace_Entry e = {};
    e.type = TRACE_INPUT;
    e.input.event = TRACE_INPUT_READ;
    e.input.port = port;
    e.input.raw = raw;
    e.input.effective = effective;
    e.input.control = m_Port3F;
    e.input.player = player;
    e.input.device = 0;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(port);
    UNUSED(raw);
    UNUSED(effective);
    UNUSED(player);
#endif
}

void GameGearIOPorts::LogIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    if (event == TRACE_IO_COUNTER_LATCH)
        effective = m_pVideo->GetHCounter();
    GS_Trace_Entry e = {};
    e.type = TRACE_IO;
    e.io.event = event;
    e.io.port = port;
    e.io.raw = raw;
    e.io.effective = effective;
    e.io.previous = previous;
    e.io.auxiliary = auxiliary;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(port);
    UNUSED(raw);
    UNUSED(effective);
    UNUSED(previous);
    UNUSED(auxiliary);
#endif
}

void GameGearIOPorts::LogGearToGearEvent(u8 event, u8 data)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    GS_Trace_Entry e = {};
    e.type = TRACE_GEARTOGEAR;
    e.geartogear.event = event;
    e.geartogear.data = data;
    e.geartogear.control = m_Ports[5];
    e.geartogear.link_cycle = m_geartogear_cycle;

    if (event == TRACE_GEARTOGEAR_TX_START ||
        event == TRACE_GEARTOGEAR_TX_END ||
        event == TRACE_GEARTOGEAR_TX_ABORT)
    {
        e.geartogear.bit_cycles = m_geartogear_tx.bit_cycles;
    }
    else if (event == TRACE_GEARTOGEAR_RX_START ||
        event == TRACE_GEARTOGEAR_RX_END ||
        event == TRACE_GEARTOGEAR_RX_ERROR)
    {
        e.geartogear.bit_cycles = m_geartogear_rx.bit_cycles;
    }

    e.geartogear.flags =
        (m_geartogear_nmi.parallel_latch ? TRACE_GEARTOGEAR_FLAG_PARALLEL_NMI : 0) |
        (m_geartogear_nmi.serial_latch ? TRACE_GEARTOGEAR_FLAG_SERIAL_NMI : 0) |
        (m_geartogear_nmi.output_asserted ? TRACE_GEARTOGEAR_FLAG_NMI_ASSERTED : 0) |
        (m_geartogear_nmi.parallel_armed ? TRACE_GEARTOGEAR_FLAG_NINT_ARMED : 0);
    e.geartogear.local_drive_mask = m_geartogear_local_state.drive_mask;
    e.geartogear.local_levels = m_geartogear_local_state.levels;
    e.geartogear.remote_drive_mask = m_geartogear_remote_state.drive_mask;
    e.geartogear.remote_levels = m_geartogear_remote_state.levels;
    e.geartogear.resolved_pins = ResolveGearToGearPins();
    e.geartogear.contention_mask = GetGearToGearContentionMask();
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(event);
    UNUSED(data);
#endif
}

void GameGearIOPorts::Reset()
{
    m_Port3F = 0xFF;
    m_Ports[0] = 0xC0;
    m_Ports[1] = 0x7F;
    m_Ports[2] = 0xFF;
    m_Ports[3] = 0x00;
    m_Ports[4] = 0xFF;
    m_Ports[5] = 0x00;
    ResetGearToGearHardware();
}

void GameGearIOPorts::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_Port3F), sizeof(m_Port3F));
    stream.write(reinterpret_cast<const char*> (m_Ports), sizeof(m_Ports));
    SaveGearToGearState(stream);
}

void GameGearIOPorts::LoadState(std::istream& stream, int version)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_Port3F), sizeof(m_Port3F));
    stream.read(reinterpret_cast<char*> (m_Ports), sizeof(m_Ports));

    if (version >= 107)
        LoadGearToGearState(stream);
    else
        ResetGearToGearHardware();

    m_geartogear_remote_state.drive_mask = 0;
    m_geartogear_remote_state.levels = 0x7F;
    m_geartogear_has_pending_remote_event = false;
    m_geartogear_local_state = ComputeLocalWireState();
    m_geartogear_has_published_state = false;
}

void GameGearIOPorts::SetGearToGearCallbacks(
    GS_GearToGear_Publish_Callback publish_callback,
    GS_GearToGear_Sample_Callback sample_callback,
    GS_GearToGear_Poll_Callback poll_callback,
    GS_GearToGear_Fence_Callback fence_callback,
    GS_GearToGear_Sync_Callback sync_callback,
    void* user_data)
{
    m_geartogear_publish_callback = publish_callback;
    m_geartogear_sample_callback = sample_callback;
    m_geartogear_poll_callback = poll_callback;
    m_geartogear_fence_callback = fence_callback;
    m_geartogear_sync_callback = sync_callback;
    m_geartogear_user_data = user_data;
}

void GameGearIOPorts::SetGearToGearTransportActive(bool active, u64 cycle)
{
    m_geartogear_cycle = cycle;
    m_geartogear_transport_active = active;
    m_geartogear_next_sync_cycle = cycle;
    m_geartogear_has_published_state = false;

    if (active)
        RefreshLocalWireState(cycle, true);
}

void GameGearIOPorts::SetGearToGearCableConnected(bool connected, u64 cycle)
{
    m_geartogear_cycle = cycle;
    m_geartogear_has_pending_remote_event = false;

    if (connected)
    {
        GS_GearToGear_WireState state;
        state.drive_mask = 0;
        state.levels = 0x7F;

        if (m_geartogear_sample_callback)
            m_geartogear_sample_callback(cycle, &state, m_geartogear_user_data);

        m_geartogear_cable_connected = true;
        ApplyRemoteWireState(state, cycle, false);
        m_geartogear_nmi.previous_pc6 = ResolveGearToGearPin(6);
    }
    else
    {
        bool old_pc6 = ResolveGearToGearPin(6);
        bool remote_changed = m_geartogear_remote_state.drive_mask != 0 || m_geartogear_remote_state.levels != 0x7F;
        m_geartogear_cable_connected = false;
        m_geartogear_remote_state.drive_mask = 0;
        m_geartogear_remote_state.levels = 0x7F;
        AbortGearToGearRx();

        bool new_pc6 = ResolveGearToGearPin(6);

        if (old_pc6 != new_pc6)
            HandleGearToGearPC6Edge(old_pc6, new_pc6);
        else
            m_geartogear_nmi.previous_pc6 = new_pc6;

        if (remote_changed)
            TraceGearToGearEvent(TRACE_GEARTOGEAR_REMOTE_WIRE);
    }

    TraceGearToGearEvent(TRACE_GEARTOGEAR_CABLE, connected ? 1 : 0);
}

void GameGearIOPorts::BeginInstruction(u64 cycle)
{
    AdvanceGearToGearTo(cycle);
    m_geartogear_cycle = cycle;
}

void GameGearIOPorts::EndInstruction(u64 cycle)
{
    AdvanceGearToGearTo(cycle);

    if (m_geartogear_nmi.parallel_arm_delay > 0)
    {
        m_geartogear_nmi.parallel_arm_delay--;

        if (m_geartogear_nmi.parallel_arm_delay == 0 && (m_Ports[2] & 0x80) == 0)
        {
            m_geartogear_nmi.parallel_armed = true;
        }
    }

    if (m_geartogear_cable_connected && m_geartogear_sync_callback && cycle >= m_geartogear_next_sync_cycle)
    {
        m_geartogear_sync_callback(cycle, GEARTOGEAR_MAX_LEAD_CYCLES, m_geartogear_user_data);
        m_geartogear_next_sync_cycle = cycle + GEARTOGEAR_MAX_SYNC_CYCLES;
    }
}

void GameGearIOPorts::RebaseGearToGear(u64 cycle)
{
    m_geartogear_cycle = cycle;
    m_geartogear_next_sync_cycle = cycle;
    m_geartogear_has_pending_remote_event = false;

    GS_GearToGear_WireState remote;
    remote.drive_mask = 0;
    remote.levels = 0x7F;

    if (m_geartogear_cable_connected && m_geartogear_sample_callback)
        m_geartogear_sample_callback(cycle, &remote, m_geartogear_user_data);

    ApplyRemoteWireState(remote, cycle, false);
    m_geartogear_nmi.previous_pc6 = ResolveGearToGearPin(6);
    RefreshLocalWireState(cycle, m_geartogear_transport_active, false);
}

bool GameGearIOPorts::IsGearToGearCableConnected() const
{
    return m_geartogear_cable_connected;
}

GS_GearToGear_WireState GameGearIOPorts::GetGearToGearWireState() const
{
    return m_geartogear_local_state;
}

u8 GameGearIOPorts::GetGearToGearResolvedPins() const
{
    return ResolveGearToGearPins();
}

u8 GameGearIOPorts::GetGearToGearContentionMask() const
{
    if (!m_geartogear_cable_connected)
        return 0;

    u8 both = m_geartogear_local_state.drive_mask &
        m_geartogear_remote_state.drive_mask;
    u8 different = m_geartogear_local_state.levels ^
        m_geartogear_remote_state.levels;
    return both & different & 0x7F;
}

GS_GearToGear_DebugState GameGearIOPorts::GetGearToGearDebugState() const
{
    GS_GearToGear_DebugState state = {};
    state.parallel_data = m_Ports[1];
    state.direction_nint = m_Ports[2];
    state.tx_data = m_Ports[3];
    state.rx_data = m_Ports[4];
    state.serial_control = m_Ports[5];
    state.serial_status = ReadGearToGearStatus();
    state.local_state = m_geartogear_local_state;
    state.remote_state = m_geartogear_remote_state;
    state.resolved_pins = ResolveGearToGearPins();
    state.contention_mask = GetGearToGearContentionMask();
    state.tx_busy = m_geartogear_tx.busy;
    state.tx_line = m_geartogear_tx.line;
    state.tx_frame_data = m_geartogear_tx.data;
    state.tx_phase = m_geartogear_tx.phase;
    state.tx_bit_cycles = m_geartogear_tx.bit_cycles;
    state.tx_next_cycle = m_geartogear_tx.next_cycle;
    state.rx_state = (u8)m_geartogear_rx.state;
    state.rx_shift = m_geartogear_rx.shift;
    state.rx_bit = m_geartogear_rx.bit;
    state.rx_bit_cycles = m_geartogear_rx.bit_cycles;
    state.rx_next_cycle = m_geartogear_rx.next_cycle;
    state.rx_ready = m_geartogear_rx.ready;
    state.frame_error = m_geartogear_rx.frame_error;
    state.parallel_nmi = m_geartogear_nmi.parallel_latch;
    state.serial_nmi = m_geartogear_nmi.serial_latch;
    state.nmi_asserted = m_geartogear_nmi.output_asserted;
    state.nint_armed = m_geartogear_nmi.parallel_armed;
    state.nint_arm_delay = m_geartogear_nmi.parallel_arm_delay;
    state.cycle = m_geartogear_cycle;
    return state;
}

void GameGearIOPorts::ResetGearToGearHardware()
{
    m_geartogear_local_state.drive_mask = 0;
    m_geartogear_local_state.levels = 0x7F;
    m_geartogear_remote_state.drive_mask = 0;
    m_geartogear_remote_state.levels = 0x7F;
    m_geartogear_last_published_state.drive_mask = 0;
    m_geartogear_last_published_state.levels = 0x7F;
    m_geartogear_has_published_state = false;
    m_geartogear_has_pending_remote_event = false;

    m_geartogear_tx.busy = false;
    m_geartogear_tx.line = true;
    m_geartogear_tx.data = 0;
    m_geartogear_tx.phase = 0;
    m_geartogear_tx.bit_cycles = kGearToGearBitCycles[0];
    m_geartogear_tx.next_cycle = 0;

    m_geartogear_rx.state = GearToGearRxState::Idle;
    m_geartogear_rx.ready = false;
    m_geartogear_rx.frame_error = false;
    m_geartogear_rx.shift = 0;
    m_geartogear_rx.bit = 0;
    m_geartogear_rx.bit_cycles = kGearToGearBitCycles[0];
    m_geartogear_rx.half_bit_cycles = kGearToGearHalfBitCycles[0];
    m_geartogear_rx.next_cycle = 0;

    m_geartogear_nmi.parallel_latch = false;
    m_geartogear_nmi.serial_latch = false;
    m_geartogear_nmi.output_asserted = false;
    m_geartogear_nmi.parallel_armed = false;
    m_geartogear_nmi.parallel_arm_delay = 0;
    m_geartogear_nmi.previous_pc6 = true;
    m_geartogear_last_nmi_trace_state = 0;

    m_geartogear_local_state = ComputeLocalWireState();
}

GS_GearToGear_WireState GameGearIOPorts::ComputeLocalWireState() const
{
    GS_GearToGear_WireState state;
    state.drive_mask = 0;
    state.levels = m_Ports[1] & 0x7F;

    for (int bit = 0; bit < 7; bit++)
    {
        if ((m_Ports[2] & (1 << bit)) == 0)
            state.drive_mask |= (u8)(1 << bit);
    }

    if (m_Ports[5] & 0x10)
    {
        state.drive_mask |= 0x10;
        if (m_geartogear_tx.line)
            state.levels |= 0x10;
        else
            state.levels &= (u8)~0x10;
    }

    if (m_Ports[5] & 0x20)
        state.drive_mask &= (u8)~0x20;

    state.drive_mask &= 0x7F;
    state.levels &= 0x7F;
    return state;
}

void GameGearIOPorts::RefreshLocalWireState(u64 cycle, bool force_publish, bool detect_rx_edge)
{
    GS_GearToGear_WireState previous_state = m_geartogear_local_state;
    bool old_pc5 = ResolveGearToGearPin(5);
    bool old_pc6 = ResolveGearToGearPin(6);

    m_geartogear_cycle = cycle;
    m_geartogear_local_state = ComputeLocalWireState();

    bool new_pc5 = ResolveGearToGearPin(5);
    bool new_pc6 = ResolveGearToGearPin(6);

    if (detect_rx_edge && old_pc5 != new_pc5)
        HandleGearToGearRxEdge(old_pc5, new_pc5, cycle);

    if (old_pc6 != new_pc6)
        HandleGearToGearPC6Edge(old_pc6, new_pc6);
    else
        m_geartogear_nmi.previous_pc6 = new_pc6;

    bool wire_changed = previous_state.drive_mask != m_geartogear_local_state.drive_mask ||
        previous_state.levels != m_geartogear_local_state.levels;

    if (wire_changed)
        TraceGearToGearEvent(TRACE_GEARTOGEAR_LOCAL_WIRE);

    bool changed = !m_geartogear_has_published_state ||
        m_geartogear_local_state.drive_mask != m_geartogear_last_published_state.drive_mask ||
        m_geartogear_local_state.levels != m_geartogear_last_published_state.levels;

    if (m_geartogear_transport_active && m_geartogear_publish_callback && (force_publish || changed))
    {
        m_geartogear_publish_callback(cycle, &m_geartogear_local_state, m_geartogear_user_data);
        m_geartogear_last_published_state = m_geartogear_local_state;
        m_geartogear_has_published_state = true;
    }
}

bool GameGearIOPorts::ResolveGearToGearPin(int bit) const
{
    u8 mask = (u8)(1 << bit);
    bool local_drives = (m_geartogear_local_state.drive_mask & mask) != 0;
    bool remote_drives = m_geartogear_cable_connected && (m_geartogear_remote_state.drive_mask & mask) != 0;
    bool local_level = (m_geartogear_local_state.levels & mask) != 0;
    bool remote_level = (m_geartogear_remote_state.levels & mask) != 0;

    if (local_drives && remote_drives)
        return local_level && remote_level;
    if (local_drives)
        return local_level;
    if (remote_drives)
        return remote_level;
    return true;
}

u8 GameGearIOPorts::ResolveGearToGearPins() const
{
    u8 pins = 0;
    for (int bit = 0; bit < 7; bit++)
    {
        if (ResolveGearToGearPin(bit))
            pins |= (u8)(1 << bit);
    }
    return pins;
}

void GameGearIOPorts::ApplyRemoteWireState(const GS_GearToGear_WireState& state, u64 cycle, bool detect_edges)
{
    GS_GearToGear_WireState previous_state = m_geartogear_remote_state;
    bool old_pc5 = ResolveGearToGearPin(5);
    bool old_pc6 = ResolveGearToGearPin(6);

    m_geartogear_cycle = cycle;
    m_geartogear_remote_state.drive_mask = state.drive_mask & 0x7F;
    m_geartogear_remote_state.levels = state.levels & 0x7F;

    bool new_pc5 = ResolveGearToGearPin(5);
    bool new_pc6 = ResolveGearToGearPin(6);

    if (detect_edges && old_pc5 != new_pc5)
        HandleGearToGearRxEdge(old_pc5, new_pc5, cycle);

    if (detect_edges && old_pc6 != new_pc6)
        HandleGearToGearPC6Edge(old_pc6, new_pc6);
    else
        m_geartogear_nmi.previous_pc6 = new_pc6;

    if (previous_state.drive_mask != m_geartogear_remote_state.drive_mask ||
        previous_state.levels != m_geartogear_remote_state.levels)
    {
        TraceGearToGearEvent(TRACE_GEARTOGEAR_REMOTE_WIRE);
    }
}

void GameGearIOPorts::HandleGearToGearRxEdge(bool old_level, bool new_level, u64 cycle)
{
    if ((m_Ports[5] & 0x20) == 0)
        return;

    if (old_level && !new_level && m_geartogear_rx.state == GearToGearRxState::Idle)
    {
        u8 baud = (m_Ports[5] >> 6) & 0x03;
        m_geartogear_rx.bit_cycles = kGearToGearBitCycles[baud];
        m_geartogear_rx.half_bit_cycles = kGearToGearHalfBitCycles[baud];
        m_geartogear_rx.state = GearToGearRxState::ConfirmStart;
        m_geartogear_rx.next_cycle = cycle + m_geartogear_rx.half_bit_cycles;
    }
}

void GameGearIOPorts::HandleGearToGearPC6Edge(bool old_level, bool new_level)
{
    m_geartogear_nmi.previous_pc6 = new_level;
    bool pc6_input = (m_Ports[2] & 0x40) != 0;

    if (old_level && !new_level && pc6_input && m_geartogear_nmi.parallel_armed)
    {
        m_geartogear_nmi.parallel_latch = true;
        UpdateGearToGearNMI();
    }
}

void GameGearIOPorts::UpdateGearToGearNMI()
{
    bool asserted = m_geartogear_nmi.parallel_latch || m_geartogear_nmi.serial_latch;

    if (asserted && !m_geartogear_nmi.output_asserted && m_pProcessor)
        m_pProcessor->RequestNMI();

    m_geartogear_nmi.output_asserted = asserted;

    u8 state = (m_geartogear_nmi.parallel_latch ? 0x01 : 0) |
        (m_geartogear_nmi.serial_latch ? 0x02 : 0) |
        (m_geartogear_nmi.output_asserted ? 0x04 : 0);

    if (state != m_geartogear_last_nmi_trace_state)
    {
        m_geartogear_last_nmi_trace_state = state;
        TraceGearToGearEvent(TRACE_GEARTOGEAR_NMI);
    }
}

u8 GameGearIOPorts::ReadGearToGearPort(u8 port)
{
    switch (port)
    {
        case 0x01:
            FenceGearToGearRead();
            return (m_Ports[1] & 0x80) | ResolveGearToGearPins();
        case 0x02:
            return m_Ports[2];
        case 0x03:
            return m_Ports[3];
        case 0x04:
        {
            FenceGearToGearRead();
            u8 value = m_Ports[4];
            m_geartogear_rx.ready = false;
            m_geartogear_rx.frame_error = false;
            m_geartogear_nmi.serial_latch = false;
            UpdateGearToGearNMI();
            return value;
        }
        case 0x05:
            FenceGearToGearRead();
            return ReadGearToGearStatus();
        default:
            return 0xFF;
    }
}

void GameGearIOPorts::WriteGearToGearPort(u8 port, u8 value)
{
    switch (port)
    {
        case 0x01:
            m_Ports[1] = value;
            RefreshLocalWireState(m_geartogear_cycle, false);
            break;
        case 0x02:
            m_Ports[2] = value;

            if (value & 0x80)
            {
                m_geartogear_nmi.parallel_latch = false;
                m_geartogear_nmi.parallel_armed = false;
                m_geartogear_nmi.parallel_arm_delay = 0;
                UpdateGearToGearNMI();
            }
            else
            {
                m_geartogear_nmi.parallel_armed = false;
                m_geartogear_nmi.parallel_arm_delay = 2;
            }

            RefreshLocalWireState(m_geartogear_cycle, false);
            break;
        case 0x03:
            m_Ports[3] = value;
            StartGearToGearTx(value, m_geartogear_cycle);
            break;
        case 0x04:
            break;
        case 0x05:
        {
            u8 old_value = m_Ports[5];
            m_Ports[5] = value & 0xF8;

            if ((old_value & 0x08) && !(m_Ports[5] & 0x08))
            {
                m_geartogear_nmi.serial_latch = false;
                UpdateGearToGearNMI();
            }

            if ((old_value & 0x10) && !(m_Ports[5] & 0x10))
                AbortGearToGearTx(m_geartogear_cycle);

            if ((old_value & 0x20) && !(m_Ports[5] & 0x20))
            {
                AbortGearToGearRx();
                m_geartogear_rx.ready = false;
                m_geartogear_rx.frame_error = false;
                m_geartogear_nmi.serial_latch = false;
                UpdateGearToGearNMI();
            }

            bool ron_enabled = !(old_value & 0x20) &&
                (m_Ports[5] & 0x20);

            RefreshLocalWireState(m_geartogear_cycle, false, !ron_enabled);
            break;
        }
        default:
            break;
    }
}

void GameGearIOPorts::FenceGearToGearRead()
{
    if (m_geartogear_cable_connected && m_geartogear_fence_callback)
    {
        m_geartogear_fence_callback(m_geartogear_cycle,m_geartogear_user_data);
    }

    AdvanceGearToGearTo(m_geartogear_cycle);
}

u8 GameGearIOPorts::ReadGearToGearStatus() const
{
    u8 value = m_Ports[5] & 0xF8;
    value |= m_geartogear_tx.busy ? 0x01 : 0x00;
    value |= m_geartogear_rx.ready ? 0x02 : 0x00;
    value |= m_geartogear_rx.frame_error ? 0x04 : 0x00;
    return value;
}

void GameGearIOPorts::StartGearToGearTx(u8 value, u64 cycle)
{
    if ((m_Ports[5] & 0x10) == 0 || m_geartogear_tx.busy)
        return;

    u8 baud = (m_Ports[5] >> 6) & 0x03;
    m_geartogear_tx.busy = true;
    m_geartogear_tx.data = value;
    m_geartogear_tx.phase = 0;
    m_geartogear_tx.bit_cycles = kGearToGearBitCycles[baud];
    m_geartogear_tx.line = false;
    m_geartogear_tx.next_cycle = cycle + m_geartogear_tx.bit_cycles;
    RefreshLocalWireState(cycle, false);
    TraceGearToGearEvent(TRACE_GEARTOGEAR_TX_START, value);
}

void GameGearIOPorts::AbortGearToGearTx(u64 cycle)
{
    bool was_busy = m_geartogear_tx.busy;
    u8 data = m_geartogear_tx.data;
    m_geartogear_tx.busy = false;
    m_geartogear_tx.line = true;
    m_geartogear_tx.phase = 0;
    m_geartogear_tx.next_cycle = cycle;
    if (was_busy)
        TraceGearToGearEvent(TRACE_GEARTOGEAR_TX_ABORT, data);
}

void GameGearIOPorts::ProcessGearToGearTxBoundary()
{
    u64 cycle = m_geartogear_tx.next_cycle;
    bool completed = false;

    if (m_geartogear_tx.phase < 8)
    {
        m_geartogear_tx.line = (m_geartogear_tx.data & (1 << m_geartogear_tx.phase)) != 0;
        m_geartogear_tx.phase++;
        m_geartogear_tx.next_cycle += m_geartogear_tx.bit_cycles;
    }
    else if (m_geartogear_tx.phase == 8)
    {
        m_geartogear_tx.line = true;
        m_geartogear_tx.phase = 9;
        m_geartogear_tx.next_cycle += m_geartogear_tx.bit_cycles;
    }
    else
    {
        m_geartogear_tx.busy = false;
        m_geartogear_tx.line = true;
        m_geartogear_tx.phase = 10;
        m_geartogear_tx.next_cycle = cycle;
        completed = true;
    }

    RefreshLocalWireState(cycle, false);
    if (completed)
    {
        TraceGearToGearEvent(TRACE_GEARTOGEAR_TX_END,
            m_geartogear_tx.data);
    }
}

void GameGearIOPorts::AbortGearToGearRx()
{
    m_geartogear_rx.state = GearToGearRxState::Idle;
    m_geartogear_rx.shift = 0;
    m_geartogear_rx.bit = 0;
    m_geartogear_rx.next_cycle = 0;
}

void GameGearIOPorts::ProcessGearToGearRxSample()
{
    bool line = ResolveGearToGearPin(5);

    switch (m_geartogear_rx.state)
    {
        case GearToGearRxState::ConfirmStart:
            if (line)
                AbortGearToGearRx();
            else
            {
                m_geartogear_rx.state = GearToGearRxState::Data;
                m_geartogear_rx.bit = 0;
                m_geartogear_rx.shift = 0;
                m_geartogear_rx.next_cycle += m_geartogear_rx.bit_cycles;
                TraceGearToGearEvent(TRACE_GEARTOGEAR_RX_START);
            }
            break;
        case GearToGearRxState::Data:
            if (line)
                m_geartogear_rx.shift |= (u8)(1 << m_geartogear_rx.bit);

            if (m_geartogear_rx.bit < 7)
                m_geartogear_rx.bit++;
            else
                m_geartogear_rx.state = GearToGearRxState::Stop;

            m_geartogear_rx.next_cycle += m_geartogear_rx.bit_cycles;
            break;
        case GearToGearRxState::Stop:
            if (line)
            {
                m_Ports[4] = m_geartogear_rx.shift;
                m_geartogear_rx.ready = true;
                m_geartogear_rx.frame_error = false;
                TraceGearToGearEvent(TRACE_GEARTOGEAR_RX_END, m_geartogear_rx.shift);
            }
            else
            {
                m_geartogear_rx.frame_error = true;
                TraceGearToGearEvent(TRACE_GEARTOGEAR_RX_ERROR, m_geartogear_rx.shift);
            }

            if (m_Ports[5] & 0x08)
            {
                m_geartogear_nmi.serial_latch = true;
                UpdateGearToGearNMI();
            }

            m_geartogear_rx.state = GearToGearRxState::Idle;
            m_geartogear_rx.next_cycle = 0;
            break;
        default:
            break;
    }
}

void GameGearIOPorts::FetchPendingRemoteEvent(u64 target_cycle)
{
    if (m_geartogear_has_pending_remote_event || !m_geartogear_cable_connected || !m_geartogear_poll_callback)
    {
        return;
    }

    GS_GearToGear_WireEvent event;
    if (m_geartogear_poll_callback(target_cycle, &event, m_geartogear_user_data))
    {
        event.state.drive_mask &= 0x7F;
        event.state.levels &= 0x7F;
        m_geartogear_pending_remote_event = event;
        m_geartogear_has_pending_remote_event = true;
    }
}

void GameGearIOPorts::AdvanceGearToGearTo(u64 target_cycle)
{
    while (true)
    {
        FetchPendingRemoteEvent(target_cycle);

        u64 next_cycle = 0;
        bool has_work = false;

        if (m_geartogear_has_pending_remote_event)
        {
            next_cycle = m_geartogear_pending_remote_event.cycle;
            has_work = true;
        }

        if (m_geartogear_rx.state != GearToGearRxState::Idle && (!has_work || m_geartogear_rx.next_cycle < next_cycle))
        {
            next_cycle = m_geartogear_rx.next_cycle;
            has_work = true;
        }

        if (m_geartogear_tx.busy && (!has_work || m_geartogear_tx.next_cycle < next_cycle))
        {
            next_cycle = m_geartogear_tx.next_cycle;
            has_work = true;
        }

        if (!has_work || next_cycle > target_cycle)
            break;

        m_geartogear_cycle = next_cycle;

        while (m_geartogear_has_pending_remote_event && m_geartogear_pending_remote_event.cycle == next_cycle)
        {
            GS_GearToGear_WireState state =
                m_geartogear_pending_remote_event.state;
            m_geartogear_has_pending_remote_event = false;
            ApplyRemoteWireState(state, next_cycle, true);
            FetchPendingRemoteEvent(target_cycle);
        }

        if (m_geartogear_rx.state != GearToGearRxState::Idle && m_geartogear_rx.next_cycle == next_cycle)
        {
            ProcessGearToGearRxSample();
        }

        if (m_geartogear_tx.busy && m_geartogear_tx.next_cycle == next_cycle)
        {
            ProcessGearToGearTxBoundary();
        }
    }

    m_geartogear_cycle = target_cycle;
}

void GameGearIOPorts::SaveGearToGearState(std::ostream& stream)
{
    u64 tx_delta = m_geartogear_tx.busy &&
        m_geartogear_tx.next_cycle > m_geartogear_cycle ?
        m_geartogear_tx.next_cycle - m_geartogear_cycle : 0;
    u64 rx_delta = m_geartogear_rx.state != GearToGearRxState::Idle &&
        m_geartogear_rx.next_cycle > m_geartogear_cycle ?
        m_geartogear_rx.next_cycle - m_geartogear_cycle : 0;
    u8 rx_state = (u8)m_geartogear_rx.state;

    stream.write(reinterpret_cast<const char*>(&m_geartogear_tx.busy), sizeof(m_geartogear_tx.busy));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_tx.line), sizeof(m_geartogear_tx.line));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_tx.data), sizeof(m_geartogear_tx.data));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_tx.phase), sizeof(m_geartogear_tx.phase));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_tx.bit_cycles), sizeof(m_geartogear_tx.bit_cycles));
    stream.write(reinterpret_cast<const char*>(&tx_delta), sizeof(tx_delta));

    stream.write(reinterpret_cast<const char*>(&rx_state), sizeof(rx_state));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_rx.ready), sizeof(m_geartogear_rx.ready));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_rx.frame_error), sizeof(m_geartogear_rx.frame_error));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_rx.shift), sizeof(m_geartogear_rx.shift));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_rx.bit), sizeof(m_geartogear_rx.bit));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_rx.bit_cycles), sizeof(m_geartogear_rx.bit_cycles));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_rx.half_bit_cycles), sizeof(m_geartogear_rx.half_bit_cycles));
    stream.write(reinterpret_cast<const char*>(&rx_delta), sizeof(rx_delta));

    stream.write(reinterpret_cast<const char*>(&m_geartogear_nmi.parallel_latch), sizeof(m_geartogear_nmi.parallel_latch));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_nmi.serial_latch), sizeof(m_geartogear_nmi.serial_latch));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_nmi.output_asserted), sizeof(m_geartogear_nmi.output_asserted));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_nmi.parallel_armed), sizeof(m_geartogear_nmi.parallel_armed));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_nmi.parallel_arm_delay), sizeof(m_geartogear_nmi.parallel_arm_delay));
    stream.write(reinterpret_cast<const char*>(&m_geartogear_nmi.previous_pc6), sizeof(m_geartogear_nmi.previous_pc6));
}

void GameGearIOPorts::LoadGearToGearState(std::istream& stream)
{
    u64 tx_delta = 0;
    u64 rx_delta = 0;
    u8 rx_state = 0;

    stream.read(reinterpret_cast<char*>(&m_geartogear_tx.busy), sizeof(m_geartogear_tx.busy));
    stream.read(reinterpret_cast<char*>(&m_geartogear_tx.line), sizeof(m_geartogear_tx.line));
    stream.read(reinterpret_cast<char*>(&m_geartogear_tx.data), sizeof(m_geartogear_tx.data));
    stream.read(reinterpret_cast<char*>(&m_geartogear_tx.phase), sizeof(m_geartogear_tx.phase));
    stream.read(reinterpret_cast<char*>(&m_geartogear_tx.bit_cycles), sizeof(m_geartogear_tx.bit_cycles));
    stream.read(reinterpret_cast<char*>(&tx_delta), sizeof(tx_delta));

    stream.read(reinterpret_cast<char*>(&rx_state), sizeof(rx_state));
    stream.read(reinterpret_cast<char*>(&m_geartogear_rx.ready), sizeof(m_geartogear_rx.ready));
    stream.read(reinterpret_cast<char*>(&m_geartogear_rx.frame_error), sizeof(m_geartogear_rx.frame_error));
    stream.read(reinterpret_cast<char*>(&m_geartogear_rx.shift), sizeof(m_geartogear_rx.shift));
    stream.read(reinterpret_cast<char*>(&m_geartogear_rx.bit), sizeof(m_geartogear_rx.bit));
    stream.read(reinterpret_cast<char*>(&m_geartogear_rx.bit_cycles), sizeof(m_geartogear_rx.bit_cycles));
    stream.read(reinterpret_cast<char*>(&m_geartogear_rx.half_bit_cycles), sizeof(m_geartogear_rx.half_bit_cycles));
    stream.read(reinterpret_cast<char*>(&rx_delta), sizeof(rx_delta));

    stream.read(reinterpret_cast<char*>(&m_geartogear_nmi.parallel_latch), sizeof(m_geartogear_nmi.parallel_latch));
    stream.read(reinterpret_cast<char*>(&m_geartogear_nmi.serial_latch), sizeof(m_geartogear_nmi.serial_latch));
    stream.read(reinterpret_cast<char*>(&m_geartogear_nmi.output_asserted), sizeof(m_geartogear_nmi.output_asserted));
    stream.read(reinterpret_cast<char*>(&m_geartogear_nmi.parallel_armed), sizeof(m_geartogear_nmi.parallel_armed));
    stream.read(reinterpret_cast<char*>(&m_geartogear_nmi.parallel_arm_delay), sizeof(m_geartogear_nmi.parallel_arm_delay));
    stream.read(reinterpret_cast<char*>(&m_geartogear_nmi.previous_pc6), sizeof(m_geartogear_nmi.previous_pc6));

    if (m_geartogear_tx.bit_cycles == 0)
        m_geartogear_tx.bit_cycles = kGearToGearBitCycles[0];
    if (m_geartogear_rx.bit_cycles == 0)
        m_geartogear_rx.bit_cycles = kGearToGearBitCycles[0];
    if (m_geartogear_rx.half_bit_cycles == 0)
        m_geartogear_rx.half_bit_cycles = kGearToGearHalfBitCycles[0];

    if (rx_state > (u8)GearToGearRxState::Stop)
        rx_state = (u8)GearToGearRxState::Idle;

    m_geartogear_rx.state = (GearToGearRxState::State)rx_state;
    m_geartogear_tx.next_cycle = m_geartogear_cycle + tx_delta;
    m_geartogear_rx.next_cycle = m_geartogear_cycle + rx_delta;
    m_geartogear_last_nmi_trace_state =
        (m_geartogear_nmi.parallel_latch ? 0x01 : 0) |
        (m_geartogear_nmi.serial_latch ? 0x02 : 0) |
        (m_geartogear_nmi.output_asserted ? 0x04 : 0);
}
