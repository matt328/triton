#pragma once

namespace Triton::Graphics {
   class Renderer;
}

namespace Triton::Game::Ecs::RenderSystem {
   void update(entt::registry& registry, Graphics::Renderer& renderer);
}