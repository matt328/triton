#pragma once

#include "RendererBase.hpp"
#include <vulkan/vulkan_structs.hpp>

class Clear final : public RendererBase {
 public:
   explicit Clear(const RendererBaseCreateInfo& createInfo);

   void fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) override;

   void update() override;

 private:
   std::vector<std::unique_ptr<vk::raii::Framebuffer>> framebuffers;
   vk::Extent2D framebufferSize;
   std::unique_ptr<vk::raii::RenderPass> renderPass;
};
