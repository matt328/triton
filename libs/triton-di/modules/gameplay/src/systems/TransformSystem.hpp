#pragma once
#include <gp/Registry.hpp>

namespace tr {
class TransformSystem {
public:
  explicit TransformSystem(std::shared_ptr<Registry> newRegistry);
  ~TransformSystem() = default;

  TransformSystem(const TransformSystem&) = delete;
  TransformSystem& operator=(const TransformSystem&) = delete;
  TransformSystem(TransformSystem&&) = delete;
  TransformSystem& operator=(TransformSystem&&) = delete;

  auto update() const -> void;

private:
  std::shared_ptr<Registry> registry;
};
}
