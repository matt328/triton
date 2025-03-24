#pragma once

#include "fx/IGraphicsContext.hpp"

namespace tr {

class VkGraphicsContext : public IGraphicsContext {
public:
  explicit VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy,
                             std::shared_ptr<IRenderContext> newRenderContext);
  ~VkGraphicsContext() override = default;

  VkGraphicsContext(const VkGraphicsContext&) = default;
  VkGraphicsContext(VkGraphicsContext&&) = delete;
  auto operator=(const VkGraphicsContext&) -> VkGraphicsContext& = default;
  auto operator=(VkGraphicsContext&&) -> VkGraphicsContext& = delete;

  auto getResourceProxy() -> std::shared_ptr<IResourceProxy> override;
  auto getRenderContext() -> std::shared_ptr<IRenderContext> override;

private:
  std::shared_ptr<IResourceProxy> resourceProxy;
  std::shared_ptr<IRenderContext> renderContext;
};

}
