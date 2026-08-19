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

#define GUI_DEBUG_TRACE_LOGGER_IMPORT
#include "gui_debug_trace_logger.h"

#include "imgui.h"
#include "gui.h"
#include "gui_filedialogs.h"
#include "gui_debug_constants.h"
#include "gui_debug_text.h"
#include "config.h"
#include "emu.h"
#include "gui_debug.h"
#include "utils.h"
#include "trace_logger_formatter.h"
#include "log.h"
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

static bool trace_logger_enabled = false;
static int trace_logger_output = 0;
static FILE* trace_logger_file = NULL;
static char trace_logger_file_buffer[1024 * 1024];
static char trace_logger_file_path[1024];
static char trace_logger_disk_directory[4096] = {};
static u64 trace_logger_disk_limit = 0;
static u64 trace_logger_disk_bytes = 0;
static u64 trace_logger_disk_entries = 0;
static u64 trace_logger_disk_next_sequence = 0;
static u64 trace_logger_last_flush = 0;
static bool trace_logger_draining = false;
static bool trace_logger_disk_error = false;
static bool trace_logger_follow_latest = true;
static bool trace_logger_scroll_to_bottom = false;
static bool trace_logger_wait_for_scroll_away = false;
static bool trace_logger_choose_output_path = false;
static const GS_Trace_Entry* trace_logger_previous_entry = NULL;
static const u32 k_trace_logger_capacities[] = {100000, 500000, 1000000, 2000000, 5000000};
static const char* const k_trace_logger_capacity_names[] = {"100K", "500K", "1M", "2M", "5M"};
static const char* const k_trace_logger_capacity_labels[] = {"100K (10 MB)", "500K (50 MB)", "1M (100 MB)", "2M (200 MB)", "5M (500 MB)"};
static const char* const k_trace_logger_disk_size_names[] = {"10MB", "50MB", "100MB", "250MB", "500MB", "1GB", "unbounded"};
static const u64 k_trace_logger_disk_sizes[] = {10ULL * 1024 * 1024, 50ULL * 1024 * 1024, 100ULL * 1024 * 1024,
    250ULL * 1024 * 1024, 500ULL * 1024 * 1024, 1024ULL * 1024 * 1024, 0};

static void trace_logger_menu(void);
static void trace_logger_sync_flags(void);
static u32 trace_logger_get_config_flags(void);
static void trace_logger_set_config_flags(u32 flags);
static void trace_logger_menu_event_filter(const char* label, int* filter, u32 mask);
static bool trace_logger_apply_capacity(void);
static bool trace_logger_start(u32 flags, bool update_config);
static bool trace_logger_start_disk(const char* directory, char* error, size_t error_size);
static bool trace_logger_stop(bool show_status);
static bool trace_logger_stop_disk(bool show_status, bool flush_entries);
static void format_entry_text(const GS_Trace_Entry& entry, bool cycles,
    const GS_Trace_Entry* previous, char* buf, int buf_size);
static void format_entry_text(const GS_Trace_Entry& entry, char* buf, int buf_size);
static void render_entry_colored(const GS_Trace_Entry& entry, u64 index);
static void render_cpu_entry_colored(const GS_Trace_Entry& entry, int prefix_length);

static const char* trace_logger_directory(void)
{
    if (config_debug.trace_disk_dir_option == 1)
    {
        const char* directory = emu_get_core()->GetCartridge()->GetFileDirectory();
        if (directory && directory[0])
            return directory;
    }
    else if (config_debug.trace_disk_dir_option == 2 && !config_debug.trace_disk_path.empty())
        return config_debug.trace_disk_path.c_str();
    return config_root_path;
}

static void trace_logger_menu_event_filter(const char* label, int* filter, u32 mask)
{
    bool enabled = ((u32)*filter & mask) != 0;
    if (ImGui::MenuItem(label, "", &enabled))
    {
        if (enabled)
            *filter |= (int)mask;
        else
            *filter &= ~(int)mask;
    }
}

static bool trace_logger_path_exists(const char* path)
{
    FILE* file = fopen_utf8(path, "rb");
    if (!file)
        return false;
    fclose(file);
    return true;
}

static bool trace_logger_close_file(void)
{
    bool success = true;
    if (trace_logger_file)
    {
        if (fflush(trace_logger_file) != 0)
            success = false;
        if (fclose(trace_logger_file) != 0)
            success = false;
        trace_logger_file = NULL;
    }
    if (!success)
        trace_logger_disk_error = true;
    return success;
}

static bool trace_logger_open_file(const char* directory, char* error, size_t error_size)
{
    const char* name = emu_get_core()->GetCartridge()->GetFileName();
    if (!name || !name[0])
        name = "Gearsystem";

    char base[512];
    snprintf(base, sizeof(base), "%s", name);
    char* extension = strrchr(base, '.');
    if (extension)
        *extension = 0;

    time_t now = time(NULL);
    struct tm local;
    if (!get_local_time(now, &local))
    {
        snprintf(error, error_size, "Unable to read local time");
        return false;
    }
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H%M%S", &local);

    for (int suffix = 1; suffix <= 1000; suffix++)
    {
        if (suffix == 1)
            snprintf(trace_logger_file_path, sizeof(trace_logger_file_path), "%s/%s - Trace - %s.txt", directory, base, timestamp);
        else
            snprintf(trace_logger_file_path, sizeof(trace_logger_file_path), "%s/%s - Trace - %s (%d).txt", directory, base, timestamp, suffix);
        if (!trace_logger_path_exists(trace_logger_file_path))
            break;
        if (suffix == 1000)
        {
            snprintf(error, error_size, "Unable to create a unique trace file name");
            return false;
        }
    }

    trace_logger_file = fopen_utf8(trace_logger_file_path, "wb");
    if (!trace_logger_file)
    {
        snprintf(error, error_size, "Unable to open trace file: %s", strerror(errno));
        trace_logger_file_path[0] = 0;
        return false;
    }
    setvbuf(trace_logger_file, trace_logger_file_buffer, _IOFBF, sizeof(trace_logger_file_buffer));
    return true;
}

void gui_debug_trace_logger_init(void)
{
    trace_logger_enabled = false;
    trace_logger_output = config_debug.trace_output;
    trace_logger_file = NULL;
    trace_logger_file_path[0] = 0;
    strncpy_fit(trace_logger_disk_directory, config_debug.trace_disk_path.c_str(), sizeof(trace_logger_disk_directory));
    if (!trace_logger_apply_capacity())
    {
        config_debug.trace_capacity = 0;
        trace_logger_apply_capacity();
    }
}

static bool trace_logger_apply_capacity(void)
{
    TraceLogger* logger = emu_get_core()->GetTraceLogger();
    u32 capacity = config_debug.trace_output == gui_TraceOutput_Disk ? TRACE_BUFFER_SIZE :
        k_trace_logger_capacities[config_debug.trace_capacity];
    if (!logger->SetCapacity(capacity))
    {
        gui_set_error_message("Unable to allocate the selected trace logger capacity.");
        return false;
    }
    return true;
}

static bool trace_logger_start_disk(const char* directory, char* error, size_t error_size)
{
    if (!trace_logger_apply_capacity())
        return false;
    const char* output_directory = directory && directory[0] ? directory : trace_logger_directory();
    if (!output_directory || !output_directory[0] || !trace_logger_open_file(output_directory, error, error_size))
        return false;

    TraceLogger* logger = emu_get_core()->GetTraceLogger();
    logger->Reset();
    trace_logger_disk_limit = k_trace_logger_disk_sizes[config_debug.trace_disk_size];
    trace_logger_disk_bytes = 0;
    trace_logger_disk_entries = 0;
    trace_logger_disk_next_sequence = logger->GetSequence();
    trace_logger_last_flush = SDL_GetTicks();
    trace_logger_disk_error = false;
    return true;
}

bool gui_debug_trace_logger_start(u32 flags)
{
    return trace_logger_start(flags, true);
}

static bool trace_logger_start(u32 flags, bool update_config)
{
    if (flags == 0)
    {
        flags = TRACE_FLAG_CPU | TRACE_FLAG_CPU_IRQ;
        update_config = true;
    }
    if (update_config)
        trace_logger_set_config_flags(flags);

    if (trace_logger_enabled)
    {
        trace_logger_sync_flags();
        return true;
    }

    char error[256] = {};
    trace_logger_output = config_debug.trace_output;
    if (trace_logger_output == gui_TraceOutput_Disk)
    {
        if (!trace_logger_start_disk(NULL, error, sizeof(error)))
        {
            if (error[0])
                gui_set_error_message(error);
            return false;
        }
    }
    else if (!trace_logger_apply_capacity())
        return false;

    trace_logger_enabled = true;
    trace_logger_follow_latest = true;
    trace_logger_scroll_to_bottom = true;
    trace_logger_wait_for_scroll_away = false;
    trace_logger_sync_flags();
    gui_set_status_message("Trace recording started", 3000);
    return true;
}

void gui_debug_trace_logger_update(void)
{
    if (!trace_logger_enabled || trace_logger_output != 1 || !trace_logger_file || trace_logger_draining)
        return;

    trace_logger_draining = true;
    TraceLogger* logger = emu_get_core()->GetTraceLogger();
    u64 sequence = logger->GetSequence();
    u64 oldest = sequence - logger->GetCount();
    bool limit_reached = false;
    if (trace_logger_disk_next_sequence < oldest)
    {
        trace_logger_disk_error = true;
    }

    while (!trace_logger_disk_error && trace_logger_disk_next_sequence < sequence)
    {
        u32 index = (u32)(trace_logger_disk_next_sequence - oldest);
        const GS_Trace_Entry& entry = logger->GetEntry(index);
        GS_Trace_Format_Options options = {};
        options.bank = config_debug.trace_bank;
        options.registers = config_debug.trace_registers;
        options.flags = config_debug.trace_flags;
        options.bytes = config_debug.trace_bytes;
        options.cycles = config_debug.trace_cycles;
        if (index > 0)
            options.previous = &logger->GetEntry(index - 1);
        char entry_text[GS_TRACE_FORMAT_BUFFER_SIZE];
        char line[GS_TRACE_FORMAT_BUFFER_SIZE + 64];
        trace_logger_format_entry(entry, options, entry_text, sizeof(entry_text));
        if (config_debug.trace_counter)
            snprintf(line, sizeof(line), "%06llu %s", (unsigned long long)trace_logger_disk_entries, entry_text);
        else
            snprintf(line, sizeof(line), "%s", entry_text);
        size_t length = strlen(line);
        line[length++] = '\n';
        line[length] = 0;
        if (trace_logger_disk_limit && trace_logger_disk_bytes + length > trace_logger_disk_limit)
        {
            limit_reached = true;
            break;
        }
        if (fwrite(line, 1, length, trace_logger_file) != length)
        {
            trace_logger_disk_error = true;
            break;
        }
        trace_logger_disk_bytes += length;
        trace_logger_disk_entries++;
        trace_logger_disk_next_sequence++;
    }

    u64 now = SDL_GetTicks();
    if (!trace_logger_disk_error && now - trace_logger_last_flush >= 1000)
    {
        if (fflush(trace_logger_file) != 0)
        {
            trace_logger_disk_error = true;
        }
        trace_logger_last_flush = now;
    }
    trace_logger_draining = false;

    if (trace_logger_disk_error)
    {
        trace_logger_stop_disk(true, false);
    }
    else if (limit_reached)
    {
        if (trace_logger_stop_disk(false, false))
            gui_set_status_message("Trace recording stopped: maximum file size reached", 4000);
    }
}

bool gui_debug_trace_logger_stop(void)
{
    return trace_logger_stop(true);
}

static bool trace_logger_stop(bool show_status)
{
    if (!trace_logger_enabled)
        return true;

    trace_logger_scroll_to_bottom = trace_logger_follow_latest;

    if (trace_logger_output == gui_TraceOutput_Disk)
        return trace_logger_stop_disk(show_status, true);

    trace_logger_enabled = false;
    emu_get_core()->GetTraceLogger()->SetEnabledFlags(0);
    if (show_status)
        gui_set_status_message("Trace recording stopped", 3000);
    return true;
}

static bool trace_logger_stop_disk(bool show_status, bool flush_entries)
{
    if (flush_entries && trace_logger_file && !trace_logger_draining)
        gui_debug_trace_logger_update();
    if (!trace_logger_enabled)
        return !trace_logger_disk_error;

    bool success = !trace_logger_disk_error;
    if (trace_logger_file && !trace_logger_close_file())
        success = false;

    trace_logger_enabled = false;
    emu_get_core()->GetTraceLogger()->SetEnabledFlags(0);
    if (!success)
    {
        Error("Error closing trace log file: %s", trace_logger_file_path);
        gui_set_error_message("Trace recording stopped with a disk write error.");
    }
    else if (show_status)
        gui_set_status_message("Trace recording stopped", 3000);
    return success;
}

void gui_debug_trace_logger_shutdown(void)
{
    trace_logger_stop(false);
}

void gui_debug_trace_logger_reset(void)
{
    trace_logger_stop(false);
    emu_get_core()->GetTraceLogger()->Reset();
}

bool gui_debug_trace_logger_is_enabled(void)
{
    return trace_logger_enabled;
}

const char* gui_debug_trace_logger_get_output_path(void)
{
    return trace_logger_file_path;
}

void gui_debug_trace_logger_set_output_directory(const char* path)
{
    strncpy_fit(trace_logger_disk_directory, path, sizeof(trace_logger_disk_directory));
    config_debug.trace_disk_path.assign(path);
}

int gui_debug_trace_logger_memory_size_index(const char* size)
{
    if (size)
    {
        for (int i = 0; i < IM_ARRAYSIZE(k_trace_logger_capacity_names); i++)
        {
            if (strcmp(size, k_trace_logger_capacity_names[i]) == 0)
                return i;
        }
    }
    return -1;
}

int gui_debug_trace_logger_disk_size_index(const char* size)
{
    if (size)
    {
        for (int i = 0; i < IM_ARRAYSIZE(k_trace_logger_disk_size_names); i++)
        {
            if (strcmp(size, k_trace_logger_disk_size_names[i]) == 0)
                return i;
        }
    }
    return -1;
}

const char* gui_debug_trace_logger_memory_size_name(int index)
{
    if (index < 0 || index >= IM_ARRAYSIZE(k_trace_logger_capacity_names))
        return k_trace_logger_capacity_names[0];
    return k_trace_logger_capacity_names[index];
}

const char* gui_debug_trace_logger_disk_size_name(int index)
{
    if (index < 0 || index >= IM_ARRAYSIZE(k_trace_logger_disk_size_names))
        return k_trace_logger_disk_size_names[2];
    return k_trace_logger_disk_size_names[index];
}

bool gui_debug_trace_logger_configure(int output, int memory_size, int disk_size, const char* output_path)
{
    if (trace_logger_enabled)
        return false;
    if (output < gui_TraceOutput_Memory || output > gui_TraceOutput_Disk)
        return false;
    if (memory_size < 0 || memory_size >= IM_ARRAYSIZE(k_trace_logger_capacities))
        return false;
    if (disk_size < 0 || disk_size >= IM_ARRAYSIZE(k_trace_logger_disk_sizes))
        return false;

    int previous_output = config_debug.trace_output;
    int previous_memory_size = config_debug.trace_capacity;
    int previous_disk_size = config_debug.trace_disk_size;
    int previous_dir_option = config_debug.trace_disk_dir_option;
    std::string previous_path = config_debug.trace_disk_path;

    config_debug.trace_output = output;
    config_debug.trace_capacity = memory_size;
    config_debug.trace_disk_size = disk_size;
    if (output == gui_TraceOutput_Disk && output_path && output_path[0])
    {
        config_debug.trace_disk_dir_option = Directory_Location_Custom;
        gui_debug_trace_logger_set_output_directory(output_path);
    }

    if (!trace_logger_apply_capacity())
    {
        config_debug.trace_output = previous_output;
        config_debug.trace_capacity = previous_memory_size;
        config_debug.trace_disk_size = previous_disk_size;
        config_debug.trace_disk_dir_option = previous_dir_option;
        config_debug.trace_disk_path = previous_path;
        strncpy_fit(trace_logger_disk_directory, previous_path.c_str(), sizeof(trace_logger_disk_directory));
        return false;
    }
    trace_logger_output = output;
    return true;
}

void gui_debug_trace_logger_set_event_filters(const u32* filters)
{
    if (!filters)
        return;
    TraceLogger* logger = emu_get_core()->GetTraceLogger();
    for (int i = 0; i < TRACE_TYPE_COUNT; i++)
        logger->SetEventFilter((GS_Trace_Type)i, filters[i]);
    config_debug.trace_vdp_events = (int)filters[TRACE_VDP];
    config_debug.trace_input_events = (int)filters[TRACE_INPUT];
    config_debug.trace_io_events = (int)filters[TRACE_IO];
    config_debug.trace_psg_events = (int)filters[TRACE_PSG];
    config_debug.trace_ym2413_events = (int)filters[TRACE_YM2413];
    config_debug.trace_mapper_events = (int)filters[TRACE_MAPPER];
}

void gui_debug_window_trace_logger(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(340, 168), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(544, 362), ImGuiCond_FirstUseEver);

    ImGui::Begin("Trace Logger", &config_debug.show_trace_logger, ImGuiWindowFlags_MenuBar);

    trace_logger_menu();

    TraceLogger* tl = emu_get_core()->GetTraceLogger();

    if (ImGui::Button(trace_logger_enabled ? "Stop" : "Start"))
    {
        if (trace_logger_enabled)
        {
            gui_debug_trace_logger_stop();
        }
        else
        {
            trace_logger_start(trace_logger_get_config_flags(), false);
        }
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(trace_logger_enabled && config_debug.trace_output == gui_TraceOutput_Disk);
    if (ImGui::Button("Clear"))
    {
        gui_debug_trace_logger_clear();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(trace_logger_enabled);
    ImGui::SetNextItemWidth(90.0f);
    int previous_output = config_debug.trace_output;
    if (ImGui::Combo("##trace_output", &config_debug.trace_output, "Memory\0Disk\0\0"))
    {
        if (!trace_logger_apply_capacity())
            config_debug.trace_output = previous_output;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(trace_logger_enabled);
    ImGui::SetNextItemWidth(145.0f);
    if (config_debug.trace_output == gui_TraceOutput_Memory)
    {
        int previous_capacity = config_debug.trace_capacity;
        if (ImGui::Combo("##trace_capacity", &config_debug.trace_capacity, k_trace_logger_capacity_labels, IM_ARRAYSIZE(k_trace_logger_capacity_labels)) && !trace_logger_apply_capacity())
            config_debug.trace_capacity = previous_capacity;
    }
    else
    {
        ImGui::Combo("##trace_disk_size", &config_debug.trace_disk_size, "10 MB\0" "50 MB\0" "100 MB\0" "250 MB\0" "500 MB\0" "1 GB\0" "Unbounded\0\0");
    }
    ImGui::EndDisabled();
    if (config_debug.trace_output == gui_TraceOutput_Memory && ImGui::IsItemHovered())
    {
        double memory_mib = ((double)k_trace_logger_capacities[config_debug.trace_capacity] * sizeof(GS_Trace_Entry)) / (1024.0 * 1024.0);
        ImGui::SetTooltip("Preallocated memory: %.1f MiB (%u bytes per entry).", memory_mib, (u32)sizeof(GS_Trace_Entry));
    }

    if (config_debug.trace_output == gui_TraceOutput_Memory)
    {
        ImGui::SameLine();
        ImGui::Text("Entries: %u / %u", tl->GetCount(), tl->GetCapacity());
    }
    if (config_debug.trace_output == gui_TraceOutput_Disk && trace_logger_file_path[0] != '\0')
    {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##trace_disk_file", trace_logger_file_path, sizeof(trace_logger_file_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
    }

    if (trace_logger_enabled)
        trace_logger_sync_flags();

    u32 count = tl->GetCount();
    ImGui::PushFont(gui_default_font);
    float line_height = ImGui::GetTextLineHeightWithSpacing();
    float content_height = (float)count * line_height;
    ImGui::SetNextWindowContentSize(ImVec2(0.0f, content_height));
    if ((trace_logger_enabled && trace_logger_follow_latest) || trace_logger_scroll_to_bottom)
        ImGui::SetNextWindowScroll(ImVec2(-1.0f, content_height));

    if (ImGui::BeginChild("##logger", ImVec2(ImGui::GetContentRegionAvail().x, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        float scroll_y = ImGui::GetScrollY();
        float scroll_max_y = ImGui::GetScrollMaxY();
        bool at_bottom = scroll_y >= scroll_max_y - 0.5f;
        bool user_scrolling = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
            (ImGui::GetIO().MouseWheel != 0.0f || ImGui::IsMouseDragging(ImGuiMouseButton_Left));
        if (trace_logger_enabled)
        {
            if (trace_logger_scroll_to_bottom)
            {
                trace_logger_follow_latest = true;
                trace_logger_wait_for_scroll_away = false;
            }
            else if (trace_logger_follow_latest && user_scrolling)
            {
                trace_logger_follow_latest = false;
                trace_logger_wait_for_scroll_away = true;
            }
            else if (!trace_logger_follow_latest)
            {
                if (trace_logger_wait_for_scroll_away)
                {
                    if (!at_bottom)
                        trace_logger_wait_for_scroll_away = false;
                }
                else if (at_bottom)
                    trace_logger_follow_latest = true;
            }
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)count, line_height);

        while (clipper.Step())
        {
            for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; item++)
            {
                const GS_Trace_Entry& entry = tl->GetEntry((u32)item);
                trace_logger_previous_entry = item > 0 ? &tl->GetEntry((u32)item - 1) : NULL;
                u64 entry_number = tl->GetSequence() - (u64)count + (u64)item;
                render_entry_colored(entry, entry_number);
            }
        }

        trace_logger_scroll_to_bottom = false;
    }

    ImGui::EndChild();
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();

    if (trace_logger_choose_output_path)
    {
        trace_logger_choose_output_path = false;
        gui_file_dialog_choose_trace_path();
    }
}

void gui_debug_trace_logger_clear(void)
{
    emu_get_core()->GetTraceLogger()->Reset();
}

void gui_debug_save_log(const char* file_path)
{
    FILE* file = fopen_utf8(file_path, "w");

    if (file != NULL)
    {
        TraceLogger* tl = emu_get_core()->GetTraceLogger();
        u32 count = tl->GetCount();
        char buf[GS_TRACE_FORMAT_BUFFER_SIZE];

        for (u32 i = 0; i < count; i++)
        {
            const GS_Trace_Entry& entry = tl->GetEntry(i);
            trace_logger_previous_entry = i > 0 ? &tl->GetEntry(i - 1) : NULL;
            format_entry_text(entry, buf, sizeof(buf));
            if (config_debug.trace_counter)
                fprintf(file, "%012llu %s\n", (unsigned long long)(tl->GetSequence() - count + i), buf);
            else
                fprintf(file, "%s\n", buf);
        }

        fclose(file);
    }
}

static void trace_logger_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save Log As...", NULL, false, config_debug.trace_output == gui_TraceOutput_Memory))
        {
            gui_file_dialog_save_log();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Settings"))
    {
        ImGui::MenuItem("Event Counter", "", &config_debug.trace_counter);
        ImGui::MenuItem("Master Clock Cycles", "", &config_debug.trace_cycles);
        if (ImGui::BeginMenu("CPU"))
        {
            ImGui::MenuItem("Bank Number", "", &config_debug.trace_bank);
            ImGui::MenuItem("Registers", "", &config_debug.trace_registers);
            ImGui::MenuItem("Flags", "", &config_debug.trace_flags);
            ImGui::MenuItem("Bytes", "", &config_debug.trace_bytes);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Disk Output"))
        {
            ImGui::BeginDisabled(trace_logger_enabled);
            ImGui::SetNextItemWidth(180.0f);
            ImGui::Combo("##trace_disk_dir", &config_debug.trace_disk_dir_option, "Default Location\0Same as ROM\0Custom Location\0\0");

            switch ((Directory_Location)config_debug.trace_disk_dir_option)
            {
                default:
                case Directory_Location_Default:
                    ImGui::Text("%s", config_root_path);
                    break;
                case Directory_Location_ROM:
                    if (!emu_is_empty())
                        ImGui::Text("%s", emu_get_core()->GetCartridge()->GetFileDirectory());
                    break;
                case Directory_Location_Custom:
                    if (ImGui::MenuItem("Choose..."))
                        trace_logger_choose_output_path = true;
                    ImGui::PushItemWidth(450.0f);
                    if (ImGui::InputText("##trace_disk_path", trace_logger_disk_directory, sizeof(trace_logger_disk_directory), ImGuiInputTextFlags_AutoSelectAll))
                        config_debug.trace_disk_path.assign(trace_logger_disk_directory);
                    ImGui::PopItemWidth();
                    break;
            }
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Filters"))
    {
        if (ImGui::BeginMenu("CPU"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_cpu_enabled);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_cpu_enabled);
            ImGui::MenuItem("Instructions", "", &config_debug.trace_cpu);
            ImGui::MenuItem("IRQs", "", &config_debug.trace_cpu_irq);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("VDP"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_vdp);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_vdp);
            trace_logger_menu_event_filter("Registers", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_REGISTERS);
            trace_logger_menu_event_filter("Interrupts", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_INTERRUPTS);
            trace_logger_menu_event_filter("Status", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_STATUS);
            trace_logger_menu_event_filter("Sprites", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_SPRITES);
            trace_logger_menu_event_filter("State", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_STATE);
            trace_logger_menu_event_filter("Data", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_DATA);
            trace_logger_menu_event_filter("CRAM", &config_debug.trace_vdp_events, TRACE_VDP_EVENT_CRAM);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Input"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_input);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_input);
            trace_logger_menu_event_filter("Reads", &config_debug.trace_input_events, TRACE_INPUT_EVENT_READS);
            trace_logger_menu_event_filter("Changes", &config_debug.trace_input_events, TRACE_INPUT_EVENT_CHANGES);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("I/O"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_io);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_io);
            trace_logger_menu_event_filter("Control", &config_debug.trace_io_events, TRACE_IO_EVENT_CONTROL);
            trace_logger_menu_event_filter("Counters", &config_debug.trace_io_events, TRACE_IO_EVENT_COUNTERS);
            trace_logger_menu_event_filter("Game Gear Registers", &config_debug.trace_io_events, TRACE_IO_EVENT_GAMEGEAR);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("PSG"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_psg);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_psg);
            trace_logger_menu_event_filter("Tone", &config_debug.trace_psg_events, TRACE_PSG_EVENT_TONE);
            trace_logger_menu_event_filter("Volume", &config_debug.trace_psg_events, TRACE_PSG_EVENT_VOLUME);
            trace_logger_menu_event_filter("Noise", &config_debug.trace_psg_events, TRACE_PSG_EVENT_NOISE);
            trace_logger_menu_event_filter("Stereo", &config_debug.trace_psg_events, TRACE_PSG_EVENT_STEREO);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("YM2413"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_ym2413);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_ym2413);
            trace_logger_menu_event_filter("Registers", &config_debug.trace_ym2413_events, TRACE_YM2413_EVENT_REGISTERS);
            trace_logger_menu_event_filter("Mixer", &config_debug.trace_ym2413_events, TRACE_YM2413_EVENT_MIXER);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Mapper"))
        {
            ImGui::MenuItem("Enabled", "", &config_debug.trace_mapper);
            ImGui::Separator();
            ImGui::BeginDisabled(!config_debug.trace_mapper);
            trace_logger_menu_event_filter("ROM", &config_debug.trace_mapper_events, TRACE_MAPPER_EVENT_ROM);
            trace_logger_menu_event_filter("RAM", &config_debug.trace_mapper_events, TRACE_MAPPER_EVENT_RAM);
            trace_logger_menu_event_filter("Control", &config_debug.trace_mapper_events, TRACE_MAPPER_EVENT_CONTROL);
            trace_logger_menu_event_filter("EEPROM", &config_debug.trace_mapper_events, TRACE_MAPPER_EVENT_EEPROM);
            trace_logger_menu_event_filter("Flash", &config_debug.trace_mapper_events, TRACE_MAPPER_EVENT_FLASH);
            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

static void trace_logger_sync_flags(void)
{
    TraceLogger* logger = emu_get_core()->GetTraceLogger();
    logger->SetEnabledFlags(trace_logger_get_config_flags());
    logger->SetEventFilter(TRACE_VDP, (u32)config_debug.trace_vdp_events);
    logger->SetEventFilter(TRACE_INPUT, (u32)config_debug.trace_input_events);
    logger->SetEventFilter(TRACE_IO, (u32)config_debug.trace_io_events);
    logger->SetEventFilter(TRACE_PSG, (u32)config_debug.trace_psg_events);
    logger->SetEventFilter(TRACE_YM2413, (u32)config_debug.trace_ym2413_events);
    logger->SetEventFilter(TRACE_MAPPER, (u32)config_debug.trace_mapper_events);
}

static u32 trace_logger_get_config_flags(void)
{
    u32 flags = 0;
    if (config_debug.trace_cpu_enabled && config_debug.trace_cpu)     flags |= TRACE_FLAG_CPU;
    if (config_debug.trace_cpu_enabled && config_debug.trace_cpu_irq) flags |= TRACE_FLAG_CPU_IRQ;
    if (config_debug.trace_vdp)           flags |= TRACE_FLAG_VDP;
    if (config_debug.trace_input)         flags |= TRACE_FLAG_INPUT;
    if (config_debug.trace_io)            flags |= TRACE_FLAG_IO;
    if (config_debug.trace_psg)           flags |= TRACE_FLAG_PSG;
    if (config_debug.trace_ym2413)        flags |= TRACE_FLAG_YM2413;
    if (config_debug.trace_mapper)        flags |= TRACE_FLAG_MAPPER;
    return flags;
}

static void trace_logger_set_config_flags(u32 flags)
{
    config_debug.trace_cpu_enabled = (flags & (TRACE_FLAG_CPU | TRACE_FLAG_CPU_IRQ)) != 0;
    config_debug.trace_cpu = (flags & TRACE_FLAG_CPU) != 0;
    config_debug.trace_cpu_irq = (flags & TRACE_FLAG_CPU_IRQ) != 0;
    config_debug.trace_vdp = (flags & TRACE_FLAG_VDP) != 0;
    config_debug.trace_input = (flags & TRACE_FLAG_INPUT) != 0;
    config_debug.trace_io = (flags & TRACE_FLAG_IO) != 0;
    config_debug.trace_psg = (flags & TRACE_FLAG_PSG) != 0;
    config_debug.trace_ym2413 = (flags & TRACE_FLAG_YM2413) != 0;
    config_debug.trace_mapper = (flags & TRACE_FLAG_MAPPER) != 0;
}

static void format_entry_text(const GS_Trace_Entry& entry, bool cycles,
    const GS_Trace_Entry* previous, char* buf, int buf_size)
{
    GS_Trace_Format_Options options = {};
    options.bank = config_debug.trace_bank;
    options.registers = config_debug.trace_registers;
    options.flags = config_debug.trace_flags;
    options.bytes = config_debug.trace_bytes;
    options.cycles = cycles;
    options.previous = previous;
    trace_logger_format_entry(entry, options, buf, (size_t)buf_size);
}

static void format_entry_text(const GS_Trace_Entry& entry, char* buf, int buf_size)
{
    format_entry_text(entry, config_debug.trace_cycles, trace_logger_previous_entry, buf, buf_size);
}

static void render_cpu_entry_colored(const GS_Trace_Entry& entry, int prefix_length)
{
    if (config_debug.trace_bank)
    {
        ImGui::TextColored(violet, "%03X:", entry.cpu.bank);
        ImGui::SameLine(0, 0);
    }

    ImGui::TextColored(cyan, "%04X", entry.cpu.pc);

    if (config_debug.trace_registers)
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, "  AF:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.af);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " BC:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.bc);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " DE:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.de);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " HL:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.hl);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " IX:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.ix);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " IY:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.iy);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " SP:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%04X", entry.cpu.sp);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " I:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%02X", entry.cpu.i);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " R:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%02X", entry.cpu.r);
        ImGui::SameLine(0, 0);
        ImGui::TextColored(magenta, " IM:");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(white, "%u", entry.cpu.im);
    }

    if (config_debug.trace_flags)
    {
        u8 f = (u8)entry.cpu.af;
        ImGui::SameLine(0, 0);
        ImGui::TextColored(yellow, " %c%c%c%c%c%c%c%c",
                 (f & FLAG_SIGN) ? 'S' : 's',
                 (f & FLAG_ZERO) ? 'Z' : 'z',
                 (f & FLAG_Y) ? 'Y' : 'y',
                 (f & FLAG_HALF) ? 'H' : 'h',
                 (f & FLAG_X) ? 'X' : 'x',
                 (f & FLAG_PARITY) ? 'P' : 'p',
                 (f & FLAG_NEGATIVE) ? 'N' : 'n',
                 (f & FLAG_CARRY) ? 'C' : 'c');
    }

    if (entry.cpu.name[0] != 0)
    {
        std::string instr = entry.cpu.name;
        size_t pos;
        pos = instr.find("{n}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_white);
        pos = instr.find("{o}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_brown);
        pos = instr.find("{e}");
        if (pos != std::string::npos)
            instr.replace(pos, 3, c_blue);

        ImGui::SameLine(0, 0);
        TextColoredEx("  %s%s", c_white.c_str(), instr.c_str());
    }
    else
    {
        ImGui::SameLine(0, 0);
        ImGui::TextColored(gray, "  ???");
    }

    if (config_debug.trace_bytes)
    {
        char bytes[32];
        trace_log_format_cpu_bytes(entry, bytes, sizeof(bytes));
        float char_width = ImGui::CalcTextSize("A").x;
        float bytes_column = char_width * 35;
        if (config_debug.trace_bank)      bytes_column += char_width * 4;
        if (config_debug.trace_registers) bytes_column += char_width * 80;
        if (config_debug.trace_flags)     bytes_column += char_width * 9;
        bytes_column += char_width * prefix_length;
        ImGui::SameLine(bytes_column);
        ImGui::TextColored(gray, "%s", bytes);
    }
}

static void render_entry_colored(const GS_Trace_Entry& entry, u64 index)
{
    char buf[GS_TRACE_FORMAT_BUFFER_SIZE];
    int prefix_length = 0;

    if (config_debug.trace_counter)
    {
        char counter[32];
        snprintf(counter, sizeof(counter), "%06llu ", (unsigned long long)index);
        prefix_length += (int)strlen(counter);
        ImGui::TextColored(gray, "%s", counter);
        ImGui::SameLine(0, 0);
    }

    if (config_debug.trace_cycles)
    {
        char cycles[64];
        trace_log_format_cycle_prefix(entry, trace_logger_previous_entry, cycles, sizeof(cycles));
        prefix_length += (int)strlen(cycles);
        ImGui::TextColored(gray, "%s", cycles);
        ImGui::SameLine(0, 0);
    }

    switch (entry.type)
    {
        case TRACE_CPU:
            render_cpu_entry_colored(entry, prefix_length);
            break;
        case TRACE_CPU_IRQ:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(red, "%s", buf);
            break;
        case TRACE_VDP:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(green, "%s", buf);
            break;
        case TRACE_INPUT:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(yellow, "%s", buf);
            break;
        case TRACE_PSG:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(blue, "%s", buf);
            break;
        case TRACE_YM2413:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(violet, "%s", buf);
            break;
        case TRACE_IO:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(orange, "%s", buf);
            break;
        case TRACE_MAPPER:
            format_entry_text(entry, false, NULL, buf, sizeof(buf));
            ImGui::TextColored(magenta, "%s", buf);
            break;
        default:
            break;
    }
}
