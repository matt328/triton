#include "RenderSystem.hpp"
#include "Renderable.hpp"
#include "graphics/RenderDevice.hpp"

RenderSystem::RenderSystem(const RenderDevice& renderDevice) : renderDevice(renderDevice) {
}

void RenderSystem::update(entt::registry& registry, float dt) const {
   for (const auto view = registry.view<Renderable>(); const auto entity : view) {
      auto& renderable = view.get<Renderable>(entity);
      renderDevice.enqueue(renderable);
   }
}
