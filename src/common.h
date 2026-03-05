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

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#endif
#include "definitions.h"
#include "log.h"
#include <miniz.h>

inline u16 read_u16_le(const u8* p)
{
    return (u16)p[0] | ((u16)p[1] << 8);
}

inline u32 read_u32_le(const u8* p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

inline u16 read_u16_be(const u8* p)
{
    return (u16)p[1] | ((u16)p[0] << 8);
}

inline u32 read_u32_be(const u8* p)
{
    return (u32)p[3] | ((u32)p[2] << 8) | ((u32)p[1] << 16) | ((u32)p[0] << 24);
}

inline u8 hi(u16 a)
{
    return (u8)(a >> 8);
}

inline u8 lo(u16 a)
{
    return (u8)(a & 0xFF);
}

inline int as_hex(const char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 0xA;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 0xA;
    return 0;
}

inline unsigned int pow_2_ceil(u16 n)
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    ++n;
    return n;
}

inline bool is_pow2(u32 x)
{
    return x && ((x & (x - 1)) == 0);
}

inline void get_date_time_string(time_t timestamp, char* buffer, size_t size)
{
    struct tm* timeinfo = localtime(&timestamp);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

inline void get_current_date_time_string(char* buffer, size_t size)
{
    time_t timestamp = time(NULL);
    get_date_time_string(timestamp, buffer, size);
}

inline bool is_hex_digit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

template<typename T>
inline bool parse_hex_string(const char* str, size_t len, T* result, size_t max_digits = sizeof(T) * 2)
{
    if (len == 0)
        return false;

    while (len > 1 && str[0] == '0')
    {
        str++;
        len--;
    }

    if (len > max_digits)
        return false;

    *result = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (!is_hex_digit(str[i]))
            return false;

        *result = (*result << 4);

        if (str[i] >= '0' && str[i] <= '9')
            *result |= (str[i] - '0');
        else if (str[i] >= 'a' && str[i] <= 'f')
            *result |= (str[i] - 'a' + 10);
        else // (str[i] >= 'A' && str[i] <= 'F')
            *result |= (str[i] - 'A' + 10);
    }
    return true;
}

inline bool parse_hex_string(const char* str, size_t len, u8* result)
{
    return parse_hex_string<u8>(str, len, result, 2);
}

inline bool parse_hex_string(const char* str, size_t len, u16* result)
{
    return parse_hex_string<u16>(str, len, result, 4);
}

inline bool parse_hex_string(const char* str, size_t len, u32* result)
{
    return parse_hex_string<u32>(str, len, result, 8);
}

template<typename T>
inline bool parse_hex_with_prefix(const std::string& hex_str, T* result)
{
    const char* str = hex_str.c_str();
    size_t len = hex_str.length();

    if (len >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        len -= 2;
    }
    else if (len >= 1 && str[0] == '$')
    {
        str += 1;
        len -= 1;
    }

    return parse_hex_string(str, len, result);
}

inline char* strncpy_fit(char* dest, const char* src, size_t dest_size)
{
    if (dest_size == 0)
        return dest;

    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';

    return dest;
}

inline char* strncat_fit(char* dest, const char* src, size_t dest_size)
{
    if (dest_size != 0)
        dest_size -= strlen(dest) + 1;

    return strncat(dest, src, dest_size);
}

inline bool create_directory_if_not_exists(const char* path)
{
#if defined(_WIN32)
    int res = _mkdir(path);
#else
    int res = mkdir(path, 0755);
#endif
    if (res == 0 || errno == EEXIST)
        return true;
    else
        return false;
}

inline bool remove_directory_and_contents(const char* path)
{
#if defined(_WIN32)
    std::string search = std::string(path) + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search.c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    do
    {
        const char* name = fd.cFileName;
        if (strcmp(name, ".") && strcmp(name, ".."))
        {
            std::string item = std::string(path) + "\\" + name;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                remove_directory_and_contents(item.c_str());
                RemoveDirectoryA(item.c_str());
            }
            else
            {
                DeleteFileA(item.c_str());
            }
        }
    }
    while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return RemoveDirectoryA(path) != 0;
#else
    DIR* dir = opendir(path);
    if (!dir)
        return false;

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL)
    {
        const char* name = entry->d_name;
        if (strcmp(name, ".") && strcmp(name, ".."))
        {
            std::string item = std::string(path) + "/" + name;
            struct stat st;
            if (stat(item.c_str(), &st) == 0) {
                if (S_ISDIR(st.st_mode))
                    remove_directory_and_contents(item.c_str());
                else
                    unlink(item.c_str());
            }
        }
    }

    closedir(dir);
    return (rmdir(path) == 0);
#endif
}

#if defined(_WIN32)
inline std::wstring utf8_to_wstring(const char* utf8_str)
{
    if (!utf8_str || utf8_str[0] == '\0')
        return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    if (size_needed <= 0)
        return std::wstring();

    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, &wstr[0], size_needed);
    return wstr;
}

#define open_ifstream_utf8(stream, path, mode) \
    do { \
        std::wstring wpath = utf8_to_wstring(path); \
        stream.open(wpath.c_str(), mode); \
    } while(0)

#define open_ofstream_utf8(stream, path, mode) \
    do { \
        std::wstring wpath = utf8_to_wstring(path); \
        stream.open(wpath.c_str(), mode); \
    } while(0)

inline FILE* fopen_utf8(const char* path, const char* mode)
{
    std::wstring wpath = utf8_to_wstring(path);
    std::wstring wmode = utf8_to_wstring(mode);
    return _wfopen(wpath.c_str(), wmode.c_str());
}
#else
#define open_ifstream_utf8(stream, path, mode) \
    stream.open(path, mode)

#define open_ofstream_utf8(stream, path, mode) \
    stream.open(path, mode)

inline FILE* fopen_utf8(const char* path, const char* mode)
{
    return fopen(path, mode);
}
#endif

inline bool extract_zip_to_folder(const char* zip_path, const char* out_folder)
{
    using namespace std;
    Debug("Extracting ZIP archive %s...", zip_path);
    Debug("Output folder: %s", out_folder);

    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    mz_bool status = mz_zip_reader_init_file(&zip_archive, zip_path, 0);

    if (!status)
    {
        Error("mz_zip_reader_init_file() failed!");
        return false;
    }

    string out_dir(out_folder);

    if (!create_directory_if_not_exists(out_dir.c_str()))
    {
        Error("Failed to create directory %s", out_dir.c_str());
        mz_zip_reader_end(&zip_archive);
        return false;
    }

    mz_uint num_files = mz_zip_reader_get_num_files(&zip_archive);

    for (mz_uint i = 0; i < num_files; i++)
    {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
        {
            Error("mz_zip_reader_file_stat failed for file %d", i);
            continue;
        }

        if (file_stat.m_is_directory)
            continue;

        string out_path = out_dir + "/" + file_stat.m_filename;

        if (!mz_zip_reader_extract_to_file(&zip_archive, i, out_path.c_str(), 0))
        {
            Error("Failed to extract %s", out_path.c_str());
        }
        else
        {
            Debug("Extracted: %s", out_path.c_str());
        }
    }

    mz_zip_reader_end(&zip_archive);
    Debug("Extraction complete.");

    return true;
}

#endif /* COMMON_H */
