#pragma once

#include "fx/IFrameworkContext.hpp"

namespace tr {
class FrameworkContextImpl : public IFrameworkContext {
public:
  explicit FrameworkContextImpl(std::shared_ptr<IGameLoop> newGameLoop);
  ~FrameworkContextImpl() override = default;

  FrameworkContextImpl(const FrameworkContextImpl&) = default;
  FrameworkContextImpl(FrameworkContextImpl&&) = delete;
  auto operator=(const FrameworkContextImpl&) -> FrameworkContextImpl& = default;
  auto operator=(FrameworkContextImpl&&) -> FrameworkContextImpl& = delete;

  auto getGameLoop() -> std::shared_ptr<IGameLoop> override;

private:
  std::shared_ptr<IGameLoop> gameLoop;
};
}
