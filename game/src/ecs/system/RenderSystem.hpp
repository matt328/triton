#pragma once

namespace Triton::Graphics {
   class VkRenderer;
}

namespace Triton::Game::Ecs::RenderSystem {
   void update(entt::registry& registry, Graphics::VkRenderer& renderer);
}