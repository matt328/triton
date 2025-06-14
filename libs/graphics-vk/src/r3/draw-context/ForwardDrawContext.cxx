#include "ForwardDrawContext.hpp"

namespace tr {
ForwardDrawContext::ForwardDrawContext(ContextId newId,
                                       std::shared_ptr<BufferSystem> newBufferSystem,
                                       ForwardDrawContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto ForwardDrawContext::bind([[maybe_unused]] const Frame* frame,
                              [[maybe_unused]] vk::raii::CommandBuffer& commandBuffer,
                              [[maybe_unused]] const vk::raii::PipelineLayout& layout) -> void {
}

auto ForwardDrawContext::dispatch([[maybe_unused]] const Frame* frame,
                                  [[maybe_unused]] vk::raii::CommandBuffer& commandBuffer) -> void {
}

auto ForwardDrawContext::getPushConstantSize() -> size_t {
  return sizeof(0);
}

[[nodiscard]] auto ForwardDrawContext::getGraphInfo() const -> PassGraphInfo {
  auto pgInfo = PassGraphInfo{};

  pgInfo.imageWrites = {
      ImageUsageInfo{
          .alias = ImageAlias::GeometryColorImage,
          .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
          .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
          .aspectFlags = vk::ImageAspectFlagBits::eColor,
          .layout = vk::ImageLayout::eColorAttachmentOptimal,
          .clearValue = {vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}},
      },
      ImageUsageInfo{.alias = ImageAlias::DepthImage,
                     .accessFlags = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                     .stageFlags = vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                                   vk::PipelineStageFlagBits2::eLateFragmentTests,
                     .aspectFlags = vk::ImageAspectFlagBits::eDepth,
                     .layout = vk::ImageLayout::eDepthAttachmentOptimal,
                     .clearValue = vk::ClearValue{
                         .depthStencil = vk::ClearDepthStencilValue{.depth = 1.0f, .stencil = 0}}}};

  pgInfo.bufferReads = {BufferUsageInfo{
                            .alias = BufferAlias::IndirectCommand,
                            .accessFlags = vk::AccessFlagBits2::eIndirectCommandRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eDrawIndirect,
                        },
                        BufferUsageInfo{
                            .alias = BufferAlias::IndirectCommandCount,
                            .accessFlags = vk::AccessFlagBits2::eIndirectCommandRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eDrawIndirect,
                        },
                        BufferUsageInfo{
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
  return pgInfo;
}
}
