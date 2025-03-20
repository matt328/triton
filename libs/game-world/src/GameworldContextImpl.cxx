#include "GameworldContextImpl.hpp"

namespace tr {
GameworldContextImpl::GameworldContextImpl(std::shared_ptr<IGameplaySystem> newGameplaySystem)
    : gameplaySystem{std::move(newGameplaySystem)} {
}

}
