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

#ifndef MARKIII_LINK_H
#define MARKIII_LINK_H

#include "geartogear.h"

enum GS_MarkIII_Key
{
    MarkIIIKey1,
    MarkIIIKey2,
    MarkIIIKeySpace,
    MarkIIIKeyReturn
};

struct GS_MarkIII_LinkDebugState
{
    bool peripheral_attached;
    bool transport_active;
    bool cable_connected;
    u8 control;
    u8 port_a;
    u8 port_b;
    u8 port_c;
    u8 port_c_latch;
    u8 selected_row;
    GS_LinkCable_WireState local_state;
    GS_LinkCable_WireState remote_state;
    u64 cycle;
};

class MarkIIILink
{
public:
    MarkIIILink();
    void Reset();
    u8 DoInput(u8 port);
    void DoOutput(u8 port, u8 value);
    void SetCallbacks(
        GS_LinkCable_Publish_Callback publish_callback,
        GS_LinkCable_Sample_Callback sample_callback,
        GS_LinkCable_Poll_Callback poll_callback,
        GS_LinkCable_Fence_Callback fence_callback,
        GS_LinkCable_Sync_Callback sync_callback,
        void* user_data);
    void SetPeripheralAttached(bool attached, u64 cycle);
    void SetTransportActive(bool active, u64 cycle);
    void SetCableConnected(bool connected, u64 cycle);
    void BeginInstruction(u64 cycle);
    void EndInstruction(u64 cycle);
    void Rebase(u64 cycle);
    void KeyPressed(GS_MarkIII_Key key);
    void KeyReleased(GS_MarkIII_Key key);
    void ReleaseAllKeys();
    bool IsPeripheralAttached() const;
    bool IsKeyboardSelected() const;
    bool IsCableConnected() const;
    u8 GetSelectedRow() const;
    GS_MarkIII_LinkDebugState GetDebugState() const;
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream, int version);

private:
    void ResetPPI();
    u8 ReadPortA() const;
    u8 ReadPortB();
    u8 ReadPortC() const;
    void WriteControl(u8 value);
    u8 GetPortCOutputMask() const;
    GS_LinkCable_WireState ComputeLocalWireState() const;
    void RefreshLocalWireState(u64 cycle, bool force_publish);
    void ApplyRemoteWireState(const GS_LinkCable_WireState& state);
    bool ReadRemoteLine(u8 mask) const;
    void FetchPendingRemoteEvent(u64 target_cycle);
    void AdvanceTo(u64 target_cycle);
    void FenceRead();
    void SetKeyState(GS_MarkIII_Key key, bool pressed);

private:
    GS_LinkCable_Publish_Callback m_publish_callback;
    GS_LinkCable_Sample_Callback m_sample_callback;
    GS_LinkCable_Poll_Callback m_poll_callback;
    GS_LinkCable_Fence_Callback m_fence_callback;
    GS_LinkCable_Sync_Callback m_sync_callback;
    void* m_user_data;
    bool m_peripheral_attached;
    bool m_transport_active;
    bool m_cable_connected;
    u8 m_control;
    u8 m_port_a;
    u8 m_port_b;
    u8 m_port_c;
    u8 m_keyboard_a[8];
    GS_LinkCable_WireState m_local_state;
    GS_LinkCable_WireState m_remote_state;
    GS_LinkCable_WireState m_last_published_state;
    bool m_has_published_state;
    bool m_has_pending_remote_event;
    GS_LinkCable_WireEvent m_pending_remote_event;
    u64 m_cycle;
    u64 m_next_sync_cycle;
};

#endif /* MARKIII_LINK_H */
