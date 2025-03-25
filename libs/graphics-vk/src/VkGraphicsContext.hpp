#pragma once

#include "fx/IGraphicsContext.hpp"

namespace tr {

class VkGraphicsContext : public IGraphicsContext {
public:
  explicit VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy,
                             std::shared_ptr<IRenderContext> newRenderContext,
                             std::shared_ptr<IWindow> newWindow,
                             std::shared_ptr<IGuiSystem> newGuiSystem);
  ~VkGraphicsContext() override = default;

  VkGraphicsContext(const VkGraphicsContext&) = default;
  VkGraphicsContext(VkGraphicsContext&&) = delete;
  auto operator=(const VkGraphicsContext&) -> VkGraphicsContext& = default;
  auto operator=(VkGraphicsContext&&) -> VkGraphicsContext& = delete;

  auto getResourceProxy() -> std::shared_ptr<IResourceProxy> override;
  auto getRenderContext() -> std::shared_ptr<IRenderContext> override;
  auto getWindow() -> std::shared_ptr<IWindow> override;
  auto getGuiSystem() -> std::shared_ptr<IGuiSystem> override;

private:
  std::shared_ptr<IResourceProxy> resourceProxy;
  std::shared_ptr<IRenderContext> renderContext;
  std::shared_ptr<IWindow> window;
  std::shared_ptr<IGuiSystem> guiSystem;
};

}
