#pragma once

#include "RendererBase.h"

class Clear final : public RendererBase {
 public:
  explicit Clear(const RendererBaseCreateInfo& createInfo);

  void fillCommandBuffer(vk::raii::CommandBuffer& cmd, size_t currentImage) override;
};
