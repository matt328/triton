#pragma once

namespace ed {
inline auto renderCameraInspector(tr::Camera* camera) -> void {
   if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (ImGui::DragFloat3("Position##Camera", glm::value_ptr(camera->position), 0.5f)) {}
      if (ImGui::DragFloat3("Front##Camera", glm::value_ptr(camera->front), 0.5f)) {}
      if (ImGui::DragFloat("Yaw##Camera", &camera->yaw, 0.5f)) {}
      if (ImGui::DragFloat("Pitch##Camera", &camera->pitch, 0.5f)) {}
      if (ImGui::DragFloat3("Velocity##Camera", value_ptr(camera->velocity), 0.5f)) {}
   }
}
}

