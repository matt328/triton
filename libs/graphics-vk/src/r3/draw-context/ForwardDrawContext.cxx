#include "ForwardDrawContext.hpp"
#include "buffers/BufferSystem.hpp"
#include "task/Frame.hpp"

namespace tr {
ForwardDrawContext::ForwardDrawContext(ContextId newId,
                                       std::shared_ptr<BufferSystem> newBufferSystem,
                                       ForwardDrawContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto ForwardDrawContext::bind([[maybe_unused]] const Frame* frame,
                              vk::raii::CommandBuffer& commandBuffer,
                              const vk::raii::PipelineLayout& layout) -> void {
  const auto pushConstants = PushConstants{
      .objectDataBufferAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.objectData))
              .or_else([] {
                Log.warn("getBufferAddress could not find an address for objectData buffer");
                return std::optional<uint64_t>{0};
              })
              .value(),
      .regionDataAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.geometryRegion))
              .value_or(0L),
      .indexBufferAddress = bufferSystem->getBufferAddress(createInfo.indexData).value_or(0L),
      .positionBufferAddress =
          bufferSystem->getBufferAddress(createInfo.vertexPosition).value_or(0L),
      .colorBufferAddress = bufferSystem->getBufferAddress(createInfo.vertexColor).value_or(0L),
      .texCoordBufferAddress =
          bufferSystem->getBufferAddress(createInfo.vertexTexCoord).value_or(0L),
      .normalBufferAddress = bufferSystem->getBufferAddress(createInfo.vertexNormal).value_or(0L),
  };
  commandBuffer.pushConstants<PushConstants>(layout,
                                             vk::ShaderStageFlagBits::eVertex,
                                             0,
                                             pushConstants);
}

auto ForwardDrawContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer)
    -> void {

  commandBuffer.setViewportWithCount(createInfo.viewport);
  commandBuffer.setScissorWithCount(createInfo.scissor);

  const auto indirectCommandBuffer =
      bufferSystem->getVkBuffer(frame->getLogicalBuffer(createInfo.indirectCommand));
  const auto indirectCommandCountBuffer =
      bufferSystem->getVkBuffer(frame->getLogicalBuffer(createInfo.indirectCount));

  commandBuffer.drawIndirectCount(**indirectCommandBuffer,
                                  0,
                                  **indirectCommandCountBuffer,
                                  0,
                                  frame->getObjectCount(),
                                  sizeof(vk::DrawIndirectCommand));
}

auto ForwardDrawContext::getPushConstantSize() -> size_t {
  return sizeof(PushConstants);
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
