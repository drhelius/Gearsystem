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
#include <fstream>
#include <time.h>
#if defined(_WIN32)
#define NOMINMAX
#include <direct.h>
#include <windows.h>
#endif
#include "definitions.h"
#include "log.h"

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

inline u16 hi(u16 a)
{
    return (u16)(a >> 8);
}

inline u16 lo(u16 a)
{
    return (u16)(a & 0xFF);
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
    if (len == 0 || len > max_digits)
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

inline char* strncpy_fit(char* dest, const char* src, size_t dest_size)
{
    if (dest_size != 0)
        dest_size -= 1;

    return strncpy(dest, src, dest_size);
}

inline char* strncat_fit(char* dest, const char* src, size_t dest_size)
{
    if (dest_size != 0)
        dest_size -= strlen(dest) + 1;

    return strncat(dest, src, dest_size);
}

#if defined(_WIN32)
inline std::wstring utf8_to_wstring(const char* utf8_str)
{
    if (!utf8_str || utf8_str[0] == '\0')
        return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    if (size_needed <= 0)
        return std::wstring();

    std::wstring wstr(size_needed - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, &wstr[0], size_needed);
    return wstr;
}

inline FILE* fopen_utf8(const char* path, const char* mode)
{
    if (!path || !mode)
        return NULL;

    std::wstring wpath = utf8_to_wstring(path);
    std::wstring wmode = utf8_to_wstring(mode);

    if (wpath.empty() || wmode.empty())
        return NULL;

    return _wfopen(wpath.c_str(), wmode.c_str());
}

inline void open_ifstream_utf8(std::ifstream& stream, const char* path, std::ios_base::openmode mode = std::ios_base::in)
{
    if (!path)
        return;

    std::wstring wpath = utf8_to_wstring(path);
    if (wpath.empty())
        return;

    stream.open(wpath.c_str(), mode);
}

inline void open_ofstream_utf8(std::ofstream& stream, const char* path, std::ios_base::openmode mode = std::ios_base::out)
{
    if (!path)
        return;

    std::wstring wpath = utf8_to_wstring(path);
    if (wpath.empty())
        return;

    stream.open(wpath.c_str(), mode);
}
#else
inline FILE* fopen_utf8(const char* path, const char* mode)
{
    return fopen(path, mode);
}

inline void open_ifstream_utf8(std::ifstream& stream, const char* path, std::ios_base::openmode mode = std::ios_base::in)
{
    stream.open(path, mode);
}

inline void open_ofstream_utf8(std::ofstream& stream, const char* path, std::ios_base::openmode mode = std::ios_base::out)
{
    stream.open(path, mode);
}
#endif

#endif /* COMMON_H */
