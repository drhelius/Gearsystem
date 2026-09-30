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

#ifndef INPUT_H
#define	INPUT_H

#include "definitions.h"

class Memory;
class Processor;
class Video;
class TraceLogger;

class Input
{
public:
    struct stPhaser
    {
        int x;
        int y;
    };

    struct stPaddle
    {
        float x;
        u8 reg;
        bool flip;
    };

    struct stSportsPad
    {
        bool enabled;
        float pending_x;
        float pending_y;
        u8 absolute_x;
        u8 absolute_y;
        u8 relative_x;
        u8 relative_y;
        u8 relative_phase;
        u8 absolute_phase;
        u8 mode;
        bool th;
        u32 absolute_phase_cycles;
        u64 unknown_cycles;
        u64 th_idle_cycles;
        u64 last_clock;
    };

public:
    Input(Processor* pProcessor, Video* pVideo, const u64* pMasterClockCycles);
    void Init();
    void Reset(bool bGameGear, bool bPAL = false, bool bGameGearInSMSMode = false);
    void KeyPressed(GS_Joypads joypad, GS_Keys key);
    void KeyReleased(GS_Joypads joypad, GS_Keys key);
    bool IsKeyPressed(GS_Joypads joypad, GS_Keys key) const;
    void SetReset(bool pressed);
    void EnablePhaser(bool enable);
    void SetPhaser(int x, int y);
    void SetPhaserOffset(int x, int y);
    stPhaser* GetPhaser();
    bool IsPhaserEnabled();
    void EnablePaddle(bool enable);
    void SetPaddle(float x);
    bool IsPaddleEnabled();
    void EnableSportsPad(GS_Joypads joypad, bool enable);
    void MoveSportsPad(GS_Joypads joypad, float x, float y);
    bool IsSportsPadEnabled(GS_Joypads joypad) const;
    INLINE bool IsAnySportsPadEnabled() const;
    void WriteSportsPadControl(u8 previous, u8 value);
    void SetTraceLogger(TraceLogger* pTraceLogger);
    u8 GetPortDC();
    u8 GetPortDD();
    u8 GetPort00();
    u8 GetGlassesRegistry();
    void SetGlassesRegistry(u8 value);
    void SaveState(std::ostream& stream);
    void LoadState(std::istream& stream, int version = GS_SAVESTATE_VERSION);

private:
    enum SportsPadMode
    {
        SportsPadModeUnknown = 0,
        SportsPadModeRelative,
        SportsPadModeAbsolute
    };

    INLINE void TraceInputChangeEvent(u8 player, u8 key, u8 previous, u8 effective);
    INLINE u8 GetControllerPort(u8 port);
    void LogInputChangeEvent(u8 player, u8 key, u8 previous, u8 effective);
    void ResetSportsPad(stSportsPad* sports_pad);
    void SyncSportsPad(stSportsPad* sports_pad);
    void ConsumeSportsPadAbsolute(stSportsPad* sports_pad);
    u8 ConsumeSportsPadRelative(float* pending);
    void SportsPadTHChanged(u8 port, bool th);
    u8 GetSportsPadPort(u8 port);
    Processor* m_pProccesor;
    Video* m_pVideo;
    TraceLogger* m_pTraceLogger;
    const u64* m_pMasterClockCycles;
    u8 m_Joypad1;
    u8 m_Joypad2;
    u8 m_GlassesRegistry;
    bool m_bGameGear;
    bool m_bGameGearInSMSMode;
    bool m_bPhaser;
    stPhaser m_Phaser;
    stPhaser m_PhaserOffset;
    bool m_bPaddle;
    stPaddle m_Paddle;
    stSportsPad m_SportsPad[2];
    u32 m_iSportsPadClockRate;
    u32 m_iSportsPadCyclesPerPhase;
    bool m_bResetPressed;
};

#include "Video.h"
#include "TraceLogger.h"

INLINE void Input::TraceInputChangeEvent(u8 player, u8 key, u8 previous, u8 effective)
{
    if (m_pTraceLogger->IsEventEnabled(TRACE_INPUT, TRACE_INPUT_CHANGE))
        LogInputChangeEvent(player, key, previous, effective);
}

INLINE bool Input::IsAnySportsPadEnabled() const
{
    return m_SportsPad[0].enabled || m_SportsPad[1].enabled;
}

INLINE u8 Input::GetControllerPort(u8 port)
{
    u8 joypad = port == 0 ? m_Joypad1 : m_Joypad2;

    if (m_SportsPad[port].enabled && !m_bGameGear)
        return GetSportsPadPort(port);

    if ((port == 0) && m_bPaddle && !m_bGameGear)
    {
        m_Paddle.flip ^= 0x01;
        u8 paddle_bits = (m_Paddle.flip == 0x00) ? m_Paddle.reg : (m_Paddle.reg >> 4);
        return (joypad & 0x10) | (paddle_bits & 0x0F) | (m_Paddle.flip << 5);
    }

    return joypad & 0x3F;
}

inline u8 Input::GetPortDC()
{
    if (m_bPhaser && !m_bGameGear)
    {
        return (m_Joypad1 & Key_1) ? 0xFF : 0xEF;
    }
    else
    {
        u8 port_1 = GetControllerPort(0);
        u8 port_2 = GetControllerPort(1);
        return port_1 | ((port_2 << 6) & 0xC0);
    }
}

inline u8 Input::GetPortDD()
{
    u8 dd;
    if (m_bPhaser && !m_bGameGear)
    {
        dd = ((GetControllerPort(1) >> 2) & 0x0F) | 0xF0;

        if (m_pVideo->IsPhaserDetected())
            dd = UnsetBit(dd, 6);
    }
    else
    {
        dd = ((GetControllerPort(1) >> 2) & 0x0F) | 0xF0;
    }

    if (!m_bGameGear && m_bResetPressed)
        dd = UnsetBit(dd, 4);

    return dd;
}

inline u8 Input::GetPort00()
{
    return m_Joypad1 & Key_Start;
}

#endif	/* INPUT_H */
