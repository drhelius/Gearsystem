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

#ifndef CARTRIDGE_H
#define	CARTRIDGE_H

#include <list>
#include "definitions.h"

class Cartridge
{
public:
    enum CartridgeTypes
    {
        CartridgeRomOnlyMapper,
        CartridgeSegaMapper,
		CartridgeCodemastersMapper,
        CartridgeSG1000Mapper,
        CartridgeKoreanMapper,
        CartridgeMSXMapper,
        CartridgeNotSupported
    };
    
    enum CartridgeZones
    {
        CartridgeJapanSMS,
        CartridgeExportSMS,
        CartridgeJapanGG,
        CartridgeExportGG,
        CartridgeInternationalGG,
        CartridgeUnknownZone
    };

    enum CartridgeRegions
    {
        CartridgeNTSC,
        CartridgePAL,
        CartridgeUnknownRegion
    };

    enum CartridgeSystem
    {
        CartridgeSMS,
        CartridgeGG,
        CartridgeSG1000,
        CartridgeUnknownSystem
    };

    struct ForceConfiguration
    {
        CartridgeTypes type;
        CartridgeZones zone;
        CartridgeRegions region;
        CartridgeSystem system;
    };

public:
    Cartridge();
    ~Cartridge();
    void Init();
    void Reset();
    u32 GetCRC() const;
    bool IsGameGear() const;
    bool IsSG1000() const;
    bool IsPAL() const;
    bool IsValidROM() const;
    bool IsReady() const;
    bool HasRAMWithoutBattery() const;
    CartridgeTypes GetType() const;
    CartridgeZones GetZone() const;
    void ForceConfig(ForceConfiguration config);
    int GetROMSize() const;
    int GetROMBankCount() const;
    const char* GetFilePath() const;
    const char* GetFileName() const;
    u8* GetROM() const;
    bool LoadFromFile(const char* path);
    bool LoadFromBuffer(const u8* buffer, int size);
    void SetGameGenieCheat(const char* szCheat);
    void ClearGameGenieCheats();

private:
    bool GatherMetadata(u32 crc);
    void GetInfoFromDB(u32 crc);
    bool LoadFromZipFile(const u8* buffer, int size);
    bool TestValidROM(u16 location);

private:
    u8* m_pROM;
    int m_iROMSize;
    CartridgeTypes m_Type;
    CartridgeZones m_Zone;
    bool m_bValidROM;
    bool m_bReady;
    char m_szFilePath[512];
    char m_szFileName[512];
    int m_iROMBankCount;
    bool m_bGameGear;
    bool m_bSG1000;
    bool m_bPAL;
    bool m_bRAMWithoutBattery;
    u32 m_iCRC;

    struct GameGenieCode
    {
        int address;
        u8 old_value;
    };

    std::list<GameGenieCode> m_GameGenieList;
};

#endif	/* CARTRIDGE_H */
