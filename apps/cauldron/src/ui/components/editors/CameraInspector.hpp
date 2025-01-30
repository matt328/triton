#pragma once

#include "gp/components/Camera.hpp"

namespace ed {

/*
  Camera is a bit different since right now gameplay creates and owns it, not the editor.
  This gets into the question of if the camera movement causes the project to become unsaved. Is the
  camera position and orientation part of the project?
  Maybe hold off on this until we have a need to restore the camera position. And maybe that can be
  different like being able to bookmark multiple camera positions/orientations to look at certain
  things quickly.
*/

inline auto renderCameraInspector(tr::Camera* camera) -> void {
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {

    if (ImGui::DragFloat3("Position##Camera", glm::value_ptr(camera->position), 0.5f)) {}
    if (ImGui::DragFloat("Yaw##Camera", &camera->yaw, 0.5f)) {}
    if (ImGui::DragFloat("Pitch##Camera", &camera->pitch, 0.5f)) {}

    ImGui::Separator();

    ImGui::BeginDisabled(true);
    if (ImGui::DragFloat3("Front##Camera", glm::value_ptr(camera->front), 0.5f)) {}
    if (ImGui::DragFloat3("Velocity##Camera", value_ptr(camera->velocity), 0.5f)) {}
    ImGui::EndDisabled();
  }
}

}
