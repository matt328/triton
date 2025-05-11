#pragma once

#include "api/fx/IGraphicsContext.hpp"

namespace tr {

class VkGraphicsContext : public IGraphicsContext {
public:
  explicit VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy,
                             std::shared_ptr<IRenderContext> newRenderContext,
                             std::shared_ptr<IWindow> newWindow);
  ~VkGraphicsContext() override = default;

  VkGraphicsContext(const VkGraphicsContext&) = default;
  VkGraphicsContext(VkGraphicsContext&&) = delete;
  auto operator=(const VkGraphicsContext&) -> VkGraphicsContext& = default;
  auto operator=(VkGraphicsContext&&) -> VkGraphicsContext& = delete;

  auto getResourceProxy() -> std::shared_ptr<IResourceProxy> override;
  auto getRenderContext() -> std::shared_ptr<IRenderContext> override;
  auto getWindow() -> std::shared_ptr<IWindow> override;

private:
  std::shared_ptr<IResourceProxy> resourceProxy;
  std::shared_ptr<IRenderContext> renderContext;
  std::shared_ptr<IWindow> window;
};

}
