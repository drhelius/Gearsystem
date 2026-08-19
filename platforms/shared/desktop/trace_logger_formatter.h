#ifndef TRACE_LOGGER_FORMATTER_H
#define TRACE_LOGGER_FORMATTER_H

#include "gearsystem.h"

#define GS_TRACE_FORMAT_BUFFER_SIZE 512

struct GS_Trace_Format_Options
{
    bool bank;
    bool registers;
    bool flags;
    bool bytes;
    bool cycles;
    const GS_Trace_Entry* previous;
};

void trace_log_format_cpu_bytes(const GS_Trace_Entry& entry, char* buffer, size_t buffer_size);
void trace_log_format_cycle_prefix(const GS_Trace_Entry& entry, const GS_Trace_Entry* previous,
                                   char* buffer, size_t buffer_size);
void trace_logger_format_entry(const GS_Trace_Entry& entry,
    const GS_Trace_Format_Options& options, char* buffer, size_t buffer_size);

#endif
