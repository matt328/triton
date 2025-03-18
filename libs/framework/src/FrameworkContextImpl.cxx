#include "FrameworkContextImpl.hpp"

namespace tr {

FrameworkContextImpl::FrameworkContextImpl(std::shared_ptr<IGameLoop> newGameLoop)
    : gameLoop{std::move(newGameLoop)} {
}

auto FrameworkContextImpl::getGameLoop() -> std::shared_ptr<IGameLoop> {
  return gameLoop;
}

}
