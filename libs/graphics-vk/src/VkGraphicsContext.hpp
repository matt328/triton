#pragma once

#include "fx/IGraphicsContext.hpp"

namespace tr {

class VkGraphicsContext : public IGraphicsContext {
public:
  VkGraphicsContext() = default;
  ~VkGraphicsContext() = default;

  VkGraphicsContext(const VkGraphicsContext&) = default;
  VkGraphicsContext(VkGraphicsContext&&) = delete;
  auto operator=(const VkGraphicsContext&) -> VkGraphicsContext& = default;
  auto operator=(VkGraphicsContext&&) -> VkGraphicsContext& = delete;
};

}
