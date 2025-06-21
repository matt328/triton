#include "CullingPass.hpp"
#include "bk/DebugPaths.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/render-pass/PipelineFactory.hpp"

namespace tr {

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
                      .shaderFile = (getShaderRootPath() / "compute2.comp.spv").string(),
                      .entryPoint = "main"};

  const auto pipelineCreateInfo = PipelineCreateInfo{.id = id,
                                                     .pipelineType = PipelineType::Compute,
                                                     .pipelineLayoutInfo = pipelineLayoutInfo,
                                                     .shaderStageInfo = {shaderStageInfo}};

  std::tie(this->pipelineLayout, this->pipeline) =
      pipelineFactory->createPipeline(pipelineCreateInfo);
}

auto CullingPass::getId() const -> PassId {
  return id;
}

auto CullingPass::execute(Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);

  for (const auto& handle : dispatchableContexts) {
    const auto& dispatchContext = contextFactory->getDispatchContext(handle);
    dispatchContext->bind(frame, cmdBuffer, *pipelineLayout);
    dispatchContext->dispatch(frame, cmdBuffer);
  }
}

auto CullingPass::registerDispatchContext(Handle<IDispatchContext> handle) -> void {
  dispatchableContexts.push_back(handle);
}

auto CullingPass::getGraphInfo() const -> PassGraphInfo {
  auto passGraphInfo = PassGraphInfo{};

  for (const auto& handle : dispatchableContexts) {
    const auto& dispatchContext = contextFactory->getDispatchContext(handle);
    const auto contextInfo = dispatchContext->getGraphInfo();
    passGraphInfo.bufferReads.insert(contextInfo.bufferReads.begin(),
                                     contextInfo.bufferReads.end());
    passGraphInfo.bufferWrites.insert(contextInfo.bufferWrites.begin(),
                                      contextInfo.bufferWrites.end());
    passGraphInfo.imageReads.insert(contextInfo.imageReads.begin(), contextInfo.imageReads.end());
    passGraphInfo.imageWrites.insert(contextInfo.imageWrites.begin(),
                                     contextInfo.imageWrites.end());
  }

  return passGraphInfo;
}

}
