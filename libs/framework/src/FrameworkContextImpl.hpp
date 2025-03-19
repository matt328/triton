#pragma once

#include "fx/IFrameworkContext.hpp"

namespace tr {
class FrameworkContextImpl : public IFrameworkContext {
public:
  FrameworkContextImpl(std::shared_ptr<IGameLoop> newGameLoop,
                       std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar);
  ~FrameworkContextImpl() override = default;

  FrameworkContextImpl(const FrameworkContextImpl&) = default;
  FrameworkContextImpl(FrameworkContextImpl&&) = delete;
  auto operator=(const FrameworkContextImpl&) -> FrameworkContextImpl& = default;
  auto operator=(FrameworkContextImpl&&) -> FrameworkContextImpl& = delete;

  auto getGameLoop() -> std::shared_ptr<IGameLoop> override;
  auto getGuiCallbackRegistrar() -> std::shared_ptr<IGuiCallbackRegistrar> override;

private:
  std::shared_ptr<IGameLoop> gameLoop;
  std::shared_ptr<IGuiCallbackRegistrar> guiCallbackRegistrar;
};
}
