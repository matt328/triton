#include "VkGraphicsContext.hpp"

namespace tr {

VkGraphicsContext::VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy,
                                     std::shared_ptr<IRenderContext> newRenderContext,
                                     std::shared_ptr<IWindow> newWindow,
                                     std::shared_ptr<IGuiSystem> newGuiSystem)
    : resourceProxy{std::move(newResourceProxy)},
      renderContext{std::move(newRenderContext)},
      window{std::move(newWindow)},
      guiSystem{std::move(newGuiSystem)} {
}

auto VkGraphicsContext::getResourceProxy() -> std::shared_ptr<IResourceProxy> {
  return resourceProxy;
}

auto VkGraphicsContext::getRenderContext() -> std::shared_ptr<IRenderContext> {
  return renderContext;
}

auto VkGraphicsContext::getWindow() -> std::shared_ptr<IWindow> {
  return window;
}

auto VkGraphicsContext::getGuiSystem() -> std::shared_ptr<IGuiSystem> {
  return guiSystem;
}

}
