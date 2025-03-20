#pragma once

#include "fx/IGameworldContext.hpp"

namespace tr {

class IGameplaySystem;

class GameworldContextImpl : public IGameworldContext {
public:
  explicit GameworldContextImpl(std::shared_ptr<IGameplaySystem> newGameplaySystem);
  ~GameworldContextImpl() override = default;

  GameworldContextImpl(const GameworldContextImpl&) = default;
  GameworldContextImpl(GameworldContextImpl&&) = delete;
  auto operator=(const GameworldContextImpl&) -> GameworldContextImpl& = default;
  auto operator=(GameworldContextImpl&&) -> GameworldContextImpl& = delete;

  auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> override;
  auto getEntityServiceProvider() -> std::shared_ptr<IEntityServiceProvider> override;

private:
  std::shared_ptr<IEntityServiceProvider> entityServiceProvider;
  std::shared_ptr<IGameplaySystem> gameplaySystem;
};

}
