#pragma once

namespace ed {

constexpr auto red = ImVec4(0.77f, .42f, .36f, 1.f);
constexpr auto orange = ImVec4(.949f, .784f, .608f, 1.f);
constexpr auto green = ImVec4(0.443f, .694f, .529f, 1.f);
constexpr auto blue = ImVec4(.388f, .541f, .592f, 1.f);
constexpr auto white = ImVec4(1.f, 1.f, 1.f, 1.f);

const auto colorMap = std::unordered_map<std::string, ImVec4>{{"info", green},
                                                              {"trace", blue},
                                                              {"debug", white},
                                                              {"warning", orange},
                                                              {"error", red},
                                                              {"critical", red}};

struct AppLog {
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets;
  bool AutoScroll{true};
  ImFont* font{};
  std::mutex mtx;

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
    va_list args = {};
    va_start(args, fmt);
    {
      std::lock_guard<std::mutex> lock(mtx);
      Buf.appendfv(fmt, args);

      va_end(args);
      for (int new_size = Buf.size(); old_size < new_size; old_size++) {
        if (Buf[old_size] == '\n') {
          LineOffsets.push_back(old_size + 1);
        }
      }
    }
  }

  static auto getColor(const char* begin, const char* end) {
    for (const auto& [key, value] : colorMap) {
      if (std::search(begin, end, key.begin(), key.end()) != end) {
        return value;
      }
    }
    return white;
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

      {
        std::lock_guard<std::mutex> lock(mtx);
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        ImGui::PushFont(font);
        if (Filter.IsActive()) {
          for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
            const char* line_start = buf + LineOffsets[line_no];
            const char* line_end =
                (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;

            if (Filter.PassFilter(line_start, line_end)) {
              ImGui::TextUnformatted(line_start, line_end);
            }
          }
        } else {
          ImGuiListClipper clipper;
          clipper.Begin(LineOffsets.Size);
          while (clipper.Step()) {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
              const char* line_start = buf + LineOffsets[line_no];
              const char* line_end =
                  (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
              auto color = getColor(line_start, line_end);

              ImGui::PushStyleColor(ImGuiCol_Text, color);
              ImGui::TextUnformatted(line_start, line_end);
              ImGui::PopStyleColor();
            }
          }
          clipper.End();
        }
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
