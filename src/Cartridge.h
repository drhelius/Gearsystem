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

#include "definitions.h"

class Cartridge
{
public:
    enum CartridgeTypes
    {
        CartridgeRomOnlyMapper,
        CartridgeSegaMapper,
		CartridgeCodemastersMapper,
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

public:
    Cartridge();
    ~Cartridge();
    void Init();
    void Reset();
    bool IsGameGear() const;
    bool IsValidROM() const;
    bool IsLoadedROM() const;
    CartridgeTypes GetType() const;
    CartridgeZones GetZone() const;
    void ForzeZone(CartridgeZones zone);
    int GetROMSize() const;
    int GetROMBankCount() const;
    const char* GetFilePath() const;
    const char* GetFileName() const;
    u8* GetTheROM() const;
    bool LoadFromFile(const char* path);
    bool LoadFromBuffer(const u8* buffer, int size);

private:
    unsigned int Pow2Ceil(u16 n);
    bool GatherMetadata(u32 crc);
    void GetInfoFromDB(u32 crc);
    bool LoadFromZipFile(const u8* buffer, int size);
    bool TestValidROM(u16 location);

private:
    u8* m_pTheROM;
    int m_iROMSize;
    CartridgeTypes m_Type;
    CartridgeZones m_Zone;
    bool m_bValidROM;
    bool m_bLoaded;
    char m_szFilePath[512];
    char m_szFileName[512];
    int m_iROMBankCount;
    bool m_bGameGear;
};

#endif	/* CARTRIDGE_H */
