#pragma once

#include "gp/IGameplayContext.hpp"

namespace tr {

class GameplayContextImpl : public IGameplayContext {
public:
  explicit GameplayContextImpl(std::shared_ptr<IGameplaySystem> newGameplaySystem);
  ~GameplayContextImpl() override = default;

  GameplayContextImpl(const GameplayContextImpl&) = default;
  GameplayContextImpl(GameplayContextImpl&&) = delete;
  auto operator=(const GameplayContextImpl&) -> GameplayContextImpl& = default;
  auto operator=(GameplayContextImpl&&) -> GameplayContextImpl& = delete;

  auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> override;

private:
  std::shared_ptr<IGameplaySystem> gameplaySystem;
};

}
