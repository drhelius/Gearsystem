/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/
 *
 */

#include <cstring>
#include "MarkIIILink.h"

MarkIIILink::MarkIIILink()
{
    m_publish_callback = NULL;
    m_sample_callback = NULL;
    m_poll_callback = NULL;
    m_fence_callback = NULL;
    m_sync_callback = NULL;
    m_user_data = NULL;
    m_peripheral_attached = false;
    m_transport_active = false;
    m_cable_connected = false;
    m_cycle = 0;
    m_next_sync_cycle = 0;
    m_has_published_state = false;
    m_has_pending_remote_event = false;
    Reset();
}

void MarkIIILink::Reset()
{
    ResetPPI();
    ReleaseAllKeys();
    m_remote_state.drive_mask = 0;
    m_remote_state.levels = 0x7F;
    m_has_published_state = false;
    m_has_pending_remote_event = false;
    m_local_state = ComputeLocalWireState();
}

void MarkIIILink::ResetPPI()
{
    m_control = 0x9B;
    m_port_a = 0;
    m_port_b = 0;
    m_port_c = 0;
}

u8 MarkIIILink::DoInput(u8 port)
{
    switch (port & 0x03)
    {
        case 0:
            return ReadPortA();
        case 1:
            return ReadPortB();
        case 2:
            return ReadPortC();
        default:
            return 0xFF;
    }
}

void MarkIIILink::DoOutput(u8 port, u8 value)
{
    switch (port & 0x03)
    {
        case 0:
            m_port_a = value;
            break;
        case 1:
            m_port_b = value;
            break;
        case 2:
            m_port_c = value;
            RefreshLocalWireState(m_cycle, false);
            break;
        case 3:
            WriteControl(value);
            break;
    }
}

void MarkIIILink::SetCallbacks(
    GS_LinkCable_Publish_Callback publish_callback,
    GS_LinkCable_Sample_Callback sample_callback,
    GS_LinkCable_Poll_Callback poll_callback,
    GS_LinkCable_Fence_Callback fence_callback,
    GS_LinkCable_Sync_Callback sync_callback,
    void* user_data)
{
    m_publish_callback = publish_callback;
    m_sample_callback = sample_callback;
    m_poll_callback = poll_callback;
    m_fence_callback = fence_callback;
    m_sync_callback = sync_callback;
    m_user_data = user_data;
}

void MarkIIILink::SetPeripheralAttached(bool attached, u64 cycle)
{
    if (m_peripheral_attached == attached)
        return;

    m_cycle = cycle;
    m_peripheral_attached = attached;
    m_has_pending_remote_event = false;
    m_has_published_state = false;

    if (attached)
    {
        ResetPPI();
        ReleaseAllKeys();
        m_local_state = ComputeLocalWireState();
        RefreshLocalWireState(cycle, m_transport_active);
    }
    else
    {
        m_transport_active = false;
        m_cable_connected = false;
        m_remote_state.drive_mask = 0;
        m_remote_state.levels = 0x7F;
        ReleaseAllKeys();
    }
}

void MarkIIILink::SetTransportActive(bool active, u64 cycle)
{
    m_cycle = cycle;
    m_transport_active = active && m_peripheral_attached;
    m_next_sync_cycle = cycle;
    m_has_published_state = false;

    if (m_transport_active)
        RefreshLocalWireState(cycle, true);
}

void MarkIIILink::SetCableConnected(bool connected, u64 cycle)
{
    m_cycle = cycle;
    m_has_pending_remote_event = false;

    if (connected && m_peripheral_attached)
    {
        GS_LinkCable_WireState state;
        state.drive_mask = 0;
        state.levels = 0x7F;

        if (m_sample_callback)
            m_sample_callback(cycle, &state, m_user_data);

        m_cable_connected = true;
        ApplyRemoteWireState(state);
    }
    else
    {
        m_cable_connected = false;
        m_remote_state.drive_mask = 0;
        m_remote_state.levels = 0x7F;
    }
}

void MarkIIILink::BeginInstruction(u64 cycle)
{
    AdvanceTo(cycle);
}

void MarkIIILink::EndInstruction(u64 cycle)
{
    AdvanceTo(cycle);

    if (m_cable_connected && m_sync_callback &&
        cycle >= m_next_sync_cycle)
    {
        m_sync_callback(cycle, LINK_CABLE_MAX_LEAD_CYCLES,
            m_user_data);
        m_next_sync_cycle = cycle + LINK_CABLE_MAX_SYNC_CYCLES;
    }
}

void MarkIIILink::Rebase(u64 cycle)
{
    m_cycle = cycle;
    m_next_sync_cycle = cycle;
    m_has_pending_remote_event = false;

    GS_LinkCable_WireState remote;
    remote.drive_mask = 0;
    remote.levels = 0x7F;

    if (m_cable_connected && m_sample_callback)
        m_sample_callback(cycle, &remote, m_user_data);

    ApplyRemoteWireState(remote);
    m_local_state = ComputeLocalWireState();
    m_has_published_state = false;
    RefreshLocalWireState(cycle, m_transport_active);
}

void MarkIIILink::KeyPressed(GS_MarkIII_Key key)
{
    SetKeyState(key, true);
}

void MarkIIILink::KeyReleased(GS_MarkIII_Key key)
{
    SetKeyState(key, false);
}

void MarkIIILink::ReleaseAllKeys()
{
    memset(m_keyboard_a, 0xFF, sizeof(m_keyboard_a));
}

bool MarkIIILink::IsPeripheralAttached() const
{
    return m_peripheral_attached;
}

bool MarkIIILink::IsKeyboardSelected() const
{
    return GetSelectedRow() != 7;
}

bool MarkIIILink::IsCableConnected() const
{
    return m_cable_connected;
}

u8 MarkIIILink::GetSelectedRow() const
{
    return m_port_c & 0x07;
}

GS_MarkIII_LinkDebugState MarkIIILink::GetDebugState() const
{
    GS_MarkIII_LinkDebugState state = {};
    state.peripheral_attached = m_peripheral_attached;
    state.transport_active = m_transport_active;
    state.cable_connected = m_cable_connected;
    state.control = m_control;
    state.port_a = ReadPortA();
    if ((m_control & 0x02) == 0)
    {
        state.port_b = m_port_b;
    }
    else
    {
        state.port_b = 0x1F;
        if (ReadRemoteLine(0x20))
            state.port_b |= 0x20;
        if (ReadRemoteLine(0x40))
            state.port_b |= 0x40;
    }
    state.port_c = ReadPortC();
    state.port_c_latch = m_port_c;
    state.selected_row = GetSelectedRow();
    state.local_state = m_local_state;
    state.remote_state = m_remote_state;
    state.cycle = m_cycle;
    return state;
}

void MarkIIILink::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_control),
        sizeof(m_control));
    stream.write(reinterpret_cast<const char*>(&m_port_a),
        sizeof(m_port_a));
    stream.write(reinterpret_cast<const char*>(&m_port_b),
        sizeof(m_port_b));
    stream.write(reinterpret_cast<const char*>(&m_port_c),
        sizeof(m_port_c));
}

void MarkIIILink::LoadState(std::istream& stream, int version)
{
    if (version >= 109)
    {
        stream.read(reinterpret_cast<char*>(&m_control),
            sizeof(m_control));
        stream.read(reinterpret_cast<char*>(&m_port_a),
            sizeof(m_port_a));
        stream.read(reinterpret_cast<char*>(&m_port_b),
            sizeof(m_port_b));
        stream.read(reinterpret_cast<char*>(&m_port_c),
            sizeof(m_port_c));
    }
    else
    {
        ResetPPI();
    }

    ReleaseAllKeys();
    m_remote_state.drive_mask = 0;
    m_remote_state.levels = 0x7F;
    m_local_state = ComputeLocalWireState();
    m_has_published_state = false;
    m_has_pending_remote_event = false;
}

u8 MarkIIILink::ReadPortA() const
{
    if ((m_control & 0x10) == 0)
        return m_port_a;

    return m_keyboard_a[GetSelectedRow()];
}

u8 MarkIIILink::ReadPortB()
{
    if ((m_control & 0x02) == 0)
        return m_port_b;

    FenceRead();

    u8 value = 0x1F;
    if (ReadRemoteLine(0x20))
        value |= 0x20;
    if (ReadRemoteLine(0x40))
        value |= 0x40;
    return value;
}

u8 MarkIIILink::ReadPortC() const
{
    u8 output_mask = GetPortCOutputMask();
    return (m_port_c & output_mask) | ((u8)~output_mask);
}

void MarkIIILink::WriteControl(u8 value)
{
    if (value & 0x80)
    {
        m_control = value;
        m_port_a = 0;
        m_port_b = 0;
        m_port_c = 0;
    }
    else
    {
        u8 bit = (value >> 1) & 0x07;
        u8 mask = (u8)(1 << bit);
        if (value & 0x01)
            m_port_c |= mask;
        else
            m_port_c &= (u8)~mask;
    }

    RefreshLocalWireState(m_cycle, false);
}

u8 MarkIIILink::GetPortCOutputMask() const
{
    u8 mask = 0;
    if ((m_control & 0x01) == 0)
        mask |= 0x0F;
    if ((m_control & 0x08) == 0)
        mask |= 0xF0;
    return mask;
}

GS_LinkCable_WireState MarkIIILink::ComputeLocalWireState() const
{
    GS_LinkCable_WireState state;
    u8 output_mask = GetPortCOutputMask();
    state.drive_mask = output_mask & 0x60;
    state.levels = m_port_c & 0x60;
    return state;
}

void MarkIIILink::RefreshLocalWireState(u64 cycle, bool force_publish)
{
    m_cycle = cycle;
    m_local_state = ComputeLocalWireState();

    bool changed = !m_has_published_state ||
        m_local_state.drive_mask != m_last_published_state.drive_mask ||
        m_local_state.levels != m_last_published_state.levels;

    if (m_transport_active && m_publish_callback &&
        (force_publish || changed))
    {
        m_publish_callback(cycle, &m_local_state, m_user_data);
        m_last_published_state = m_local_state;
        m_has_published_state = true;
    }
}

void MarkIIILink::ApplyRemoteWireState(
    const GS_LinkCable_WireState& state)
{
    m_remote_state.drive_mask = state.drive_mask & 0x60;
    m_remote_state.levels = state.levels & 0x60;
}

bool MarkIIILink::ReadRemoteLine(u8 mask) const
{
    if (!m_cable_connected || (m_remote_state.drive_mask & mask) == 0)
        return true;

    return (m_remote_state.levels & mask) != 0;
}

void MarkIIILink::FetchPendingRemoteEvent(u64 target_cycle)
{
    if (!m_cable_connected || m_has_pending_remote_event ||
        !m_poll_callback)
    {
        return;
    }

    GS_LinkCable_WireEvent event;
    if (m_poll_callback(target_cycle, &event, m_user_data))
    {
        event.state.drive_mask &= 0x60;
        event.state.levels &= 0x60;
        m_pending_remote_event = event;
        m_has_pending_remote_event = true;
    }
}

void MarkIIILink::AdvanceTo(u64 target_cycle)
{
    FetchPendingRemoteEvent(target_cycle);

    while (m_has_pending_remote_event &&
        m_pending_remote_event.cycle <= target_cycle)
    {
        m_cycle = m_pending_remote_event.cycle;
        GS_LinkCable_WireState state = m_pending_remote_event.state;
        m_has_pending_remote_event = false;
        ApplyRemoteWireState(state);
        FetchPendingRemoteEvent(target_cycle);
    }

    m_cycle = target_cycle;
}

void MarkIIILink::FenceRead()
{
    if (m_cable_connected && m_fence_callback)
        m_fence_callback(m_cycle, m_user_data);

    AdvanceTo(m_cycle);
}

void MarkIIILink::SetKeyState(GS_MarkIII_Key key, bool pressed)
{
    u8 row = 0;
    u8 mask = 0;

    switch (key)
    {
        case MarkIIIKey1:
            row = 0;
            mask = 0x01;
            break;
        case MarkIIIKey2:
            row = 1;
            mask = 0x01;
            break;
        case MarkIIIKeySpace:
            row = 1;
            mask = 0x10;
            break;
        case MarkIIIKeyReturn:
            row = 5;
            mask = 0x40;
            break;
        default:
            return;
    }

    if (pressed)
        m_keyboard_a[row] &= (u8)~mask;
    else
        m_keyboard_a[row] |= mask;
}
