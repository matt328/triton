#pragma once

namespace tr {

class TransformSystem {
public:
  explicit TransformSystem() = default;
  ~TransformSystem() = default;

  TransformSystem(const TransformSystem&) = delete;
  auto operator=(const TransformSystem&) -> TransformSystem& = delete;
  TransformSystem(TransformSystem&&) = delete;
  auto operator=(TransformSystem&&) -> TransformSystem& = delete;

  auto update(entt::registry& registry) -> void;

private:
  mutable TracySharedLockableN(std::shared_mutex, registryMutex, "CameraSystem");
};

}
