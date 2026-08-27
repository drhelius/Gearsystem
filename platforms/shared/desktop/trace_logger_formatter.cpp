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

static const char* vdp_register_name(u8 reg)
{
    static const char* names[] = {
        "CONTROL 1", "CONTROL 2", "NAME TABLE", "COLOR TABLE", "PATTERN TABLE",
        "SPRITE ATTR", "SPRITE PATTERN", "BACKDROP COLOR", "H SCROLL", "V SCROLL",
        "V INTERRUPT"
    };
    return reg < sizeof(names) / sizeof(names[0]) ? names[reg] : "UNKNOWN";
}

static const char* vdp_code_name(u8 code)
{
    static const char* names[] = {"VRAM READ", "VRAM WRITE", "REGISTER WRITE", "CRAM WRITE"};
    return code < sizeof(names) / sizeof(names[0]) ? names[code] : "UNKNOWN";
}

static const char* input_device_name(u8 device)
{
    static const char* names[] = {"Joypad", "Light Phaser", "Paddle", "Sports Pad"};
    return device < sizeof(names) / sizeof(names[0]) ? names[device] : "Unknown";
}

static const char* input_key_name(u8 key)
{
    switch (key)
    {
        case Key_Up:    return "UP";
        case Key_Down:  return "DOWN";
        case Key_Left:  return "LEFT";
        case Key_Right: return "RIGHT";
        case Key_1:     return "BUTTON 1";
        case Key_2:     return "BUTTON 2";
        case Key_Start: return "START";
        default:        return "UNKNOWN";
    }
}

static const char* input_port_name(u8 port)
{
    if (port == 0x00)
        return "START";
    if (port >= 0xC0)
        return (port & 0x01) ? "PORT B" : "PORT A";
    return "PORT";
}

static const char* game_gear_register_name(u8 port, bool write)
{
    switch (port)
    {
        case 0x00: return "START";
        case 0x01: return "PDR";
        case 0x02: return "DDR/NINT";
        case 0x03: return "TX DATA";
        case 0x04: return "RX DATA";
        case 0x05: return write ? "SCTRL" : "SSTATUS";
        default: return "REGISTER";
    }
}

static u32 geartogear_baud_rate(u8 control)
{
    static const u32 rates[4] = { 4800, 2400, 1200, 300 };
    return rates[(control >> 6) & 0x03];
}

static const char* eeprom_state_name(u8 state)
{
    static const char* names[] = {"START", "OPCODE", "READ", "WRITE"};
    return state < sizeof(names) / sizeof(names[0]) ? names[state] : "UNKNOWN";
}

static void format_mapper_banks(const GS_Trace_Entry& entry, char* buffer, size_t size)
{
    snprintf(buffer, size, "$%03X/$%03X/$%03X/$%03X/$%03X/$%03X",
             entry.mapper.banks[0], entry.mapper.banks[1], entry.mapper.banks[2],
             entry.mapper.banks[3], entry.mapper.banks[4], entry.mapper.banks[5]);
}

static void format_ym2413_register(u8 reg, u8 value, bool write, char* buffer, size_t size)
{
    if (reg <= 0x07)
    {
        static const char* names[] = {
            "USER MOD CONTROL", "USER CAR CONTROL", "USER MOD LEVEL", "USER WAVE/FEEDBACK",
            "USER MOD ENVELOPE", "USER CAR ENVELOPE", "USER MOD SUSTAIN", "USER CAR SUSTAIN"
        };
        if (!write)
            snprintf(buffer, size, "%s", names[reg]);
        else if (reg <= 0x01)
            snprintf(buffer, size, "%s AM:%s VIB:%s EG:%s KSR:%s MULT:$%X", names[reg],
                     value & 0x80 ? "on" : "off", value & 0x40 ? "on" : "off",
                     value & 0x20 ? "on" : "off", value & 0x10 ? "on" : "off", value & 0x0F);
        else if (reg == 0x02)
            snprintf(buffer, size, "%s KSL:%u TL:$%02X", names[reg], (value >> 6) & 0x03, value & 0x3F);
        else if (reg == 0x03)
            snprintf(buffer, size, "%s KSL:%u C-WF:%u M-WF:%u FB:%u", names[reg],
                     (value >> 6) & 0x03, (value >> 4) & 0x01, (value >> 3) & 0x01, value & 0x07);
        else if (reg <= 0x05)
            snprintf(buffer, size, "%s AR:$%X DR:$%X", names[reg], (value >> 4) & 0x0F, value & 0x0F);
        else
            snprintf(buffer, size, "%s SL:$%X RR:$%X", names[reg], (value >> 4) & 0x0F, value & 0x0F);
    }
    else if (reg == 0x0E)
    {
        if (write)
            snprintf(buffer, size, "RHYTHM Mode:%s BD:%s SD:%s TOM:%s CYM:%s HH:%s",
                     value & 0x20 ? "on" : "off", value & 0x10 ? "on" : "off",
                     value & 0x08 ? "on" : "off", value & 0x04 ? "on" : "off",
                     value & 0x02 ? "on" : "off", value & 0x01 ? "on" : "off");
        else
            snprintf(buffer, size, "RHYTHM CONTROL");
    }
    else if (reg >= 0x10 && reg <= 0x18)
        snprintf(buffer, size, "F-NUM LOW Ch:%u", reg - 0x10);
    else if (reg >= 0x20 && reg <= 0x28)
    {
        if (write)
            snprintf(buffer, size, "KEY/BLOCK Ch:%u FHi:%u Block:%u Key:%s Sustain:%s",
                     reg - 0x20, value & 0x01, (value >> 1) & 0x07,
                     value & 0x10 ? "on" : "off", value & 0x20 ? "on" : "off");
        else
            snprintf(buffer, size, "KEY/BLOCK Ch:%u", reg - 0x20);
    }
    else if (reg >= 0x30 && reg <= 0x38)
    {
        if (write)
            snprintf(buffer, size, "INSTRUMENT/VOLUME Ch:%u Instrument:$%X Volume:$%X",
                     reg - 0x30, (value >> 4) & 0x0F, value & 0x0F);
        else
            snprintf(buffer, size, "INSTRUMENT/VOLUME Ch:%u", reg - 0x30);
    }
    else
        snprintf(buffer, size, "UNUSED");
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

void trace_log_format_cpu_bytes(const GS_Trace_Entry& entry, char* buffer, size_t buffer_size)
{
    size_t offset = 0;
    buffer[0] = 0;
    for (u8 i = 0; i < entry.cpu.size && offset + 4 < buffer_size; i++)
        offset += (size_t)snprintf(buffer + offset, buffer_size - offset, "%02X ", entry.cpu.opcodes[i]);
}

static void format_cpu(const GS_Trace_Entry& entry,
                       const GS_Trace_Format_Options& options, char* buffer, size_t size)
{
    char mnemonic[80] = "???";
    if (entry.cpu.name[0] != 0)
        strip_tags(entry.cpu.name, mnemonic, sizeof(mnemonic));

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

void trace_logger_format_entry(const GS_Trace_Entry& entry,
    const GS_Trace_Format_Options& options, char* buffer, size_t buffer_size)
{
    char cycles[48];
    char text[GS_TRACE_FORMAT_BUFFER_SIZE];
    cycles[0] = 0;
    if (options.cycles)
        trace_log_format_cycle_prefix(entry, options.previous, cycles, sizeof(cycles));

    switch (entry.type)
    {
        case TRACE_CPU:
            format_cpu(entry, options, text, sizeof(text));
            break;
        case TRACE_CPU_IRQ:
            snprintf(text, sizeof(text), "[CPU] %s PC:$%04X -> $%04X",
                     entry.irq.type == 2 ? "NMI" : "IRQ", entry.irq.pc, entry.irq.vector);
            break;
        case TRACE_VDP:
        {
            switch (entry.vdp.event)
            {
                case TRACE_VDP_REG_WRITE:
                    snprintf(text, sizeof(text), "[VDP] REG R%02u %s <- $%02X Line:%u H:%u",
                             entry.vdp.reg, vdp_register_name(entry.vdp.reg), entry.vdp.effective,
                             entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_VINT_REQUEST:
                    snprintf(text, sizeof(text), "[VDP] VINT IRQ %s Line:%u H:%u",
                             entry.vdp.effective ? "ASSERTED" : "CLEARED", entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_HINT_REQUEST:
                    snprintf(text, sizeof(text), "[VDP] HINT IRQ %s Line:%u H:%u",
                             entry.vdp.effective ? "ASSERTED" : "CLEARED", entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_VINT_FLAG:
                    snprintf(text, sizeof(text), "[VDP] VINT FLAG Status:$%02X->$%02X Line:%u H:%u",
                             entry.vdp.status_before, entry.vdp.status_after, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_STATUS_READ:
                    snprintf(text, sizeof(text), "[VDP] STATUS READ -> $%02X Status:$%02X->$%02X Line:%u H:%u",
                             entry.vdp.effective, entry.vdp.status_before, entry.vdp.status_after,
                             entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_SPRITE_OVERFLOW:
                    if (entry.vdp.auxiliary)
                    {
                        snprintf(text, sizeof(text), "[VDP] SPRITE OVERFLOW Sprite:%u Status:$%02X->$%02X Line:%u H:%u",
                                 entry.vdp.auxiliary, entry.vdp.status_before, entry.vdp.status_after,
                                 entry.vdp.line, entry.vdp.hpos);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[VDP] SPRITE OVERFLOW Status:$%02X->$%02X Line:%u H:%u",
                                 entry.vdp.status_before, entry.vdp.status_after,
                                 entry.vdp.line, entry.vdp.hpos);
                    }
                    break;
                case TRACE_VDP_SPRITE_COLLISION:
                    snprintf(text, sizeof(text), "[VDP] SPRITE COLLISION X:%u Status:$%02X->$%02X Line:%u H:%u",
                             entry.vdp.auxiliary, entry.vdp.status_before, entry.vdp.status_after,
                             entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_DISPLAY_LATCH:
                    snprintf(text, sizeof(text), "[VDP] DISPLAY %s R1:$%02X Line:%u H:%u",
                             entry.vdp.effective ? "ON" : "OFF", entry.vdp.raw,
                             entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_SCROLL_X_LATCH:
                    snprintf(text, sizeof(text), "[VDP] SCROLL X LATCH $%02X Line:%u H:%u",
                             entry.vdp.effective, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_SCROLL_Y_LATCH:
                    snprintf(text, sizeof(text), "[VDP] SCROLL Y LATCH $%02X Line:%u H:%u",
                             entry.vdp.effective, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_MODE_CHANGE:
                    snprintf(text, sizeof(text), "[VDP] MODE %u->%u Line:%u H:%u",
                             entry.vdp.raw, entry.vdp.effective, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_TIMING:
                    snprintf(text, sizeof(text), "[VDP] H COUNTER LATCH $%02X Line:%u H:%u",
                             entry.vdp.effective, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_CONTROL:
                    if (entry.vdp.auxiliary == 1)
                    {
                        snprintf(text, sizeof(text), "[VDP] CONTROL BYTE 1 $%02X Address:$%04X Line:%u H:%u",
                                 entry.vdp.raw, entry.vdp.address, entry.vdp.line, entry.vdp.hpos);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[VDP] CONTROL BYTE 2 $%02X %s Address:$%04X Line:%u H:%u",
                                 entry.vdp.raw, vdp_code_name(entry.vdp.code), entry.vdp.address,
                                 entry.vdp.line, entry.vdp.hpos);
                    }
                    break;
                case TRACE_VDP_DATA_READ:
                    snprintf(text, sizeof(text), "[VDP] DATA READ VRAM[$%04X] -> $%02X Line:%u H:%u",
                             entry.vdp.address, entry.vdp.effective, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_DATA_WRITE:
                    snprintf(text, sizeof(text), "[VDP] DATA WRITE VRAM[$%04X] <- $%02X Line:%u H:%u",
                             entry.vdp.address, entry.vdp.effective, entry.vdp.line, entry.vdp.hpos);
                    break;
                case TRACE_VDP_CRAM_WRITE:
                    snprintf(text, sizeof(text), "[VDP] CRAM[$%02X] <- $%02X Color:$%03X Line:%u H:%u",
                             entry.vdp.address, entry.vdp.raw, entry.vdp.auxiliary,
                             entry.vdp.line, entry.vdp.hpos);
                    break;
                default:
                    snprintf(text, sizeof(text), "[VDP] UNKNOWN Event:%u Line:%u H:%u",
                             entry.vdp.event, entry.vdp.line, entry.vdp.hpos);
                    break;
            }
            break;
        }
        case TRACE_INPUT:
            if (entry.input.event == TRACE_INPUT_READ)
            {
                if (entry.input.raw != entry.input.effective)
                {
                    snprintf(text, sizeof(text), "[INP] READ P%u %s %s ($%02X) -> $%02X Raw:$%02X Control:$%02X",
                             entry.input.player, input_device_name(entry.input.device), input_port_name(entry.input.port),
                             entry.input.port, entry.input.effective, entry.input.raw, entry.input.control);
                }
                else
                {
                    snprintf(text, sizeof(text), "[INP] READ P%u %s %s ($%02X) -> $%02X Control:$%02X",
                             entry.input.player, input_device_name(entry.input.device), input_port_name(entry.input.port),
                             entry.input.port, entry.input.effective, entry.input.control);
                }
            }
            else if (entry.input.event == TRACE_INPUT_CHANGE)
            {
                bool pressed = entry.input.player == 0 ? entry.input.effective == 0 :
                    (entry.input.effective & entry.input.port) == 0;
                if (entry.input.player == 0)
                {
                    snprintf(text, sizeof(text), "[INP] CHANGE RESET %s State:$%02X->$%02X",
                             pressed ? "PRESSED" : "RELEASED", entry.input.raw, entry.input.effective);
                }
                else
                {
                    snprintf(text, sizeof(text), "[INP] CHANGE P%u %s %s %s State:$%02X->$%02X",
                             entry.input.player, input_device_name(entry.input.device), input_key_name(entry.input.port),
                             pressed ? "PRESSED" : "RELEASED", entry.input.raw, entry.input.effective);
                }
            }
            else
                snprintf(text, sizeof(text), "[INP] UNKNOWN Event:%u", entry.input.event);
            break;
        case TRACE_IO:
        {
            switch (entry.io.event)
            {
                case TRACE_IO_CONTROL:
                    if (entry.io.port & 0x01)
                    {
                        snprintf(text, sizeof(text), "[IO] CONTROL Port:$%02X $%02X->$%02X",
                                 entry.io.port, entry.io.previous, entry.io.effective);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[IO] MEMORY CONTROL Port:$%02X <- $%02X",
                                 entry.io.port, entry.io.effective);
                    }
                    break;
                case TRACE_IO_COUNTER_READ:
                    snprintf(text, sizeof(text), "[IO] %c COUNTER READ Port:$%02X -> $%02X",
                             entry.io.auxiliary ? 'H' : 'V', entry.io.port, entry.io.effective);
                    break;
                case TRACE_IO_COUNTER_LATCH:
                {
                    const char* trigger = entry.io.auxiliary == 1 ? "TH-A" :
                        (entry.io.auxiliary == 2 ? "TH-B" : "TH-A/TH-B");
                    snprintf(text, sizeof(text), "[IO] H COUNTER LATCH $%02X Trigger:%s Control:$%02X->$%02X",
                             entry.io.effective, trigger, entry.io.previous, entry.io.raw);
                    break;
                }
                case TRACE_IO_GAMEGEAR_READ:
                    if (entry.io.raw != entry.io.effective)
                    {
                        snprintf(text, sizeof(text), "[IO] GG READ %s ($%02X) -> $%02X Raw:$%02X",
                                 game_gear_register_name(entry.io.port, false), entry.io.port,
                                 entry.io.effective, entry.io.raw);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[IO] GG READ %s ($%02X) -> $%02X",
                                 game_gear_register_name(entry.io.port, false), entry.io.port, entry.io.effective);
                    }
                    break;
                case TRACE_IO_GAMEGEAR_WRITE:
                    if (entry.io.raw != entry.io.effective)
                    {
                        snprintf(text, sizeof(text),
                                 "[IO] GG WRITE %s ($%02X) Raw:$%02X "
                                 "Effective:$%02X Previous:$%02X",
                                 game_gear_register_name(entry.io.port, true), entry.io.port,
                                 entry.io.raw, entry.io.effective, entry.io.previous);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[IO] GG WRITE %s ($%02X) $%02X->$%02X",
                                 game_gear_register_name(entry.io.port, true), entry.io.port,
                                 entry.io.previous, entry.io.effective);
                    }
                    break;
                default:
                    snprintf(text, sizeof(text), "[IO] UNKNOWN Event:%u Port:$%02X",
                             entry.io.event, entry.io.port);
                    break;
            }
            break;
        }
        case TRACE_GEARTOGEAR:
        {
            const char* state = entry.geartogear.data ? "CONNECTED" :
                "DISCONNECTED";
            u32 baud = geartogear_baud_rate(entry.geartogear.control);
            switch (entry.geartogear.event)
            {
                case TRACE_GEARTOGEAR_CABLE:
                    snprintf(text, sizeof(text),
                             "[G2G] CABLE %s Link:%llu", state,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_TX_START:
                    snprintf(text, sizeof(text),
                             "[G2G] TX START Data:$%02X Baud:%u Bit:%u Link:%llu",
                             entry.geartogear.data, baud,
                             entry.geartogear.bit_cycles,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_TX_END:
                    snprintf(text, sizeof(text),
                             "[G2G] TX END Data:$%02X Baud:%u Link:%llu",
                             entry.geartogear.data, baud,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_TX_ABORT:
                    snprintf(text, sizeof(text),
                             "[G2G] TX ABORT Data:$%02X Baud:%u Link:%llu",
                             entry.geartogear.data, baud,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_RX_START:
                    snprintf(text, sizeof(text),
                             "[G2G] RX START Baud:%u Bit:%u Link:%llu",
                             baud, entry.geartogear.bit_cycles,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_RX_END:
                    snprintf(text, sizeof(text),
                             "[G2G] RX END Data:$%02X Baud:%u Link:%llu",
                             entry.geartogear.data, baud,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_RX_ERROR:
                    snprintf(text, sizeof(text),
                             "[G2G] RX FRAME ERROR Shift:$%02X Baud:%u Link:%llu",
                             entry.geartogear.data, baud,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_NMI:
                    snprintf(text, sizeof(text),
                             "[G2G] NMI Parallel:%u Serial:%u Asserted:%u Armed:%u Link:%llu",
                             (entry.geartogear.flags &
                                 TRACE_GEARTOGEAR_FLAG_PARALLEL_NMI) ? 1 : 0,
                             (entry.geartogear.flags &
                                 TRACE_GEARTOGEAR_FLAG_SERIAL_NMI) ? 1 : 0,
                             (entry.geartogear.flags &
                                 TRACE_GEARTOGEAR_FLAG_NMI_ASSERTED) ? 1 : 0,
                             (entry.geartogear.flags &
                                 TRACE_GEARTOGEAR_FLAG_NINT_ARMED) ? 1 : 0,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                case TRACE_GEARTOGEAR_LOCAL_WIRE:
                case TRACE_GEARTOGEAR_REMOTE_WIRE:
                    snprintf(text, sizeof(text),
                             "[G2G] %s WIRE Drive:$%02X Level:$%02X Pins:$%02X Contention:$%02X Link:%llu",
                             entry.geartogear.event ==
                                 TRACE_GEARTOGEAR_LOCAL_WIRE ?
                                 "LOCAL" : "REMOTE MAPPED",
                             entry.geartogear.event ==
                                 TRACE_GEARTOGEAR_LOCAL_WIRE ?
                                 entry.geartogear.local_drive_mask :
                                 entry.geartogear.remote_drive_mask,
                             entry.geartogear.event ==
                                 TRACE_GEARTOGEAR_LOCAL_WIRE ?
                                 entry.geartogear.local_levels :
                                 entry.geartogear.remote_levels,
                             entry.geartogear.resolved_pins,
                             entry.geartogear.contention_mask,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
                default:
                    snprintf(text, sizeof(text),
                             "[G2G] UNKNOWN Event:%u Link:%llu",
                             entry.geartogear.event,
                             (unsigned long long)entry.geartogear.link_cycle);
                    break;
            }
            break;
        }
        case TRACE_PSG:
        {
            switch (entry.psg.event)
            {
                case TRACE_PSG_TONE:
                    snprintf(text, sizeof(text), "[PSG] TONE Ch:%u %s:$%02X Period:$%03X",
                             entry.psg.channel, entry.psg.value & 0x80 ? "LATCH" : "DATA",
                             entry.psg.value, entry.psg.period >> 4);
                    break;
                case TRACE_PSG_VOLUME:
                    if (entry.psg.attenuation == 0)
                    {
                        snprintf(text, sizeof(text), "[PSG] VOLUME Ch:%u Data:$%02X Att:$0 MAX",
                                 entry.psg.channel, entry.psg.value);
                    }
                    else if (entry.psg.attenuation == 0x0F)
                    {
                        snprintf(text, sizeof(text), "[PSG] VOLUME Ch:%u Data:$%02X Att:$F OFF",
                                 entry.psg.channel, entry.psg.value);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[PSG] VOLUME Ch:%u Data:$%02X Att:$%X -%u dB",
                                 entry.psg.channel, entry.psg.value, entry.psg.attenuation,
                                 entry.psg.attenuation * 2);
                    }
                    break;
                case TRACE_PSG_NOISE:
                {
                    static const char* rates[] = {"N/512", "N/1024", "N/2048", "TONE 3"};
                    u8 rate = (u8)(entry.psg.period & 0x03);
                    snprintf(text, sizeof(text), "[PSG] NOISE Data:$%02X Mode:%s Rate:%s Att:$%X",
                             entry.psg.value, entry.psg.period & 0x80 ? "White" : "Periodic",
                             rates[rate], entry.psg.attenuation);
                    break;
                }
                case TRACE_PSG_STEREO:
                    snprintf(text, sizeof(text), "[PSG] STEREO Data:$%02X Left:$%X Right:$%X",
                             entry.psg.value, (entry.psg.value >> 4) & 0x0F, entry.psg.value & 0x0F);
                    break;
                default:
                    snprintf(text, sizeof(text), "[PSG] UNKNOWN Event:%u Data:$%02X",
                             entry.psg.event, entry.psg.value);
                    break;
            }
            break;
        }
        case TRACE_YM2413:
            if (entry.ym2413.event == TRACE_YM2413_MIXER)
            {
                snprintf(text, sizeof(text), "[YM] MIXER Data:$%02X PSG:%s FM:%s%s",
                         entry.ym2413.value, entry.ym2413.psg_enabled ? "on" : "off",
                         entry.ym2413.fm_enabled ? "on" : "off", entry.ym2413.accepted ? "" : " IGNORED");
            }
            else if (entry.ym2413.event == TRACE_YM2413_REGISTER)
            {
                u8 reg = entry.ym2413.port == 0xF0 ? entry.ym2413.value : entry.ym2413.reg;
                char description[128];
                format_ym2413_register(reg, entry.ym2413.value, entry.ym2413.port != 0xF0,
                                       description, sizeof(description));
                if (entry.ym2413.port == 0xF0)
                {
                    snprintf(text, sizeof(text), "[YM] REGISTER SELECT R:$%02X %s%s",
                             reg, description, entry.ym2413.accepted ? "" : " IGNORED");
                }
                else
                {
                    snprintf(text, sizeof(text), "[YM] REGISTER R:$%02X <- $%02X %s%s",
                             reg, entry.ym2413.value, description,
                             entry.ym2413.accepted ? "" : " IGNORED");
                }
            }
            else
                snprintf(text, sizeof(text), "[YM] UNKNOWN Event:%u", entry.ym2413.event);
            break;
        case TRACE_MAPPER:
        {
            char banks[80];
            format_mapper_banks(entry, banks, sizeof(banks));
            switch (entry.mapper.event)
            {
                case TRACE_MAPPER_ROM:
                    snprintf(text, sizeof(text), "[MAP] %s ROM Addr:$%04X Value:$%02X Banks:%s",
                             mapper_name(entry.mapper.mapper), entry.mapper.address, entry.mapper.value, banks);
                    break;
                case TRACE_MAPPER_RAM:
                    if (entry.mapper.ram_bank >= 0)
                    {
                        snprintf(text, sizeof(text), "[MAP] %s RAM Addr:$%04X Value:$%02X %s Bank:$%03X Banks:%s",
                                 mapper_name(entry.mapper.mapper), entry.mapper.address, entry.mapper.value,
                                 entry.mapper.flags & 0x01 ? "ENABLED" : "DISABLED",
                                 (u16)entry.mapper.ram_bank, banks);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[MAP] %s RAM Addr:$%04X Value:$%02X %s Banks:%s",
                                 mapper_name(entry.mapper.mapper), entry.mapper.address, entry.mapper.value,
                                 entry.mapper.flags & 0x01 ? "ENABLED" : "DISABLED", banks);
                    }
                    break;
                case TRACE_MAPPER_CONTROL:
                    if (entry.mapper.mapper == Cartridge::CartridgeCodemastersMapper)
                    {
                        snprintf(text, sizeof(text), "[MAP] CODEMASTERS CONTROL Addr:$%04X Value:$%02X RAM:%s ROM Bank:$%03X Banks:%s",
                                 entry.mapper.address, entry.mapper.value,
                                 entry.mapper.flags & 0x01 ? "on" : "off", entry.mapper.auxiliary, banks);
                    }
                    else if (entry.mapper.mapper == Cartridge::CartridgeJanggunMapper)
                    {
                        snprintf(text, sizeof(text), "[MAP] JANGGUN CONTROL Addr:$%04X Value:$%02X Page:%u Reverse:%s Banks:%s",
                                 entry.mapper.address, entry.mapper.value, entry.mapper.auxiliary,
                                 entry.mapper.flags & 0x01 ? "on" : "off", banks);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[MAP] %s CONTROL Addr:$%04X Value:$%02X Flags:$%02X Data:$%04X Banks:%s",
                                 mapper_name(entry.mapper.mapper), entry.mapper.address, entry.mapper.value,
                                 entry.mapper.flags, entry.mapper.auxiliary, banks);
                    }
                    break;
                case TRACE_MAPPER_EEPROM:
                {
                    u8 lines = entry.mapper.flags >> 4;
                    u8 state = (entry.mapper.flags >> 2) & 0x03;
                    const char* write_state = entry.mapper.flags & 0x02 ? "locked" : "enabled";
                    if (entry.mapper.address >= 0x8008 && entry.mapper.address < 0x8088)
                    {
                        snprintf(text, sizeof(text), "[MAP] 93C46 EEPROM DIRECT[$%02X] <- $%02X State:%s Write:%s",
                                 entry.mapper.auxiliary, entry.mapper.value, eeprom_state_name(state), write_state);
                    }
                    else if (entry.mapper.address == 0x8000)
                    {
                        snprintf(text, sizeof(text), "[MAP] 93C46 EEPROM LINES Data:$%02X CS:%u CLK:%u DI:%u DO:%u State:%s Opcode:$%04X",
                                 entry.mapper.value, (lines >> 2) & 0x01, (lines >> 1) & 0x01,
                                 lines & 0x01, (lines >> 3) & 0x01, eeprom_state_name(state),
                                 entry.mapper.auxiliary);
                    }
                    else
                    {
                        snprintf(text, sizeof(text), "[MAP] 93C46 EEPROM CONTROL Addr:$%04X Value:$%02X %s Write:%s State:%s Opcode:$%04X",
                                 entry.mapper.address, entry.mapper.value,
                                 entry.mapper.flags & 0x01 ? "ENABLED" : "DISABLED", write_state,
                                 eeprom_state_name(state), entry.mapper.auxiliary);
                    }
                    break;
                }
                case TRACE_MAPPER_FLASH:
                    if (entry.mapper.flags == 0)
                    {
                        snprintf(text, sizeof(text), "[MAP] IRATA FLASH %s ID Addr:$%04X Value:$%02X",
                                 entry.mapper.address == 0x5555 && entry.mapper.value == 0x90 ? "ENTER" : "EXIT",
                                 entry.mapper.address, entry.mapper.value);
                    }
                    else if (entry.mapper.flags == 1)
                    {
                        if (entry.mapper.address == 0x5555 && entry.mapper.value == 0x80)
                        {
                            snprintf(text, sizeof(text), "[MAP] IRATA FLASH BEGIN ERASE Addr:$%04X Value:$%02X",
                                     entry.mapper.address, entry.mapper.value);
                        }
                        else
                        {
                            snprintf(text, sizeof(text), "[MAP] IRATA FLASH ERASE SECTOR Addr:$%04X Value:$%02X Bank:$%03X",
                                     entry.mapper.address, entry.mapper.value, entry.mapper.auxiliary);
                        }
                    }
                    else
                    {
                        if (entry.mapper.address == 0x5555 && entry.mapper.value == 0xA0)
                        {
                            snprintf(text, sizeof(text), "[MAP] IRATA FLASH BEGIN PROGRAM Addr:$%04X Value:$%02X",
                                     entry.mapper.address, entry.mapper.value);
                        }
                        else
                        {
                            snprintf(text, sizeof(text), "[MAP] IRATA FLASH PROGRAM Addr:$%04X Value:$%02X Bank:$%03X",
                                     entry.mapper.address, entry.mapper.value, entry.mapper.auxiliary);
                        }
                    }
                    break;
                default:
                    snprintf(text, sizeof(text), "[MAP] %s UNKNOWN Event:%u Addr:$%04X Value:$%02X",
                             mapper_name(entry.mapper.mapper), entry.mapper.event,
                             entry.mapper.address, entry.mapper.value);
                    break;
            }
            break;
        }
        default:
            snprintf(text, sizeof(text), "[???]");
            break;
    }

    snprintf(buffer, buffer_size, "%s%s", cycles, text);
}
