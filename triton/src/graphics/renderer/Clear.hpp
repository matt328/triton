#pragma once

#include "RendererBase.hpp"

class Clear final : public RendererBase {
 public:
   explicit Clear(const RendererBaseCreateInfo& createInfo);

   void fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) override;

   void update() override;
};