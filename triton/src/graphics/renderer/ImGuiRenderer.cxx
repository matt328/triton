#include "ImGuiRenderer.hpp"
#include "graphics/renderer/RendererBase.hpp"

ImGuiRenderer::ImGuiRenderer(const RendererBaseCreateInfo& createInfo) : RendererBase(createInfo) {
}

void ImGuiRenderer::fillCommandBuffer(const vk::raii::CommandBuffer& cmd, size_t currentImage) {
}

void ImGuiRenderer::update() {
}
