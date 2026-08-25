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

#ifndef GEARTOGEAR_MANAGER_H
#define GEARTOGEAR_MANAGER_H

#include "geartogear.h"

#define GEARTOGEAR_DETACH_US 500000

enum GearToGearMode
{
    GearToGearModeDisabled,
    GearToGearModeConnected,
    GearToGearModeFault
};

struct GearToGearStatus
{
    GearToGearMode mode;
    bool active;
    bool cable_connected;
    bool pacing_peer;
    bool local_hardware_ready;
    bool remote_hardware_ready;
    u8 session;
    u8 local_peer_id;
    int peer_count;
    u32 remote_generation;
    u64 local_anchor;
    u64 bus_anchor;
    u64 bus_cycle;
    u64 local_progress;
    u64 local_promise;
    u64 remote_progress;
    u64 remote_promise;
    u64 events_published;
    u64 events_consumed;
    u64 state_ring_overruns;
    u64 baseline_samples;
    u64 fence_calls;
    u64 fence_waits;
    u64 fence_wait_us;
    u64 fence_wait_max_us;
    u64 sync_calls;
    u64 barrier_waits;
    u64 barrier_wait_us;
    u64 barrier_wait_max_us;
    u64 barrier_wait_over_1ms;
    u64 barrier_wait_over_10ms;
    u64 barrier_wait_over_50ms;
    u64 sync_gap_max_us;
    u64 sync_gap_over_50ms;
    u64 spin_iterations;
    u64 sleep_calls;
    u64 peer_detaches;
    u64 peer_detach_max_age_us;
    u64 slot_reclaims;
    u64 seqlock_retries;
    u64 attachments;
    char endpoint[64];
    char last_error[160];
};

class GearToGearManager
{
public:
    GearToGearManager();
    ~GearToGearManager();
    bool Connect(u8 session, u64 local_cycle);
    void Stop();
    void Pump(u64 local_cycle);
    void SetHardwareReady(bool ready, u64 local_cycle);
    void PublishState(u64 local_cycle, const GS_GearToGear_WireState& state, bool force = false);
    bool SampleRemoteState(u64 local_cycle, GS_GearToGear_WireState& state);
    bool PollRemoteEvent(u64 through_local_cycle, GS_GearToGear_WireEvent& event);
    void PublishProgress(u64 local_cycle, u32 lead_cycles);
    void Fence(u64 local_cycle);
    void Synchronize(u64 local_cycle, u32 lead_cycles);
    bool IsActive() const;
    bool IsCableConnected() const;
    bool IsHardwareReady() const;
    bool IsPacingPeer() const;
    bool ConsumeLocalAttachmentChanged();
    bool ConsumeRemoteIdentityChanged();
    void SetNormalBarrierStallUs(u32 stall_us);
    void ResetMetrics();
    GearToGearStatus GetStatus();

private:
    struct Shared;

    bool Map(u8 session);
    void Unmap();
    bool ClaimSlot(u64 local_cycle, bool reattach);
    bool EnsureAttached(u64 local_cycle);
    void ReapStalePeers(u64 now_us, bool preserve_idle = false);
    int FindRemoteSlot(u64 now_us, u32* generation = NULL) const;
    void UpdateRemoteIdentity(u64 now_us);
    bool SharedAtomicsLockFree() const;
    u64 ToBusCycle(u64 local_cycle) const;
    u64 FromBusCycle(u64 bus_cycle) const;
    u64 GetClockMicroseconds() const;
    void SetFault(const char* message);
    void RefreshStatus();
    void RecordBarrierWait(u64 wait);

private:
    Shared* m_shared;
    void* m_mapping_handle;
    int m_mapping_fd;
    int m_slot;
    u32 m_generation;
    u8 m_session;
    u64 m_local_anchor;
    u64 m_bus_anchor;
    u64 m_last_local_cycle;
    u64 m_last_sync_exit_us;
    int m_remote_slot;
    u32 m_remote_generation;
    u32 m_remote_read_index;
    GS_GearToGear_WireState m_remote_state;
    bool m_remote_sampled;
    GS_GearToGear_WireState m_last_published_state;
    bool m_has_last_published_state;
    bool m_local_attachment_changed;
    bool m_remote_identity_changed;
    bool m_hardware_ready;
    u32 m_normal_barrier_stall_us;
    GearToGearStatus m_status;
};

inline u32 geartogear_normal_barrier_stall_us()
{
#if defined(_WIN32)
    return 5000;
#elif defined(__APPLE__)
    return 100;
#else
    return 250;
#endif
}

inline u64 geartogear_heartbeat_age(u64 now, u64 heartbeat)
{
    return heartbeat <= now ? now - heartbeat : 0;
}

inline bool geartogear_lease_is_unchanged_and_stale(u64 now, u64 observed_heartbeat, u32 observed_generation,
    u64 current_heartbeat, u32 current_generation)
{
    return current_heartbeat == observed_heartbeat &&
        current_generation == observed_generation &&
        geartogear_heartbeat_age(now, current_heartbeat) > GEARTOGEAR_DETACH_US;
}

#endif /* GEARTOGEAR_MANAGER_H */
