#pragma once

#include "RenderSystem.hpp"

class RenderDevice;

class Game {
 public:
   explicit Game(RenderDevice& renderDevice);
   ~Game() = default;

   void update(double t, float dt) const;
   void blendState(double alpha);

 private:
   std::unique_ptr<entt::registry> registry;
   std::unique_ptr<RenderSystem> renderSystem;
};
