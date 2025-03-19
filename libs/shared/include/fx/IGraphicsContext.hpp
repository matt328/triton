#pragma once

namespace tr {

class IGraphicsContext {
public:
  IGraphicsContext() = default;
  virtual ~IGraphicsContext() = default;

  IGraphicsContext(const IGraphicsContext&) = default;
  IGraphicsContext(IGraphicsContext&&) = delete;
  auto operator=(const IGraphicsContext&) -> IGraphicsContext& = default;
  auto operator=(IGraphicsContext&&) -> IGraphicsContext& = delete;
};

}
