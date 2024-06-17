#pragma once

namespace tr::gfx {
   struct RenderData;
   struct CameraData;
   class RenderContext;
}

namespace tr::gp::ecs::RenderDataSystem {
   void update(const entt::registry& registry, gfx::RenderData& renderData);
}