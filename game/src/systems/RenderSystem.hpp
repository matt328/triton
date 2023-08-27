#pragma once

#include "components/Camera.hpp"
#include "RenderObject.hpp"
#include "components/Renderable.hpp"

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

   [[nodiscard]] std::tuple<glm::mat4, glm::mat4, glm::mat4> getCameraParams() const;

 private:
   std::vector<RenderObject> renderObjects;
   Camera currentCamera;
};
