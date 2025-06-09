#include "CullingPass.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/render-pass/PipelineFactory.hpp"

namespace tr {

const std::filesystem::path SHADER_ROOT = std::filesystem::current_path() / "assets" / "shaders";

CullingPass::CullingPass(std::shared_ptr<ContextFactory> newContextFactory,
                         std::shared_ptr<PipelineFactory> newPipelineFactory,
                         PassId newPassId)
    : contextFactory{std::move(newContextFactory)},
      pipelineFactory{std::move(newPipelineFactory)},
      id{newPassId} {

  const auto pipelineLayoutInfo =
      PipelineLayoutInfo{.pushConstantInfoList = {PushConstantInfo{
                             .stageFlags = vk::ShaderStageFlagBits::eCompute,
                             .offset = 0,
                             .size = 104, // TODO(matt) sizeof()
                         }}};

  const auto shaderStageInfo =
      ShaderStageInfo{.stage = vk::ShaderStageFlagBits::eCompute,
                      .shaderFile = (SHADER_ROOT / "compute2.comp.spv").string(),
                      .entryPoint = "main"};

  const auto pipelineCreateInfo = PipelineCreateInfo{.id = id,
                                                     .pipelineType = PipelineType::Compute,
                                                     .pipelineLayoutInfo = pipelineLayoutInfo,
                                                     .shaderStageInfo = {shaderStageInfo}};

  auto [layout, pipeline] = pipelineFactory->createPipeline(pipelineCreateInfo);
}

auto CullingPass::getId() const -> PassId {
  return id;
}

auto CullingPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  cmdBuffer.begin(vk::CommandBufferBeginInfo{});
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);

  for (const auto& handle : dispatchableContexts) {
    const auto& dispatchContext = contextFactory->getDispatchContext(handle);
    dispatchContext->bind(frame, cmdBuffer, *pipelineLayout);
    dispatchContext->dispatch(frame, cmdBuffer);
  }

  cmdBuffer.end();
}

auto CullingPass::registerDispatchContext(Handle<IDispatchContext> handle) -> void {
  dispatchableContexts.push_back(handle);
}

auto CullingPass::getGraphInfo() const -> PassGraphInfo {
}

}
