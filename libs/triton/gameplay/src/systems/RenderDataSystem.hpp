#pragma once

namespace tr::cm {
   struct RenderData;
}

namespace tr::gp::ecs::RenderDataSystem {
   void update(const entt::registry& registry, cm::RenderData& renderData);
}