#pragma once

#include "cm/sdf/VoxelMetrics.hpp"

#include <misc/cpp/imgui_stdlib.h>

namespace ed::ui::components {
   struct TerrainDebugger {
      TerrainDebugger() = default;

      static void render() {
         if (ImGui::Begin("Terrain Debugger", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            const auto& cubes = tr::cm::sdf::VoxelDebugger::getInstance().getActiveCubePositions();

            auto names = std::vector<const char*>{};
            names.reserve(cubes.size());
            for (const auto& name : cubes | std::views::keys) {
               names.push_back(name.c_str());
            }

            static int item_current = 0;
            ImGui::Combo("Cube", &item_current, names.data(), static_cast<int>(names.size()));
         }
         ImGui::End();
      };
   };
}