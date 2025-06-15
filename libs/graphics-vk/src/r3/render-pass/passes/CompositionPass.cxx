#include "CompositionPass.hpp"
#include "bk/DebugPaths.hpp"
#include "img/ImageManager.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/render-pass/PipelineCreateInfo.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "task/Frame.hpp"

namespace tr {

CompositionPass::CompositionPass(std::shared_ptr<ImageManager> newImageManager,
                                 std::shared_ptr<ContextFactory> newDrawContextFactory,
                                 std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                                 std::shared_ptr<PipelineFactory> newPipelineFactory,
                                 CompositionPassCreateInfo createInfo,
                                 PassId newPassId)
    : imageManager{std::move(newImageManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      aliasRegistry{std::move(newAliasRegistry)},
      pipelineFactory{std::move(newPipelineFactory)},
      swapchainAlias{createInfo.swapchainImage},
      id{newPassId} {
  Log.trace("Creating CompositionPass");
  const auto swapchainImageHandle = aliasRegistry->getHandle(swapchainAlias);
  const auto colorFormats = {imageManager->getImageMetadata(swapchainImageHandle).format};

  const auto extent = imageManager->getImageMetadata(swapchainImageHandle).extent;

  const auto vertexStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .shaderFile = (getShaderRootPath() / "static.vert.spv").string(),
      .entryPoint = "main",
  };

  const auto fragmentStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .shaderFile = (getShaderRootPath() / "static.frag.spv").string(),
      .entryPoint = "main",
  };

  const auto pipelineLayoutInfo = PipelineLayoutInfo{
      .pushConstantInfoList = {PushConstantInfo{.stageFlags = vk::ShaderStageFlagBits::eVertex,
                                                .offset = 0,
                                                .size = 36}}};

  const auto pipelineCreateInfo =
      PipelineCreateInfo{.id = id,
                         .pipelineType = PipelineType::Graphics,
                         .pipelineLayoutInfo = pipelineLayoutInfo,
                         .colorAttachmentFormats = colorFormats,
                         .shaderStageInfo = {vertexStage, fragmentStage}};

  std::tie(this->pipelineLayout, this->pipeline) =
      pipelineFactory->createPipeline(pipelineCreateInfo);

  renderingInfo =
      vk::RenderingInfo{.renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0}, .extent = extent},
                        .layerCount = 1};

  colorAttachmentInfo = std::make_optional(vk::RenderingAttachmentInfo{
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{
              .color = vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}},
  });
}

[[nodiscard]] auto CompositionPass::getId() const -> PassId {
  return id;
}

auto CompositionPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {

  const auto swapchainHandle = aliasRegistry->getHandle(swapchainAlias);

  const auto logicalHandle = frame->getLogicalImage(swapchainHandle);
  const auto& image = imageManager->getImage(logicalHandle);

  colorAttachmentInfo->setImageView(image.getImageView());

  assert(colorAttachmentInfo->imageView);

  renderingInfo.setColorAttachmentCount(1);
  renderingInfo.setColorAttachments(*colorAttachmentInfo);

  cmdBuffer.beginRendering(renderingInfo);
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
  for (const auto& handle : drawableContexts) {
    const auto& drawContext = drawContextFactory->getDispatchContext(handle);
    drawContext->bind(frame, cmdBuffer, *pipelineLayout);
    drawContext->dispatch(frame, cmdBuffer);
  }
  cmdBuffer.endRendering();
}

auto CompositionPass::registerDispatchContext(Handle<IDispatchContext> handle) -> void {
  drawableContexts.push_back(handle);
}

[[nodiscard]] auto CompositionPass::getGraphInfo() const -> PassGraphInfo {
  auto graphInfo = PassGraphInfo{};

  for (const auto& handle : drawableContexts) {
    const auto& dispatchContext = drawContextFactory->getDispatchContext(handle);
    const auto contextInfo = dispatchContext->getGraphInfo();
    graphInfo.bufferReads.insert(contextInfo.bufferReads.begin(), contextInfo.bufferReads.end());
    graphInfo.bufferWrites.insert(contextInfo.bufferWrites.begin(), contextInfo.bufferWrites.end());
    graphInfo.imageReads.insert(contextInfo.imageReads.begin(), contextInfo.imageReads.end());
    graphInfo.imageWrites.insert(contextInfo.imageWrites.begin(), contextInfo.imageWrites.end());
  }

  return graphInfo;
}

}
