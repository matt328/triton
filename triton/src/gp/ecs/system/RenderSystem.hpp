#pragma once

namespace tr::gfx {
   struct RenderObject;
   struct CameraData;
   class Renderer;
}

namespace tr::gp::ecs::RenderSystem {
   void update(entt::registry& registry,
               entt::delegate<void(gfx::RenderObject)>& renderObjectProducer,
               entt::delegate<void(gfx::CameraData)>& cameraDataProducer);
}