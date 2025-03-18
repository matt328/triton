#include "GameplayContextImpl.hpp"

namespace tr {

GameplayContextImpl::GameplayContextImpl(std::shared_ptr<IGameplaySystem> newGameplaySystem)
    : gameplaySystem{std::move(newGameplaySystem)} {
}

auto GameplayContextImpl::getGameplaySystem() -> std::shared_ptr<IGameplaySystem> {
  return gameplaySystem;
}

}
