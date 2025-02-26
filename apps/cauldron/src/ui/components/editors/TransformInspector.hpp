#pragma once

namespace ed {

inline auto renderTransformInspector(std::string_view name,
                                     glm::vec3* position,
                                     glm::vec3* rotation) -> void {
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::DragFloat3("Position##Transform", glm::value_ptr(*position), 0.5f)) {
      // listener(name, *transform);
    }
    if (ImGui::DragFloat3("Rotation##Transform", glm::value_ptr(*rotation), 0.5f)) {
      // listener(name, *transform);
    }
  }
}

}
