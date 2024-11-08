#pragma once

#include "gp/IGameplaySystem.hpp"
#include "gp/action/IActionSystem.hpp"

namespace tr::gp {
   class GameplaySystem : public IGameplaySystem {
    public:
      explicit GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem);
      ~GameplaySystem() override = default;

      void update() override;
      void fixedUpdate() override;
   };
}