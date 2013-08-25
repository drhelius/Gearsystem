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

#include <string>
#include <algorithm>
#include "Cartridge.h"
#include "miniz/miniz.c"

Cartridge::Cartridge()
{
    InitPointer(m_pTheROM);
    m_iROMSize = 0;
    m_Type = CartridgeNotSupported;
    m_bValidROM = false;
    m_bLoaded = false;
    m_szFilePath[0] = 0;
    m_szFileName[0] = 0;
    m_iROMBankCount = 0;
}

Cartridge::~Cartridge()
{
    SafeDeleteArray(m_pTheROM);
}

void Cartridge::Init()
{
    Reset();
}

void Cartridge::Reset()
{
    SafeDeleteArray(m_pTheROM);
    m_Type = CartridgeNotSupported;
    m_bValidROM = false;
    m_bLoaded = false;
    m_szFilePath[0] = 0;
    m_szFileName[0] = 0;
    m_iROMBankCount = 0;
}

bool Cartridge::IsValidROM() const
{
    return m_bValidROM;
}

bool Cartridge::IsLoadedROM() const
{
    return m_bLoaded;
}

Cartridge::CartridgeTypes Cartridge::GetType() const
{
    return m_Type;
}

int Cartridge::GetROMSize() const
{
    return m_iROMSize;
}

int Cartridge::GetROMBankCount() const
{
    return m_iROMBankCount;
}

const char* Cartridge::GetFilePath() const
{
    return m_szFilePath;
}

const char* Cartridge::GetFileName() const
{
    return m_szFileName;
}

u8* Cartridge::GetTheROM() const
{
    return m_pTheROM;
}

bool Cartridge::LoadFromZipFile(const u8* buffer, int size)
{
    using namespace std;

    mz_zip_archive zip_archive;
    mz_bool status;
    memset(&zip_archive, 0, sizeof (zip_archive));

    status = mz_zip_reader_init_mem(&zip_archive, (void*) buffer, size, 0);
    if (!status)
    {
        Log("mz_zip_reader_init_mem() failed!");
        return false;
    }

    for (unsigned int i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++)
    {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
        {
            Log("mz_zip_reader_file_stat() failed!");
            mz_zip_reader_end(&zip_archive);
            return false;
        }

        Log("ZIP Content - Filename: \"%s\", Comment: \"%s\", Uncompressed size: %u, Compressed size: %u", file_stat.m_filename, file_stat.m_comment, (unsigned int) file_stat.m_uncomp_size, (unsigned int) file_stat.m_comp_size);

        string fn((const char*) file_stat.m_filename);
        transform(fn.begin(), fn.end(), fn.begin(), (int(*)(int)) tolower);
        string extension = fn.substr(fn.find_last_of(".") + 1);

        if ((extension == "sms") || (extension == "gg"))
        {
            void *p;
            size_t uncomp_size;

            p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_stat.m_filename, &uncomp_size, 0);
            if (!p)
            {
                Log("mz_zip_reader_extract_file_to_heap() failed!");
                mz_zip_reader_end(&zip_archive);
                return false;
            }

            bool ok = LoadFromBuffer((const u8*) p, uncomp_size);

            free(p);
            mz_zip_reader_end(&zip_archive);

            return ok;
        }
    }
    return false;
}

bool Cartridge::LoadFromFile(const char* path)
{
    using namespace std;

    Log("Loading %s...", path);

    Reset();

    strcpy(m_szFilePath, path);

    std::string pathstr(path);
    std::string filename;

    size_t pos = pathstr.find_last_of("\\");
    if (pos != std::string::npos)
    {
        filename.assign(pathstr.begin() + pos + 1, pathstr.end());
    }
    else
    {
        pos = pathstr.find_last_of("/");
        if (pos != std::string::npos)
        {
            filename.assign(pathstr.begin() + pos + 1, pathstr.end());
        }
        else
        {
            filename = pathstr;
        }
    }
    
    strcpy(m_szFileName, filename.c_str());

    ifstream file(path, ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
        int size = static_cast<int> (file.tellg());
        char* memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();

        string fn(path);
        transform(fn.begin(), fn.end(), fn.begin(), (int(*)(int)) tolower);
        string extension = fn.substr(fn.find_last_of(".") + 1);

        if (extension == "zip")
        {
            Log("Loading from ZIP...");
            m_bLoaded = LoadFromZipFile(reinterpret_cast<u8*> (memblock), size);
        }
        else
        {
            m_bLoaded = LoadFromBuffer(reinterpret_cast<u8*> (memblock), size);
        }

        if (m_bLoaded)
        {
            Log("ROM loaded", path);
        }
        else
        {
            Log("There was a problem loading the memory for file %s...", path);
        }

        SafeDeleteArray(memblock);
    }
    else
    {
        Log("There was a problem loading the file %s...", path);
        m_bLoaded = false;
    }

    if (!m_bLoaded)
    {
        Reset();
    }

    return m_bLoaded;
}

bool Cartridge::LoadFromBuffer(const u8* buffer, int size)
{
    if (IsValidPointer(buffer))
    {
        m_iROMSize = size;
        m_pTheROM = new u8[m_iROMSize];
        memcpy(m_pTheROM, buffer, m_iROMSize);
        return GatherMetadata();
    }
    else
        return false;
}

unsigned int Cartridge::Pow2Ceil(u16 n)
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    ++n;
    return n;
}

bool Cartridge::TestValidROM(u16 location)
{
    char tmrsega[9] = {0};
    tmrsega[8] = 0;

    for (int i = 0; i < 8; i++)
    {
        tmrsega[i] = m_pTheROM[location + i];
    }
    
    return (strcmp(tmrsega, "TMR SEGA") == 0);
}

bool Cartridge::GatherMetadata()
{
    u16 headerLocation = 0x7FF0;
    m_bValidROM = false;
    
    if (!TestValidROM(headerLocation))
    {
        headerLocation = 0x1FF0;
        if (!TestValidROM(headerLocation))
        {
            headerLocation = 0x3FF0;
            if (!TestValidROM(headerLocation))
            {
                return m_bValidROM;
            }
        }
    }

    m_bValidROM = true;
    m_Type = Cartridge::CartridgeSegaMapper;

    Log("ROM Size: %d KB", m_iROMSize / 1024);
    Log("ROM Bank Count: %d", m_iROMBankCount);
    
    if (m_bValidROM)
    {
        Log("ROM is Valid. Header found at: %X", headerLocation);
    }
    else
    {
        Log("ROM is NOT Valid. No header found");
    }

    switch (m_Type)
    {
        case Cartridge::CartridgeSegaMapper:
            Log("SEGA mapper found");
            break;
        case Cartridge::CartridgeCodemastersMapper:
            Log("Codemasters mapper found");
            break;
        case Cartridge::CartridgeNotSupported:
            Log("Cartridge not supported!!");
            break;
        default:
            break;
    }

    return (m_Type != CartridgeNotSupported);
}

