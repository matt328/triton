#include "ImGuiContext.hpp"

namespace tr {

ImGuiContext::ImGuiContext(ContextId newId,
                           std::shared_ptr<BufferSystem> newBufferSystem,
                           ImGuiContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto ImGuiContext::bind([[maybe_unused]] const Frame* frame,
                        [[maybe_unused]] vk::raii::CommandBuffer& commandBuffer,
                        [[maybe_unused]] const vk::raii::PipelineLayout& layout) -> void {
}

auto ImGuiContext::dispatch([[maybe_unused]] const Frame* frame,
                            vk::raii::CommandBuffer& commandBuffer) -> void {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // guiRenderer->render();

  ImGui::Render();

  commandBuffer.setViewportWithCount(createInfo.viewport);
  commandBuffer.setScissorWithCount(createInfo.scissor);

  auto* dd = ImGui::GetDrawData();
  if (dd != nullptr) {
    ImGui_ImplVulkan_RenderDrawData(dd, *commandBuffer);
  }
}

auto ImGuiContext::getPushConstantSize() -> size_t {
  return 0L;
}

[[nodiscard]] auto ImGuiContext::getGraphInfo() const -> PassGraphInfo {
  auto passGraphInfo = PassGraphInfo{};

  return passGraphInfo;
}

}
