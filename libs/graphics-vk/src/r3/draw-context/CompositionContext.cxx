#include "CompositionContext.hpp"

namespace tr {

CompositionContext::CompositionContext(ContextId newId,
                                       std::shared_ptr<BufferSystem> newBufferSystem,
                                       CompositionContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto CompositionContext::bind(const Frame* frame,
                              vk::raii::CommandBuffer& commandBuffer,
                              const vk::raii::PipelineLayout& layout) -> void {
}

auto CompositionContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer)
    -> void {
}

auto CompositionContext::getPushConstantSize() -> size_t {
}

auto CompositionContext::getGraphInfo() const -> PassGraphInfo {
  auto graphInfo = PassGraphInfo{};

  graphInfo.imageWrites = {ImageUsageInfo{
      .alias = ImageAlias::SwapchainImage,
      .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
      .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .layout = vk::ImageLayout::eColorAttachmentOptimal,
      .clearValue = {vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}},
  }};

  graphInfo.bufferReads = {BufferUsageInfo{
                               .alias = BufferAlias::ObjectData,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = BufferAlias::ObjectPositions,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = BufferAlias::ObjectRotations,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = BufferAlias::ObjectScales,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = BufferAlias::GeometryRegion,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = GlobalBufferAlias::Index,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = GlobalBufferAlias::Position,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = GlobalBufferAlias::Normal,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = GlobalBufferAlias::TexCoord,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           },
                           BufferUsageInfo{
                               .alias = GlobalBufferAlias::Color,
                               .accessFlags = vk::AccessFlagBits2::eShaderRead,
                               .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                           }};

  return graphInfo;
}

}
