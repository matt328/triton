#pragma once
#include "gp/Registry.hpp"

namespace tr {
struct RenderData;
/// @brief ECS 'System' that extracts GameWorld data to hand off to Renderer.
class RenderDataSystem {
 public:
   explicit RenderDataSystem(std::shared_ptr<Registry> newRegistry);
   ~RenderDataSystem() = default;

   RenderDataSystem(const RenderDataSystem& other) = delete;
   RenderDataSystem(RenderDataSystem&& other) noexcept = delete;
   auto operator=(const RenderDataSystem& other) -> RenderDataSystem& = delete;
   auto operator=(RenderDataSystem&& other) noexcept -> RenderDataSystem& = delete;

   auto update(RenderData& renderData) const -> void;

 private:
   std::shared_ptr<Registry> registry;

   static auto convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) -> glm::mat4;
};

}

