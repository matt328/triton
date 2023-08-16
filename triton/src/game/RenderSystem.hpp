#pragma once

#include "game/RenderObject.hpp"
#include "game/Renderable.hpp"

class RenderSystem {
 public:
   RenderSystem() = default;
   ~RenderSystem() = default;

   RenderSystem(const RenderSystem&) = delete;
   RenderSystem(RenderSystem&&) = delete;
   RenderSystem& operator=(const RenderSystem&) = delete;
   RenderSystem& operator=(RenderSystem&&) = delete;

   void update(entt::registry& registry);

   [[nodiscard]] const std::vector<RenderObject>& getRenderObjects() const {
      return renderObjects;
   };

 private:
   std::vector<RenderObject> renderObjects;
};
