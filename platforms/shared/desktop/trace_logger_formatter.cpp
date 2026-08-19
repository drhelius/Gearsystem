#include <stdio.h>
#include <string.h>
#include "trace_logger_formatter.h"

static void strip_tags(const char* source, char* destination, size_t size)
{
    size_t output = 0;
    for (size_t input = 0; source[input] && output + 1 < size; input++)
    {
        if (source[input] == '{')
        {
            const char* end = strchr(source + input, '}');
            if (end)
            {
                input = (size_t)(end - source);
                continue;
            }
        }
        destination[output++] = source[input];
    }
    destination[output] = 0;
}

static const char* mapper_name(u8 mapper)
{
    static const char* names[] = {
        "ROM", "SEGA", "CODEMASTERS", "SG1000", "KOREAN", "K-MSXSMS", "K-SMS32",
        "K-MSX32", "K-XOR1F", "K-MSX8", "K-XORFF", "K-HICOM", "K-FFFE", "K-BFFC",
        "K-FFF3", "K-MDFFF5", "K-MDFFF0", "MSX", "JANGGUN", "4PAK", "JUMBO",
        "93C46", "IRATA"
    };
    return mapper < sizeof(names) / sizeof(names[0]) ? names[mapper] : "UNKNOWN";
}

void trace_log_format_cycle_prefix(const GS_Trace_Entry& entry, const GS_Trace_Entry* previous,
                                   char* buffer, size_t buffer_size)
{
    if (!previous)
        snprintf(buffer, buffer_size, "@%012llu                ", (unsigned long long)entry.cycle);
    else if (entry.cycle < previous->cycle)
        snprintf(buffer, buffer_size, "@%012llu RESET          ", (unsigned long long)entry.cycle);
    else
        snprintf(buffer, buffer_size, "@%012llu +%-12llu ", (unsigned long long)entry.cycle,
                 (unsigned long long)(entry.cycle - previous->cycle));
}

GS_Disassembler_Record* trace_log_get_cpu_record(Memory* memory, const GS_Trace_Entry& entry)
{
    GS_Disassembler_Record* record = memory->GetDisassemblerRecord(entry.cpu.pc, entry.cpu.bank);
    bool valid = IsValidPointer(record) && record->bank == entry.cpu.bank &&
                 record->size == entry.cpu.size && entry.cpu.size <= sizeof(entry.cpu.opcodes);
    if (valid)
    {
        for (u8 i = 0; i < entry.cpu.size; i++)
        {
            if (record->opcodes[i] != entry.cpu.opcodes[i])
            {
                valid = false;
                break;
            }
        }
    }
    return valid ? record : NULL;
}

void trace_log_format_cpu_bytes(const GS_Trace_Entry& entry, char* buffer, size_t buffer_size)
{
    size_t offset = 0;
    buffer[0] = 0;
    for (u8 i = 0; i < entry.cpu.size && offset + 4 < buffer_size; i++)
        offset += (size_t)snprintf(buffer + offset, buffer_size - offset, "%02X ", entry.cpu.opcodes[i]);
}

static void format_cpu(const GS_Trace_Entry& entry, Memory* memory,
                       const GS_Trace_Format_Options& options, char* buffer, size_t size)
{
    char mnemonic[80] = "???";
    GS_Disassembler_Record* record = trace_log_get_cpu_record(memory, entry);
    if (IsValidPointer(record))
        strip_tags(record->name, mnemonic, sizeof(mnemonic));

    char bank[16] = "";
    if (options.bank)
        snprintf(bank, sizeof(bank), "%03X:", entry.cpu.bank);

    char registers[160] = "";
    if (options.registers)
    {
        snprintf(registers, sizeof(registers),
                 "AF:%04X BC:%04X DE:%04X HL:%04X IX:%04X IY:%04X SP:%04X I:%02X R:%02X IM:%d ",
                 entry.cpu.af, entry.cpu.bc, entry.cpu.de, entry.cpu.hl, entry.cpu.ix,
                 entry.cpu.iy, entry.cpu.sp, entry.cpu.i, entry.cpu.r, entry.cpu.im);
    }

    char flags[24] = "";
    if (options.flags)
    {
        u8 value = (u8)entry.cpu.af;
        snprintf(flags, sizeof(flags), "%c%c%c%c%c%c%c%c ",
                 value & FLAG_SIGN ? 'S' : 's', value & FLAG_ZERO ? 'Z' : 'z',
                 value & FLAG_Y ? 'Y' : 'y', value & FLAG_HALF ? 'H' : 'h',
                 value & FLAG_X ? 'X' : 'x', value & FLAG_PARITY ? 'P' : 'p',
                 value & FLAG_NEGATIVE ? 'N' : 'n', value & FLAG_CARRY ? 'C' : 'c');
    }

    char bytes[32] = "";
    if (options.bytes)
        trace_log_format_cpu_bytes(entry, bytes, sizeof(bytes));

    snprintf(buffer, size, "[CPU] %s%04X  %s%s%-24s %s", bank, entry.cpu.pc,
             registers, flags, mnemonic, bytes);
}

void trace_logger_format_entry(const GS_Trace_Entry& entry, Memory* memory,
                               const GS_Trace_Format_Options& options,
                               char* buffer, size_t buffer_size)
{
    char cycles[48];
    char text[GS_TRACE_FORMAT_BUFFER_SIZE];
    cycles[0] = 0;
    if (options.cycles)
        trace_log_format_cycle_prefix(entry, options.previous, cycles, sizeof(cycles));

    switch (entry.type)
    {
        case TRACE_CPU:
            format_cpu(entry, memory, options, text, sizeof(text));
            break;
        case TRACE_CPU_IRQ:
            snprintf(text, sizeof(text), "[CPU] %s PC:$%04X Vector:$%04X",
                     entry.irq.type == 2 ? "NMI" : "IRQ", entry.irq.pc, entry.irq.vector);
            break;
        case TRACE_VDP:
        {
            static const char* names[] = {"REG", "VINT", "HINT", "VFLAG", "STATUS", "SPR OVR", "SPR COL",
                "DISPLAY", "SCROLL X", "SCROLL Y", "MODE", "TIMING", "CONTROL", "DATA RD", "DATA WR", "CRAM"};
            const char* name = entry.vdp.event < 16 ? names[entry.vdp.event] : "???";
            if (entry.vdp.event == TRACE_VDP_STATUS_READ)
            {
                snprintf(text, sizeof(text), "[VDP] %-8s Line:%u H:%u Before:$%02X Result:$%02X After:$%02X",
                         name, entry.vdp.line, entry.vdp.hpos, entry.vdp.status_before,
                         entry.vdp.effective, entry.vdp.status_after);
            }
            else if (entry.vdp.event == TRACE_VDP_CRAM_WRITE)
            {
                snprintf(text, sizeof(text), "[VDP] %-8s Line:%u H:%u Index:$%02X Raw:$%02X Color:$%03X",
                         name, entry.vdp.line, entry.vdp.hpos, entry.vdp.address,
                         entry.vdp.raw, entry.vdp.auxiliary);
            }
            else
            {
                snprintf(text, sizeof(text), "[VDP] %-8s Line:%u H:%u Raw:$%02X Effective:$%02X Addr:$%04X Code:%u Aux:$%04X",
                         name, entry.vdp.line, entry.vdp.hpos, entry.vdp.raw, entry.vdp.effective,
                         entry.vdp.address, entry.vdp.code, entry.vdp.auxiliary);
            }
            break;
        }
        case TRACE_INPUT:
            snprintf(text, sizeof(text), "[INP] %-6s Port:$%02X Player:%u Raw:$%02X Effective:$%02X Control:$%02X Device:%u",
                     entry.input.event == TRACE_INPUT_READ ? "READ" : "CHANGE",
                     entry.input.port, entry.input.player, entry.input.raw, entry.input.effective,
                     entry.input.control, entry.input.device);
            break;
        case TRACE_IO:
        {
            static const char* names[] = {"CONTROL", "COUNTER", "LATCH", "GG READ", "GG WRITE"};
            const char* name = entry.io.event < 5 ? names[entry.io.event] : "???";
            snprintf(text, sizeof(text), "[IO] %-8s Port:$%02X Raw:$%02X Effective:$%02X Previous:$%02X Aux:$%02X",
                     name, entry.io.port, entry.io.raw, entry.io.effective, entry.io.previous, entry.io.auxiliary);
            break;
        }
        case TRACE_PSG:
        {
            static const char* names[] = {"TONE", "VOLUME", "NOISE", "STEREO"};
            const char* name = entry.psg.event < 4 ? names[entry.psg.event] : "???";
            snprintf(text, sizeof(text), "[PSG] %-7s Raw:$%02X Channel:%u Latch:$%02X Period:$%04X Attenuation:%u",
                     name, entry.psg.value, entry.psg.channel, entry.psg.latch,
                     entry.psg.period, entry.psg.attenuation);
            break;
        }
        case TRACE_YM2413:
            snprintf(text, sizeof(text), "[YM] %s Port:$%02X Reg:$%02X Raw:$%02X Effective:$%02X Accepted:%s PSG:%s FM:%s",
                     entry.ym2413.event == TRACE_YM2413_MIXER ? "MIXER" : "REGISTER",
                     entry.ym2413.port, entry.ym2413.reg, entry.ym2413.value, entry.ym2413.effective,
                     entry.ym2413.accepted ? "yes" : "no", entry.ym2413.psg_enabled ? "on" : "off",
                     entry.ym2413.fm_enabled ? "on" : "off");
            break;
        case TRACE_MAPPER:
        {
            static const char* names[] = {"ROM", "RAM", "CONTROL", "EEPROM", "FLASH"};
            const char* name = entry.mapper.event < 5 ? names[entry.mapper.event] : "???";
            char state[32] = "";
            if (entry.mapper.flags_valid)
                snprintf(state, sizeof(state), " Flags:$%02X", entry.mapper.flags);
            snprintf(text, sizeof(text), "[MAP] %s %-7s Addr:$%04X Raw:$%02X Banks:%03X/%03X/%03X/%03X/%03X/%03X RAM:%d%s Aux:$%04X",
                     mapper_name(entry.mapper.mapper), name, entry.mapper.address, entry.mapper.value,
                     entry.mapper.banks[0], entry.mapper.banks[1], entry.mapper.banks[2],
                     entry.mapper.banks[3], entry.mapper.banks[4], entry.mapper.banks[5],
                     entry.mapper.ram_bank, state, entry.mapper.auxiliary);
            break;
        }
        default:
            snprintf(text, sizeof(text), "[???]");
            break;
    }

    snprintf(buffer, buffer_size, "%s%s", cycles, text);
}
