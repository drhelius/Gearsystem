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

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <new>
#include <thread>
#if defined(_WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "geartogear_manager.h"
#include "geartogear_wire.h"
#include "log.h"

#define GEARTOGEAR_SHM_MAGIC 0x47324752
#define GEARTOGEAR_SHM_VERSION 1
#define GEARTOGEAR_SHARED_EVENT_COUNT 64
#define GEARTOGEAR_BARRIER_SLEEP_US 100

#define GEARTOGEAR_PEER_FREE 0
#define GEARTOGEAR_PEER_ACTIVE 1
#define GEARTOGEAR_PEER_CLAIMING 2
// A ready peer is an active transport slot with native Game Gear hardware.
#define GEARTOGEAR_PEER_READY 3

static u64 geartogear_saturating_add(u64 value, u64 add)
{
    return add > (~0ULL - value) ? ~0ULL : value + add;
}

struct GearToGearManager::Shared
{
    struct Peer
    {
        std::atomic<u32> state;
        std::atomic<u32> generation;
        std::atomic<u64> heartbeat_us;
        std::atomic<u64> progress_cycle;
        std::atomic<u64> promise_cycle;
        std::atomic<u32> write_index;
        GearToGearSharedWireEvent events[GEARTOGEAR_SHARED_EVENT_COUNT];

        Peer() : state(0), generation(0), heartbeat_us(0),
            progress_cycle(0), promise_cycle(0), write_index(0) {}
    };

    std::atomic<u32> magic;
    u32 version;
    u8 session;
    u8 reserved[3];
    Peer peers[GEARTOGEAR_MAX_PEERS];

    Shared() : magic(0), version(GEARTOGEAR_SHM_VERSION), session(0)
    {
        memset(reserved, 0, sizeof(reserved));
    }
};

static void StoreMax(std::atomic<u64>& value, u64 desired)
{
    u64 current = value.load(std::memory_order_relaxed);
    while (current < desired && !value.compare_exchange_weak(current, desired,
        std::memory_order_release, std::memory_order_relaxed))
    {
    }
}

static bool IsPeerActive(u32 state)
{
    return state == GEARTOGEAR_PEER_ACTIVE || state == GEARTOGEAR_PEER_READY;
}

static bool IsPeerReady(u32 state)
{
    return state == GEARTOGEAR_PEER_READY;
}

GearToGearManager::GearToGearManager()
{
    m_shared = NULL;
    m_mapping_handle = NULL;
    m_mapping_fd = -1;
    m_slot = -1;
    m_generation = 0;
    m_session = 0;
    m_local_anchor = 0;
    m_bus_anchor = 0;
    m_last_local_cycle = 0;
    m_last_sync_exit_us = 0;
    m_remote_slot = -1;
    m_remote_generation = 0;
    m_remote_read_index = 0;
    m_remote_state.drive_mask = 0;
    m_remote_state.levels = 0x7F;
    m_remote_sampled = false;
    m_last_published_state.drive_mask = 0;
    m_last_published_state.levels = 0x7F;
    m_has_last_published_state = false;
    m_local_attachment_changed = false;
    m_remote_identity_changed = false;
    m_hardware_ready = false;
    m_normal_barrier_stall_us = geartogear_normal_barrier_stall_us();
    memset(&m_status, 0, sizeof(m_status));
    m_status.mode = GearToGearModeDisabled;
}

GearToGearManager::~GearToGearManager()
{
    Stop();
}

bool GearToGearManager::Connect(u8 session, u64 local_cycle)
{
    Stop();

    if (session == 0)
    {
        SetFault("Gear-to-Gear session must be between 1 and 255");
        return false;
    }

    if (!Map(session))
        return false;

    m_session = session;

    if (!ClaimSlot(local_cycle, false))
    {
        Unmap();
        SetFault("Gear-to-Gear session is full");
        return false;
    }

    memset(&m_status, 0, sizeof(m_status));
    m_status.mode = GearToGearModeConnected;
    m_status.active = true;
    m_status.session = session;
    m_status.attachments = 1;

    snprintf(m_status.endpoint, sizeof(m_status.endpoint), "Shared session %u", session);

    u64 now = GetClockMicroseconds();
    ReapStalePeers(now);
    UpdateRemoteIdentity(now);
    RefreshStatus();

    Log("Gear-to-Gear: connected to shared session %u as peer %u", session, m_slot + 1);
    return true;
}

void GearToGearManager::Stop()
{
    if (m_shared && m_slot >= 0)
    {
        Shared::Peer& peer = m_shared->peers[m_slot];
        if (peer.generation.load(std::memory_order_acquire) == m_generation)
            peer.state.store(0, std::memory_order_release);
    }

    Unmap();

    m_slot = -1;
    m_generation = 0;
    m_session = 0;
    m_local_anchor = 0;
    m_bus_anchor = 0;
    m_last_local_cycle = 0;
    m_last_sync_exit_us = 0;
    m_remote_slot = -1;
    m_remote_generation = 0;
    m_remote_read_index = 0;
    m_remote_state.drive_mask = 0;
    m_remote_state.levels = 0x7F;
    m_remote_sampled = false;
    m_has_last_published_state = false;
    m_local_attachment_changed = false;
    m_remote_identity_changed = false;
    m_hardware_ready = false;

    memset(&m_status, 0, sizeof(m_status));
    m_status.mode = GearToGearModeDisabled;
}

void GearToGearManager::Pump(u64 local_cycle)
{
    if (!EnsureAttached(local_cycle))
        return;

    u64 now = GetClockMicroseconds();
    m_shared->peers[m_slot].heartbeat_us.store(now, std::memory_order_release);

    if (m_hardware_ready)
    {
        u64 bus_cycle = ToBusCycle(local_cycle);
        StoreMax(m_shared->peers[m_slot].progress_cycle, bus_cycle);
        StoreMax(m_shared->peers[m_slot].promise_cycle, geartogear_saturating_add(bus_cycle, GEARTOGEAR_MAX_LEAD_CYCLES));
    }

    ReapStalePeers(now);
    UpdateRemoteIdentity(now);
}

void GearToGearManager::SetHardwareReady(bool ready, u64 local_cycle)
{
    if (!EnsureAttached(local_cycle) || ready == m_hardware_ready)
        return;

    Shared::Peer& local = m_shared->peers[m_slot];

    if (!ready)
    {
        local.state.store(GEARTOGEAR_PEER_ACTIVE, std::memory_order_release);
        m_hardware_ready = false;
        m_has_last_published_state = false;
        m_local_attachment_changed = true;
        UpdateRemoteIdentity(GetClockMicroseconds());
        return;
    }

    u64 now = GetClockMicroseconds();
    ReapStalePeers(now);

    u64 bus_anchor = 0;

    for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
    {
        if (i == m_slot)
            continue;

        Shared::Peer& peer = m_shared->peers[i];

        if (IsPeerReady(peer.state.load(std::memory_order_acquire)))
        {
            bus_anchor = MAX(bus_anchor,
                peer.progress_cycle.load(std::memory_order_acquire));
        }
    }

    m_generation = local.generation.fetch_add(1, std::memory_order_acq_rel) + 1;
    m_local_anchor = local_cycle;
    m_bus_anchor = bus_anchor;
    m_last_sync_exit_us = 0;
    m_remote_slot = -1;
    m_remote_generation = 0;
    m_remote_read_index = 0;
    m_remote_state.drive_mask = 0;
    m_remote_state.levels = 0x7F;
    m_remote_sampled = false;
    m_has_last_published_state = false;
    m_local_attachment_changed = true;
    m_remote_identity_changed = true;

    local.write_index.store(0, std::memory_order_relaxed);
    local.progress_cycle.store(bus_anchor, std::memory_order_relaxed);
    local.promise_cycle.store(geartogear_saturating_add(bus_anchor, GEARTOGEAR_MAX_LEAD_CYCLES), std::memory_order_relaxed);
    local.heartbeat_us.store(now, std::memory_order_relaxed);
    local.state.store(GEARTOGEAR_PEER_READY, std::memory_order_release);
    m_hardware_ready = true;

    UpdateRemoteIdentity(now);
}

void GearToGearManager::PublishState(u64 local_cycle, const GS_GearToGear_WireState& input_state, bool force)
{
    if (!EnsureAttached(local_cycle) || !m_hardware_ready)
        return;

    GS_GearToGear_WireState state;
    state.drive_mask = input_state.drive_mask & 0x7F;
    state.levels = input_state.levels & 0x7F;

    if (!force && m_has_last_published_state &&
        state.drive_mask == m_last_published_state.drive_mask &&
        state.levels == m_last_published_state.levels)
    {
        return;
    }

    Shared::Peer& peer = m_shared->peers[m_slot];
    peer.heartbeat_us.store(GetClockMicroseconds(), std::memory_order_release);

    u32 index = peer.write_index.load(std::memory_order_relaxed);
    GearToGearSharedWireEvent& event = peer.events[index % GEARTOGEAR_SHARED_EVENT_COUNT];
    geartogear_publish_shared_event(event, m_generation, ToBusCycle(local_cycle), state.drive_mask, state.levels);
    peer.write_index.store(index + 1, std::memory_order_release);

    m_last_published_state = state;
    m_has_last_published_state = true;
    m_status.events_published++;
}

bool GearToGearManager::SampleRemoteState(u64 local_cycle, GS_GearToGear_WireState& state)
{
    state.drive_mask = 0;
    state.levels = 0x7F;

    if (!EnsureAttached(local_cycle) || !IsCableConnected())
        return false;

    u64 now = GetClockMicroseconds();
    ReapStalePeers(now);
    UpdateRemoteIdentity(now);

    if (m_remote_slot < 0)
    {
        m_remote_sampled = false;
        return false;
    }

    Shared::Peer& peer = m_shared->peers[m_remote_slot];
    u32 generation = m_remote_generation;
    u32 write_index = peer.write_index.load(std::memory_order_acquire);
    u32 count = MIN(write_index, (u32)GEARTOGEAR_SHARED_EVENT_COUNT);

    for (u32 offset = 0; offset < count; offset++)
    {
        GearToGearLocalWireEvent event;
        GearToGearSharedWireEvent& source = peer.events[(write_index - 1 - offset) % GEARTOGEAR_SHARED_EVENT_COUNT];

        if (!geartogear_read_shared_event(source, generation, event))
        {
            m_status.seqlock_retries++;
            continue;
        }

        state.drive_mask = geartogear_map_remote_bits_to_local(event.drive_mask);
        state.levels = geartogear_map_remote_bits_to_local(event.levels);
        break;
    }

    if (!IsPeerReady(peer.state.load(std::memory_order_acquire)) ||
        peer.generation.load(std::memory_order_acquire) != generation)
    {
        UpdateRemoteIdentity(GetClockMicroseconds());
        state.drive_mask = 0;
        state.levels = 0x7F;
        return false;
    }

    m_remote_read_index = write_index;
    m_remote_state = state;
    m_remote_sampled = true;
    m_status.baseline_samples++;
    return true;
}

bool GearToGearManager::PollRemoteEvent(u64 through_local_cycle, GS_GearToGear_WireEvent& event)
{
    if (!EnsureAttached(through_local_cycle) || !IsCableConnected())
        return false;

    u64 now = GetClockMicroseconds();
    ReapStalePeers(now);
    UpdateRemoteIdentity(now);

    if (m_remote_slot < 0 || !m_remote_sampled)
        return false;

    Shared::Peer& peer = m_shared->peers[m_remote_slot];

    if (!IsPeerReady(peer.state.load(std::memory_order_acquire)) ||
        peer.generation.load(std::memory_order_acquire) != m_remote_generation)
    {
        UpdateRemoteIdentity(GetClockMicroseconds());
        return false;
    }

    u32 write_index = peer.write_index.load(std::memory_order_acquire);
    u32 distance = write_index - m_remote_read_index;

    if (distance > GEARTOGEAR_SHARED_EVENT_COUNT)
    {
        m_status.state_ring_overruns++;
#if !defined(NDEBUG)
        assert(false && "Gear-to-Gear event ring overrun");
#endif
        m_remote_read_index = write_index - GEARTOGEAR_SHARED_EVENT_COUNT;
        distance = GEARTOGEAR_SHARED_EVENT_COUNT;
    }

    if (distance == 0)
        return false;

    GearToGearLocalWireEvent local_event;
    GearToGearSharedWireEvent& source = peer.events[m_remote_read_index % GEARTOGEAR_SHARED_EVENT_COUNT];

    if (!geartogear_read_shared_event(source, m_remote_generation, local_event))
    {
        m_status.seqlock_retries++;
        return false;
    }

    if (local_event.cycle > ToBusCycle(through_local_cycle))
        return false;

    if (!IsPeerReady(peer.state.load(std::memory_order_acquire)) ||
        peer.generation.load(std::memory_order_acquire) != m_remote_generation)
    {
        UpdateRemoteIdentity(GetClockMicroseconds());
        return false;
    }

    m_remote_read_index++;
    event.cycle = FromBusCycle(local_event.cycle);
    event.state.drive_mask = geartogear_map_remote_bits_to_local(local_event.drive_mask);
    event.state.levels = geartogear_map_remote_bits_to_local(local_event.levels);
    m_remote_state = event.state;
    m_status.events_consumed++;
    return true;
}

void GearToGearManager::PublishProgress(u64 local_cycle, u32 lead_cycles)
{
    if (!EnsureAttached(local_cycle) || !m_hardware_ready)
        return;

    Shared::Peer& local = m_shared->peers[m_slot];
    u64 bus_cycle = ToBusCycle(local_cycle);
    StoreMax(local.progress_cycle, bus_cycle);
    StoreMax(local.promise_cycle, geartogear_saturating_add(bus_cycle, lead_cycles));
    local.heartbeat_us.store(GetClockMicroseconds(), std::memory_order_release);
}

void GearToGearManager::Fence(u64 local_cycle)
{
    if (!EnsureAttached(local_cycle) || !IsCableConnected())
        return;

    m_status.fence_calls++;
    u64 target = ToBusCycle(local_cycle);
    PublishProgress(local_cycle, GEARTOGEAR_MAX_LEAD_CYCLES);

    u64 now = GetClockMicroseconds();
    ReapStalePeers(now);
    UpdateRemoteIdentity(now);

    if (m_remote_slot < 0)
        return;

    int remote_slot = m_remote_slot;
    u32 remote_generation = m_remote_generation;
    u64 wait_started = 0;
    u64 progress_time = now;
    u64 previous_progress = 0;

    for (;;)
    {
        Shared::Peer& remote = m_shared->peers[remote_slot];
        if (!IsPeerReady(remote.state.load(std::memory_order_acquire)) ||
            remote.generation.load(std::memory_order_acquire) != remote_generation)
        {
            break;
        }

        u64 progress = remote.progress_cycle.load(std::memory_order_acquire);

        if (progress >= target)
            break;

        if (wait_started == 0)
        {
            wait_started = GetClockMicroseconds();
            progress_time = wait_started;
            previous_progress = progress;
            m_status.fence_waits++;
        }

        now = GetClockMicroseconds();
        if (progress != previous_progress)
        {
            previous_progress = progress;
            progress_time = now;
        }

        m_shared->peers[m_slot].heartbeat_us.store(now, std::memory_order_release);
        ReapStalePeers(now);

        if (now - progress_time >= m_normal_barrier_stall_us)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(GEARTOGEAR_BARRIER_SLEEP_US));
            m_status.sleep_calls++;
        }
        else
        {
            std::this_thread::yield();
            m_status.spin_iterations++;
        }
    }

    if (wait_started != 0)
    {
        u64 wait = GetClockMicroseconds() - wait_started;
        m_status.fence_wait_us += wait;
        m_status.fence_wait_max_us = MAX(m_status.fence_wait_max_us, wait);
    }

    UpdateRemoteIdentity(GetClockMicroseconds());
}

void GearToGearManager::Synchronize(u64 local_cycle, u32 lead_cycles)
{
    if (!EnsureAttached(local_cycle) || !IsCableConnected())
        return;

    m_status.sync_calls++;
    u64 now = GetClockMicroseconds();

    if (m_last_sync_exit_us != 0)
    {
        u64 gap = now - m_last_sync_exit_us;
        m_status.sync_gap_max_us = MAX(m_status.sync_gap_max_us, gap);
        if (gap >= 50000)
            m_status.sync_gap_over_50ms++;
    }

    ReapStalePeers(now);
    UpdateRemoteIdentity(now);
    PublishProgress(local_cycle, lead_cycles);

    u64 cycle = ToBusCycle(local_cycle);
    u64 wait_started = 0;
    u64 progress_time = now;
    u64 previous_floor = 0;

    for (;;)
    {
        u64 floor = ~0ULL;

        for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
        {
            Shared::Peer& peer = m_shared->peers[i];

            if (IsPeerReady(peer.state.load(std::memory_order_acquire)))
            {
                floor = MIN(floor, peer.promise_cycle.load(std::memory_order_acquire));
            }
        }

        if (floor == ~0ULL || cycle <= floor)
            break;

        if (wait_started == 0)
        {
            wait_started = GetClockMicroseconds();
            progress_time = wait_started;
            previous_floor = floor;
            m_status.barrier_waits++;
        }

        now = GetClockMicroseconds();

        if (floor != previous_floor)
        {
            previous_floor = floor;
            progress_time = now;
        }

        m_shared->peers[m_slot].heartbeat_us.store(now, std::memory_order_release);
        ReapStalePeers(now);

        if (now - progress_time >= m_normal_barrier_stall_us)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(GEARTOGEAR_BARRIER_SLEEP_US));
            m_status.sleep_calls++;
        }
        else
        {
            std::this_thread::yield();
            m_status.spin_iterations++;
        }
    }

    now = GetClockMicroseconds();

    if (wait_started != 0)
        RecordBarrierWait(now - wait_started);

    m_last_sync_exit_us = now;
    UpdateRemoteIdentity(now);
}

bool GearToGearManager::IsActive() const
{
    if (!m_shared || m_slot < 0)
        return false;

    const Shared::Peer& peer = m_shared->peers[m_slot];
    return IsPeerActive(peer.state.load(std::memory_order_acquire)) &&
        peer.generation.load(std::memory_order_acquire) == m_generation;
}

bool GearToGearManager::IsCableConnected() const
{
    if (!IsActive() || m_remote_slot < 0)
        return false;

    const Shared::Peer& peer = m_shared->peers[m_remote_slot];
    u64 heartbeat = peer.heartbeat_us.load(std::memory_order_acquire);
    return m_hardware_ready &&
        IsPeerReady(peer.state.load(std::memory_order_acquire)) &&
        peer.generation.load(std::memory_order_acquire) == m_remote_generation &&
        geartogear_heartbeat_age(GetClockMicroseconds(), heartbeat) <= GEARTOGEAR_DETACH_US;
}

bool GearToGearManager::IsHardwareReady() const
{
    return IsActive() && m_hardware_ready;
}

bool GearToGearManager::IsPacingPeer() const
{
    if (!IsActive())
        return false;

    for (int i = 0; i < m_slot; i++)
    {
        if (IsPeerActive(m_shared->peers[i].state.load(std::memory_order_acquire)))
            return false;
    }

    return true;
}

bool GearToGearManager::ConsumeLocalAttachmentChanged()
{
    bool changed = m_local_attachment_changed;
    m_local_attachment_changed = false;
    return changed;
}

bool GearToGearManager::ConsumeRemoteIdentityChanged()
{
    bool changed = m_remote_identity_changed;
    m_remote_identity_changed = false;
    return changed;
}

void GearToGearManager::SetNormalBarrierStallUs(u32 stall_us)
{
    m_normal_barrier_stall_us = stall_us;
}

void GearToGearManager::ResetMetrics()
{
    m_status.events_published = 0;
    m_status.events_consumed = 0;
    m_status.state_ring_overruns = 0;
    m_status.baseline_samples = 0;
    m_status.fence_calls = 0;
    m_status.fence_waits = 0;
    m_status.fence_wait_us = 0;
    m_status.fence_wait_max_us = 0;
    m_status.sync_calls = 0;
    m_status.barrier_waits = 0;
    m_status.barrier_wait_us = 0;
    m_status.barrier_wait_max_us = 0;
    m_status.barrier_wait_over_1ms = 0;
    m_status.barrier_wait_over_10ms = 0;
    m_status.barrier_wait_over_50ms = 0;
    m_status.sync_gap_max_us = 0;
    m_status.sync_gap_over_50ms = 0;
    m_status.spin_iterations = 0;
    m_status.sleep_calls = 0;
    m_status.peer_detaches = 0;
    m_status.peer_detach_max_age_us = 0;
    m_status.slot_reclaims = 0;
    m_status.seqlock_retries = 0;
    m_status.attachments = 0;
    m_last_sync_exit_us = GetClockMicroseconds();
}

GearToGearStatus GearToGearManager::GetStatus()
{
    if (m_shared)
    {
        u64 now = GetClockMicroseconds();
        ReapStalePeers(now);
        UpdateRemoteIdentity(now);
    }
    RefreshStatus();
    return m_status;
}

void GearToGearManager::RecordBarrierWait(u64 wait)
{
    m_status.barrier_wait_us += wait;
    m_status.barrier_wait_max_us = MAX(m_status.barrier_wait_max_us, wait);

    if (wait >= 1000)
        m_status.barrier_wait_over_1ms++;
    if (wait >= 10000)
        m_status.barrier_wait_over_10ms++;
    if (wait >= 50000)
        m_status.barrier_wait_over_50ms++;
}

bool GearToGearManager::Map(u8 session)
{
    bool created = false;

#if defined(_WIN32)
    char name[64];
    snprintf(name, sizeof(name),
        "Local\\gearsystem-geartogear-%u", session);

    HANDLE mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)sizeof(Shared), name);

    if (!mapping)
    {
        SetFault("Failed to create Gear-to-Gear shared memory");
        return false;
    }

    created = GetLastError() != ERROR_ALREADY_EXISTS;
    void* address = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared));

    if (!address)
    {
        CloseHandle(mapping);
        SetFault("Failed to map Gear-to-Gear shared memory");
        return false;
    }

    m_mapping_handle = mapping;
    m_shared = (Shared*)address;
#else
    char name[64];
    snprintf(name, sizeof(name), "/gearsystem-geartogear-%u", session);

    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    created = fd >= 0;

    if (!created && errno == EEXIST)
        fd = shm_open(name, O_RDWR, 0600);

    if (fd < 0)
    {
        SetFault("Failed to open Gear-to-Gear shared memory");
        return false;
    }

    if (created && ftruncate(fd, sizeof(Shared)) != 0)
    {
        close(fd);
        shm_unlink(name);
        SetFault("Failed to size Gear-to-Gear shared memory");
        return false;
    }

    if (!created)
    {
        u64 started = GetClockMicroseconds();
        struct stat status;

        while (fstat(fd, &status) != 0 || status.st_size < (off_t)sizeof(Shared))
        {
            if (GetClockMicroseconds() - started > GEARTOGEAR_DETACH_US)
            {
                close(fd);
                SetFault("Gear-to-Gear shared memory sizing timed out");
                return false;
            }
            std::this_thread::yield();
        }
    }

    void* address = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (address == MAP_FAILED)
    {
        close(fd);
        SetFault("Failed to map Gear-to-Gear shared memory");
        return false;
    }

    m_mapping_fd = fd;
    m_shared = (Shared*)address;
#endif

    if (created)
    {
        new (m_shared) Shared();
        m_shared->session = session;
        m_shared->magic.store(GEARTOGEAR_SHM_MAGIC, std::memory_order_release);
    }
    else
    {
        u64 started = GetClockMicroseconds();
        for (;;)
        {
            u32 magic = m_shared->magic.load(std::memory_order_acquire);

            if (magic == GEARTOGEAR_SHM_MAGIC)
                break;

            if (magic != 0)
            {
                Unmap();
                SetFault("Incompatible Gear-to-Gear shared memory");
                return false;
            }

            if (GetClockMicroseconds() - started > GEARTOGEAR_DETACH_US)
            {
                Unmap();
                SetFault("Gear-to-Gear shared memory initialization timed out");
                return false;
            }

            std::this_thread::yield();
        }

        if (m_shared->version != GEARTOGEAR_SHM_VERSION || m_shared->session != session)
        {
            Unmap();
            SetFault("Incompatible Gear-to-Gear shared memory");
            return false;
        }
    }

    if (!SharedAtomicsLockFree())
    {
        Unmap();
        SetFault("Gear-to-Gear shared atomics are not lock-free");
        return false;
    }

    return true;
}

void GearToGearManager::Unmap()
{
    if (!m_shared)
        return;

#if defined(_WIN32)
    UnmapViewOfFile(m_shared);
    if (m_mapping_handle)
        CloseHandle((HANDLE)m_mapping_handle);
#else
    munmap(m_shared, sizeof(Shared));
    if (m_mapping_fd >= 0)
        close(m_mapping_fd);
#endif

    m_shared = NULL;
    m_mapping_handle = NULL;
    m_mapping_fd = -1;
}

bool GearToGearManager::ClaimSlot(u64 local_cycle, bool reattach)
{
    u64 now = GetClockMicroseconds();
    ReapStalePeers(now, true);

    u64 bus_anchor = 0;

    for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
    {
        Shared::Peer& peer = m_shared->peers[i];
        if (IsPeerReady(peer.state.load(std::memory_order_acquire)))
        {
            bus_anchor = MAX(bus_anchor, peer.progress_cycle.load(std::memory_order_acquire));
        }
    }

    for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
    {
        Shared::Peer& peer = m_shared->peers[i];
        u32 expected = GEARTOGEAR_PEER_FREE;
        if (!peer.state.compare_exchange_strong(expected, GEARTOGEAR_PEER_CLAIMING, std::memory_order_acq_rel))
        {
            continue;
        }

        m_slot = i;
        m_generation = peer.generation.fetch_add(1, std::memory_order_acq_rel) + 1;
        m_local_anchor = local_cycle;
        m_bus_anchor = bus_anchor;
        m_last_local_cycle = local_cycle;

        peer.write_index.store(0, std::memory_order_relaxed);
        peer.progress_cycle.store(bus_anchor, std::memory_order_relaxed);
        peer.promise_cycle.store(geartogear_saturating_add(bus_anchor, GEARTOGEAR_MAX_LEAD_CYCLES), std::memory_order_relaxed);
        peer.heartbeat_us.store(now, std::memory_order_relaxed);
        peer.state.store(GEARTOGEAR_PEER_ACTIVE, std::memory_order_release);

        m_has_last_published_state = false;
        m_remote_slot = -1;
        m_remote_generation = 0;
        m_remote_read_index = 0;
        m_remote_sampled = false;
        m_remote_state.drive_mask = 0;
        m_remote_state.levels = 0x7F;
        m_hardware_ready = false;
        m_local_attachment_changed = true;
        m_remote_identity_changed = true;
        m_status.attachments++;

        if (reattach)
        {
            m_status.slot_reclaims++;
            m_status.mode = GearToGearModeConnected;
            m_status.active = true;
            m_status.last_error[0] = '\0';
            Log("Gear-to-Gear: reattached to shared session %u", m_session);
        }

        return true;
    }

    return false;
}

bool GearToGearManager::EnsureAttached(u64 local_cycle)
{
    if (!m_shared)
        return false;

    m_last_local_cycle = local_cycle;

    if (m_slot >= 0)
    {
        Shared::Peer& peer = m_shared->peers[m_slot];

        if (IsPeerActive(peer.state.load(std::memory_order_acquire)) &&
            peer.generation.load(std::memory_order_acquire) == m_generation)
        {
            return true;
        }
    }

    m_slot = -1;
    m_generation = 0;
    m_hardware_ready = false;

    if (ClaimSlot(local_cycle, true))
        return true;

    SetFault("Gear-to-Gear session is full after reattachment");
    return false;
}

void GearToGearManager::ReapStalePeers(u64 now_us, bool preserve_idle)
{
    if (!m_shared)
        return;

    for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
    {
        if (i == m_slot)
            continue;

        Shared::Peer& peer = m_shared->peers[i];
        u64 heartbeat = peer.heartbeat_us.load(std::memory_order_acquire);
        u32 generation = peer.generation.load(std::memory_order_acquire);
        u64 age = geartogear_heartbeat_age(now_us, heartbeat);

        u32 observed_state = peer.state.load(std::memory_order_acquire);
        if (!IsPeerActive(observed_state) || age <= GEARTOGEAR_DETACH_US)
        {
            continue;
        }

        if (preserve_idle && peer.write_index.load(std::memory_order_acquire) == 0)
        {
            continue;
        }

        if (!IsPeerActive(peer.state.load(std::memory_order_acquire)))
            continue;

        u32 current_generation = peer.generation.load(std::memory_order_acquire);
        u64 current_heartbeat = peer.heartbeat_us.load(std::memory_order_acquire);

        if (!geartogear_lease_is_unchanged_and_stale(now_us, heartbeat, generation, current_heartbeat, current_generation))
        {
            continue;
        }

        u32 expected = observed_state;

        if (peer.state.compare_exchange_strong(expected, 0, std::memory_order_acq_rel))
        {
            m_status.peer_detaches++;
            m_status.peer_detach_max_age_us =
                MAX(m_status.peer_detach_max_age_us, age);
        }
    }
}

int GearToGearManager::FindRemoteSlot(u64 now_us, u32* generation) const
{
    if (!m_shared)
        return -1;

    for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
    {
        if (i == m_slot)
            continue;

        const Shared::Peer& peer = m_shared->peers[i];

        if (!IsPeerReady(peer.state.load(std::memory_order_acquire)))
            continue;

        u64 heartbeat = peer.heartbeat_us.load(std::memory_order_acquire);

        if (geartogear_heartbeat_age(now_us, heartbeat) > GEARTOGEAR_DETACH_US)
        {
            continue;
        }

        if (generation)
            *generation = peer.generation.load(std::memory_order_acquire);

        return i;
    }

    return -1;
}

void GearToGearManager::UpdateRemoteIdentity(u64 now_us)
{
    u32 generation = 0;
    int slot = m_hardware_ready ? FindRemoteSlot(now_us, &generation) : -1;

    if (slot == m_remote_slot && generation == m_remote_generation)
        return;

    m_remote_slot = slot;
    m_remote_generation = generation;
    m_remote_read_index = 0;
    m_remote_sampled = false;
    m_remote_state.drive_mask = 0;
    m_remote_state.levels = 0x7F;
    m_remote_identity_changed = true;
}

bool GearToGearManager::SharedAtomicsLockFree() const
{
    if (!m_shared->magic.is_lock_free())
        return false;

    for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
    {
        const Shared::Peer& peer = m_shared->peers[i];
        if (!peer.state.is_lock_free() || !peer.generation.is_lock_free() ||
            !peer.heartbeat_us.is_lock_free() ||
            !peer.progress_cycle.is_lock_free() ||
            !peer.promise_cycle.is_lock_free() ||
            !peer.write_index.is_lock_free())
        {
            return false;
        }

        for (int event = 0; event < GEARTOGEAR_SHARED_EVENT_COUNT; event++)
        {
            if (!geartogear_shared_event_atomics_lock_free(peer.events[event]))
            {
                return false;
            }
        }
    }

    return true;
}

u64 GearToGearManager::ToBusCycle(u64 local_cycle) const
{
    if (local_cycle >= m_local_anchor)
    {
        return geartogear_saturating_add(m_bus_anchor,
            local_cycle - m_local_anchor);
    }

    u64 distance = m_local_anchor - local_cycle;
    return distance > m_bus_anchor ? 0 : m_bus_anchor - distance;
}

u64 GearToGearManager::FromBusCycle(u64 bus_cycle) const
{
    if (bus_cycle >= m_bus_anchor)
    {
        return geartogear_saturating_add(m_local_anchor, bus_cycle - m_bus_anchor);
    }

    u64 distance = m_bus_anchor - bus_cycle;
    return distance > m_local_anchor ? 0 : m_local_anchor - distance;
}

u64 GearToGearManager::GetClockMicroseconds() const
{
    return (u64)std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void GearToGearManager::SetFault(const char* message)
{
    m_status.mode = GearToGearModeFault;
    m_status.active = false;
    m_status.cable_connected = false;
    snprintf(m_status.last_error, sizeof(m_status.last_error), "%s", message);
    Error("Gear-to-Gear: %s", message);
}

void GearToGearManager::RefreshStatus()
{
    m_status.active = IsActive();
    m_status.session = m_session;
    m_status.local_anchor = m_local_anchor;
    m_status.bus_anchor = m_bus_anchor;
    m_status.bus_cycle = m_status.active ? ToBusCycle(m_last_local_cycle) : 0;
    m_status.remote_generation = m_remote_generation;
    m_status.local_hardware_ready = false;
    m_status.remote_hardware_ready = false;
    m_status.local_progress = 0;
    m_status.local_promise = 0;
    m_status.remote_progress = 0;
    m_status.remote_promise = 0;
    m_status.local_peer_id = 0;
    m_status.peer_count = 0;

    if (m_shared)
    {
        u64 now = GetClockMicroseconds();

        for (int i = 0; i < GEARTOGEAR_MAX_PEERS; i++)
        {
            Shared::Peer& peer = m_shared->peers[i];
            u64 heartbeat = peer.heartbeat_us.load(std::memory_order_acquire);
            u32 state = peer.state.load(std::memory_order_acquire);

            if (IsPeerActive(state) && geartogear_heartbeat_age(now, heartbeat) <= GEARTOGEAR_DETACH_US)
            {
                m_status.peer_count++;

                if (i == m_slot)
                {
                    m_status.local_peer_id = (u8)m_status.peer_count;
                    m_status.local_hardware_ready = IsPeerReady(state);

                    if (m_status.local_hardware_ready)
                    {
                        m_status.local_progress = peer.progress_cycle.load(
                            std::memory_order_acquire);
                        m_status.local_promise = peer.promise_cycle.load(
                            std::memory_order_acquire);
                    }
                }
                else if (IsPeerReady(state))
                {
                    m_status.remote_hardware_ready = true;
                    m_status.remote_progress = peer.progress_cycle.load(std::memory_order_acquire);
                    m_status.remote_promise = peer.promise_cycle.load(std::memory_order_acquire);
                }
            }
        }
    }

    m_status.cable_connected = m_status.active && m_status.local_hardware_ready && m_status.remote_hardware_ready;
    m_status.pacing_peer = m_status.cable_connected && IsPacingPeer();
}
