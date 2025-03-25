#pragma once

namespace tr {

class IResourceProxy;
class IRenderContext;
class IWindow;
class IGuiSystem;

class IGraphicsContext {
public:
  IGraphicsContext() = default;
  virtual ~IGraphicsContext() = default;

  IGraphicsContext(const IGraphicsContext&) = default;
  IGraphicsContext(IGraphicsContext&&) = delete;
  auto operator=(const IGraphicsContext&) -> IGraphicsContext& = default;
  auto operator=(IGraphicsContext&&) -> IGraphicsContext& = delete;

  virtual auto getResourceProxy() -> std::shared_ptr<IResourceProxy> = 0;
  virtual auto getRenderContext() -> std::shared_ptr<IRenderContext> = 0;
  virtual auto getWindow() -> std::shared_ptr<IWindow> = 0;
  virtual auto getGuiSystem() -> std::shared_ptr<IGuiSystem> = 0;
};

}
