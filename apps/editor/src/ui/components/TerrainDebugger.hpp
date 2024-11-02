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

            static int item_current = -1;
            if (ImGui::Combo("Cube", &item_current, names.data(), static_cast<int>(names.size()))) {
               Log.debug("Item Selected: {0}", names[item_current]);
            }

            if (item_current != -1) {
               auto selectedCube = cubes.at(names[item_current]);
               for (auto vertex : selectedCube.vertices) {
                  ImGui::DragFloat3("Vertex", glm::value_ptr(vertex.pos));
               }
            }
         }
         ImGui::End();
      };
   };
}
