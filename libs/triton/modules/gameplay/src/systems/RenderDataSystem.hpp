#pragma once

#include "gp/EntityService.hpp"
namespace tr {

struct RenderData;

/// @brief ECS 'System' that extracts GameWorld data to hand off to Renderer.
/// Guards registry access with a shared lock since it only reads.
class RenderDataSystem {
public:
  explicit RenderDataSystem(std::shared_ptr<EntityService> newEntityService);
  ~RenderDataSystem() = default;

  RenderDataSystem(const RenderDataSystem& other) = delete;
  RenderDataSystem(RenderDataSystem&& other) noexcept = delete;
  auto operator=(const RenderDataSystem& other) -> RenderDataSystem& = delete;
  auto operator=(RenderDataSystem&& other) noexcept -> RenderDataSystem& = delete;

  auto update(RenderData& renderData) -> void;

private:
  std::shared_ptr<EntityService> entityService;
  auto convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) -> glm::mat4;
};

}
