#include "VkGraphicsContext.hpp"

namespace tr {

VkGraphicsContext::VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy)
    : resourceProxy{std::move(newResourceProxy)} {
}

auto VkGraphicsContext::getResourceProxy() -> std::shared_ptr<IResourceProxy> {
  return resourceProxy;
}

}
