#pragma once

#include "IGameplaySystem.hpp"

namespace tr::gp {
   class GameplaySystem : public IGameplaySystem {
    public:
      GameplaySystem() = default;
      ~GameplaySystem() = default;

      void update() override;
      void fixedUpdate() override;
   };
}