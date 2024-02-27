#pragma once

namespace tr::gfx {
   class Renderer;
}

namespace tr::gp::ecs::RenderSystem {
   void update(entt::registry& registry, gfx::Renderer& renderer);
}