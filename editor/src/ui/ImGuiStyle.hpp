#pragma once
namespace ImGuiEx {
   struct ImVec3 {
      float x, y, z;
      ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
         x = _x;
         y = _y;
         z = _z;
      }
   };

   inline static void imgui_easy_theming(ImVec3 color_for_text,
                                         ImVec3 color_for_head,
                                         ImVec3 color_for_area,
                                         ImVec3 color_for_body,
                                         ImVec3 color_for_pops) {
      ImGuiStyle& style = ImGui::GetStyle();

      style.WindowRounding = 3;
      style.ChildRounding = 3;
      style.FrameRounding = 3;
      style.PopupRounding = 3;
      style.ScrollbarRounding = 3;
      style.GrabRounding = 3;
      style.TabRounding = 3;
      style.WindowTitleAlign = ImVec2{0.5f, 0.5f};
      style.ButtonTextAlign = ImVec2{0.5f, 0.5f};

      style.Colors[ImGuiCol_Text] =
          ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
      style.Colors[ImGuiCol_TextDisabled] =
          ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.58f);
      style.Colors[ImGuiCol_WindowBg] =
          ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.71f);
      style.Colors[ImGuiCol_Border] =
          ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
      style.Colors[ImGuiCol_BorderShadow] =
          ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
      style.Colors[ImGuiCol_FrameBg] =
          ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
      style.Colors[ImGuiCol_FrameBgHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
      style.Colors[ImGuiCol_FrameBgActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_TitleBg] =
          ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
      style.Colors[ImGuiCol_TitleBgCollapsed] =
          ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.75f);
      style.Colors[ImGuiCol_TitleBgActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_MenuBarBg] =
          ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.47f);
      style.Colors[ImGuiCol_ScrollbarBg] =
          ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
      style.Colors[ImGuiCol_ScrollbarGrab] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.21f);
      style.Colors[ImGuiCol_ScrollbarGrabHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
      style.Colors[ImGuiCol_ScrollbarGrabActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_CheckMark] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.80f);
      style.Colors[ImGuiCol_SliderGrab] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
      style.Colors[ImGuiCol_SliderGrabActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_Button] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
      style.Colors[ImGuiCol_ButtonHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
      style.Colors[ImGuiCol_ButtonActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_Header] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.76f);
      style.Colors[ImGuiCol_HeaderHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
      style.Colors[ImGuiCol_HeaderActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_ResizeGrip] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.15f);
      style.Colors[ImGuiCol_ResizeGripHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
      style.Colors[ImGuiCol_ResizeGripActive] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_PlotLines] =
          ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
      style.Colors[ImGuiCol_PlotLinesHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_PlotHistogram] =
          ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
      style.Colors[ImGuiCol_PlotHistogramHovered] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
      style.Colors[ImGuiCol_TextSelectedBg] =
          ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.43f);
      style.Colors[ImGuiCol_PopupBg] =
          ImVec4(color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f);
   }

   inline static void setupImGuiStyle() {
      static ImVec3 color_for_text = ImVec3(236.f / 255.f, 240.f / 255.f, 241.f / 255.f);
      static ImVec3 color_for_head = ImVec3(38.f / 255.f, 118.f / 255.f, 166.f / 255.f);
      static ImVec3 color_for_area = ImVec3(57.f / 255.f, 79.f / 255.f, 105.f / 255.f);
      static ImVec3 color_for_body = ImVec3(44.f / 255.f, 62.f / 255.f, 80.f / 255.f);
      static ImVec3 color_for_pops = ImVec3(33.f / 255.f, 46.f / 255.f, 60.f / 255.f);
      imgui_easy_theming(color_for_text,
                         color_for_head,
                         color_for_area,
                         color_for_body,
                         color_for_pops);
   }
}
