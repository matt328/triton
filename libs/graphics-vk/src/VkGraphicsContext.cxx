#include "VkGraphicsContext.hpp"

namespace tr {

VkGraphicsContext::VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy,
                                     std::shared_ptr<IRenderContext> newRenderContext)
    : resourceProxy{std::move(newResourceProxy)}, renderContext{std::move(newRenderContext)} {
}

auto VkGraphicsContext::getResourceProxy() -> std::shared_ptr<IResourceProxy> {
  return resourceProxy;
}

auto VkGraphicsContext::getRenderContext() -> std::shared_ptr<IRenderContext> {
  return renderContext;
}

}
