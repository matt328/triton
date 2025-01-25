#pragma once

#include "gp/components/Transform.hpp"

namespace ed {

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
