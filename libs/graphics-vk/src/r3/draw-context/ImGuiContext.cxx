#include "ImGuiContext.hpp"
#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "bk/Chrono.h"
#include "task/Frame.hpp"

namespace tr {

ImGuiContext::ImGuiContext(ContextId newId,
                           std::shared_ptr<BufferSystem> newBufferSystem,
                           std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                           ImGuiContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)},
      guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)},
      createInfo{newCreateInfo} {
  guiCallbackRegistrar->ready();
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

  guiCallbackRegistrar->render(frame->getEditorState());

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

  passGraphInfo.imageWrites = {ImageUsageInfo{
      .alias = ImageAlias::GuiColorImage,
      .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
      .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .layout = vk::ImageLayout::eColorAttachmentOptimal,
      .clearValue = {vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}},
  }};

  return passGraphInfo;
}

}
