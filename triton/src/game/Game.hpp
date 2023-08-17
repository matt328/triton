#pragma once

#include "RenderSystem.hpp"
#include "TransformSystem.hpp"
#include "InputSystem.hpp"

class RenderDevice;

class Game {
 public:
   explicit Game(RenderDevice& renderDevice);
   ~Game() = default;

   Game(const Game&) = delete;
   Game(Game&&) = delete;
   Game& operator=(const Game&) = delete;
   Game& operator=(Game&&) = delete;

   void update() const;
   void blendState(double alpha);
   void keyPressed(const int key, int scancode, const int action, int mods) const;

 private:
   entt::delegate<void(int, int, int, int)> keyDelegate;
   std::unique_ptr<entt::registry> registry;
   std::shared_ptr<RenderSystem> renderSystem;
   std::unique_ptr<TransformSystem> transformSystem;
   std::unique_ptr<InputSystem> inputSystem;
};
