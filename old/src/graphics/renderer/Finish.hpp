#pragma once

#include "graphics/renderer/RendererBase.hpp"

namespace Triton {

   class Finish : public RendererBase {
    public:
      explicit Finish(const RendererBaseCreateInfo& createInfo);

      void fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) override;

      void update() override;

    private:
      std::vector<std::unique_ptr<vk::raii::Framebuffer>> framebuffers;
      vk::Extent2D framebufferSize;
      std::unique_ptr<vk::raii::RenderPass> renderPass;
   };
}