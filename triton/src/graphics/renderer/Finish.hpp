#pragma once

#include "graphics/renderer/RendererBase.hpp"

class Finish : public RendererBase {
 public:
   explicit Finish(const RendererBaseCreateInfo& createInfo);

   void fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) override;

   void update() override;
};
