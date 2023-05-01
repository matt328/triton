#pragma once

#include "RendererBase.hpp"
#include "graphics/Swapchain.hpp"

class Clear final : public RendererBase {
 public:
   explicit Clear(Swapchain& swapchain);

   ~Clear() override;

   [[nodiscard]] const std::string getName() const override {
      return "clear";
   };

   void fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) override;

   void update() override;

   void resetFramebuffers(const FramebufferInfo& info) override;

 private:
   std::unique_ptr<vk::raii::RenderPass> renderPass;
};
