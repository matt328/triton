#pragma once

#include "IGameplaySystem.hpp"
#include "action/IActionSystem.hpp"

namespace tr::gp {
   class GameplaySystem : public IGameplaySystem {
    public:
      explicit GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem);
      ~GameplaySystem() override = default;

      void update() override;
      void fixedUpdate() override;
   };
}