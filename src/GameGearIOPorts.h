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

#ifndef GAMEGEARIOPORTS_H
#define	GAMEGEARIOPORTS_H

#include "IOPorts.h"
#include "geartogear.h"

class Audio;
class Video;
class Input;
class Cartridge;
class Memory;
class Processor;
class TraceLogger;

class GameGearIOPorts : public IOPorts
{
public:
    GameGearIOPorts(Audio* pAudio, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Memory* pMemory, Processor* pProcessor);
    virtual ~GameGearIOPorts();
    void Reset();
    virtual u8 DoInput(u8 port);
    virtual void DoOutput(u8 port, u8 value);
    virtual void SaveState(std::ostream& stream);
    virtual void LoadState(std::istream& stream, int version);
    void SetTraceLogger(TraceLogger* pTraceLogger);
    void SetGearToGearCallbacks(
        GS_GearToGear_Publish_Callback publish_callback,
        GS_GearToGear_Sample_Callback sample_callback,
        GS_GearToGear_Poll_Callback poll_callback,
        GS_GearToGear_Fence_Callback fence_callback,
        GS_GearToGear_Sync_Callback sync_callback,
        void* user_data);
    void SetGearToGearTransportActive(bool active, u64 cycle);
    void SetGearToGearCableConnected(bool connected, u64 cycle);
    void BeginInstruction(u64 cycle);
    void EndInstruction(u64 cycle);
    void RebaseGearToGear(u64 cycle);
    bool IsGearToGearCableConnected() const;
    GS_GearToGear_WireState GetGearToGearWireState() const;
    u8 GetGearToGearResolvedPins() const;
    u8 GetGearToGearContentionMask() const;
    GS_GearToGear_DebugState GetGearToGearDebugState() const;

private:
    struct GearToGearTxState
    {
        bool busy;
        bool line;
        u8 data;
        u8 phase;
        u32 bit_cycles;
        u64 next_cycle;
    };

    struct GearToGearRxState
    {
        enum State
        {
            Idle = 0,
            ConfirmStart,
            Data,
            Stop
        } state;

        bool ready;
        bool frame_error;
        u8 shift;
        u8 bit;
        u32 bit_cycles;
        u32 half_bit_cycles;
        u64 next_cycle;
    };

    struct GearToGearNmiState
    {
        bool parallel_latch;
        bool serial_latch;
        bool output_asserted;
        bool parallel_armed;
        u8 parallel_arm_delay;
        bool previous_pc6;
    };

    INLINE void TraceInputReadEvent(u8 port, u8 raw, u8 effective, u8 player);
    INLINE void TraceIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous = 0, u8 auxiliary = 0);
    INLINE void TraceGearToGearEvent(u8 event, u8 data = 0);
    INLINE bool IsNativeGameGearMode() const;
    u8 ReadGearToGearPort(u8 port);
    void WriteGearToGearPort(u8 port, u8 value);
    void ResetGearToGearHardware();
    GS_GearToGear_WireState ComputeLocalWireState() const;
    void RefreshLocalWireState(u64 cycle, bool force_publish, bool detect_rx_edge = true);
    bool ResolveGearToGearPin(int bit) const;
    u8 ResolveGearToGearPins() const;
    void ApplyRemoteWireState(const GS_GearToGear_WireState& state, u64 cycle, bool detect_edges);
    void HandleGearToGearRxEdge(bool old_level, bool new_level, u64 cycle);
    void HandleGearToGearPC6Edge(bool old_level, bool new_level);
    void UpdateGearToGearNMI();
    void FenceGearToGearRead();
    u8 ReadGearToGearStatus() const;
    void StartGearToGearTx(u8 value, u64 cycle);
    void AbortGearToGearTx(u64 cycle);
    void ProcessGearToGearTxBoundary();
    void AbortGearToGearRx();
    void ProcessGearToGearRxSample();
    void FetchPendingRemoteEvent(u64 target_cycle);
    void AdvanceGearToGearTo(u64 target_cycle);
    void SaveGearToGearState(std::ostream& stream);
    void LoadGearToGearState(std::istream& stream);
    void LogInputReadEvent(u8 port, u8 raw, u8 effective, u8 player);
    void LogIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary);
    void LogGearToGearEvent(u8 event, u8 data);
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Memory* m_pMemory;
    Cartridge* m_pCartridge;
    Processor* m_pProcessor;
    TraceLogger* m_pTraceLogger;
    u8 m_Port3F;
    u8 m_Ports[6];

    GS_GearToGear_Publish_Callback m_geartogear_publish_callback;
    GS_GearToGear_Sample_Callback m_geartogear_sample_callback;
    GS_GearToGear_Poll_Callback m_geartogear_poll_callback;
    GS_GearToGear_Fence_Callback m_geartogear_fence_callback;
    GS_GearToGear_Sync_Callback m_geartogear_sync_callback;
    void* m_geartogear_user_data;
    bool m_geartogear_transport_active;
    bool m_geartogear_cable_connected;
    u64 m_geartogear_cycle;
    u64 m_geartogear_next_sync_cycle;
    GS_GearToGear_WireState m_geartogear_local_state;
    GS_GearToGear_WireState m_geartogear_remote_state;
    GS_GearToGear_WireState m_geartogear_last_published_state;
    bool m_geartogear_has_published_state;
    bool m_geartogear_has_pending_remote_event;
    GS_GearToGear_WireEvent m_geartogear_pending_remote_event;
    GearToGearTxState m_geartogear_tx;
    GearToGearRxState m_geartogear_rx;
    GearToGearNmiState m_geartogear_nmi;
    u8 m_geartogear_last_nmi_trace_state;
};

#include "Audio.h"
#include "Video.h"
#include "Input.h"
#include "Cartridge.h"
#include "Memory.h"
#include "Processor.h"
#include "TraceLogger.h"

INLINE bool GameGearIOPorts::IsNativeGameGearMode() const
{
    return m_pCartridge->IsReady() && m_pCartridge->IsGameGear() && !m_pCartridge->IsGameGearInSMSMode();
}

INLINE void GameGearIOPorts::TraceInputReadEvent(u8 port, u8 raw, u8 effective, u8 player)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_INPUT, TRACE_INPUT_READ))
        LogInputReadEvent(port, raw, effective, player);
}

INLINE void GameGearIOPorts::TraceIOEvent(u8 event, u8 port, u8 raw, u8 effective, u8 previous, u8 auxiliary)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_IO, event))
        LogIOEvent(event, port, raw, effective, previous, auxiliary);
}

INLINE void GameGearIOPorts::TraceGearToGearEvent(u8 event, u8 data)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_GEARTOGEAR, event))
        LogGearToGearEvent(event, data);
}

inline u8 GameGearIOPorts::DoInput(u8 port)
{
    if (port < 0x07)
    {
        if (port == 0x00)
        {
            u8 start_button = m_pInput->GetPort00();
            u8 raw = start_button;
            if (m_pCartridge->GetZone() != Cartridge::CartridgeJapanGG)
                start_button |= 0x40;
            TraceInputReadEvent(port, raw, start_button, 1);
            TraceIOEvent(TRACE_IO_GAMEGEAR_READ, port, raw, start_button);
            return start_button;
        }
        else if (port < 6)
        {
            u8 raw = m_Ports[port];
            u8 value = IsNativeGameGearMode() ? ReadGearToGearPort(port) : raw;
            TraceIOEvent(TRACE_IO_GAMEGEAR_READ, port, raw, value);
            return value;
        }
        else
            return 0xFF;
    }
    else if (port < 0x40)
    {
        // Reads return $FF (GG)
        Debug("--> ** Attempting to read from port $%X", port);
        return 0xFF;
    }
    else if (port < 0x80)
    {
        // Reads from even addresses return the V counter
        if ((port & 0x01) == 0x00)
        {
            u8 value = m_pVideo->GetVCounter();
            TraceIOEvent(TRACE_IO_COUNTER_READ, port, value, value, 0, 0);
            return value;
        }
        // Reads from odd addresses return the H counter
        else
        {
            u8 value = m_pVideo->GetHCounter();
            TraceIOEvent(TRACE_IO_COUNTER_READ, port, value, value, 0, 1);
            return value;
        }
    }
    else if (port < 0xC0)
    {
        // Reads from even addresses return the VDP data port contents
        if ((port & 0x01) == 0x00)
            return m_pVideo->GetDataPort();
        // Reads from odd address return the VDP status flags
        else
            return m_pVideo->GetStatusFlags();
    }
    else
    {
        switch (port)
        {
            // Reads from $C0 and $DC return the I/O port A/B register.
            case 0xC0:
            case 0xDC:
            {
                u8 value = m_pInput->GetPortDC();
                TraceInputReadEvent(port, value, value, 1);
                return value;
            }
            // Reads from $C1 and $DD return the I/O port B/misc. register.
            case 0xC1:
            case 0xDD:
            {
                u8 raw = m_pInput->GetPortDD();
                u8 value = (raw & 0x3F) | (m_Port3F & 0xC0);
                TraceInputReadEvent(port, raw, value, 2);
                return value;
            }
            // The remaining locations return $FF.
            default:
            {
                Debug("--> ** Attempting to read from port $%X", port);
                return 0xFF;
            }
        }
    }
}

inline void GameGearIOPorts::DoOutput(u8 port, u8 value)
{
    if (port < 0x07)
    {
        if (port == 0x06)
        {
            // SN76489 PSG
            m_pAudio->WriteGGStereoRegister(value);
        }
        else if (port != 0x00)
        {
            u8 previous = m_Ports[port];
            if (IsNativeGameGearMode())
                WriteGearToGearPort(port, value);
            else
                m_Ports[port] = value;
            TraceIOEvent(TRACE_IO_GAMEGEAR_WRITE, port, value, m_Ports[port], previous);
        }
    }
    else if (port < 0x40)
    {
        // Writes to even addresses go to memory control register.
        if ((port & 0x01) == 0x00)
        {
            Debug("--> ** Output to memory control port $%X: %X", port, value);
            m_pMemory->SetPort3E(value);
            TraceIOEvent(TRACE_IO_CONTROL, port, value, value);
        }
        // Writes to odd addresses go to I/O control register.
        else
        {
            u8 previous = m_Port3F;
            bool th_changed_a = (value & 0x02) && (value & 0x20) && !(m_Port3F & 0x20);
            bool th_changed_b = (value & 0x08) && (value & 0x80) && !(m_Port3F & 0x80);

            if (th_changed_a || th_changed_b)
            {
                m_pVideo->LatchHCounter();
                TraceIOEvent(TRACE_IO_COUNTER_LATCH, port, value, 0, previous, (th_changed_a ? 1 : 0) | (th_changed_b ? 2 : 0));
            }

            m_Port3F = value;
            TraceIOEvent(TRACE_IO_CONTROL, port, value, m_Port3F, previous);
        }
    }
    else if (port < 0x80)
    {
        // Writes to any address go to the SN76489 PSG
        m_pAudio->WriteAudioRegister(value);
    }
    else if (port < 0xC0)
    {
        // Writes to even addresses go to the VDP data port.
        if ((port & 0x01) == 0x00)
            m_pVideo->WriteData(value);
        // Writes to odd addresses go to the VDP control port.
        else
            m_pVideo->WriteControl(value);
    }
#ifdef GS_DEBUG
    else
    {
        // Writes have no effect.
        if ((port == 0xDE) || (port == 0xDF))
        {
            Debug("--> ** Output to keyboard port $%X: %X", port, value);
        }
        else if ((port == 0xF0) || (port == 0xF1) || (port == 0xF2))
        {
            Debug("--> ** Output to YM2413 port $%X: %X", port, value);
        }
        else
        {
            Debug("--> ** Output to port $%X: %X", port, value);
        }
    }
#endif
}

#endif	/* GAMEGEARIOPORTS_H */
