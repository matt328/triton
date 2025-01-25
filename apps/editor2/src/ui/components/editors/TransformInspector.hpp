#pragma once

#include "gp/components/Transform.hpp"

namespace ed {

inline auto renderTransformInspector(tr::Transform* transform) -> void {
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::DragFloat3("Position##Transform", glm::value_ptr(transform->position), 0.5f)) {}
    if (ImGui::DragFloat3("Rotation##Transform", glm::value_ptr(transform->rotation), 0.5f)) {}
  }
}

using TransformListener =
    std::function<void(std::string_view name, const tr::Transform& transform)>;

class TransformInspector {
public:
  TransformInspector() = default;
  ~TransformInspector() = default;

  TransformInspector(const TransformInspector&) = delete;
  TransformInspector(TransformInspector&&) = delete;
  auto operator=(const TransformInspector&) -> TransformInspector& = delete;
  auto operator=(TransformInspector&&) -> TransformInspector& = delete;

  auto render(std::string_view name, tr::Transform* transform) -> void;

  auto setTransformListener(const TransformListener& newListener) -> void;

private:
  TransformListener listener;
};

}
