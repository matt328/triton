#pragma once

namespace ed::ui::cmp {
   inline auto renderTransformInspector(tr::gp::cmp::Transform* transform) -> void {
      if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
         if (ImGui::DragFloat3("Position##Transform", glm::value_ptr(transform->position), 0.5f)) {}
         if (ImGui::DragFloat3("Rotation##Transform", glm::value_ptr(transform->rotation), 0.5f)) {}
      }
   }
}