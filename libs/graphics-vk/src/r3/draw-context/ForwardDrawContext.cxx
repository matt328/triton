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
  pgInfo.bufferReads = {BufferUsageInfo{
                            .alias = BufferAlias::IndirectCommand,
                            .accessFlags = vk::AccessFlagBits2::eIndirectCommandRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                        },
                        BufferUsageInfo{
                            .alias = BufferAlias::IndirectCommandCount,
                            .accessFlags = vk::AccessFlagBits2::eIndirectCommandRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eVertexShader,
                        },
                        BufferUsageInfo{
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
                            .alias = BufferAlias::IndexData,
                            .accessFlags = vk::AccessFlagBits2::eShaderRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                        },
                        BufferUsageInfo{
                            .alias = BufferAlias::VertexPositions,
                            .accessFlags = vk::AccessFlagBits2::eShaderRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                        },
                        BufferUsageInfo{
                            .alias = BufferAlias::VertexNormal,
                            .accessFlags = vk::AccessFlagBits2::eShaderRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                        },
                        BufferUsageInfo{
                            .alias = BufferAlias::VertexTexCoord,
                            .accessFlags = vk::AccessFlagBits2::eShaderRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                        },
                        BufferUsageInfo{
                            .alias = BufferAlias::VertexColor,
                            .accessFlags = vk::AccessFlagBits2::eShaderRead,
                            .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                        }};
  return pgInfo;
}
}
