#pragma once

#include <glm/fwd.hpp>

class RenderObject;

class IGame {
 public:
   IGame() = default;
   IGame(const IGame&) = default;
   IGame(IGame&&) = delete;
   IGame& operator=(const IGame&) = default;
   IGame& operator=(IGame&&) = delete;

   virtual ~IGame() = default;

   virtual std::vector<RenderObject> getRenderObjects() = 0;
   virtual std::tuple<glm::mat4, glm::mat4, glm::mat4> getCameraParams() = 0;

   virtual void update() = 0;
   virtual void blendState(double blendFactor) = 0;
   virtual void keyPressed(int key, int scancode, int action, int mods) = 0;
};
