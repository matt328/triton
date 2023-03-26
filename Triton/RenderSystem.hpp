#pragma once

class RenderDevice;

class RenderSystem {
 public:
   explicit RenderSystem(const RenderDevice& renderDevice);
   ~RenderSystem() = default;

   void update(entt::registry& registry, float dt) const;

 private:
   const RenderDevice& renderDevice;
};
