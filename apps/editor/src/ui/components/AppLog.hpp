#pragma once

#include <imgui.h>
namespace ed::ui::cmp {
   struct AppLog {
      ImGuiTextBuffer Buf;
      ImGuiTextFilter Filter;
      ImVector<int> LineOffsets;
      bool AutoScroll{true};
      ImFont* font{};

      AppLog() {
         Clear();
      }

      void Clear() {
         Buf.clear();
         LineOffsets.clear();
         LineOffsets.push_back(0);
      }

      void AddLog(const char* fmt, ...) IM_FMTARGS(2) {
         int old_size = Buf.size();
         va_list args = nullptr;
         va_start(args, fmt);
         Buf.appendfv(fmt, args);
         va_end(args);
         for (int new_size = Buf.size(); old_size < new_size; old_size++) {
            if (Buf[old_size] == '\n') {
               LineOffsets.push_back(old_size + 1);
            }
         }
      }

      void Draw(const char* title, bool* p_open = nullptr) {
         if (!ImGui::Begin(title, p_open)) {
            ImGui::End();
            return;
         }

         // Main window
         bool clear = ImGui::Button("Clear");
         ImGui::SameLine();
         bool mark = ImGui::Button("Mark");
         ImGui::SameLine();
         Filter.Draw("Filter", 300.0f);
         ImGui::SameLine();
         ImGui::Checkbox("Auto-scroll", &AutoScroll);

         ImGui::Separator();

         if (ImGui::BeginChild("scrolling",
                               ImVec2(0, 0),
                               ImGuiChildFlags_None,
                               ImGuiWindowFlags_HorizontalScrollbar)) {
            if (clear) {
               Clear();
            }

            if (mark) {
               AddLog("----------\n");
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            ImGui::PushFont(font);
            if (Filter.IsActive()) {
               for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
                  const char* line_start = buf + LineOffsets[line_no];
                  const char* line_end = (line_no + 1 < LineOffsets.Size)
                                             ? (buf + LineOffsets[line_no + 1] - 1)
                                             : buf_end;

                  if (Filter.PassFilter(line_start, line_end)) {
                     ImGui::TextUnformatted(line_start, line_end);
                  }
               }
            } else {
               ImGuiListClipper clipper;
               clipper.Begin(LineOffsets.Size);
               while (clipper.Step()) {
                  for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd;
                       line_no++) {
                     const char* line_start = buf + LineOffsets[line_no];
                     const char* line_end = (line_no + 1 < LineOffsets.Size)
                                                ? (buf + LineOffsets[line_no + 1] - 1)
                                                : buf_end;
                     auto color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                     const std::string info = "info";

                     const auto result =
                         std::search(line_start, line_end, info.begin(), info.end());
                     if (result != line_end) {
                        color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                     }

                     ImGui::PushStyleColor(ImGuiCol_Text, color);
                     ImGui::TextUnformatted(line_start, line_end);
                     ImGui::PopStyleColor();
                  }
               }
               clipper.End();
            }
            ImGui::PopFont();
            ImGui::PopStyleVar();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
               ImGui::SetScrollHereY(1.0f);
         }
         ImGui::EndChild();
         ImGui::End();
      }
   };
}