#include "ImGuiRenderer.hpp"
#include "graphics/renderer/RendererBase.hpp"

namespace Triton {

   ImGuiRenderer::ImGuiRenderer([[maybe_unused]] const RendererBaseCreateInfo& createInfo) {
   }

   void ImGuiRenderer::fillCommandBuffer([[maybe_unused]] const vk::raii::CommandBuffer& cmd,
                                         [[maybe_unused]] size_t currentImage) {
   }

   void ImGuiRenderer::update() {
   }
}