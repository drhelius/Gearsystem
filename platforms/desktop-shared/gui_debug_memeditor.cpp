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
#include <stdexcept>
#include <algorithm>
#include <SDL.h>
#include "gui_debug_memeditor.h"
#include "gui_debug_constants.h"
#include "../../src/gearsystem.h"

MemEditor::MemEditor()
{
    m_title[0] = 0;
    m_separator_column_width = 8.0f;
    m_selection_start = 0;
    m_selection_end = 0;
    m_bytes_per_row = 16;
    m_row_scroll_top = 0;
    m_row_scroll_bottom = 0;
    m_editing_address = -1;
    m_set_keyboard_here = false;
    m_uppercase_hex = true;
    m_gray_out_zeros = true;
    m_preview_data_type = 0;
    m_preview_endianess = 0;
    m_jump_to_address = -1;
    InitPointer(m_mem_data);
    m_mem_size = 0;
    m_mem_base_addr = 0;
    m_hex_addr_format[0] = 0;
    m_hex_addr_digits = 2;
    m_mem_word = 1;
    m_goto_address[0] = 0;
    m_find_next[0] = 0;
    m_add_bookmark = false;
    m_watch_window = false;
    m_add_watch = false;
    InitPointer(m_gui_font);
    InitPointer(m_draw_list);
    m_search_window = false;
    m_search_operator = 0;
    m_search_compare_type = 0;
    m_search_data_type = 0;
    m_search_compare_specific_value_str[0] = 0;
    m_search_compare_specific_value = 0;
    m_search_compare_specific_address_str[0] = 0;
    m_search_compare_specific_address = 0;
    InitPointer(m_search_data);
    m_search_auto = false;
}

MemEditor::~MemEditor()
{
    SafeDeleteArray(m_search_data);
}

void MemEditor::Reset(const char* title, uint8_t* mem_data, int mem_size, int base_display_addr, int word)
{
    if (!IsValidPointer(mem_data) || (mem_size <= 0))
        return;

    snprintf(m_title, sizeof(m_title), "%s", title);
    m_mem_data = mem_data;
    m_mem_size = mem_size;
    m_mem_base_addr = base_display_addr;
    m_mem_word = word;

    if (m_mem_word < 1)
        m_mem_word = 1;
    else if (m_mem_word > 2)
        m_mem_word = 2;

    m_hex_addr_digits = 1;
    int size = m_mem_base_addr + m_mem_size - 1;

    while (size >>= 4)
    {
        m_hex_addr_digits++;
    }

    snprintf(m_hex_addr_format, 8, "%%0%dX", m_hex_addr_digits);

    SafeDeleteArray(m_search_data);
    m_search_data = new uint8_t[m_mem_size * m_mem_word];
    memcpy(m_search_data, m_mem_data, m_mem_size * m_mem_word);
}

void MemEditor::Draw(bool ascii, bool preview, bool options, bool cursors)
{
    if ((m_mem_word > 1) && ((m_preview_data_type < 2) || (m_preview_data_type > 3)))
        m_preview_data_type = 2;

    ImVec4 addr_color = cyan;
    ImVec4 ascii_color = magenta;
    ImVec4 column_color = yellow;
    ImVec4 normal_color = white;
    ImVec4 highlight_color = orange;
    ImVec4 gray_color = mid_gray;

    int total_rows = (m_mem_size + (m_bytes_per_row - 1)) / m_bytes_per_row;
    int separator_count = (m_bytes_per_row - 1) / 4;
    int byte_column_count = 2 + m_bytes_per_row + separator_count + 2;
    int byte_cell_padding = 0;
    int ascii_padding = 4;
    int character_cell_padding = 0;
    int max_chars_per_cell = 2 * m_mem_word;
    ImVec2 character_size = ImGui::CalcTextSize("0");
    float footer_height = 0;

    if (options)
        footer_height += ImGui::GetFrameHeightWithSpacing();
    if (preview)
        footer_height += ((character_size.y + 4) * 3) + 4;
    if (cursors)
        footer_height += ImGui::GetFrameHeightWithSpacing();

    char buf[32];

    if (ImGui::BeginChild("##mem", ImVec2(ImGui::GetContentRegionAvail().x, -footer_height), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav))
    {
        m_draw_list = ImGui::GetWindowDrawList();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.5, 0));

        if (ImGui::BeginTable("##header", byte_column_count, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible))
        {
            char addr_spaces[32];
            int addr_padding = m_hex_addr_digits - 2;
            snprintf(addr_spaces, 32, "ADDR %*s", addr_padding, "");
            ImGui::TableSetupColumn(addr_spaces);
            ImGui::TableSetupColumn("");

            for (int i = 0; i < m_bytes_per_row; i++) {
                if (IsColumnSeparator(i, m_bytes_per_row))
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_separator_column_width);

                snprintf(buf, 32, "%02X", i);

                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x * max_chars_per_cell + (6 + byte_cell_padding) * 1);
            }

            if ((m_mem_word == 1) && ascii)
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, character_size.x * ascii_padding);
                ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, (character_size.x + character_cell_padding * 1) * m_bytes_per_row);
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored(addr_color, "%s", ImGui::TableGetColumnName(0));

            for (int i = 1; i < (ImGui::TableGetColumnCount() - 1); i++) {
                ImGui::TableNextColumn();
                ImGui::TextColored(column_color, "%s", ImGui::TableGetColumnName(i));
            }

            if ((m_mem_word == 1) && ascii)
            {
                ImGui::TableNextColumn();
                ImGui::TextColored(ascii_color, "%s", ImGui::TableGetColumnName(ImGui::TableGetColumnCount() - 1));
            }

            ImGui::EndTable();
        }

        if (ImGui::BeginTable("##hex", byte_column_count, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            m_row_scroll_top = (int)(ImGui::GetScrollY() / character_size.y);
            m_row_scroll_bottom = m_row_scroll_top + (int)(ImGui::GetWindowHeight() / character_size.y);

            ImGui::TableSetupColumn("ADDR");
            ImGui::TableSetupColumn("");

            for (int i = 0; i < m_bytes_per_row; i++) {
                if (IsColumnSeparator(i, m_bytes_per_row))
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_separator_column_width);

                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x * max_chars_per_cell + (6 + byte_cell_padding) * 1);
            }

            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, character_size.x * ascii_padding);
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, (character_size.x + character_cell_padding * 1) * m_bytes_per_row);

            ImGuiListClipper clipper;
            clipper.Begin(total_rows);

            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    ImGui::TableNextRow();
                    int address = (row * m_bytes_per_row);

                    ImGui::TableNextColumn();
                    char single_addr[32];
                    snprintf(single_addr, 32, "%s:  ", m_hex_addr_format);
                    ImGui::Text(single_addr, address + m_mem_base_addr);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.75f, 0.0f));
                    for (int x = 0; x < m_bytes_per_row; x++)
                    {
                        int byte_address = address + x;

                        ImGui::TableNextColumn();
                        if (IsColumnSeparator(x, m_bytes_per_row))
                            ImGui::TableNextColumn();

                        ImVec2 cell_start_pos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().CellPadding;
                        ImVec2 cell_size = (character_size * ImVec2((float)max_chars_per_cell, 1)) + (ImVec2(2, 2) * ImGui::GetStyle().CellPadding) + ImVec2((float)(1 + byte_cell_padding), 0);

                        ImVec2 hover_cell_size = cell_size;

                        if (IsColumnSeparator(x + 1, m_bytes_per_row))
                        {
                            hover_cell_size.x += m_separator_column_width + 1;
                        }

                        bool cell_hovered = ImGui::IsMouseHoveringRect(cell_start_pos, cell_start_pos + hover_cell_size, false) && ImGui::IsWindowHovered();

                        DrawSelectionBackground(x, byte_address, cell_start_pos, cell_size);

                        if (cell_hovered)
                        {
                            HandleSelection(byte_address, row);
                        }

                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                        if (m_editing_address == byte_address)
                        {
                            ImGui::PushItemWidth((character_size).x * (2 * m_mem_word));

                            if (m_mem_word == 1)
                                snprintf(buf, 32, "%02X", m_mem_data[byte_address]);
                            else if (m_mem_word == 2)
                            {
                                uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                                snprintf(buf, 32, "%04X", mem_data_16[byte_address]);
                            }

                            if (m_set_keyboard_here)
                            {
                                ImGui::SetKeyboardFocusHere();
                                m_set_keyboard_here = false;
                            }

                            ImGui::PushStyleColor(ImGuiCol_Text, yellow);
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, dark_cyan);

                            if (ImGui::InputText("##editing_input", buf, (m_mem_word == 1) ? 3 : 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AlwaysOverwrite))
                            {
                                u16 value = 0;
                                if (parse_hex_string(buf, strlen(buf), &value))
                                {
                                    if (m_mem_word == 1)
                                        m_mem_data[byte_address] = (uint8_t)value;
                                    else if (m_mem_word == 2)
                                    {
                                        uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                                        mem_data_16[byte_address] = value;
                                    }

                                    if (byte_address < (m_mem_size - 1))
                                    {
                                        m_editing_address = byte_address + 1;
                                        m_selection_end = m_selection_start = m_editing_address;
                                        m_set_keyboard_here = true;
                                    }
                                    else
                                        m_editing_address = -1;
                                }
                                else
                                {
                                    m_editing_address = -1;
                                }
                            }

                            ImGui::PopStyleColor();
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            ImGui::PushItemWidth((character_size).x);

                            uint16_t data = 0;

                            if (m_mem_word == 1)
                                data = m_mem_data[byte_address];
                            else if (m_mem_word == 2)
                            {
                                uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                                data = mem_data_16[byte_address];
                            }

                            bool gray_out = m_gray_out_zeros && (data== 0);
                            bool highlight = (byte_address >= m_selection_start && byte_address < (m_selection_start + (DataPreviewSize() / m_mem_word)));

                            ImVec4 color = highlight ? highlight_color : (gray_out ? gray_color : normal_color);
                            if (m_mem_word == 1)
                                ImGui::TextColored(color, m_uppercase_hex ? "%02X" : "%02x", data);
                            else if (m_mem_word == 2)
                                ImGui::TextColored(color, m_uppercase_hex ? "%04X" : "%04x", data);

                            if (cell_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            {
                                m_editing_address = byte_address;
                                m_set_keyboard_here = true;
                            }

                            DrawContexMenu(byte_address, cell_hovered, options);
                        }

                        ImGui::PopItemWidth();
                        ImGui::PopStyleVar();

                        DrawSelectionFrame(x, row, byte_address, cell_start_pos, cell_size);
                    }

                    ImGui::PopStyleVar();

                    if ((m_mem_word == 1) && ascii)
                    {
                        ImGui::TableNextColumn();
                        float column_x = ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2.0f);
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        ImVec2 window_pos = ImGui::GetWindowPos();
                        draw_list->AddLine(ImVec2(window_pos.x + column_x, window_pos.y), ImVec2(window_pos.x + column_x, window_pos.y + 9999), ImGui::GetColorU32(dark_magenta));

                        ImGui::TableNextColumn();

                        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                        if (ImGui::BeginTable("##ascii_column", m_bytes_per_row))
                        {
                            for (int x = 0; x < m_bytes_per_row; x++)
                            {
                                snprintf(buf, 32, "##ascii_cell%d", x);
                                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x + character_cell_padding * 1);
                            }

                            ImGui::TableNextRow();

                            for (int x = 0; x < m_bytes_per_row; x++)
                            {
                                ImGui::TableNextColumn();

                                int byte_address = address + x;
                                ImVec2 cell_start_pos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().CellPadding;
                                ImVec2 cell_size = (character_size * ImVec2(1, 1)) + (ImVec2(2, 2) * ImGui::GetStyle().CellPadding) + ImVec2((float)(1 + byte_cell_padding), 0);

                                DrawSelectionAsciiBackground(byte_address, cell_start_pos, cell_size);

                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (character_cell_padding * 1) / 2);
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                                ImGui::PushItemWidth(character_size.x);

                                unsigned char c = m_mem_data[byte_address];

                                bool gray_out = m_gray_out_zeros && (c < 32 || c >= 128);
                                ImGui::TextColored(gray_out ? gray_color : normal_color, "%c", (c >= 32 && c < 128) ? c : '.');

                                ImGui::PopItemWidth();
                                ImGui::PopStyleVar();
                            }

                            ImGui::EndTable();
                        }
                        ImGui::PopStyleVar();
                    }
                }
            }

            if (m_jump_to_address >= 0 && m_jump_to_address < m_mem_size)
            {
                ImGui::SetScrollY((m_jump_to_address / m_bytes_per_row) * character_size.y);
                m_selection_start = m_selection_end = m_jump_to_address;
                m_jump_to_address = -1;
            }

            ImGui::EndTable();

        }

        ImGui::PopStyleVar();

    }
    ImGui::EndChild();

    if (cursors)
        DrawCursors();
    if (preview)
        DrawDataPreview(m_selection_start);
    if (options)
        DrawOptions();
}

void MemEditor::DrawWatchWindow()
{
    if (m_watch_window)
        WatchWindow();
}

void MemEditor::DrawSearchWindow()
{
    if (m_search_window)
        SearchWindow();
}

bool MemEditor::IsColumnSeparator(int current_column, int column_count)
{
    return (current_column > 0) && (current_column < column_count) && ((current_column % 4) == 0);
}

void MemEditor::DrawSelectionBackground(int x, int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    ImVec4 background_color = dark_cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;

    if (address < start || address > end)
        return;

    if (IsColumnSeparator(x + 1, m_bytes_per_row) && (address != end))
    {
        cell_size.x += m_separator_column_width + 1;
    }

    m_draw_list->AddRectFilled(cell_pos, cell_pos + cell_size + ImVec2(1, 0), ImColor(background_color));
}

void MemEditor::DrawSelectionAsciiBackground(int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec4 background_color = dark_cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;

    if (address < start || address > end)
        return;
    drawList->AddRectFilled(cell_pos, cell_pos + cell_size, ImColor(background_color));
}

void MemEditor::DrawSelectionFrame(int x, int y, int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    m_draw_list->Flags = ImDrawListFlags_None;
    ImVec4 frame_color = cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;
    int lines = (end / m_bytes_per_row) - (start / m_bytes_per_row) + 1;
    bool multiline = lines > 1;
    int start_x = start % m_bytes_per_row;
    int end_x = end % m_bytes_per_row;

    if (address < start || address > end)
        return;

    if (IsColumnSeparator(x + 1, m_bytes_per_row) && (address != end))
    {
        cell_size.x += m_separator_column_width + 1;
    }

    if ((x == 0) || (address == start))
        m_draw_list->AddLine(cell_pos + ImVec2(-1.0f, -1.0f), cell_pos + ImVec2(-1.0f, cell_size.y), ImColor(frame_color), 1.0f);

    if ((x == (m_bytes_per_row - 1)) || (address == end))
        m_draw_list->AddLine(cell_pos + ImVec2(cell_size.x, multiline && (address == end) && (x != (m_bytes_per_row - 1)) ? 0.0f : -1.0f), cell_pos + ImVec2(cell_size.x, cell_size.y), ImColor(frame_color), 1.0f);

    if ((y == 0) || ((address - m_bytes_per_row) < start))
        m_draw_list->AddLine(cell_pos + ImVec2(-1.0f, -1.0f), cell_pos + ImVec2(cell_size.x, -1.0f), ImColor(frame_color), 1.0f);

    if ((address + m_bytes_per_row) > end)
        m_draw_list->AddLine(cell_pos + ImVec2(-1.0f, cell_size.y), cell_pos + ImVec2(cell_size.x, cell_size.y), ImColor(frame_color), 1.0f);

    if ((address == end) && (x != (m_bytes_per_row - 1)) && ((lines > 2) || (lines > 1 && (end_x >= start_x))))
         m_draw_list->AddLine(cell_pos + ImVec2(cell_size.x, 0.0f), cell_pos + ImVec2(cell_size.x + cell_size.x, 0.0f), ImColor(frame_color), 1.0f);
}

void MemEditor::HandleSelection(int address, int row)
{
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        m_selection_end = address;

        if (m_selection_start != m_selection_end)
        {
            if (row > (m_row_scroll_bottom - 3))
            {
                ImGui::SetScrollY(ImGui::GetScrollY() + 5);
            }
            else if (row < (m_row_scroll_top + 4))
            {
                ImGui::SetScrollY(ImGui::GetScrollY() - 5);
            }
        }
    }
    else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        m_selection_start = address;
        m_selection_end = address;
    }
    else if (m_selection_start > m_selection_end)
    {
        int tmp = m_selection_start;
        m_selection_start = m_selection_end;
        m_selection_end = tmp;
    }

    if (m_editing_address != m_selection_start)
    {
        m_editing_address = -1;
    }
}

void MemEditor::DrawCursors()
{
    ImGui::PushItemWidth(55);
    char buf[32];
    snprintf(buf, 32, m_hex_addr_format, 0);
    ImVec2 character_size = ImGui::CalcTextSize("0");

    ImGui::PushItemWidth((character_size.x * (strlen(buf) + 1)) + 2);
    if (ImGui::InputTextWithHint("##gotoaddr", buf, m_goto_address, m_hex_addr_digits + 1, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
    {
        u32 address_value = 0;
        if (parse_hex_string(m_goto_address, strlen(m_goto_address), &address_value))
        {
            JumpToAddress((int)address_value);
        }
        m_goto_address[0] = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("GoTo"))
    {
        u32 address_value = 0;
        if (parse_hex_string(m_goto_address, strlen(m_goto_address), &address_value))
        {
            JumpToAddress((int)address_value);
        }
        m_goto_address[0] = 0;
    }

    ImGui::SameLine();
    ImGui::TextColored(dark_gray, "|");
    ImGui::SameLine();

    ImGui::PushItemWidth((m_mem_word == 1 ? character_size.x * 3 : character_size.x * 5) + 2);
    if (ImGui::InputTextWithHint("##findnext", m_mem_word == 1 ? "00" : "0000", m_find_next, m_mem_word == 1 ? 3 : 5, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
    {
        u32 find_value = 0;
        if (parse_hex_string(m_find_next, strlen(m_find_next), &find_value))
        {
            FindNextValue((int)find_value);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Find Next"))
    {
        u32 find_value = 0;
        if (parse_hex_string(m_find_next, strlen(m_find_next), &find_value))
        {
            FindNextValue((int)find_value);
        }
    }

    ImGui::SameLine();

    char range_addr[32];
    char region_text[32];
    char single_addr[32];
    char selection_text[32];
    char all_text[128];
    snprintf(range_addr, 32, "%s-%s", m_hex_addr_format, m_hex_addr_format);
    snprintf(region_text, 32, range_addr, m_mem_base_addr, m_mem_base_addr + m_mem_size - 1);
    snprintf(single_addr, 32, "%s", m_hex_addr_format);
    if (m_selection_start == m_selection_end)
        snprintf(selection_text, 32, single_addr, m_mem_base_addr + m_selection_start);
    else
        snprintf(selection_text, 32, range_addr, m_mem_base_addr + m_selection_start, m_mem_base_addr + m_selection_end);
    snprintf(all_text, 128, "REGION: %s SELECTION: %s", region_text, selection_text);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(all_text).x 
    - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);

    ImVec4 color = ImVec4(0.1f,0.9f,0.9f,1.0f);

    ImGui::TextColored(color, "REGION:");
    ImGui::SameLine();
    ImGui::Text("%s", region_text);
    ImGui::SameLine();
    ImGui::TextColored(color, " SELECTION:");
    ImGui::SameLine();
    ImGui::Text("%s", selection_text);
}

void MemEditor::DrawOptions()
{
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("context");

    if (ImGui::BeginPopup("context"))
    {
        ImGui::Text("Columns:   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::SliderInt("##columns", &m_bytes_per_row, 4, 32);
        ImGui::Text("Preview as:");
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::Combo("##preview_type", &m_preview_data_type, "Uint8\0Int8\0Uint16\0Int16\0UInt32\0Int32\0\0");
        ImGui::Text("Preview as:");
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::Combo("##preview_endianess", &m_preview_endianess, "Little Endian\0Big Endian\0\0");
        ImGui::Checkbox("Uppercase hex", &m_uppercase_hex);
        ImGui::Checkbox("Gray out zeros", &m_gray_out_zeros);

        ImGui::EndPopup();
    }
}

void MemEditor::DrawDataPreview(int address)
{
    ImGui::Separator();

    if (address < 0 || address >= m_mem_size)
        return;

    int data = 0;
    int data_size = DataPreviewSize();
    int final_address = address * m_mem_word;

    for (int i = 0; i < data_size; i++)
    {
        if (m_preview_endianess == 0)
            data |= m_mem_data[final_address + i] << (i * 8);
        else
            data |= m_mem_data[final_address + data_size - i - 1] << (i * 8);
    }

    ImVec4 color = orange;

    ImGui::TextColored(color, "Dec:");
    ImGui::SameLine();
    if (final_address + data_size <= (m_mem_size * m_mem_word))
        DrawDataPreviewAsDec(data);
    else
        ImGui::Text(" ");

    ImGui::TextColored(color, "Hex:");
    ImGui::SameLine();
    if (final_address + data_size <= (m_mem_size * m_mem_word))
        DrawDataPreviewAsHex(data);
    else
        ImGui::Text(" ");

    ImGui::TextColored(color, "Bin:");
    ImGui::SameLine();
    if (final_address + data_size <= (m_mem_size * m_mem_word))
        DrawDataPreviewAsBin(data);
    else
        ImGui::Text(" ");
}

void MemEditor::DrawDataPreviewAsHex(int data)
{
    int data_size = DataPreviewSize();
    const char* format = ((data_size == 1) ? "%02X" : (data_size == 2 ? "%04X" : "%08X"));

    ImGui::Text(format, data);
}

void MemEditor::DrawDataPreviewAsDec(int data)
{
    switch (m_preview_data_type)
    {
        case 0:
        {
            ImGui::Text("%u (Uint8)", (uint8_t)data);
            break;
        }
        case 1:
        {
            ImGui::Text("%d (Int8)", (int8_t)data);
            break;
        }
        case 2:
        {
            ImGui::Text("%u (Uint16)", (uint16_t)data);
            break;
        }
        case 3:
        {
            ImGui::Text("%d (Int16)", (int16_t)data);
            break;
        }
        case 4:
        {
            ImGui::Text("%u (Uint32)", (uint32_t)data);
            break;
        }
        case 5:
        {
            ImGui::Text("%d (Int32)", (int32_t)data);
            break;
        }
    }
}
void MemEditor::DrawDataPreviewAsBin(int data)
{
    int data_size = DataPreviewSize();

    std::string bin = "";
    for (int i = 0; i < data_size * 8; i++)
    {
        if ((i % 4) == 0 && i > 0)
            bin = " " + bin;
        bin = ((data >> i) & 1 ? "1" : "0") + bin;
    }

    ImGui::Text("%s", bin.c_str());
}

int MemEditor::DataPreviewSize()
{
    switch (m_preview_data_type)
    {
        case 0:
        case 1:
            return 1;
        case 2:
        case 3:
            return 2;
        case 4:
        case 5:
            return 4;
        default:
            return 1;
    }
}

void MemEditor::DrawContexMenu(int address, bool cell_hovered, bool options)
{
    char id[16];
    snprintf(id, 16, "##context_%d", address);

    if (cell_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        ImGui::OpenPopup(id);

    if (ImGui::BeginPopup(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
    {
        PushGuiFont();

        if ((address < m_selection_start) || (address > m_selection_end))
            m_selection_start = m_selection_end = address;

        if (ImGui::Selectable("Copy"))
        {
            Copy();
        }

        if (ImGui::Selectable("Copy As Decimal"))
        {
            Copy(true);
        }

        if (ImGui::Selectable("Paste"))
        {
            Paste();
        }

        if (ImGui::Selectable("Select All"))
        {
            SelectAll();
        }

        if (options)
        {
            if (ImGui::Selectable("Add Bookmark..."))
            {
                m_add_bookmark = true;
            }

            if (ImGui::Selectable("Add Watch..."))
            {
                m_add_watch = true;
            }
        }

        PopGuiFont();

        ImGui::EndPopup();
    }
}

void MemEditor::BookMarkPopup()
{
    char popup_title[64];
    snprintf(popup_title, 64, "Add %s Bookmark", m_title);

    if (m_add_bookmark)
    {
        ImGui::OpenPopup(popup_title);
        m_add_bookmark = false;
    }

    PushGuiFont();

    if (ImGui::BeginPopupModal(popup_title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char address[9] = "";
        static char name[32] = "";
        int initial_address = m_selection_start + m_mem_base_addr;

        if (address[0] == 0 && initial_address >= 0)
            snprintf(address, 9, m_hex_addr_format, initial_address);

        ImGui::Text("Name:");
        ImGui::PushItemWidth(200);
        ImGui::SetItemDefaultFocus();
        ImGui::InputText("##name", name, IM_ARRAYSIZE(name));

        ImGui::Text("Address:");

        char buf[32];
        snprintf(buf, 32, m_hex_addr_format, m_mem_base_addr);
        ImVec2 character_size = ImGui::CalcTextSize("0");

        ImGui::PushItemWidth(character_size.x * (strlen(buf) + 1));
        ImGui::InputTextWithHint("##bookaddr", buf, address, m_hex_addr_digits + 1, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(90, 0)))
        {
            u32 bookmark_address_value = 0;
            if (parse_hex_string(address, strlen(address), &bookmark_address_value))
            {
                int bookmark_address = (int)bookmark_address_value;

                if (strlen(name) == 0)
                {
                    snprintf(name, 32, "Bookmark_%06X", bookmark_address);
                }

                if (bookmark_address >= m_mem_base_addr && bookmark_address < (m_mem_base_addr + m_mem_size))
                {
                    Bookmark bookmark;
                    bookmark.address = bookmark_address;
                    snprintf(bookmark.name, 32, "%s", name);
                    m_bookmarks.push_back(bookmark);
                }

                ImGui::CloseCurrentPopup();
                address[0] = 0;
                name[0] = 0;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0)))
        {
            ImGui::CloseCurrentPopup();
            address[0] = 0;
            name[0] = 0;
        }
        ImGui::EndPopup();
    }

    PopGuiFont();
}

void MemEditor::WatchPopup()
{
    char popup_title[64];
    snprintf(popup_title, 64, "Add %s Watch", m_title);

    if (m_add_watch)
    {
        ImGui::OpenPopup(popup_title);
        m_add_watch = false;
    }

    PushGuiFont();

    if (ImGui::BeginPopupModal(popup_title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char address[9] = "";
        static char notes[128] = "";
        int initial_address = m_selection_start + m_mem_base_addr;

        if (address[0] == 0 && initial_address >= 0)
            snprintf(address, 9, m_hex_addr_format, initial_address);

        ImGui::Text("Address:");

        char buf[32];
        snprintf(buf, 32, m_hex_addr_format, m_mem_base_addr);
        ImVec2 character_size = ImGui::CalcTextSize("0");

        ImGui::PushItemWidth(character_size.x * (strlen(buf) + 1));
        ImGui::SetItemDefaultFocus();

        ImGui::InputTextWithHint("##bookaddr", buf, address, m_hex_addr_digits + 1, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

        ImGui::Text("Description:");
        ImGui::PushItemWidth(200);
        ImGui::InputText("##name", notes, IM_ARRAYSIZE(notes));

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(90, 0)))
        {
            u32 watch_address_value = 0;
            if (parse_hex_string(address, strlen(address), &watch_address_value))
            {
                int watch_address = (int)watch_address_value;

                if (watch_address >= m_mem_base_addr && watch_address < (m_mem_base_addr + m_mem_size))
                {
                    Watch watch;
                    watch.address = watch_address;
                    snprintf(watch.notes, 128, "%s", notes);
                    m_watches.push_back(watch);
                }

                ImGui::CloseCurrentPopup();
                address[0] = 0;
                notes[0] = 0;

                m_watch_window = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0)))
        {
            ImGui::CloseCurrentPopup();
            address[0] = 0;
            notes[0] = 0;
        }
        ImGui::EndPopup();
    }

    PopGuiFont();
}

void MemEditor::SearchCapture()
{
    memcpy(m_search_data, m_mem_data, m_mem_size);
}

void MemEditor::StepFrame()
{
    if (m_search_auto)
        SearchCapture();
}

int MemEditor::GetWordBytes()
{
    return m_mem_word;
}

char* MemEditor::GetTitle()
{
    return m_title;
}

void MemEditor::WatchWindow()
{
    ImVec4 addr_color = cyan;
    ImVec4 notes_color = violet;
    ImVec4 normal_color = white;
    ImVec4 gray_color = mid_gray;

    PushGuiFont();

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    char window_title[64];
    snprintf(window_title, 64, "%s Watches", m_title);
    ImGui::Begin(window_title, &m_watch_window);

    if (ImGui::Button("Add Watch"))
    {
        m_add_watch = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Remove All"))
    {
        RemoveWatches();
    }

    ImGui::Separator();

    PopGuiFont();

    ImVec2 character_size = ImGui::CalcTextSize("0");

    int remove = -1;

    if (ImGui::BeginTable("##hex", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("ADDRESS");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, character_size.x);

        ImGui::TableSetupColumn("VALUE", ImGuiTableColumnFlags_WidthFixed, character_size.x * ((m_mem_word * 2) + 1));

        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, character_size.x);
        ImGui::TableSetupColumn("NOTES");

        int total_rows = (int)m_watches.size();

        ImGuiListClipper clipper;
        clipper.Begin(total_rows);

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                Watch watch = m_watches[row];

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                char remove_id[64];
                snprintf(remove_id, 64, "X##remove_%s_%d", m_title, row);

                if (ImGui::SmallButton(remove_id))
                {
                    remove = row;
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Remove watch");
                    ImGui::EndTooltip();
                }

                ImGui::SameLine();

                char single_addr[32];
                snprintf(single_addr, 32, "%s:  ", m_hex_addr_format);
                ImGui::TextColored(addr_color, single_addr, watch.address);
                ImGui::TableNextColumn();
                ImGui::TableNextColumn();

                uint16_t data = 0;
                int address = watch.address - m_mem_base_addr;

                if (m_mem_word == 1)
                    data = m_mem_data[address];
                else if (m_mem_word == 2)
                {
                    uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                    data = mem_data_16[address];
                }

                bool gray_out = m_gray_out_zeros && (data == 0);
                ImVec4 color = gray_out ? gray_color : normal_color;

                if (m_mem_word == 1)
                    ImGui::TextColored(color, m_uppercase_hex ? "%02X" : "%02x", data);
                else if (m_mem_word == 2)
                    ImGui::TextColored(color, m_uppercase_hex ? "%04X" : "%04x", data);

                ImGui::TableNextColumn();
                ImGui::TableNextColumn();

                ImGui::TextColored(notes_color, "%s", watch.notes);
            }
        }
        ImGui::EndTable();
    }

    if (remove >= 0)
    {
        m_watches.erase(m_watches.begin() + remove);
    }

    ImGui::End();
}

void MemEditor::SearchWindow()
{
    ImVec4 addr_color = cyan;
    ImVec4 value_color = white;
    ImVec4 prev_color = orange;

    PushGuiFont();

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    char window_title[64];
    snprintf(window_title, 64, "%s Search", m_title);
    ImGui::Begin(window_title, &m_search_window);

    ImGui::PushItemWidth(240);
    const char* search_opeartors[] = {"Value is less than", "Value is greater than", "Value is equal to", "Value is not equal to", "Value is less than or equal to", "Value is greater than or equal to"};
    ImGui::Combo("##search_op", &m_search_operator, search_opeartors, IM_ARRAYSIZE(search_opeartors));

    ImGui::PushItemWidth(160);
    const char* search_compare_types[] = {"Previous snapshot", "Specific value", "Specific address"};
    ImGui::Combo("##search_comp", &m_search_compare_type, search_compare_types, IM_ARRAYSIZE(search_compare_types));

    if (m_search_compare_type == 1)
    {
        ImGui::SameLine();

        switch (m_search_data_type)
        {
            // Hexadecimal
            case 0:
            {
                const char* buf = m_mem_word == 1 ? "00" : "0000";
                ImVec2 character_size = ImGui::CalcTextSize("0");
                ImGui::PushItemWidth(character_size.x * (strlen(buf) + 1));

                if (ImGui::InputTextWithHint("##search_value", buf, m_search_compare_specific_value_str, m_mem_word == 1 ? 3 : 5, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
                {
                    u32 value = 0;
                    if (parse_hex_string(m_search_compare_specific_value_str, strlen(m_search_compare_specific_value_str), &value))
                    {
                        m_search_compare_specific_value = (int)value;
                    }
                }
                break;
            }
            // Signed
            case 1:
            {
                ImVec2 character_size = ImGui::CalcTextSize("0000000");
                ImGui::PushItemWidth(character_size.x);
                if (ImGui::InputScalar("##search_value", ImGuiDataType_S32, &m_search_compare_specific_value, NULL, NULL, NULL, ImGuiInputTextFlags_AutoSelectAll))
                {
                    int max_value = m_mem_word == 1 ? INT8_MAX : INT16_MAX;
                    int min_value = m_mem_word == 1 ? INT8_MIN : INT16_MIN;
                    if (m_search_compare_specific_value > max_value)
                        m_search_compare_specific_value = max_value;
                    else if (m_search_compare_specific_value < min_value)
                        m_search_compare_specific_value = min_value;
                }
                break;
            }
            // Unsigned
            case 2:
            {
                ImVec2 character_size = ImGui::CalcTextSize("0000000");
                ImGui::PushItemWidth(character_size.x);
                if (ImGui::InputScalar("##search_value", ImGuiDataType_U32, &m_search_compare_specific_value, NULL, NULL, NULL, ImGuiInputTextFlags_AutoSelectAll))
                {
                    int max_value = m_mem_word == 1 ? UINT8_MAX : UINT16_MAX;
                    if (m_search_compare_specific_value < 0)
                        m_search_compare_specific_value = 0;
                    if (m_search_compare_specific_value > max_value)
                        m_search_compare_specific_value = max_value;
                }
                break;
            }
        }
    }
    else if (m_search_compare_type == 2)
    {
        ImGui::SameLine();

        char buf[32];
        snprintf(buf, 32, m_hex_addr_format, 0);
        ImVec2 character_size = ImGui::CalcTextSize("0");

        ImGui::PushItemWidth(character_size.x * (strlen(buf) + 1));

        if (ImGui::InputTextWithHint("##search_address", buf, m_search_compare_specific_address_str, m_hex_addr_digits + 1, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            u32 address_value = 0;
            if (parse_hex_string(m_search_compare_specific_address_str, strlen(m_search_compare_specific_address_str), &address_value))
            {
                m_search_compare_specific_address = (int)address_value;

                if (m_search_compare_specific_address < 0)
                    m_search_compare_specific_address = 0;
                else if (m_search_compare_specific_address >= m_mem_size)
                    m_search_compare_specific_address = m_mem_size - 1;
            }
        }
    }

    ImGui::PushItemWidth(140);
    const char* search_types[] = {"Hexadecimal", "Signed", "Unsigned"};
    ImGui::Combo("##search_type", &m_search_data_type, search_types, IM_ARRAYSIZE(search_types));

    if (ImGui::Button("Capture"))
    {
        SearchCapture();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Take a snapshot of current memory\nThis will be used when comparing to previous values");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto Capture", &m_search_auto);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Automatically takes a snapshot each\ntime \"Step Frame (F6)\" is pressed");
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    PopGuiFont();

    CalculateSearchResults();

    if (ImGui::BeginTable("##hex", 3, ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ADDRESS");
        ImGui::TableSetupColumn("VALUE");
        ImGui::TableSetupColumn("PREVIOUS");
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin((int)m_search_results.size());

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                Search result = m_search_results[row];

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                char single_addr[32];
                snprintf(single_addr, 32, "%s:  ", m_hex_addr_format);
                ImGui::TextColored(addr_color, single_addr, result.address + m_mem_base_addr);
                ImGui::TableNextColumn();

                DrawSearchValue(result.value, value_color);
                ImGui::TableNextColumn();

                DrawSearchValue(result.prev_value, prev_color);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void MemEditor::CalculateSearchResults()
{
    if (!IsValidPointer(m_search_data) || !IsValidPointer(m_mem_data))
        return;

    m_search_results.clear();

    for (int i = 0; i < m_mem_size; i++)
    {
        int compare_value = 0;
        int current_value = 0;
        int search_value = 0;
        uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
        uint16_t* search_data_16 = (uint16_t*)m_search_data;

        if (m_mem_word == 1)
        {
            if (m_search_data_type == 1)
            {
                current_value = (int8_t)m_mem_data[i];
                search_value = (int8_t)m_search_data[i];
            }
            else
            {
                current_value = m_mem_data[i];
                search_value = m_search_data[i];
            }
        }
        else if (m_mem_word == 2)
        {
            if (m_search_data_type == 1)
            {
                current_value = (int16_t)mem_data_16[i];
                search_value = (int16_t)search_data_16[i];
            }
            else
            {
                current_value = mem_data_16[i];
                search_value = search_data_16[i];
            }
        }

        switch (m_search_compare_type)
        {
            // Previous
            case 0:
                compare_value = search_value;
                break;
            // Specific
            case 1:
                compare_value = m_search_compare_specific_value;
                break;
            // Specific address
            case 2:
                if (m_mem_word == 1)
                    compare_value = m_mem_data[m_search_compare_specific_address];
                else if (m_mem_word == 2)
                    compare_value = mem_data_16[m_search_compare_specific_address];
                break;
        }

        bool found = false;

        switch (m_search_operator)
        {
            // <
            case 0:
            {
                found = (current_value < compare_value);
                break;
            }
            // >
            case 1:
            {
                found = (current_value > compare_value);
                break;
            }
            // ==
            case 2:
            {
                found = (current_value == compare_value);
                break;
            }
            // !=
            case 3:
            {
                found = (current_value != compare_value);
                break;
            }
            // <=
            case 4:
            {
                found = (current_value <= compare_value);
                break;
            }
            // >=
            case 5:
            {
                found = (current_value >= compare_value);
                break;
            }
        }

        if (found)
        {
            Search result;
            result.address = i;
            result.value = current_value;
            result.prev_value = search_value;
            m_search_results.push_back(result);
        }
    }
}

void MemEditor::DrawSearchValue(int value, ImVec4 color)
{
    ImVec4 gray_color = mid_gray;
    bool gray_out = m_gray_out_zeros && (value == 0);
    ImVec4 final_color = gray_out ? gray_color : color;

    switch (m_search_data_type)
    {
        case 0:
            if (m_mem_word == 1)
                ImGui::TextColored(final_color, m_uppercase_hex ? "%02X" : "%02x", value);
            else if (m_mem_word == 2)
                ImGui::TextColored(final_color, m_uppercase_hex ? "%04X" : "%04x", value);
            break;
        case 1:
            if (m_mem_word == 1)
                ImGui::TextColored(final_color, "%d", (int8_t)value);
            else if (m_mem_word == 2)
                ImGui::TextColored(final_color, "%d", (int16_t)value);
            break;
        case 2:
            if (m_mem_word == 1)
                ImGui::TextColored(final_color, "%u", (uint8_t)value);
            else if (m_mem_word == 2)
                ImGui::TextColored(final_color, "%u", (uint16_t)value);
            break;
    }
}

void MemEditor::PushGuiFont()
{
    if (m_gui_font != NULL)
        ImGui::PushFont(m_gui_font);
}

void MemEditor::PopGuiFont()
{
    if (m_gui_font != NULL)
        ImGui::PopFont();
}

void MemEditor::Copy(bool as_decimal)
{
    int size = (m_selection_end - m_selection_start + 1) * m_mem_word;
    uint8_t* data = m_mem_data + (m_selection_start * m_mem_word);

    std::string text;

    for (int i = 0; i < size; i++)
    {
        char byte[8];

        if (as_decimal)
            snprintf(byte, 8, "%d", data[i]);
        else
            snprintf(byte, 8, m_uppercase_hex ? "%02X" : "%02x", data[i]);

        if (i > 0)
            text += " ";
        text += byte;
    }

    SDL_SetClipboardText(text.c_str());
}

void MemEditor::Paste()
{
    char* clipboard = SDL_GetClipboardText();

    if (clipboard != NULL)
    {
        std::string text(clipboard);

        text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
        text.erase(std::remove(text.begin(), text.end(), ' '), text.end());

        int buffer_size = (int)text.size() / 2;

        uint8_t* data = new uint8_t[buffer_size];

        for (int i = 0; i < buffer_size; i ++)
        {
            std::string byte = text.substr(i * 2, 2);

            uint8_t value = 0;
            if (parse_hex_string(byte.c_str(), byte.length(), &value))
            {
                data[i] = value;
            }
            else
            {
                delete[] data;
                SDL_free(clipboard);
                return;
            }
        }

        int selection_size = (m_selection_end - m_selection_start + 1) * m_mem_word;
        int start = m_selection_start * m_mem_word;
        int end = start + MIN(buffer_size, selection_size);

        for (int i = start; i < end; i++)
        {
            m_mem_data[i] = data[i - start];
        }

        delete[] data;
    }

    SDL_free(clipboard);
}

void MemEditor::JumpToAddress(int address)
{
    if (address >= m_mem_base_addr && address < (m_mem_base_addr + m_mem_size))
        m_jump_to_address = address - m_mem_base_addr;
}

void MemEditor::FindNextValue(int value)
{
    if (m_mem_word == 1)
        value &= 0xFF;
    else if (m_mem_word == 2)
        value &= 0xFFFF;

    int start = m_selection_start + 1;

    for (int i = 0; i < m_mem_size; i++)
    {
        int index = (start + i) % m_mem_size;
        uint16_t data = 0;

        if (m_mem_word == 1)
            data = m_mem_data[index];
        else if (m_mem_word == 2)
        {
            uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
            data = mem_data_16[index];
        }

        if (data == (uint16_t)value)
        {
            JumpToAddress(index + m_mem_base_addr);
            break;
        }
    }
}

void MemEditor::SelectAll()
{
    m_selection_start = 0;
    m_selection_end = m_mem_size - 1;
}

void MemEditor::ClearSelection()
{
    m_selection_start = m_selection_end = 0;
}

void MemEditor::SetValueToSelection(int value)
{
    int selection_size = (m_selection_end - m_selection_start + 1) * m_mem_word;
    int start = m_selection_start * m_mem_word;
    int end = start + selection_size;
    int mask = m_mem_word == 1 ? 0xFF : 0xFFFF;

    for (int i = start; i < end; i++)
    {
        m_mem_data[i] = value & mask;
    }
}

void MemEditor::SaveToTextFile(const char* file_path)
{
    int total_bytes = m_mem_size * m_mem_word;
    int row_bytes   = m_bytes_per_row * m_mem_word;
    FILE* file = fopen_utf8(file_path, "w");

    if (file)
    {
        int row_count = (total_bytes + row_bytes - 1) / row_bytes;
        for (int r = 0; r < row_count; r++)
        {
            int current_address = m_mem_base_addr + (r * m_bytes_per_row);

            fprintf(file, m_hex_addr_format, current_address);
            fprintf(file, ":    ");

            int row_start = r * row_bytes;
            int row_end = row_start + row_bytes;
            if (row_end > total_bytes)
                row_end = total_bytes;

            if (m_mem_word == 1)
                for (int i = row_start; i < row_end; i++)
                    fprintf(file, "%02X ", m_mem_data[i]);
            else if (m_mem_word == 2)
            {
                int word_count = (row_end - row_start) / 2;
                uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                int word_start = row_start / 2;
                for (int i = 0; i < word_count; i++)
                    fprintf(file, "%04X ", mem_data_16[word_start + i]);
            }
            fprintf(file, "\n");
        }

        fclose(file);
    }
}

void MemEditor::SaveToBinaryFile(const char* file_path)
{
    int size = m_mem_size * m_mem_word;

    FILE* file = fopen_utf8(file_path, "wb");

    if (file)
    {
        fwrite(m_mem_data, m_mem_word, size, file);
        fclose(file);
    }
}

void MemEditor::AddBookmark()
{
    m_add_bookmark = true;
}

void MemEditor::RemoveBookmarks()
{
    m_bookmarks.clear();
}

void MemEditor::RemoveWatches()
{
    m_watches.clear();
}

std::vector<MemEditor::Bookmark>* MemEditor::GetBookmarks()
{
    return &m_bookmarks;
}

void MemEditor::OpenWatchWindow()
{
    m_watch_window = true;
}

void MemEditor::OpenSearchWindow()
{
    if (!m_search_window)
        SearchCapture();

    m_search_window = true;
}

void MemEditor::AddWatch()
{
    m_add_watch = true;
}

void MemEditor::SetGuiFont(ImFont* gui_font)
{
    m_gui_font = gui_font;
}
