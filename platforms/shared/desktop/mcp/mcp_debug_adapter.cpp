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

#include "mcp_debug_adapter.h"
#include "log.h"
#include "../utils.h"
#include "../emu.h"
#include "../gui.h"
#include "../gui_actions.h"
#include "../gui_debug_disassembler.h"
#include "../gui_debug_memory.h"
#include "../gui_debug_memeditor.h"
#include "../config.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

struct DisassemblerBookmark
{
    u16 address;
    char name[32];
};

void DebugAdapter::Pause()
{
    emu_debug_break();
}

void DebugAdapter::Resume()
{
    emu_debug_continue();
}

void DebugAdapter::StepInto()
{
    emu_debug_step_into();
}

void DebugAdapter::StepOver()
{
    emu_debug_step_over();
}

void DebugAdapter::StepOut()
{
    emu_debug_step_out();
}

void DebugAdapter::StepFrame()
{
    emu_debug_step_frame();
}

void DebugAdapter::Reset()
{
    emu_reset(gui_get_force_configuration());
}

json DebugAdapter::GetDebugStatus()
{
    json result;

    if (!m_core)
    {
        result["error"] = "Core not initialized";
        return result;
    }

    bool is_paused = emu_is_debug_idle();

    result["paused"] = is_paused;

    if (is_paused)
    {
        Processor* cpu = m_core->GetProcessor();
        u16 pc = cpu->GetState()->PC->GetValue();

        bool at_breakpoint = cpu->BreakpointHit();

        result["at_breakpoint"] = at_breakpoint;

        std::ostringstream pc_ss;
        pc_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << pc;
        result["pc"] = pc_ss.str();
    }
    else
    {
        result["at_breakpoint"] = false;
    }

    return result;
}

void DebugAdapter::SetBreakpoint(u16 address, int type, bool read, bool write, bool execute)
{
    (void)address;
    (void)type;
    (void)read;
    (void)write;
    (void)execute;
/*
    Processor* cpu = m_core->GetProcessor();

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04X", address);

    if (type == HuC6280::HuC6280_BREAKPOINT_TYPE_ROMRAM && execute && !read && !write)
    {
        cpu->AddBreakpoint(address);
    }
    else
    {
        cpu->AddBreakpoint(type, buffer, read, write, execute);
    }
*/
}

void DebugAdapter::SetBreakpointRange(u16 start_address, u16 end_address, int type, bool read, bool write, bool execute)
{
    (void)start_address;
    (void)end_address;
    (void)type;
    (void)read;
    (void)write;
    (void)execute;
/*
    Processor* cpu = m_core->GetProcessor();

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04X-%04X", start_address, end_address);

    cpu->AddBreakpoint(type, buffer, read, write, execute);
*/
}

void DebugAdapter::ClearBreakpointByAddress(u16 address, int type, u16 end_address)
{
    (void)address;
    (void)type;
    (void)end_address;
/*
    Processor* cpu = m_core->GetProcessor();
    std::vector<HuC6280::GG_Breakpoint>* breakpoints = cpu->GetBreakpoints();

    for (int i = (int)breakpoints->size() - 1; i >= 0; i--)
    {
        HuC6280::GG_Breakpoint& bp = (*breakpoints)[i];

        if (bp.type != type)
            continue;

        if (end_address > 0 && end_address >= address)
        {
            if (bp.range && bp.address1 == address && bp.address2 == end_address)
                breakpoints->erase(breakpoints->begin() + i);
        }
        else
        {
            if (!bp.range && bp.address1 == address)
                breakpoints->erase(breakpoints->begin() + i);
        }
    }
*/
}

std::vector<BreakpointInfo> DebugAdapter::ListBreakpoints()
{
    std::vector<BreakpointInfo> result;
/*    Processor* cpu = m_core->GetProcessor();
    std::vector<HuC6280::GG_Breakpoint>* breakpoints = cpu->GetBreakpoints();

    for (const HuC6280::GG_Breakpoint& brk : *breakpoints)
    {
        BreakpointInfo info;
        info.enabled = brk.enabled;
        info.type = brk.type;
        info.address1 = brk.address1;
        info.address2 = brk.address2;
        info.read = brk.read;
        info.write = brk.write;
        info.execute = brk.execute;
        info.range = brk.range;
        info.type_name = GetBreakpointTypeName(brk.type);
        result.push_back(info);
    }
*/
    return result;
}

RegistersSnapshot DebugAdapter::GetRegisters()
{
/*
    Debug("[MCP] GetRegisters: start");

    Debug("[MCP] GetRegisters: m_core = %p", (void*)m_core);

    Processor* cpu = m_core->GetProcessor();
    Debug("[MCP] GetRegisters: cpu = %p", (void*)cpu);

    HuC6280::HuC6280_State* state = cpu->GetState();
    Debug("[MCP] GetRegisters: state = %p", (void*)state);
*/
    Debug("[MCP] GetRegisters: creating snapshot");
    RegistersSnapshot snapshot;
/*
    Debug("[MCP] GetRegisters: reading PC (ptr=%p)", (void*)state->PC);
    snapshot.PC = state->PC->GetValue();

    Debug("[MCP] GetRegisters: reading A (ptr=%p)", (void*)state->A);
    snapshot.A = state->A->GetValue();
    snapshot.X = state->X->GetValue();
    snapshot.Y = state->Y->GetValue();
    snapshot.S = state->S->GetValue();
    snapshot.P = state->P->GetValue();
    snapshot.SPEED = *state->SPEED;
    snapshot.TIMER = *state->TIMER;
    snapshot.TIMER_COUNTER = *state->TIMER_COUNTER;
    snapshot.TIMER_RELOAD = *state->TIMER_RELOAD;
    snapshot.IDR = *state->IDR;
    snapshot.IRR = *state->IRR;
*/
    Debug("[MCP] GetRegisters: done (PC=%04X)", snapshot.PC);
    return snapshot;
}

void DebugAdapter::SetRegister(const std::string& name, u32 value)
{
    (void)name;
    (void)value;
/*
    Processor* cpu = m_core->GetProcessor();
    HuC6280::HuC6280_State* state = cpu->GetState();

    if (name == "PC")
        state->PC->SetValue((u16)value);
    else if (name == "A")
        state->A->SetValue((u8)value);
    else if (name == "X")
        state->X->SetValue((u8)value);
    else if (name == "Y")
        state->Y->SetValue((u8)value);
    else if (name == "S")
        state->S->SetValue((u8)value);
    else if (name == "P")
        state->P->SetValue((u8)value);
*/
}

std::vector<MemoryAreaInfo> DebugAdapter::ListMemoryAreas()
{
    std::vector<MemoryAreaInfo> result;

    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        MemoryAreaInfo info = GetMemoryAreaInfo(i);
        if (info.data != NULL && info.size > 0)
        {
            result.push_back(info);
        }
    }

    return result;
}

std::vector<u8> DebugAdapter::ReadMemoryArea(int area, u32 offset, size_t size)
{
    std::vector<u8> result;
    MemoryAreaInfo info = GetMemoryAreaInfo(area);

    if (info.data == NULL || offset >= info.size)
        return result;

    u32 bytes_to_read = (u32)size;
    if (offset + bytes_to_read > info.size)
        bytes_to_read = info.size - offset;

    for (u32 i = 0; i < bytes_to_read; i++)
    {
        result.push_back(info.data[offset + i]);
    }

    return result;
}

void DebugAdapter::WriteMemoryArea(int area, u32 offset, const std::vector<u8>& data)
{
    MemoryAreaInfo info = GetMemoryAreaInfo(area);

    if (info.data == NULL || offset >= info.size)
        return;

    for (size_t i = 0; i < data.size() && (offset + i) < info.size; i++)
    {
        info.data[offset + i] = data[i];
    }
}

std::vector<DisasmLine> DebugAdapter::GetDisassembly(u16 start_address, u16 end_address, int bank, bool resolve_symbols)
{
    std::vector<DisasmLine> result;
    (void)start_address;
    (void)end_address;
    (void)bank;
    (void)resolve_symbols;
/*
    bool use_explicit_bank = (bank >= 0 && bank <= 0xFF);

    // Scan backwards from to find any instruction that might span into our range
    u16 scan_start = start_address;
    const int MAX_INSTRUCTION_SIZE = 7;

    for (int lookback = 1; lookback < MAX_INSTRUCTION_SIZE && scan_start > 0; lookback++)
    {
        u16 check_addr = start_address - lookback;

        if (use_explicit_bank)
        {
            u16 start_offset = start_address & 0x1FFF;
            if (lookback > start_offset)
                break;  // Would go past bank boundary
            check_addr = (start_address & 0xE000) | (start_offset - lookback);
        }

        Memory::stDisassembleRecord* record = NULL;

        if (use_explicit_bank)
        {
            record = memory->GetDisassemblerRecord(check_addr, (u8)bank);
        }
        else
        {
            record = memory->GetDisassemblerRecord(check_addr);
        }

        if (IsValidPointer(record) && record->name[0] != 0)
        {
            // Check if this instruction spans into our range
            u16 instr_end = check_addr + record->size - 1;
            if (instr_end >= start_address)
            {
                // This instruction overlaps with our range, start from here
                scan_start = check_addr;
                break;
            }
        }
    }

    u16 addr = scan_start;

    while (addr <= end_address)
    {
        Memory::stDisassembleRecord* record = NULL;

        if (use_explicit_bank)
            record = memory->GetDisassemblerRecord(addr, (u8)bank);
        else
            record = memory->GetDisassemblerRecord(addr);

        if (IsValidPointer(record) && record->name[0] != 0)
        {
            DisasmLine line;
            line.address = addr;
            line.bank = record->bank;
            line.name = record->name;
            strip_color_tags(line.name);
            line.bytes = record->bytes;
            line.segment = record->segment;
            line.size = record->size;
            line.jump = record->jump;
            line.jump_address = record->jump_address;
            line.jump_bank = record->jump_bank;
            line.has_operand_address = record->has_operand_address;
            line.operand_address = record->operand_address;
            line.operand_is_zp = record->operand_is_zp;
            line.subroutine = record->subroutine;
            line.irq = record->irq;

            if (resolve_symbols)
            {
                std::string instr = line.name;
                if (!gui_debug_resolve_symbol(record, instr, "", ""))
                    gui_debug_resolve_label(record, instr, "", "");
                line.name = instr;
            }

            result.push_back(line);

            // Move to next instruction
            // Handle wrap-around within the bank when explicit bank is used
            if (use_explicit_bank)
            {
                u16 offset_in_bank = addr & 0x1FFF;
                offset_in_bank += (u16)record->size;
                if (offset_in_bank >= 0x2000)
                {
                    // Reached end of bank
                    break;
                }
                addr = (start_address & 0xE000) | offset_in_bank;
            }
            else
            {
                addr = addr + (u16)record->size;
            }

            if (record->size == 0)
                addr++;
        }
        else
        {
            // No record at this address, try next byte
            addr++;
        }

        if (addr < start_address && !use_explicit_bank)
            break;
    }
*/
    return result;
}

const char* DebugAdapter::GetBreakpointTypeName(int type)
{
/*
    switch (type)
    {
        case HuC6280::HuC6280_BREAKPOINT_TYPE_ROMRAM:
            return "ROM/RAM";
        case HuC6280::HuC6280_BREAKPOINT_TYPE_VRAM:
            return "VRAM";
        case HuC6280::HuC6280_BREAKPOINT_TYPE_PALETTE_RAM:
            return "PALETTE";
        case HuC6280::HuC6280_BREAKPOINT_TYPE_HUC6270_REGISTER:
            return "6270 REG";
        case HuC6280::HuC6280_BREAKPOINT_TYPE_HUC6260_REGISTER:
            return "6260 REG";
        default:
            return "UNKNOWN";
    }
*/
    (void)type;
    return "UNKNOWN";
}

MemoryAreaInfo DebugAdapter::GetMemoryAreaInfo(int area)
{
    MemoryAreaInfo info;
    info.id = area;
    info.data = NULL;
    info.size = 0;
/*
    Memory* memory = m_core->GetMemory();
    Media* media = m_core->GetCartridge();
    HuC6260* huc6260 = m_core->GetHuC6260();
    HuC6270* huc6270_1 = m_core->GetHuC6270_1();
    HuC6270* huc6270_2 = m_core->GetHuC6270_2();
    Adpcm* adpcm = m_core->GetAdpcm();
    bool is_sgx = media->IsSGX();

    switch (area)
    {
        case MEMORY_EDITOR_RAM:
            info.name = "WRAM";
            info.data = memory->GetWorkingRAM();
            info.size = 0x2000 * (is_sgx ? 4 : 1);
            break;
        case MEMORY_EDITOR_ZERO_PAGE:
            info.name = "ZP";
            info.data = memory->GetWorkingRAM();
            info.size = 0x100;
            break;
        case MEMORY_EDITOR_ROM:
            info.name = "ROM";
            info.data = media->GetROM();
            info.size = media->GetROMSize();
            break;
        case MEMORY_EDITOR_CARD_RAM:
            info.name = "CARD RAM";
            info.data = memory->GetCardRAM();
            info.size = memory->GetCardRAMSize();
            break;
        case MEMORY_EDITOR_BACKUP_RAM:
            info.name = "BRAM";
            info.data = memory->GetBackupRAM();
            info.size = memory->IsBackupRamEnabled() ? 0x800 : 0;
            break;
        case MEMORY_EDITOR_PALETTES:
            info.name = "PALETTES";
            info.data = (u8*)huc6260->GetColorTable();
            info.size = 512;
            break;
        case MEMORY_EDITOR_VRAM_1:
            info.name = is_sgx ? "VRAM 1" : "VRAM";
            info.data = (u8*)huc6270_1->GetVRAM();
            info.size = HUC6270_VRAM_SIZE;
            break;
        case MEMORY_EDITOR_VRAM_2:
            if (is_sgx)
            {
                info.name = "VRAM 2";
                info.data = (u8*)huc6270_2->GetVRAM();
                info.size = HUC6270_VRAM_SIZE;
            }
            break;
        case MEMORY_EDITOR_SAT_1:
            info.name = is_sgx ? "SAT 1" : "SAT";
            info.data = (u8*)huc6270_1->GetSAT();
            info.size = HUC6270_SAT_SIZE;
            break;
        case MEMORY_EDITOR_SAT_2:
            if (is_sgx)
            {
                info.name = "SAT 2";
                info.data = (u8*)huc6270_2->GetSAT();
                info.size = HUC6270_SAT_SIZE;
            }
            break;
        case MEMORY_EDITOR_CDROM_RAM:
            if (media->IsCDROM())
            {
                info.name = "CDROM RAM";
                info.data = memory->GetCDROMRAM();
                info.size = memory->GetCDROMRAMSize();
            }
            break;
        case MEMORY_EDITOR_ADPCM_RAM:
            if (media->IsCDROM())
            {
                info.name = "ADPCM";
                info.data = adpcm->GetRAM();
                info.size = 0x10000;
            }
            break;
        case MEMORY_EDITOR_ARCADE_RAM:
            if (media->IsArcadeCard())
            {
                info.name = "ARCADE";
                info.data = memory->GetArcadeRAM();
                info.size = memory->GetArcadeCardRAMSize();
            }
            break;
        case MEMORY_EDITOR_MB128:
            if (m_core->GetInput()->GetMB128()->IsConnected())
            {
                info.name = "MB128";
                info.data = m_core->GetInput()->GetMB128()->GetRAM();
                info.size = 0x20000;
            }
            break;
        default:
            break;
    }
*/
    return info;
}

json DebugAdapter::GetMediaInfo()
{
    json info;
/*
    Media* media = m_core->GetCartridge();

    info["ready"] = media->IsReady();
    info["file_path"] = media->GetFilePath();
    info["file_name"] = media->GetFileName();
    info["file_directory"] = media->GetFileDirectory();
    info["file_extension"] = media->GetFileExtension();

    std::ostringstream crc_ss;
    crc_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << media->GetCRC();
    info["crc"] = crc_ss.str();

    info["is_hes"] = media->IsHES();
    info["is_sgx"] = media->IsSGX();
    info["is_cdrom"] = media->IsCDROM();
    info["is_gameexpress"] = media->IsGameExpress();
    info["is_arcade_card"] = media->IsArcadeCard();
    info["is_mb128"] = media->IsMB128();

    info["rom_size"] = media->GetROMSize();
    info["card_ram_size"] = media->GetCardRAMSize();

    GG_Console_Type console_type = media->GetConsoleType();
    switch (console_type)
    {
        case GG_CONSOLE_PCE:
            info["console_type"] = "PC Engine";
            break;
        case GG_CONSOLE_TG16:
            info["console_type"] = "TurboGrafx-16";
            break;
        case GG_CONSOLE_SGX:
            info["console_type"] = "SuperGrafx";
            break;
        default:
            info["console_type"] = "Unknown";
            break;
    }

    if (media->IsCDROM())
    {
        GG_CDROM_Type cdrom_type = media->GetCDROMType();
        switch (cdrom_type)
        {
            case GG_CDROM_STANDARD:
                info["cdrom_type"] = "CD-ROM²";
                break;
            case GG_CDROM_SUPER_CDROM:
                info["cdrom_type"] = "Super CD-ROM²";
                break;
            case GG_CDROM_ARCADE_CARD:
                info["cdrom_type"] = "Arcade CD-ROM²";
                break;
            default:
                info["cdrom_type"] = "Unknown";
                break;
        }
    }

    Media::HuCardMapper mapper = media->GetMapper();
    switch (mapper)
    {
        case Media::STANDARD_MAPPER:
            info["mapper"] = "Standard";
            break;
        case Media::SF2_MAPPER:
            info["mapper"] = "Street Fighter II";
            break;
        case Media::ARCADE_CARD_MAPPER:
            info["mapper"] = "Arcade Card";
            break;
        default:
            info["mapper"] = "Unknown";
            break;
    }

    info["loaded_bios"] = media->IsLoadedBios();
    if (media->IsLoadedBios())
    {
        info["bios_name"] = media->GetBiosName(true);
        info["valid_bios"] = media->IsValidBios(true);
    }

    info["backup_ram_forced"] = media->IsBackupRAMForced();
    info["preload_cdrom"] = media->IsPreloadCdRomEnabled();
*/
    return info;
}

json DebugAdapter::GetHuC6280Status()
{
    json status;
/*
    HuC6280* processor = m_core->GetHuC6280();
    HuC6280::HuC6280_State* proc_state = processor->GetState();
    Memory* memory = m_core->GetMemory();
    Input* input = m_core->GetInput();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    // Status register (P)
    ss << std::setw(2) << (int)proc_state->P->GetValue();
    status["P"] = ss.str();
    ss.str("");

    // Program Counter
    ss << std::setw(4) << proc_state->PC->GetValue();
    status["PC"] = ss.str();
    ss.str("");

    // Physical PC
    ss << std::setw(6) << memory->GetPhysicalAddress(proc_state->PC->GetValue());
    status["physical_PC"] = ss.str();
    ss.str("");

    // Stack Pointer
    ss << std::setw(4) << (0x2100 | proc_state->S->GetValue());
    status["SP"] = ss.str();
    ss.str("");

    // Registers
    ss << std::setw(2) << (int)proc_state->A->GetValue();
    status["A"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)proc_state->X->GetValue();
    status["X"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)proc_state->Y->GetValue();
    status["Y"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)proc_state->S->GetValue();
    status["S"] = ss.str();
    ss.str("");

    // MPR registers
    json mpr_array = json::array();
    for (int i = 0; i < 8; i++)
    {
        json mpr_info;
        ss << std::setw(2) << (int)memory->GetMpr(i);
        mpr_info["value"] = ss.str();
        ss.str("");
        mpr_info["index"] = i;
        mpr_array.push_back(mpr_info);
    }
    status["MPR"] = mpr_array;

    // I/O register
    ss << std::setw(2) << (int)input->GetIORegister();
    status["IO"] = ss.str();
    ss.str("");

    // Timer
    status["TIMER"] = (*proc_state->TIMER) ? true : false;
    ss << std::setw(2) << (int)*proc_state->TIMER_COUNTER;
    status["TIMER_COUNTER"] = ss.str();
    ss.str("");
    ss << std::setw(2) << (int)*proc_state->TIMER_RELOAD;
    status["TIMER_RELOAD"] = ss.str();
    ss.str("");

    // Interrupt registers
    ss << std::setw(2) << (int)*proc_state->IDR;
    status["IDR"] = ss.str();
    ss.str("");
    ss << std::setw(2) << (int)*proc_state->IRR;
    status["IRR"] = ss.str();
    ss.str("");

    // IRQ status
    status["IRQ1_enabled"] = (*proc_state->IDR & 0x02) == 0;
    status["IRQ1_asserted"] = (*proc_state->IRR & 0x02) != 0;
    status["IRQ2_enabled"] = (*proc_state->IDR & 0x01) == 0;
    status["IRQ2_asserted"] = (*proc_state->IRR & 0x01) != 0;
    status["TIQ_enabled"] = (*proc_state->IDR & 0x04) == 0;
    status["TIQ_asserted"] = (*proc_state->IRR & 0x04) != 0;

    // I/O status
    status["IO_SEL"] = input->GetSel();
    status["IO_CLR"] = input->GetClr();

    // Speed
    status["SPEED"] = *proc_state->SPEED ? "7.16 MHz" : "1.79 MHz";
*/
    return status;
}

json DebugAdapter::GetHuC6270Registers(int vdc)
{
    (void)vdc;
/*
    if (vdc < 1 || vdc > 2)
        return json::object();

    HuC6270* huc6270 = (vdc == 1) ? m_core->GetHuC6270_1() : m_core->GetHuC6270_2();
    HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();
*/
    json registers = json::array();
/*
    // HuC6270 has 20 valid registers (0x00-0x13), although the array has 32 slots
    for (int i = 0; i < 20; i++)
    {
        json reg;
        reg["index"] = i;

        std::ostringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << huc6270_state->R[i];
        reg["value"] = ss.str();

        registers.push_back(reg);
    }

    // Add address register
    json ar;
    ar["index"] = "AR";
    std::ostringstream ar_ss;
    ar_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << *huc6270_state->AR;
    ar["value"] = ar_ss.str();
    registers.push_back(ar);

    // Add status register (read-only)
    json sr;
    sr["index"] = "SR";
    std::ostringstream sr_ss;
    sr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << *huc6270_state->SR;
    sr["value"] = sr_ss.str();
    registers.push_back(sr);
*/
    return registers;
}

json DebugAdapter::WriteHuC6270Register(int vdc, int reg, u16 value)
{
    json result;
    (void)vdc;
    (void)reg;
    (void)value;
/*
    if (vdc < 1 || vdc > 2)
    {
        result["error"] = "Invalid VDC number (must be 1 or 2)";
        return result;
    }

    if (reg < 0 || reg > 20)
    {
        result["error"] = "Invalid register number (must be 0-19 or 20 for AR)";
        return result;
    }

    HuC6270* huc6270 = (vdc == 1) ? m_core->GetHuC6270_1() : m_core->GetHuC6270_2();

    if (!huc6270)
    {
        result["error"] = "VDC not available";
        return result;
    }

    HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();

    if (reg == 20)
    {
        // Write to Address Register
        *huc6270_state->AR = value;
        result["register"] = "AR";
    }
    else
    {
        // Write to data registers (0-19)
        huc6270_state->R[reg] = value;
        result["register"] = reg;
    }

    result["success"] = true;
    result["vdc"] = vdc;

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << value;
    result["value"] = ss.str();
*/
    return result;
}

json DebugAdapter::GetHuC6270Status(int vdc)
{
    if (vdc < 1 || vdc > 2)
        return json::object();

    json status;
/*
    HuC6260* huc6260 = m_core->GetHuC6260();
    HuC6270* huc6270 = (vdc == 1) ? m_core->GetHuC6270_1() : m_core->GetHuC6270_2();
    HuC6270::HuC6270_State* huc6270_state = huc6270->GetState();

    std::ostringstream ss;

    // Speed
    const char* speed_names[] = { "5.36 MHz", "7.16 MHz", "10.8 MHz" };
    status["speed"] = speed_names[huc6260->GetSpeed()];

    // Position
    status["x"] = *huc6270_state->HPOS;
    status["y"] = *huc6270_state->VPOS;

    // States
    const char* h_states[] = { "HDS", "HDW", "HDE", "HSW" };
    const char* v_states[] = { "VDS", "VDW", "VCR", "VSW" };
    status["h_state"] = h_states[*huc6270_state->H_STATE];
    status["v_state"] = v_states[*huc6270_state->V_STATE];

    // Control register
    status["background_enabled"] = (huc6270_state->R[HUC6270_REG_CR] & 0x0080) != 0;
    status["sprites_enabled"] = (huc6270_state->R[HUC6270_REG_CR] & 0x0040) != 0;

    const char* disp_output[] = { "DISP", "~BURST", "~INTHSYNC", "INVALID" };
    status["display_output"] = disp_output[(huc6270_state->R[HUC6270_REG_CR] >> 8) & 0x03];

    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
       << (int)k_huc6270_read_write_increment[(huc6270_state->R[HUC6270_REG_CR] >> 11) & 0x03];
    status["rw_increment"] = ss.str();
    ss.str("");

    // Interrupt requests
    status["int_collision"] = (huc6270_state->R[HUC6270_REG_CR] & HUC6270_CONTROL_COLLISION) != 0;
    status["int_overflow"] = (huc6270_state->R[HUC6270_REG_CR] & HUC6270_CONTROL_OVERFLOW) != 0;
    status["int_scanline"] = (huc6270_state->R[HUC6270_REG_CR] & HUC6270_CONTROL_SCANLINE) != 0;
    status["int_vblank"] = (huc6270_state->R[HUC6270_REG_CR] & HUC6270_CONTROL_VBLANK) != 0;

    // Status register
    status["collision_detected"] = (*huc6270_state->SR & HUC6270_STATUS_COLLISION) != 0;
    status["overflow_detected"] = (*huc6270_state->SR & HUC6270_STATUS_OVERFLOW) != 0;
    status["scanline_interrupt"] = (*huc6270_state->SR & HUC6270_STATUS_SCANLINE) != 0;
    status["vblank_active"] = (*huc6270_state->SR & HUC6270_STATUS_VBLANK) != 0;
    status["vram_dma_end"] = (*huc6270_state->SR & HUC6270_STATUS_VRAM_END) != 0;
    status["sat_dma_end"] = (*huc6270_state->SR & HUC6270_STATUS_SAT_END) != 0;
    status["busy"] = (*huc6270_state->SR & HUC6270_STATUS_BUSY) != 0;
*/
    return status;
}

json DebugAdapter::GetHuC6260Status()
{
    json status;
/*
    HuC6260* huc6260 = m_core->GetHuC6260();
    HuC6260::HuC6260_State* huc6260_state = huc6260->GetState();

    // Speed
    const char* speed_names[] = { "5.36 MHz", "7.16 MHz", "10.8 MHz" };
    status["speed"] = speed_names[huc6260->GetSpeed()];

    // Position
    status["x"] = *huc6260_state->HPOS;
    status["y"] = *huc6260_state->VPOS;

    // Lines
    status["lines"] = (*huc6260_state->CR & 0x04) ? HUC6270_LINES : (HUC6270_LINES - 1);

    // Sync signals
    status["hsync"] = *huc6260_state->HSYNC ? "HIGH" : "LOW";
    status["vsync"] = *huc6260_state->VSYNC ? "HIGH" : "LOW";

    // Control register
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)*huc6260_state->CR;
    status["control_reg"] = ss.str();
    ss.str("");

    // CTA register
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << *huc6260_state->CTA;
    status["cta"] = ss.str();
    ss.str("");

    // Blur and B&W
    status["blur"] = (*huc6260_state->CR & 0x04) != 0;
    status["black_white"] = (*huc6260_state->CR & 0x80) != 0;
*/
    return status;
}

json DebugAdapter::GetHuC6202Status()
{
/*
    if (!m_core->GetCartridge()->IsSGX())
        return json::object();
*/

    json status;
/*
    HuC6202* huc6202 = m_core->GetHuC6202();
    HuC6202::HuC6202_State* huc6202_state = huc6202->GetState();

    status["selected_vdc"] = *huc6202_state->VDC2_SELECTED ? 2 : 1;
    status["window_1"] = *huc6202_state->WINDOW_1;
    status["window_2"] = *huc6202_state->WINDOW_2;

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    ss << std::setw(2) << (int)*huc6202_state->PRIORITY_1;
    status["priority_1"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*huc6202_state->PRIORITY_2;
    status["priority_2"] = ss.str();
    ss.str("");

    status["irq_vdc1"] = *huc6202_state->IRQ1_1;
    status["irq_vdc2"] = *huc6202_state->IRQ1_2;

    // Window priority regions
    const char* window_names[] = { "none", "window_1", "window_2", "both" };
    json window_priority = json::array();
    for (int i = 0; i < 4; i++)
    {
        json region;
        region["region"] = window_names[i];
        region["vdc1_enabled"] = huc6202_state->WINDOW_PRIORITY[i].vdc_1_enabled;
        region["vdc2_enabled"] = huc6202_state->WINDOW_PRIORITY[i].vdc_2_enabled;
        region["priority_mode"] = huc6202_state->WINDOW_PRIORITY[i].priority_mode;
        window_priority.push_back(region);
    }
    status["window_priority"] = window_priority;
*/
    return status;
}

json DebugAdapter::GetPSGStatus()
{
    json status;
/*
    HuC6280PSG* psg = m_core->GetAudio()->GetPSG();
    HuC6280PSG::HuC6280PSG_State* psg_state = psg->GetState();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    status["channel_select"] = *psg_state->CHANNEL_SELECT;

    ss << std::setw(2) << (int)*psg_state->MAIN_AMPLITUDE;
    status["main_amplitude"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*psg_state->LFO_FREQUENCY;
    status["lfo_frequency"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*psg_state->LFO_CONTROL;
    status["lfo_control"] = ss.str();
    ss.str("");

    // Channel info
    json channels = json::array();
    for (int i = 0; i < 6; i++)
    {
        json channel;
        HuC6280PSG::HuC6280PSG_Channel* psg_channel = &psg_state->CHANNELS[i];

        channel["index"] = i;
        channel["mute"] = psg_channel->mute;
        channel["enabled"] = psg_channel->enabled;
        channel["frequency"] = psg_channel->frequency;
        channel["dda"] = psg_channel->dda_enabled;
        channel["noise"] = psg_channel->noise_enabled;

        ss << std::setw(2) << (int)psg_channel->vol_left;
        channel["vol_left"] = ss.str();
        ss.str("");

        ss << std::setw(2) << (int)psg_channel->vol_right;
        channel["vol_right"] = ss.str();
        ss.str("");

        ss << std::setw(2) << (int)psg_channel->amplitude;
        channel["amplitude"] = ss.str();
        ss.str("");

        channel["wave_index"] = psg_channel->wave_index;

        channels.push_back(channel);
    }
    status["channels"] = channels;
*/
    return status;
}

json DebugAdapter::GetCDROMStatus()
{
/*
    if (!m_core->GetCartridge()->IsCDROM())
        return json::object();
*/
    json status;
/*
    CdRom* cdrom = m_core->GetCDROM();
    CdRomMedia* cdrom_media = m_core->GetCDROMMedia();
    ScsiController* scsi_controller = m_core->GetScsiController();
    CdRom::CdRom_State* cdrom_state = cdrom->GetState();
    ScsiController::Scsi_State* scsi_state = scsi_controller->GetState();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    // CD-ROM state
    status["reset"] = *cdrom_state->RESET;
    status["bram_enabled"] = *cdrom_state->BRAM_ENABLED;

    // IRQs
    ss << std::setw(2) << (int)*cdrom_state->ENABLED_IRQS;
    status["enabled_irqs"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*cdrom_state->ACTIVE_IRQS;
    status["active_irqs"] = ss.str();
    ss.str("");

    status["irq_adpcm_half"] = (*cdrom_state->ACTIVE_IRQS & CDROM_IRQ_ADPCM_HALF) != 0;
    status["irq_adpcm_end"] = (*cdrom_state->ACTIVE_IRQS & CDROM_IRQ_ADPCM_END) != 0;
    status["irq_status"] = (*cdrom_state->ACTIVE_IRQS & CDROM_IRQ_STATUS_AND_MSG_IN) != 0;
    status["irq_data"] = (*cdrom_state->ACTIVE_IRQS & CDROM_IRQ_DATA_IN) != 0;

    // SCSI
    status["scsi_phase"] = k_scsi_phase_names[*scsi_state->PHASE];

    ss << std::setw(2) << (int)*scsi_state->DB;
    status["scsi_data_bus"] = ss.str();
    ss.str("");

    // SCSI signals
    status["scsi_bsy"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_BSY) != 0;
    status["scsi_sel"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_SEL) != 0;
    status["scsi_cd"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_CD) != 0;
    status["scsi_io"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_IO) != 0;
    status["scsi_msg"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_MSG) != 0;
    status["scsi_req"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_REQ) != 0;
    status["scsi_ack"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_ACK) != 0;
    status["scsi_atn"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_ATN) != 0;
    status["scsi_rst"] = (*scsi_state->SIGNALS & ScsiController::SCSI_SIGNAL_RST) != 0;

    // Events
    status["next_event"] = k_scsi_event_names[*scsi_state->NEXT_EVENT];
    status["cycles_to_event"] = *scsi_state->NEXT_EVENT_CYCLES;
    status["next_sector_load"] = *scsi_state->LOAD_SECTOR;
    status["cycles_to_load"] = *scsi_state->NEXT_LOAD_CYCLES;
    status["sectors_left"] = *scsi_state->LOAD_SECTOR_COUNT;

    ss << std::setw(2) << (int)*cdrom_state->FADER;
    status["fader"] = ss.str();
    ss.str("");

    // Media info
    status["media_type"] = cdrom_media->GetFileExtension();
    status["tracks"] = cdrom_media->GetTrackCount();

    GG_CdRomMSF length = cdrom_media->GetCdRomLength();
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", length.minutes, length.seconds, length.frames);
    status["length"] = time_str;
    status["sector_count"] = cdrom_media->GetSectorCount();
*/
    return status;
}

json DebugAdapter::GetArcadeCardStatus()
{
/*
    if (!m_core->GetCartridge()->IsArcadeCard())
        return json::object();

    json status;
    Memory* memory = m_core->GetMemory();
    ArcadeCardMapper* arcade_card_mapper = memory->GetArcadeCardMapper();
    ArcadeCardMapper::ArcadeCard_State* arcade_card_state = arcade_card_mapper->GetState();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    // Main registers
    ss << std::setw(8) << *arcade_card_state->REGISTER;
    status["register"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*arcade_card_state->SHIFT_AMOUNT;
    status["shift_amount"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*arcade_card_state->ROTATE_AMOUNT;
    status["rotate_amount"] = ss.str();
    ss.str("");

    // Ports
    json ports = json::array();
    for (int i = 0; i < 4; i++)
    {
        json port;
        ArcadeCardMapper::ArcadeCard_Port* port_data = &arcade_card_state->PORTS[i];

        port["index"] = i;

        ss << std::setw(8) << port_data->base;
        port["base_address"] = ss.str();
        ss.str("");

        ss << std::setw(4) << port_data->offset;
        port["offset"] = ss.str();
        ss.str("");

        ss << std::setw(4) << port_data->increment;
        port["increment"] = ss.str();
        ss.str("");

        ss << std::setw(2) << (int)port_data->control;
        port["control"] = ss.str();
        ss.str("");

        port["add_offset"] = port_data->add_offset;
        port["auto_increment"] = port_data->auto_increment;
        port["signed_offset"] = port_data->signed_offset;
        port["increment_base"] = port_data->increment_base;

        const char* trigger_names[] = { "NONE", "LOW_BYTE", "HIGH_BYTE", "REG_0A" };
        port["offset_trigger"] = trigger_names[port_data->offset_trigger];

        ports.push_back(port);
    }
    status["ports"] = ports;

    return status;
*/
    json status;
    return status;
}

json DebugAdapter::GetCDROMAudioStatus()
{
/*
    if (!m_core->GetCartridge()->IsCDROM())
        return json::object();

    json status;
    CdRomAudio* cdrom_audio = m_core->GetCDROMAudio();
    CdRomAudio::CdRomAudio_State* cdrom_audio_state = cdrom_audio->GetState();

    // State
    const char* state_names[] = { "PLAYING", "IDLE", "PAUSED", "STOPPED" };
    status["state"] = state_names[*cdrom_audio_state->CURRENT_STATE];

    // Stop event
    const char* stop_event_names[] = { "STOP", "LOOP", "IRQ" };
    status["stop_event"] = stop_event_names[*cdrom_audio_state->STOP_EVENT];

    // LBA positions
    status["start_lba"] = *cdrom_audio_state->START_LBA;
    status["stop_lba"] = *cdrom_audio_state->STOP_LBA;
    status["current_lba"] = *cdrom_audio_state->CURRENT_LBA;

    // Convert current LBA to MSF for display
    GG_CdRomMSF current_msf;
    LbaToMsf(*cdrom_audio_state->CURRENT_LBA, &current_msf);
    char pos_str[16];
    snprintf(pos_str, sizeof(pos_str), "%02d:%02d:%02d", current_msf.minutes, current_msf.seconds, current_msf.frames);
    status["current_position_msf"] = pos_str;

    // Seek info
    status["seek_cycles"] = *cdrom_audio_state->SEEK_CYCLES;

    // Sample info
    status["frame_samples"] = *cdrom_audio_state->FRAME_SAMPLES;

    return status;
*/
    json status;
    return status;
}

json DebugAdapter::GetADPCMStatus()
{
/*
    if (!m_core->GetCartridge()->IsCDROM())
        return json::object();

    json status;
    Adpcm* adpcm = m_core->GetAdpcm();
    Adpcm::Adpcm_State* adpcm_state = adpcm->GetState();

    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    // State
    status["playing"] = *adpcm_state->PLAYING;

    ss << std::setw(4) << *adpcm_state->READ_ADDRESS;
    status["read_address"] = ss.str();
    ss.str("");

    ss << std::setw(4) << *adpcm_state->WRITE_ADDRESS;
    status["write_address"] = ss.str();
    ss.str("");

    status["length"] = *adpcm_state->LENGTH;

    float frequency = (32000.0f / (16.0f - (float)*adpcm_state->SAMPLE_RATE)) / 1000.0f;
    status["frequency_khz"] = frequency;

    // Registers
    u8 status_reg = adpcm->Read(0x0C);
    ss << std::setw(2) << (int)status_reg;
    status["status_register"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*adpcm_state->CONTROL;
    status["control"] = ss.str();
    ss.str("");

    ss << std::setw(2) << (int)*adpcm_state->DMA;
    status["dma"] = ss.str();
    ss.str("");

    ss << std::setw(4) << *adpcm_state->ADDRESS;
    status["address"] = ss.str();
    ss.str("");

    // Sample info
    status["frame_samples"] = *adpcm_state->FRAME_SAMPLES;

    return status;
*/
    json status;
    return status;
}

// Base64 encoding table
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const unsigned char* data, int size)
{
    std::string result;
    result.reserve(((size + 2) / 3) * 4);

    int i = 0;
    while (i < size)
    {
        unsigned char byte1 = data[i++];
        unsigned char byte2 = (i < size) ? data[i++] : 0;
        unsigned char byte3 = (i < size) ? data[i++] : 0;

        result.push_back(base64_chars[byte1 >> 2]);
        result.push_back(base64_chars[((byte1 & 0x03) << 4) | (byte2 >> 4)]);
        result.push_back((i > size + 1) ? '=' : base64_chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)]);
        result.push_back((i > size) ? '=' : base64_chars[byte3 & 0x3F]);
    }

    return result;
}

json DebugAdapter::GetScreenshot()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    // Get runtime info for screen dimensions
    GS_RuntimeInfo runtime;
    m_core->GetRuntimeInfo(runtime);

    // Get PNG screenshot from emu
    unsigned char* png_buffer = NULL;
    int png_size = emu_get_screenshot_png(&png_buffer);

    if (png_size == 0 || !png_buffer)
    {
        result["error"] = "Failed to capture screenshot";
        return result;
    }

    // Encode PNG data to base64
    std::string base64_png = base64_encode(png_buffer, png_size);

    // Free the buffer allocated by stbi_write_png_to_mem (uses malloc internally)
    free(png_buffer);

    result["__mcp_image"] = true;
    result["data"] = base64_png;
    result["mimeType"] = "image/png";
    result["width"] = runtime.screen_width;
    result["height"] = runtime.screen_height;

    return result;
}

json DebugAdapter::LoadMedia(const std::string& file_path)
{
    json result;

    if (file_path.empty())
    {
        result["error"] = "File path is required";
        Log("[MCP] LoadMedia failed: File path is required");
        return result;
    }

    emu_load_media_async(file_path.c_str(), gui_get_force_configuration());

    int timeout_ms = 180000;
    int elapsed_ms = 0;
    while (emu_is_media_loading() && elapsed_ms < timeout_ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        elapsed_ms += 500;
    }

    if (emu_is_media_loading())
    {
        result["error"] = "Loading timed out";
        Log("[MCP] LoadMedia timed out: %s", file_path.c_str());
        return result;
    }

    if (!emu_finish_media_loading() || !m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "Failed to load media file";
        Log("[MCP] LoadMedia failed: %s", file_path.c_str());
        return result;
    }

    result["success"] = true;
    result["file_path"] = file_path;
    result["rom_name"] = m_core->GetCartridge()->GetFileName();
/*
    result["is_cdrom"] = m_core->GetCartridge()->IsCDROM();
    result["is_sgx"] = m_core->GetCartridge()->IsSGX();
*/

    return result;
}

json DebugAdapter::LoadSymbols(const std::string& file_path)
{
    json result;

    if (file_path.empty())
    {
        result["error"] = "File path is required";
        Log("[MCP] LoadSymbols failed: File path is required");
        return result;
    }

    gui_debug_load_symbols_file(file_path.c_str());

    result["success"] = true;
    result["file_path"] = file_path;

    return result;
}

json DebugAdapter::ListSaveStateSlots()
{
    json result;
    json slots = json::array();

    for (int i = 0; i < 5; i++)
    {
        json slot;
        slot["slot"] = i + 1;
        slot["selected"] = (config_emulator.save_slot == i);

/*
        if (emu_savestates[i].rom_name[0] != 0)
        {
            slot["rom_name"] = emu_savestates[i].rom_name;
            slot["timestamp"] = emu_savestates[i].timestamp;
            slot["version"] = emu_savestates[i].version;
            slot["valid"] = (emu_savestates[i].version == GS_SAVESTATE_VERSION);
            slot["has_screenshot"] = IsValidPointer(emu_savestates_screenshots[i].data);

            if (emu_savestates[i].emu_build[0] != 0)
                slot["emu_build"] = emu_savestates[i].emu_build;
        }
        else
        {
            slot["empty"] = true;
        }
*/
        slot["empty"] = true;

        slots.push_back(slot);
    }

    result["slots"] = slots;
    result["current_slot"] = config_emulator.save_slot + 1;

    return result;
}

json DebugAdapter::SelectSaveStateSlot(int slot)
{
    json result;

    if (slot < 1 || slot > 5)
    {
        result["error"] = "Invalid slot number (must be 1-5)";
        Log("[MCP] SelectSaveStateSlot failed: Invalid slot %d", slot);
        return result;
    }

    config_emulator.save_slot = slot - 1;

    result["success"] = true;
    result["slot"] = slot;

    return result;
}

json DebugAdapter::SaveState()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        Log("[MCP] SaveState failed: No media loaded");
        return result;
    }

    int slot = config_emulator.save_slot + 1;
    emu_save_state_slot(slot);

    result["success"] = true;
    result["slot"] = slot;
    result["rom_name"] = m_core->GetCartridge()->GetFileName();

    return result;
}

json DebugAdapter::LoadState()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        Log("[MCP] LoadState failed: No media loaded");
        return result;
    }

    int slot = config_emulator.save_slot + 1;

/*
    if (emu_savestates[config_emulator.save_slot].rom_name[0] == 0)
    {
        result["error"] = "Save state slot is empty";
        Log("[MCP] LoadState failed: Slot %d is empty", slot);
        return result;
    }
*/

    emu_load_state_slot(slot);

    result["success"] = true;
    result["slot"] = slot;

    return result;
}

json DebugAdapter::SetFastForwardSpeed(int speed)
{
    json result;

    if (speed < 0 || speed > 4)
    {
        result["error"] = "Invalid speed (must be 0-4: 0=1.5x, 1=2x, 2=2.5x, 3=3x, 4=Unlimited)";
        Log("[MCP] SetFastForwardSpeed failed: Invalid speed %d", speed);
        return result;
    }

    config_emulator.ffwd_speed = speed;

    result["success"] = true;
    result["speed"] = speed;
    
    const char* speed_names[] = {"1.5x", "2x", "2.5x", "3x", "Unlimited"};
    result["speed_name"] = speed_names[speed];

    return result;
}

json DebugAdapter::ToggleFastForward(bool enabled)
{
    json result;

    config_emulator.ffwd = enabled;
    gui_action_ffwd();

    result["success"] = true;
    result["enabled"] = enabled;
    result["speed"] = config_emulator.ffwd_speed;

    return result;
}

json DebugAdapter::ControllerButton(int player, const std::string& button, const std::string& action)
{
/*
    json result;

    // Validate action
    if (action != "press" && action != "release" && action != "press_and_release")
    {
        result["error"] = "Invalid action (must be: press, release, press_and_release)";
        return result;
    }

    // Convert player 1-5 to GG_Controllers enum (0-4)
    if (player < 1 || player > 5)
    {
        result["error"] = "Invalid player number (must be 1-5)";
        return result;
    }
    GG_Controllers controller = static_cast<GG_Controllers>(player - 1);

    std::string button_lower = button;
    std::transform(button_lower.begin(), button_lower.end(), button_lower.begin(), ::tolower);

    GG_Keys key = GG_KEY_NONE;
    if (button_lower == "i") key = GG_KEY_I;
    else if (button_lower == "ii") key = GG_KEY_II;
    else if (button_lower == "select") key = GG_KEY_SELECT;
    else if (button_lower == "run") key = GG_KEY_RUN;
    else if (button_lower == "up") key = GG_KEY_UP;
    else if (button_lower == "right") key = GG_KEY_RIGHT;
    else if (button_lower == "down") key = GG_KEY_DOWN;
    else if (button_lower == "left") key = GG_KEY_LEFT;
    else if (button_lower == "iii") key = GG_KEY_III;
    else if (button_lower == "iv") key = GG_KEY_IV;
    else if (button_lower == "v") key = GG_KEY_V;
    else if (button_lower == "vi") key = GG_KEY_VI;
    else
    {
        result["error"] = "Invalid button name";
        return result;
    }

    if (action == "press")
    {
        emu_key_pressed(controller, key);
    }
    else if (action == "release")
    {
        emu_key_released(controller, key);
    }
    else if (action == "press_and_release")
    {
        emu_key_pressed(controller, key);
        // Mark for delayed release - McpManager will handle releasing after some frames
        result["__delayed_release"] = true;
    }

    result["success"] = true;
    result["player"] = player;
    result["button"] = button;
    result["action"] = action;

    return result;
*/
    json result;
    (void)player;
    (void)button;
    (void)action;
    result["error"] = "Not available during Gearlynx migration";
    return result;
}

json DebugAdapter::ControllerSetType(int player, const std::string& type)
{
/*
    json result;

    // Convert player 1-5 to GG_Controllers enum (0-4)
    if (player < 1 || player > 5)
    {
        result["error"] = "Invalid player number (must be 1-5)";
        return result;
    }
    GG_Controllers controller = static_cast<GG_Controllers>(player - 1);

    // Convert type string to GG_Controller_Type enum
    GG_Controller_Type controller_type;
    if (type == "standard")
        controller_type = GG_CONTROLLER_STANDARD;
    else if (type == "avenue_pad_3")
        controller_type = GG_CONTROLLER_AVENUE_PAD_3;
    else if (type == "avenue_pad_6")
        controller_type = GG_CONTROLLER_AVENUE_PAD_6;
    else
    {
        result["error"] = "Invalid controller type (must be: standard, avenue_pad_3, avenue_pad_6)";
        return result;
    }

    emu_set_pad_type(controller, controller_type);

    result["success"] = true;
    result["player"] = player;
    result["type"] = type;

    return result;
*/
    json result;
    (void)player;
    (void)type;
    result["error"] = "Not available during Gearlynx migration";
    return result;
}

json DebugAdapter::ControllerSetTurboTap(bool enabled)
{
/*
    json result;

    emu_set_turbo_tap(enabled);

    result["success"] = true;
    result["enabled"] = enabled;

    return result;
*/
    json result;
    (void)enabled;
    result["error"] = "Not available during Gearlynx migration";
    return result;
}

json DebugAdapter::ControllerGetType(int player)
{
/*
    json result;

    // Convert player 1-5 to GG_Controllers enum (0-4)
    if (player < 1 || player > 5)
    {
        result["error"] = "Invalid player number (must be 1-5)";
        return result;
    }
    GG_Controllers controller = static_cast<GG_Controllers>(player - 1);

    GG_Controller_Type controller_type = emu_get_pad_type(controller);

    std::string type_name;
    switch (controller_type)
    {
        case GG_CONTROLLER_STANDARD:
            type_name = "standard";
            break;
        case GG_CONTROLLER_AVENUE_PAD_3:
            type_name = "avenue_pad_3";
            break;
        case GG_CONTROLLER_AVENUE_PAD_6:
            type_name = "avenue_pad_6";
            break;
        default:
            type_name = "unknown";
            break;
    }

    result["success"] = true;
    result["player"] = player;
    result["type"] = type_name;

    return result;
*/
    json result;
    (void)player;
    result["error"] = "Not available during Gearlynx migration";
    return result;
}

json DebugAdapter::ListSprites(int vdc)
{
/*
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (vdc < 1 || vdc > 2)
    {
        result["error"] = "Invalid VDC number (must be 1 or 2)";
        return result;
    }

    HuC6270* huc6270 = (vdc == 1) ? m_core->GetHuC6270_1() : m_core->GetHuC6270_2();
    u16* sat = huc6270->GetSAT();

    json sprites = json::array();

    for (int s = 0; s < 64; s++)
    {
        u16 sprite_y_raw = sat[s * 4] & 0x03FF;
        u16 sprite_x = sat[(s * 4) + 1] & 0x03FF;
        u16 pattern = (sat[(s * 4) + 2] >> 1) & 0x03FF;
        u16 sprite_flags = sat[(s * 4) + 3] & 0xB98F;

        int width_index = (sprite_flags >> 8) & 0x01;
        int height_index = (sprite_flags >> 12) & 0x03;
        int width = k_huc6270_sprite_width[width_index];
        int height = k_huc6270_sprite_height[height_index];

        bool h_flip = (sprite_flags & 0x0800) != 0;
        bool v_flip = (sprite_flags & 0x8000) != 0;
        int palette = sprite_flags & 0x0F;
        bool priority = (sprite_flags & 0x0080) != 0;

        // Apply same Y adjustment as GUI (+3 for screen positioning)
        int sprite_y = sprite_y_raw + 3;

        // Format values to match GUI display
        std::ostringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');

        ss << std::setw(3) << sprite_x;
        std::string x_hex = ss.str();
        ss.str("");

        ss << std::setw(3) << sprite_y;
        std::string y_hex = ss.str();
        ss.str("");

        ss << std::setw(3) << pattern;
        std::string pattern_hex = ss.str();
        ss.str("");

        ss << std::setw(4) << (pattern << 6);
        std::string vram_hex = ss.str();
        ss.str("");

        ss << std::setw(1) << palette;
        std::string palette_hex = ss.str();

        json sprite_info;
        sprite_info["sat_entry"] = s;
        sprite_info["sprite_x"] = x_hex + " (" + std::to_string(sprite_x) + ")";
        sprite_info["sprite_y"] = y_hex + " (" + std::to_string(sprite_y) + ")";
        sprite_info["size"] = std::to_string(width) + "x" + std::to_string(height);
        sprite_info["pattern"] = pattern_hex + " (" + std::to_string(pattern) + ")";
        sprite_info["vram_address"] = "$" + vram_hex;
        sprite_info["palette"] = palette_hex + " (" + std::to_string(palette) + ")";
        sprite_info["h_flip"] = h_flip ? "YES" : "NO";
        sprite_info["v_flip"] = v_flip ? "YES" : "NO";
        sprite_info["priority"] = priority ? "YES" : "NO";

        sprites.push_back(sprite_info);
    }

    result["sprites"] = sprites;
    result["vdc"] = vdc;

    return result;
*/
    json result;
    (void)vdc;
    result["error"] = "Not available during Gearlynx migration";
    return result;
}

json DebugAdapter::GetSpriteImage(int sprite_index, int vdc)
{
/*
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (vdc < 1 || vdc > 2)
    {
        result["error"] = "Invalid VDC number (must be 1 or 2)";
        return result;
    }

    if (sprite_index < 0 || sprite_index > 63)
    {
        result["error"] = "Invalid sprite index (must be 0-63)";
        return result;
    }

    int vdc_idx = vdc - 1;

    // Get PNG sprite from emu
    unsigned char* png_buffer = NULL;
    int png_size = emu_get_sprite_png(vdc_idx, sprite_index, &png_buffer);

    if (png_size == 0 || !png_buffer)
    {
        result["error"] = "Failed to capture sprite";
        return result;
    }

    int width = emu_debug_sprite_widths[vdc_idx][sprite_index];
    int height = emu_debug_sprite_heights[vdc_idx][sprite_index];

    // Encode PNG data to base64
    std::string base64_png = base64_encode(png_buffer, png_size);

    // Free the buffer allocated by stbi_write_png_to_mem
    free(png_buffer);

    result["__mcp_image"] = true;
    result["data"] = base64_png;
    result["mimeType"] = "image/png";
    result["width"] = width;
    result["height"] = height;
    result["sprite_index"] = sprite_index;
    result["vdc"] = vdc;

    return result;
*/
    json result;
    (void)sprite_index;
    (void)vdc;
    result["error"] = "Not available during Gearlynx migration";
    return result;
}

// Disassembler operations

json DebugAdapter::RunToAddress(u16 address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_runto_address(address);

    result["success"] = true;
    result["address"] = address;
    result["message"] = "Running to address";

    return result;
}

json DebugAdapter::AddDisassemblerBookmark(u16 address, const std::string& name)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_add_disassembler_bookmark(address, name.c_str());

    result["success"] = true;
    result["address"] = address;
    result["name"] = name.empty() ? "auto-generated" : name;

    return result;
}

json DebugAdapter::RemoveDisassemblerBookmark(u16 address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_remove_disassembler_bookmark(address);

    result["success"] = true;
    result["address"] = address;

    return result;
}

json DebugAdapter::AddSymbol(u8 bank, u16 address, const std::string& name)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    char symbol[128];
    snprintf(symbol, sizeof(symbol), "%02X:%04X %s", bank, address, name.c_str());
    gui_debug_add_symbol(symbol);

    result["success"] = true;
    result["bank"] = bank;
    result["address"] = address;
    result["name"] = name;

    return result;
}

json DebugAdapter::RemoveSymbol(u8 bank, u16 address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    gui_debug_remove_symbol(bank, address);

    result["success"] = true;
    result["bank"] = bank;
    result["address"] = address;

    return result;
}

// Memory editor operations

json DebugAdapter::SelectMemoryRange(int editor, int start_address, int end_address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number (must be 0-13)";
        return result;
    }

    gui_debug_memory_select_range(editor, start_address, end_address);

    result["success"] = true;
    result["editor"] = editor;
    result["start_address"] = start_address;
    result["end_address"] = end_address;

    return result;
}

json DebugAdapter::SetMemorySelectionValue(int editor, u8 value)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number (must be 0-13)";
        return result;
    }

    gui_debug_memory_set_selection_value(editor, value);

    result["success"] = true;
    result["editor"] = editor;
    result["value"] = value;

    return result;
}

json DebugAdapter::AddMemoryBookmark(int editor, int address, const std::string& name)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number (must be 0-13)";
        return result;
    }

    gui_debug_memory_add_bookmark(editor, address, name.c_str());

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;
    result["name"] = name.empty() ? "auto-generated" : name;

    return result;
}

json DebugAdapter::RemoveMemoryBookmark(int editor, int address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number (must be 0-13)";
        return result;
    }

    gui_debug_memory_remove_bookmark(editor, address);

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;

    return result;
}

json DebugAdapter::AddMemoryWatch(int editor, int address, const std::string& notes, int size)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number (must be 0-13)";
        return result;
    }

    gui_debug_memory_add_watch(editor, address, notes.c_str(), size);

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;
    result["notes"] = notes;
    result["size"] = size;

    return result;
}

json DebugAdapter::RemoveMemoryWatch(int editor, int address)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (editor < 0 || editor >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid editor number (must be 0-13)";
        return result;
    }

    gui_debug_memory_remove_watch(editor, address);

    result["success"] = true;
    result["editor"] = editor;
    result["address"] = address;

    return result;
}

json DebugAdapter::ListDisassemblerBookmarks()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    void* bookmarks_ptr = NULL;
    int count = gui_debug_get_disassembler_bookmarks(&bookmarks_ptr);

    std::vector<DisassemblerBookmark>* bookmarks = (std::vector<DisassemblerBookmark>*)bookmarks_ptr;

    json bookmarks_array = json::array();

    if (bookmarks)
    {
        for (const DisassemblerBookmark& bookmark : *bookmarks)
        {
            json bookmark_obj;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << bookmark.address;
            bookmark_obj["address"] = addr_ss.str();
            bookmark_obj["name"] = bookmark.name;

            bookmarks_array.push_back(bookmark_obj);
        }
    }

    result["bookmarks"] = bookmarks_array;
    result["count"] = count;

    return result;
}

json DebugAdapter::ListSymbols()
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    void* symbols_ptr = NULL;
    gui_debug_get_symbols(&symbols_ptr);

    DebugSymbol*** fixed_symbols = (DebugSymbol***)symbols_ptr;

    json symbols_array = json::array();

    if (fixed_symbols)
    {
        for (int bank = 0; bank < 0x100; bank++)
        {
            if (!fixed_symbols[bank])
                continue;

            for (int address = 0; address < 0x10000; address++)
            {
                if (fixed_symbols[bank][address])
                {
                    json symbol_obj;

                    std::ostringstream bank_ss, addr_ss;
                    bank_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << bank;
                    addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << address;

                    symbol_obj["bank"] = bank_ss.str();
                    symbol_obj["address"] = addr_ss.str();
                    symbol_obj["name"] = fixed_symbols[bank][address]->text;

                    symbols_array.push_back(symbol_obj);
                }
            }
        }
    }

    result["symbols"] = symbols_array;
    result["count"] = symbols_array.size();

    return result;
}

json DebugAdapter::ListCallStack()
{
/*
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    Memory* memory = m_core->GetMemory();
    HuC6280* processor = m_core->GetHuC6280();
    std::stack<HuC6280::GG_CallStackEntry> temp_stack = *processor->GetDisassemblerCallStack();

    void* symbols_ptr = NULL;
    gui_debug_get_symbols(&symbols_ptr);
    DebugSymbol*** fixed_symbols = (DebugSymbol***)symbols_ptr;

    json stack_array = json::array();

    while (!temp_stack.empty())
    {
        HuC6280::GG_CallStackEntry entry = temp_stack.top();
        temp_stack.pop();

        json entry_obj;

        // Format addresses as hex strings
        std::ostringstream dest_ss, src_ss, back_ss;
        dest_ss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << entry.dest;
        src_ss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << entry.src;
        back_ss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << entry.back;

        entry_obj["function"] = dest_ss.str();
        entry_obj["source"] = src_ss.str();
        entry_obj["return"] = back_ss.str();

        // Try to find symbol for destination address
        Memory::stDisassembleRecord* record = memory->GetDisassemblerRecord(entry.dest);
        if (IsValidPointer(record) && record->name[0] != 0)
        {
            if (fixed_symbols && fixed_symbols[record->bank] && fixed_symbols[record->bank][entry.dest])
            {
                entry_obj["symbol"] = fixed_symbols[record->bank][entry.dest]->text;
            }
        }

        stack_array.push_back(entry_obj);
    }

    result["stack"] = stack_array;
    result["depth"] = stack_array.size();

    return result;
*/
    json result;
    result["stack"] = json::array();
    result["depth"] = 0;
    return result;
}

json DebugAdapter::ListMemoryBookmarks(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    void* bookmarks_ptr = NULL;
    int count = gui_debug_memory_get_bookmarks(area, &bookmarks_ptr);

    std::vector<MemEditor::Bookmark>* bookmarks = (std::vector<MemEditor::Bookmark>*)bookmarks_ptr;

    json bookmarks_array = json::array();

    if (bookmarks)
    {
        for (const MemEditor::Bookmark& bookmark : *bookmarks)
        {
            json bookmark_obj;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << bookmark.address;
            bookmark_obj["address"] = addr_ss.str();
            bookmark_obj["name"] = bookmark.name;

            bookmarks_array.push_back(bookmark_obj);
        }
    }

    result["area"] = area;
    result["bookmarks"] = bookmarks_array;
    result["count"] = count;

    return result;
}

json DebugAdapter::ListMemoryWatches(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    void* watches_ptr = NULL;
    int count = gui_debug_memory_get_watches(area, &watches_ptr);

    std::vector<MemEditor::Watch>* watches = (std::vector<MemEditor::Watch>*)watches_ptr;

    json watches_array = json::array();

    if (watches)
    {
        const char* size_names[] = {"8", "16", "24", "32"};
        const char* format_names[] = {"hex", "binary", "decimal_unsigned", "decimal_signed"};

        for (const MemEditor::Watch& watch : *watches)
        {
            json watch_obj;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << watch.address;
            watch_obj["address"] = addr_ss.str();
            watch_obj["notes"] = watch.notes;

            int size_idx = (watch.size >= 0 && watch.size <= 3) ? watch.size : 0;
            int fmt_idx = (watch.format >= 0 && watch.format <= 3) ? watch.format : 0;
            watch_obj["size"] = size_names[size_idx];
            watch_obj["format"] = format_names[fmt_idx];

            watches_array.push_back(watch_obj);
        }
    }

    result["area"] = area;
    result["watches"] = watches_array;
    result["count"] = count;

    return result;
}

json DebugAdapter::GetMemorySelection(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number";
        return result;
    }

    int start = -1;
    int end = -1;
    gui_debug_memory_get_selection(area, &start, &end);

    result["area"] = area;

    if (start >= 0 && end >= 0 && start <= end)
    {
        std::ostringstream start_ss, end_ss;
        start_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << start;
        end_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << end;

        result["start"] = start_ss.str();
        result["end"] = end_ss.str();
        result["size"] = end - start + 1;
    }
    else
    {
        result["start"] = NULL;
        result["end"] = NULL;
        result["size"] = 0;
        result["note"] = "No selection";
    }

    return result;
}

json DebugAdapter::MemorySearchCapture(int area)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number (must be 0-13)";
        return result;
    }

    gui_debug_memory_search_capture(area);

    result["success"] = true;
    result["area"] = area;
    result["message"] = "Memory snapshot captured";

    return result;
}

json DebugAdapter::MemorySearch(int area, const std::string& op, const std::string& compare_type, int compare_value, const std::string& data_type)
{
    json result;

    if (!m_core || !m_core->GetCartridge()->IsReady())
    {
        result["error"] = "No media loaded";
        return result;
    }

    if (area < 0 || area >= MEMORY_EDITOR_MAX)
    {
        result["error"] = "Invalid area number (must be 0-13)";
        return result;
    }

    int op_index = 0;
    if (op == "<") op_index = 0;
    else if (op == ">") op_index = 1;
    else if (op == "==") op_index = 2;
    else if (op == "!=") op_index = 3;
    else if (op == "<=") op_index = 4;
    else if (op == ">=") op_index = 5;
    else
    {
        result["error"] = "Invalid operator";
        return result;
    }

    int compare_type_index = 0;
    if (compare_type == "previous") compare_type_index = 0;
    else if (compare_type == "value") compare_type_index = 1;
    else if (compare_type == "address") compare_type_index = 2;
    else
    {
        result["error"] = "Invalid compare_type";
        return result;
    }

    int data_type_index = 0;
    if (data_type == "hex") data_type_index = 0;
    else if (data_type == "signed") data_type_index = 1;
    else if (data_type == "unsigned") data_type_index = 2;
    else
    {
        result["error"] = "Invalid data_type";
        return result;
    }

    void* results_ptr = NULL;
    int count = gui_debug_memory_search(area, op_index, compare_type_index, compare_value, data_type_index, &results_ptr);

    result["area"] = area;
    result["count"] = count;
    result["results"] = json::array();

    if (count > 0 && results_ptr != NULL)
    {
        std::vector<MemEditor::Search>* results = (std::vector<MemEditor::Search>*)results_ptr;

        int max_results = (count > 1000) ? 1000 : count;

        for (int i = 0; i < max_results; i++)
        {
            MemEditor::Search& search = (*results)[i];
            json item;

            std::ostringstream addr_ss;
            addr_ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << search.address;

            item["address"] = addr_ss.str();
            item["value"] = search.value;
            item["previous"] = search.prev_value;

            result["results"].push_back(item);
        }

        if (count > 1000)
        {
            result["note"] = "Results limited to first 1000 matches";
            result["total_matches"] = count;
        }
    }

    return result;
}
