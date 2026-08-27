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

#include <math.h>
#include "Input.h"
#include "Memory.h"
#include "Processor.h"
#include "TraceLogger.h"

static const u64 kSportsPadRelativeTimeout = 512;
static const u64 kSportsPadModeTimeoutSeconds = 3;

Input::Input(Processor* pProcessor, Video* pVideo, const u64* pMasterClockCycles)
{
    m_pProccesor = pProcessor;
    m_pVideo = pVideo;
    m_pMasterClockCycles = pMasterClockCycles;
    InitPointer(m_pTraceLogger);
    m_bGameGear = false;
    m_Joypad1 = 0;
    m_Joypad2 = 0;
    m_GlassesRegistry = 0;
    m_bPhaser = false;
    m_Phaser.x = 0;
    m_Phaser.y = 0;
    m_PhaserOffset.x = 0;
    m_PhaserOffset.y = 0;
    m_bPaddle = false;
    m_Paddle.x = 128.0f;
    m_Paddle.reg = 128;
    m_Paddle.flip = 0;
    m_iSportsPadClockRate = GS_MASTER_CLOCK_NTSC;
    m_iSportsPadCyclesPerPhase = 1;
    m_SportsPad[0].enabled = false;
    m_SportsPad[1].enabled = false;
    ResetSportsPad(&m_SportsPad[0]);
    ResetSportsPad(&m_SportsPad[1]);
    m_bResetPressed = false;
}

void Input::Init()
{
    Reset(false);
}

void Input::Reset(bool bGameGear, bool bPAL)
{
    m_bGameGear = bGameGear;
    m_Joypad1 = 0xFF;
    m_Joypad2 = 0xFF;
    m_GlassesRegistry = 0;
    m_Phaser.x = 0;
    m_Phaser.y = 0;
    m_Paddle.x = 128.0f;
    m_Paddle.reg = 128;
    m_Paddle.flip = 0;
    m_iSportsPadClockRate = bPAL ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC;
    m_iSportsPadCyclesPerPhase = ((m_iSportsPadClockRate * 238) + 2500000) / 5000000;
    if (m_iSportsPadCyclesPerPhase == 0)
        m_iSportsPadCyclesPerPhase = 1;
    ResetSportsPad(&m_SportsPad[0]);
    ResetSportsPad(&m_SportsPad[1]);
    m_bResetPressed = false;
}

void Input::ResetSportsPad(stSportsPad* sports_pad)
{
    bool enabled = sports_pad->enabled;
    sports_pad->pending_x = 0.0f;
    sports_pad->pending_y = 0.0f;
    sports_pad->absolute_x = 0;
    sports_pad->absolute_y = 0;
    sports_pad->relative_x = 0;
    sports_pad->relative_y = 0;
    sports_pad->relative_phase = 3;
    sports_pad->absolute_phase = 0;
    sports_pad->mode = SportsPadModeUnknown;
    sports_pad->th = true;
    sports_pad->absolute_phase_cycles = 0;
    sports_pad->unknown_cycles = 0;
    sports_pad->th_idle_cycles = 0;
    sports_pad->last_clock = m_pMasterClockCycles ? *m_pMasterClockCycles : 0;
    sports_pad->enabled = enabled;
}

void Input::SyncSportsPad(stSportsPad* sports_pad)
{
    if (!m_pMasterClockCycles)
        return;

    u64 current_clock = *m_pMasterClockCycles;
    if (current_clock < sports_pad->last_clock)
    {
        sports_pad->last_clock = current_clock;
        return;
    }

    u64 elapsed = current_clock - sports_pad->last_clock;
    sports_pad->last_clock = current_clock;

    if (sports_pad->mode == SportsPadModeUnknown)
    {
        sports_pad->unknown_cycles += elapsed;
        if (sports_pad->unknown_cycles >= ((u64)m_iSportsPadClockRate * kSportsPadModeTimeoutSeconds))
        {
            sports_pad->mode = SportsPadModeAbsolute;
            sports_pad->absolute_phase = 0;
            sports_pad->absolute_phase_cycles = 0;
            ConsumeSportsPadAbsolute(sports_pad);
        }
    }
    else if (sports_pad->mode == SportsPadModeRelative)
    {
        sports_pad->th_idle_cycles += elapsed;
    }
    else
    {
        u64 phase_cycles = sports_pad->absolute_phase_cycles + elapsed;
        sports_pad->absolute_phase = (sports_pad->absolute_phase + (phase_cycles / m_iSportsPadCyclesPerPhase)) % 5;
        sports_pad->absolute_phase_cycles = phase_cycles % m_iSportsPadCyclesPerPhase;
        ConsumeSportsPadAbsolute(sports_pad);
    }
}

void Input::ConsumeSportsPadAbsolute(stSportsPad* sports_pad)
{
    int movement_x = (int)sports_pad->pending_x;
    int movement_y = (int)sports_pad->pending_y;

    sports_pad->pending_x -= movement_x;
    sports_pad->pending_y -= movement_y;
    sports_pad->absolute_x = (u8)(sports_pad->absolute_x + movement_x);
    sports_pad->absolute_y = (u8)(sports_pad->absolute_y + movement_y);
}

u8 Input::ConsumeSportsPadRelative(float* pending)
{
    int movement = (int)*pending;
    if (movement < -127)
        movement = -127;
    else if (movement > 128)
        movement = 128;

    *pending -= movement;
    return (u8)-movement;
}

void Input::SportsPadTHChanged(u8 port, bool th)
{
    stSportsPad* sports_pad = &m_SportsPad[port];
    if (!sports_pad->enabled || sports_pad->th == th)
        return;

    SyncSportsPad(sports_pad);

    if (sports_pad->mode == SportsPadModeAbsolute)
    {
        sports_pad->th = th;
        return;
    }

    if (sports_pad->mode == SportsPadModeUnknown)
    {
        sports_pad->mode = SportsPadModeRelative;
        sports_pad->relative_phase = 3;
        sports_pad->th_idle_cycles = kSportsPadRelativeTimeout + 1;
    }

    if (!th)
    {
        if ((sports_pad->th_idle_cycles > kSportsPadRelativeTimeout) || (sports_pad->relative_phase == 3))
            sports_pad->relative_phase = 0;
        else if (sports_pad->relative_phase == 1)
            sports_pad->relative_phase = 2;
        else
            sports_pad->relative_phase = 0;

        if (sports_pad->relative_phase == 0)
        {
            sports_pad->relative_x = ConsumeSportsPadRelative(&sports_pad->pending_x);
            sports_pad->relative_y = ConsumeSportsPadRelative(&sports_pad->pending_y);
        }
    }
    else
    {
        if (sports_pad->relative_phase == 0)
            sports_pad->relative_phase = 1;
        else if (sports_pad->relative_phase == 2)
            sports_pad->relative_phase = 3;
    }

    sports_pad->th = th;
    sports_pad->th_idle_cycles = 0;
}

u8 Input::GetSportsPadPort(u8 port)
{
    stSportsPad* sports_pad = &m_SportsPad[port];
    u8 joypad = port == 0 ? m_Joypad1 : m_Joypad2;
    SyncSportsPad(sports_pad);

    if (sports_pad->mode == SportsPadModeRelative)
    {
        u8 value = joypad & 0x30;
        switch (sports_pad->relative_phase)
        {
            case 0: value |= sports_pad->relative_x >> 4; break;
            case 1: value |= sports_pad->relative_x & 0x0F; break;
            case 2: value |= sports_pad->relative_y >> 4; break;
            case 3: value |= sports_pad->relative_y & 0x0F; break;
        }
        return value;
    }

    if (sports_pad->mode == SportsPadModeAbsolute)
    {
        switch (sports_pad->absolute_phase)
        {
            case 0: return 0x34 | ((joypad >> 4) & 0x03);
            case 1: return sports_pad->absolute_x >> 4;
            case 2: return 0x10 | (sports_pad->absolute_x & 0x0F);
            case 3: return sports_pad->absolute_y >> 4;
            case 4: return 0x10 | (sports_pad->absolute_y & 0x0F);
        }
    }

    return (joypad & 0x30) | 0x0F;
}

void Input::SetReset(bool pressed)
{
    bool previous = m_bResetPressed;
    m_bResetPressed = pressed;
    if (previous != pressed)
        TraceInputChangeEvent(0, Key_Start, previous ? 0 : 1, pressed ? 0 : 1);
}

void Input::KeyPressed(GS_Joypads joypad, GS_Keys key)
{
    u8 previous = joypad == Joypad_1 ? m_Joypad1 : m_Joypad2;
    if (joypad == Joypad_1)
    {
        if (!m_bGameGear && (key == Key_Start) && (m_Joypad1 & Key_Start))
            m_pProccesor->RequestNMI();
        m_Joypad1 &= ~key;
    }
    else
        m_Joypad2 &= ~key;

    TraceInputChangeEvent((u8)joypad + 1, (u8)key, previous, joypad == Joypad_1 ? m_Joypad1 : m_Joypad2);

    if (!m_bGameGear && m_bPhaser && (key == Key_1))
    {
        m_pVideo->SetPhaserCoordinates(m_Phaser.x + m_PhaserOffset.x, m_Phaser.y + m_PhaserOffset.y);
    }
}

void Input::KeyReleased(GS_Joypads joypad, GS_Keys key)
{
    u8 previous = joypad == Joypad_1 ? m_Joypad1 : m_Joypad2;
    if (joypad == Joypad_1)
        m_Joypad1 |= key;
    else
        m_Joypad2 |= key;
    TraceInputChangeEvent((u8)joypad + 1, (u8)key, previous, joypad == Joypad_1 ? m_Joypad1 : m_Joypad2);
}

void Input::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void Input::LogInputChangeEvent(u8 player, u8 key, u8 previous, u8 effective)
{
#if !defined(GS_DISABLE_DISASSEMBLER)
    if (previous == effective)
        return;
    GS_Trace_Entry e = {};
    e.type = TRACE_INPUT;
    e.input.event = TRACE_INPUT_CHANGE;
    e.input.port = key;
    e.input.raw = previous;
    e.input.effective = effective;
    e.input.player = player;
    e.input.device = m_bPaddle ? 2 : (m_bPhaser ? 1 : 0);
    if ((player > 0) && (player <= 2) && m_SportsPad[player - 1].enabled)
        e.input.device = 3;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(player);
    UNUSED(key);
    UNUSED(previous);
    UNUSED(effective);
#endif
}

bool Input::IsKeyPressed(GS_Joypads joypad, GS_Keys key) const
{
    return joypad == Joypad_1 ? !(m_Joypad1 & key) : !(m_Joypad2 & key);
}

void Input::EnablePhaser(bool enable)
{
    Debug("Light Phaser %s", enable ? "enabled" : "disabled");
    m_bPhaser = enable;
}

void Input::SetPhaser(int x, int y)
{
    m_Phaser.x = x;
    m_Phaser.y = y;
}

void Input::SetPhaserOffset(int x, int y)
{
    m_PhaserOffset.x = x;
    m_PhaserOffset.y = y;
}

Input::stPhaser* Input::GetPhaser()
{
    return &m_Phaser;
}

bool Input::IsPhaserEnabled()
{
    return m_bPhaser;
}

void Input::EnablePaddle(bool enable)
{
    Debug("Paddle %s", enable ? "enabled" : "disabled");
    m_bPaddle = enable;
}

void Input::SetPaddle(float x)
{
    m_Paddle.x += x;
    if (m_Paddle.x < 0.0f)
        m_Paddle.x = 0.0f;
    else if (m_Paddle.x > 255.0f)
        m_Paddle.x = 255.0f;

    m_Paddle.reg = (u8)floor(m_Paddle.x + 0.5f);
}

bool Input::IsPaddleEnabled()
{
    return m_bPaddle;
}

void Input::EnableSportsPad(GS_Joypads joypad, bool enable)
{
    stSportsPad* sports_pad = &m_SportsPad[(u8)joypad];
    if (sports_pad->enabled == enable)
        return;

    Debug("Sports Pad %d %s", (int)joypad + 1, enable ? "enabled" : "disabled");
    sports_pad->enabled = enable;
    ResetSportsPad(sports_pad);
}

void Input::MoveSportsPad(GS_Joypads joypad, float x, float y)
{
    stSportsPad* sports_pad = &m_SportsPad[(u8)joypad];
    if (!sports_pad->enabled)
        return;

    SyncSportsPad(sports_pad);
    sports_pad->pending_x += x;
    sports_pad->pending_y += y;

    if (sports_pad->mode == SportsPadModeAbsolute)
        ConsumeSportsPadAbsolute(sports_pad);
}

bool Input::IsSportsPadEnabled(GS_Joypads joypad) const
{
    return m_SportsPad[(u8)joypad].enabled;
}

void Input::WriteSportsPadControl(u8 previous, u8 value)
{
    static const u8 direction_masks[2] = {0x02, 0x08};
    static const u8 level_masks[2] = {0x20, 0x80};

    for (u8 port = 0; port < 2; port++)
    {
        bool previous_th = (previous & direction_masks[port]) ? true : (previous & level_masks[port]);
        bool current_th = (value & direction_masks[port]) ? true : (value & level_masks[port]);

        if (m_SportsPad[port].enabled && (m_SportsPad[port].th != previous_th))
            m_SportsPad[port].th = previous_th;

        if (previous_th != current_th)
            SportsPadTHChanged(port, current_th);
    }
}

u8 Input::GetGlassesRegistry()
{
    return m_GlassesRegistry;
}

void Input::SetGlassesRegistry(u8 value)
{
    m_GlassesRegistry = value;
}

void Input::SaveState(std::ostream& stream)
{
    using namespace std;

    SyncSportsPad(&m_SportsPad[0]);
    SyncSportsPad(&m_SportsPad[1]);

    stream.write(reinterpret_cast<const char*> (&m_Joypad1), sizeof(m_Joypad1));
    stream.write(reinterpret_cast<const char*> (&m_Joypad2), sizeof(m_Joypad2));
    stream.write(reinterpret_cast<const char*> (&m_GlassesRegistry), sizeof(m_GlassesRegistry));
    stream.write(reinterpret_cast<const char*> (&m_bPhaser), sizeof(m_bPhaser));
    stream.write(reinterpret_cast<const char*> (&m_Phaser), sizeof(m_Phaser));
    stream.write(reinterpret_cast<const char*> (&m_bPaddle), sizeof(m_bPaddle));
    stream.write(reinterpret_cast<const char*> (&m_Paddle), sizeof(m_Paddle));
    stream.write(reinterpret_cast<const char*> (&m_SportsPad), sizeof(m_SportsPad));
}

void Input::LoadState(std::istream& stream, int version)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_Joypad1), sizeof(m_Joypad1));
    stream.read(reinterpret_cast<char*> (&m_Joypad2), sizeof(m_Joypad2));
    stream.read(reinterpret_cast<char*> (&m_GlassesRegistry), sizeof(m_GlassesRegistry));
    stream.read(reinterpret_cast<char*> (&m_bPhaser), sizeof(m_bPhaser));
    stream.read(reinterpret_cast<char*> (&m_Phaser), sizeof(m_Phaser));
    stream.read(reinterpret_cast<char*> (&m_bPaddle), sizeof(m_bPaddle));
    stream.read(reinterpret_cast<char*> (&m_Paddle), sizeof(m_Paddle));

    if (version >= 108)
    {
        stream.read(reinterpret_cast<char*> (&m_SportsPad), sizeof(m_SportsPad));
        u64 current_clock = m_pMasterClockCycles ? *m_pMasterClockCycles : 0;
        m_SportsPad[0].last_clock = current_clock;
        m_SportsPad[1].last_clock = current_clock;
    }
    else
    {
        ResetSportsPad(&m_SportsPad[0]);
        ResetSportsPad(&m_SportsPad[1]);
    }
}
