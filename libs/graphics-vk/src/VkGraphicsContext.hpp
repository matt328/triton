#pragma once

#include "fx/IGraphicsContext.hpp"

namespace tr {

class VkGraphicsContext : public IGraphicsContext {
public:
  explicit VkGraphicsContext(std::shared_ptr<IResourceProxy> newResourceProxy);
  ~VkGraphicsContext() override = default;

  VkGraphicsContext(const VkGraphicsContext&) = default;
  VkGraphicsContext(VkGraphicsContext&&) = delete;
  auto operator=(const VkGraphicsContext&) -> VkGraphicsContext& = default;
  auto operator=(VkGraphicsContext&&) -> VkGraphicsContext& = delete;

  auto getResourceProxy() -> std::shared_ptr<IResourceProxy> override;

private:
  std::shared_ptr<IResourceProxy> resourceProxy;
};

}
