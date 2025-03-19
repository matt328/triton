#include "FrameworkContextImpl.hpp"

namespace tr {

FrameworkContextImpl::FrameworkContextImpl(
    std::shared_ptr<IGameLoop> newGameLoop,
    std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar)
    : gameLoop{std::move(newGameLoop)},

      guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)} {
}

auto FrameworkContextImpl::getGameLoop() -> std::shared_ptr<IGameLoop> {
  return gameLoop;
}

auto FrameworkContextImpl::getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> {
  return guiCallbackRegistrar;
}

}
