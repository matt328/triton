#include "CompositionContext.hpp"
#include "task/Frame.hpp"
#include "vk/sb/IShaderBinding.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"

namespace tr {

CompositionContext::CompositionContext(
    ContextId newId,
    std::shared_ptr<BufferSystem> newBufferSystem,
    std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
    CompositionContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)},
      shaderBindingFactory{std::move(newShaderBindingFactory)},
      createInfo{newCreateInfo} {
}

auto CompositionContext::bind(const Frame* frame,
                              vk::raii::CommandBuffer& commandBuffer,
                              const vk::raii::PipelineLayout& layout) -> void {
  const auto sbHandle = frame->getLogicalShaderBinding(createInfo.defaultShaderBinding);
  const auto& shaderBinding = shaderBindingFactory->getShaderBinding(sbHandle);
  shaderBinding.bindToPipeline(commandBuffer, vk::PipelineBindPoint::eGraphics, 0, layout);
}

auto CompositionContext::dispatch([[maybe_unused]] const Frame* frame,
                                  vk::raii::CommandBuffer& commandBuffer) -> void {
  commandBuffer.setViewportWithCount(createInfo.viewport);
  commandBuffer.setScissorWithCount(createInfo.scissor);

  commandBuffer.draw(3, 1, 0, 0);
}

auto CompositionContext::getPushConstantSize() -> size_t {
  return 0;
}

auto CompositionContext::getGraphInfo() const -> PassGraphInfo {
  auto graphInfo = PassGraphInfo{};

  // TODO figure out why this doesn't need to be here
  // graphInfo.imageReads = {ImageUsageInfo{.alias = ImageAlias::GeometryColorImage,
  //                                        .accessFlags = vk::AccessFlagBits2::eShaderSampledRead,
  //                                        .stageFlags =
  //                                        vk::PipelineStageFlagBits2::eFragmentShader,
  //                                        .aspectFlags = vk::ImageAspectFlagBits::eColor,
  //                                        .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
  //                                        .clearValue = {}}};

  graphInfo.imageWrites = {ImageUsageInfo{
      .alias = ImageAlias::SwapchainImage,
      .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
      .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .layout = vk::ImageLayout::eColorAttachmentOptimal,
      .clearValue = {vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}},
  }};

  return graphInfo;
}

}
