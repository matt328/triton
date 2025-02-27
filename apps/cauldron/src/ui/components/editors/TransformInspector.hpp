#pragma once

#include "data/DataStore.hpp"

namespace ed {

using Listener = std::function<void(std::string_view, Orientation)>;

inline auto renderTransformInspector(std::string_view name,
                                     Orientation* orientation,
                                     const Listener& listener) -> void {
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::DragFloat3("Position##Transform", glm::value_ptr(orientation->position), 0.5f)) {
      listener(name, *orientation);
    }
    if (ImGui::DragFloat3("Rotation##Transform", glm::value_ptr(orientation->rotation), 0.5f)) {
      listener(name, *orientation);
    }
  }
}

}
