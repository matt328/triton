#pragma once

#include "graphics/renderer/RendererBase.hpp"

class ImGuiRenderer : public RendererBase {

 public:
   explicit ImGuiRenderer(const RendererBaseCreateInfo& createInfo);

   void fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) override;

   void update() override;

 private:
   const ImDrawData* drawData = nullptr;

   bool createDescriptorSet(const vk::raii::Device& device);
   bool createMultiDescriptorSet(const vk::raii::Device& device);
};