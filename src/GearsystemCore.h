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

#ifndef CORE_H
#define CORE_H

#include "definitions.h"
#include "Cartridge.h"
#include "Video.h"

class Memory;
class Processor;
class Audio;
class Input;
class SegaMemoryRule;
class CodemastersMemoryRule;
class RomOnlyMemoryRule;
class SG1000MemoryRule;
class KoreanMemoryRule;
class KoreanMSXSMS8000MemoryRule;
class KoreanSMS32KB2000MemoryRule;
class KoreanMSX32KB2000MemoryRule;
class Korean2000XOR1FMemoryRule;
class KoreanMSX8KB0300MemoryRule;
class Korean0000XORFFMemoryRule;
class KoreanFFFFHiComMemoryRule;
class KoreanFFFEMemoryRule;
class KoreanBFFCMemoryRule;
class KoreanFFF3FFFCMemoryRule;
class KoreanMDFFF5MemoryRule;
class KoreanMDFFF0MemoryRule;
class MSXMemoryRule;
class JanggunMemoryRule;
class Multi4PAKAllActionMemoryRule;
class JumboDahjeeMemoryRule;
class IratahackMemoryRule;
class Eeprom93C46MemoryRule;
class MemoryRule;
class SmsIOPorts;
class GameGearIOPorts;
class BootromMemoryRule;
class TraceLogger;

class GearsystemCore
{
public:
    enum GlassesConfig
    {
        GlassesBothEyes,
        GlassesLeftEye,
        GlassesRightEye
    };

    struct GS_Debug_Run
    {
        bool step_debugger;
        bool stop_on_breakpoint;
        bool stop_on_run_to_breakpoint;
        bool stop_on_irq;
    };

public:
    GearsystemCore();
    ~GearsystemCore();
    void Init(GS_Color_Format pixelFormat = GS_PIXEL_RGBA8888);
    bool RunToVBlank(u8* pFrameBuffer, s16* pSampleBuffer, int* pSampleCount, GS_Debug_Run* debug = NULL);
    bool LoadROM(const char* szFilePath, Cartridge::ForceConfiguration* config = NULL);
    bool LoadROMFromBuffer(const u8* buffer, int size, Cartridge::ForceConfiguration* config = NULL, const char* szFilePath = NULL);
    void SaveMemoryDump();
    void SaveDisassembledROM();
    bool GetRuntimeInfo(GS_RuntimeInfo& runtime_info);
    void KeyPressed(GS_Joypads joypad, GS_Keys key);
    void KeyReleased(GS_Joypads joypad, GS_Keys key);
    void SetReset(bool pressed);
    void SetPhaser(int x, int y);
    void SetPhaserOffset(int x, int y);
    void EnablePhaser(bool enable);
    void EnablePhaserCrosshair(bool enable, Video::LightPhaserCrosshairShape shape, Video::LightPhaserCrosshairColor color);
    void SetPaddle(float x);
    void EnablePaddle(bool enable);
    void Pause(bool paused);
    bool IsPaused();
    void ResetROM(Cartridge::ForceConfiguration* config = NULL);
    void ResetROMPreservingRAM(Cartridge::ForceConfiguration* config = NULL);
    void ResetSound();
    void SaveRam();
    void SaveRam(const char* szPath, bool fullPath = false);
    void LoadRam();
    void LoadRam(const char* szPath, bool fullPath = false);
    bool SaveState(const char* path = NULL, int index = -1, bool screenshot = false);
    bool SaveState(u8* buffer, size_t& size, bool screenshot = false);
    bool LoadState(const char* path = NULL, int index = -1);
    bool LoadState(const u8* buffer, size_t size);
    bool GetSaveStateHeader(int index, const char* path, GS_SaveState_Header* header);
    bool GetSaveStateScreenshot(int index, const char* path, GS_SaveState_Screenshot* screenshot);
    void SetCheat(const char* szCheat);
    void ClearCheats();
    void SetRamModificationCallback(RamChangedCallback callback);
    Memory* GetMemory();
    Cartridge* GetCartridge();
    Processor* GetProcessor();
    Audio* GetAudio();
    Video* GetVideo();
    void SetGlassesConfig(GlassesConfig config);
    u64 GetMasterClockCycles();
    TraceLogger* GetTraceLogger();

private:
    void InitMemoryRules();
    bool AddMemoryRules();
    void Reset();
    void RenderFrameBuffer(u8* finalFrameBuffer);
    bool SaveState(std::ostream& stream, size_t& size, bool screenshot);
    bool LoadState(std::istream& stream);
    bool LoadStateV1(std::istream& stream, size_t size);
    std::string GetSaveStatePath(const char* path, int index);

private:
    Memory* m_pMemory;
    Processor* m_pProcessor;
    Audio* m_pAudio;
    Video* m_pVideo;
    Input* m_pInput;
    Cartridge* m_pCartridge;
    SegaMemoryRule* m_pSegaMemoryRule;
    CodemastersMemoryRule* m_pCodemastersMemoryRule;
    SG1000MemoryRule* m_pSG1000MemoryRule;
    RomOnlyMemoryRule* m_pRomOnlyMemoryRule;
    KoreanMemoryRule* m_pKoreanMemoryRule;
    KoreanMSXSMS8000MemoryRule* m_pKoreanMSXSMS8000MemoryRule;
    KoreanSMS32KB2000MemoryRule* m_pKoreanSMS32KB2000MemoryRule;
    KoreanMSX32KB2000MemoryRule* m_pKoreanMSX32KB2000MemoryRule;
    Korean2000XOR1FMemoryRule* m_pKorean2000XOR1FMemoryRule;
    KoreanMSX8KB0300MemoryRule* m_pKoreanMSX8KB0300MemoryRule;
    Korean0000XORFFMemoryRule* m_pKorean0000XORFFMemoryRule;
    KoreanFFFFHiComMemoryRule* m_pKoreanFFFFHiComMemoryRule;
    KoreanFFFEMemoryRule* m_pKoreanFFFEMemoryRule;
    KoreanBFFCMemoryRule* m_pKoreanBFFCMemoryRule;
    KoreanFFF3FFFCMemoryRule* m_pKoreanFFF3FFFCMemoryRule;
    KoreanMDFFF5MemoryRule* m_pKoreanMDFFF5MemoryRule;
    KoreanMDFFF0MemoryRule* m_pKoreanMDFFF0MemoryRule;
    MSXMemoryRule* m_pMSXMemoryRule;
    JanggunMemoryRule* m_pJanggunMemoryRule;
    Multi4PAKAllActionMemoryRule* m_pMulti4PAKAllActionMemoryRule;
    JumboDahjeeMemoryRule* m_pJumboDahjeeMemoryRule;
    Eeprom93C46MemoryRule* m_pEeprom93C46MemoryRule;
    SmsIOPorts* m_pSmsIOPorts;
    GameGearIOPorts* m_pGameGearIOPorts;
    BootromMemoryRule* m_pBootromMemoryRule;
    IratahackMemoryRule* m_pIratahackMemoryRule;
    bool m_bPaused;
    RamChangedCallback m_pRamChangedCallback;
    GS_Color_Format m_pixelFormat;
    GlassesConfig m_GlassesConfig;
    u64 m_master_clock_cycles;
    TraceLogger* m_trace_logger;
    u8* m_pFrameBuffer;
};

#endif	/* CORE_H */
