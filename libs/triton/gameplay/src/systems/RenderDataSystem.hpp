#pragma once

namespace tr::cm::gpu {
   struct RenderData;
}

namespace tr::gp::sys::RenderDataSystem {
   void update(const entt::registry& registry, cm::gpu::RenderData& renderData);
}