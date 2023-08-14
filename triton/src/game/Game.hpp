#pragma once

#include "RenderSystem.hpp"
#include "TransformSystem.hpp"

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

 private:
   std::unique_ptr<entt::registry> registry;
   std::shared_ptr<RenderSystem> renderSystem;
   std::unique_ptr<TransformSystem> transformSystem;
};
