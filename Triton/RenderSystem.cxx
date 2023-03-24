#include "pch.hpp"

#include "RenderSystem.hpp"
#include "Renderable.hpp"
#include "RenderDevice.hpp"

RenderSystem::RenderSystem(const RenderDevice& renderDevice) : renderDevice(renderDevice) {
}

void RenderSystem::update(entt::registry& registry, float dt) {
   auto view = registry.view<Renderable>();
   for (const auto entity : view) {
      auto& renderable = view.get<Renderable>(entity);
      renderDevice.enqueue(renderable);
   }
}
