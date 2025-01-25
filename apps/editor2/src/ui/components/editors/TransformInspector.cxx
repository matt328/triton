#include "TransformInspector.hpp"

namespace ed {

auto TransformInspector::render(std::string_view name, tr::Transform* transform) -> void {
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::DragFloat3("Position##Transform", glm::value_ptr(transform->position), 0.5f)) {
      listener(name, *transform);
    }
    if (ImGui::DragFloat3("Rotation##Transform", glm::value_ptr(transform->rotation), 0.5f)) {
      listener(name, *transform);
    }
  }
}

auto TransformInspector::setTransformListener(const TransformListener& newListener) -> void {
  listener = newListener;
}

}
