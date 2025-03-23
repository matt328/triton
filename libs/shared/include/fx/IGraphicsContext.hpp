#pragma once

namespace tr {

class IResourceProxy;

class IGraphicsContext {
public:
  IGraphicsContext() = default;
  virtual ~IGraphicsContext() = default;

  IGraphicsContext(const IGraphicsContext&) = default;
  IGraphicsContext(IGraphicsContext&&) = delete;
  auto operator=(const IGraphicsContext&) -> IGraphicsContext& = default;
  auto operator=(IGraphicsContext&&) -> IGraphicsContext& = delete;

  virtual auto getResourceProxy() -> std::shared_ptr<IResourceProxy> = 0;
};

}
