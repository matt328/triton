#include "CullingDispatchContext.hpp"
#include "buffers/BufferSystem.hpp"
#include "task/Frame.hpp"

namespace tr {

constexpr uint32_t WorkgroupSize = 64;

CullingDispatchContext::CullingDispatchContext(ContextId newId,
                                               std::shared_ptr<BufferSystem> newBufferSystem,
                                               CullingDispatchContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto CullingDispatchContext::bind(const Frame* frame,
                                  vk::raii::CommandBuffer& commandBuffer,
                                  const vk::raii::PipelineLayout& layout) -> void {
  const auto pushConstants = PushConstants{
      .objectCount = frame->getObjectCount(),
      .objectDataAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.objectData))
              .or_else([] {
                // TODO(matt): Figure out how to handle this gracefully
                Log.warn("getBufferAddress could not find an address for objectData buffer");
                return std::optional<uint64_t>{0};
              })
              .value(),
      .objectPositionsAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.objectPositions))
              .value_or(0L),
      .objectRotationsAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.objectRotations))
              .value_or(0L),
      .objectScalesAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.objectScales))
              .value_or(0L),
      .outputIndirectCommandAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.indirectCommand))
              .value_or(0L),
      .outputIndirectCountAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.indirectCount))
              .value_or(0L),
      .geometryRegionAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.geometryRegion))
              .value_or(0L),
      .indexDataAddress = bufferSystem->getBufferAddress(createInfo.indexData).value_or(0L),
      .vertexPositionAddress =
          bufferSystem->getBufferAddress(createInfo.vertexPosition).value_or(0L),
      .vertexNormalAddress = bufferSystem->getBufferAddress(createInfo.vertexNormal).value_or(0L),
      .vertexTexCoordAddress =
          bufferSystem->getBufferAddress(createInfo.vertexTexCoord).value_or(0L),
      .vertexColorAddress = bufferSystem->getBufferAddress(createInfo.vertexColor).value_or(0L),
  };
  commandBuffer.pushConstants<PushConstants>(layout,
                                             vk::ShaderStageFlagBits::eCompute,
                                             0,
                                             pushConstants);
}

auto CullingDispatchContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer)
    -> void {
  uint32_t workgroupCount = (frame->getObjectCount() + WorkgroupSize - 1) / WorkgroupSize;
  commandBuffer.dispatch(workgroupCount, 1, 1);
}

auto CullingDispatchContext::getPushConstantSize() -> size_t {
  return sizeof(PushConstants);
}

[[nodiscard]] auto CullingDispatchContext::getGraphInfo() const -> PassGraphInfo {
  auto passGraphInfo = PassGraphInfo{
      .bufferWrites =
          {
              BufferUsageInfo{
                  .alias = BufferAlias::IndirectCommand,
                  .accessFlags = vk::AccessFlagBits2::eShaderWrite,
                  .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
              },
              BufferUsageInfo{
                  .alias = BufferAlias::IndirectCommandCount,
                  .accessFlags = vk::AccessFlagBits2::eShaderWrite,
                  .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
              },
          },
      .bufferReads = {BufferUsageInfo{
                          .alias = BufferAlias::ObjectData,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = BufferAlias::ObjectPositions,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = BufferAlias::ObjectRotations,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = BufferAlias::ObjectScales,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = BufferAlias::GeometryRegion,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = GlobalBufferAlias::Index,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = GlobalBufferAlias::Position,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = GlobalBufferAlias::Normal,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = GlobalBufferAlias::TexCoord,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      },
                      BufferUsageInfo{
                          .alias = GlobalBufferAlias::Color,
                          .accessFlags = vk::AccessFlagBits2::eShaderRead,
                          .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                      }},
  };
  return passGraphInfo;
}
}
