#pragma once

#include "cm/RenderData.hpp"

namespace tr {

class IGameWorldSystem;

using GameObjectId = uint64_t;

/// Proxy object to allow framework to wire GameWorld and GraphicsEngine together.
class IGameWorldProxy {
public:
  IGameWorldProxy() = default;
  virtual ~IGameWorldProxy() = default;

  IGameWorldProxy(const IGameWorldProxy&) = default;
  IGameWorldProxy(IGameWorldProxy&&) = delete;
  auto operator=(const IGameWorldProxy&) -> IGameWorldProxy& = default;
  auto operator=(IGameWorldProxy&&) -> IGameWorldProxy& = delete;

  virtual void setRenderDataTransferHandler(const std::function<void(RenderData&)>& handler) = 0;

private:
  std::shared_ptr<IGameWorldSystem> gameWorldSystem;
};

}
