#pragma once

#include "tr/IGameplaySystem.hpp"
#include "gp/action/IActionSystem.hpp"

namespace tr::gp {
   class GameplaySystem : public IGameplaySystem {
    public:
      explicit GameplaySystem(const std::shared_ptr<IActionSystem>& actionSystem);
      ~GameplaySystem() override = default;

      GameplaySystem(const GameplaySystem&) = default;
      GameplaySystem(GameplaySystem&&) = delete;
      auto operator=(const GameplaySystem&) -> GameplaySystem& = default;
      auto operator=(GameplaySystem&&) -> GameplaySystem& = delete;

      void update() override;
      void fixedUpdate() override;

      void setRenderDataTransferHandler(const RenderDataTransferHandler& handler) override;

    private:
      RenderDataTransferHandler transferHandler;
   };
}
