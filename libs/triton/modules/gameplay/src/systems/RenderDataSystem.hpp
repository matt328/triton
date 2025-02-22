#pragma once

namespace tr {

struct RenderData;

/// @brief ECS 'System' that extracts GameWorld data to hand off to Renderer.
/// Guards registry access with a shared lock since it only reads.
class RenderDataSystem {
public:
  RenderDataSystem() = default;
  ~RenderDataSystem() = default;

  RenderDataSystem(const RenderDataSystem& other) = delete;
  RenderDataSystem(RenderDataSystem&& other) noexcept = delete;
  auto operator=(const RenderDataSystem& other) -> RenderDataSystem& = delete;
  auto operator=(RenderDataSystem&& other) noexcept -> RenderDataSystem& = delete;

  auto update(entt::registry& registry, RenderData& renderData) -> void;

private:
  auto convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) -> glm::mat4;
  mutable TracySharedLockableN(std::shared_mutex, registryMutex, "RenderDataSystem");
};

}
