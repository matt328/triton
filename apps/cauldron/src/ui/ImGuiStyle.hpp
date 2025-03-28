#pragma once
namespace ImGuiEx {
inline void setupImGuiStyle() {
  // Fork of Visual Studio style from ImThemes
  ImGuiStyle& style = ImGui::GetStyle();

  style.Alpha = 1.0f;
  style.DisabledAlpha = 0.6000000238418579f;
  style.WindowPadding = ImVec2(8.0f, 8.0f);
  style.WindowRounding = 0.0f;
  style.WindowBorderSize = 1.0f;
  style.WindowMinSize = ImVec2(32.0f, 32.0f);
  style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
  style.WindowMenuButtonPosition = ImGuiDir_Left;
  style.ChildRounding = 0.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupRounding = 0.0f;
  style.PopupBorderSize = 1.0f;
  style.FramePadding = ImVec2(4.0f, 4.0f);
  style.FrameRounding = 0.0f;
  style.FrameBorderSize = 0.0f;
  style.ItemSpacing = ImVec2(8.0f, 8.0f);
  style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
  style.CellPadding = ImVec2(4.0f, 4.0f);
  style.IndentSpacing = 21.0f;
  style.ColumnsMinSpacing = 6.0f;
  style.ScrollbarSize = 14.0f;
  style.ScrollbarRounding = 0.0f;
  style.GrabMinSize = 10.0f;
  style.GrabRounding = 0.0f;
  style.TabRounding = 0.0f;
  style.TabBorderSize = 0.0f;
  style.TabMinWidthForCloseButton = 0.0f;
  style.ColorButtonPosition = ImGuiDir_Right;
  style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
  style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

  style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled] =
      ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, 1.0f);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 0.686274528503418f);
  style.Colors[ImGuiCol_ChildBg] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 0.5882353186607361f);
  style.Colors[ImGuiCol_PopupBg] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_Border] =
      ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
  style.Colors[ImGuiCol_BorderShadow] =
      ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
  style.Colors[ImGuiCol_FrameBg] =
      ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
  style.Colors[ImGuiCol_FrameBgHovered] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_FrameBgActive] =
      ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_TitleBg] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_MenuBarBg] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 0.0f);
  style.Colors[ImGuiCol_ScrollbarBg] =
      ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrab] =
      ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_SliderGrab] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_SliderGrabActive] =
      ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_Button] =
      ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
  style.Colors[ImGuiCol_ButtonHovered] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_ButtonActive] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_Header] =
      ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
  style.Colors[ImGuiCol_HeaderHovered] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_HeaderActive] =
      ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_Separator] =
      ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
  style.Colors[ImGuiCol_SeparatorHovered] =
      ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
  style.Colors[ImGuiCol_SeparatorActive] =
      ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
  style.Colors[ImGuiCol_ResizeGrip] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_ResizeGripHovered] =
      ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
  style.Colors[ImGuiCol_ResizeGripActive] =
      ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
  style.Colors[ImGuiCol_Tab] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_TabHovered] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocused] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocusedActive] =
      ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_PlotLinesHovered] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogram] =
      ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
  style.Colors[ImGuiCol_TableHeaderBg] =
      ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
  style.Colors[ImGuiCol_TableBorderStrong] =
      ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
  style.Colors[ImGuiCol_TableBorderLight] =
      ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
  style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
  style.Colors[ImGuiCol_TextSelectedBg] =
      ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
  style.Colors[ImGuiCol_DragDropTarget] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_NavHighlight] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
  style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
  style.Colors[ImGuiCol_NavWindowingDimBg] =
      ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
  style.Colors[ImGuiCol_ModalWindowDimBg] =
      ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 0.09803921729326248f);
}
}
