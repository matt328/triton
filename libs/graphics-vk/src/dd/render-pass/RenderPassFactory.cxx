#include "dd/render-pass/RenderPassFactory.hpp"
#include "dd/render-pass/RenderPass.hpp"
#include "dd/render-pass/RenderPassConfig.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageManager.hpp"
#include "pipeline/IShaderModuleFactory.hpp"
#include <ranges>

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                                     std::shared_ptr<ImageManager> newImageManager,
                                     std::shared_ptr<IShaderModuleFactory> newShaderModuleFactory,
                                     std::shared_ptr<Device> newDevice)
    : frameManager{std::move(newFrameManager)},
      imageManager{std::move(newImageManager)},
      shaderModuleFactory{std::move(newShaderModuleFactory)},
      device{std::move(newDevice)} {
}

auto RenderPassFactory::createRenderPass(RenderPassCreateInfo& info)
    -> std::unique_ptr<RenderPass> {
  RenderPassConfig config{};

  if (info.passType == RenderPassType::Compute) {
    auto pushConstantRanges = info.computePipelineInfo->layout.pushConstantRanges |
                              std::views::transform([](const auto& rangeConfig) {
                                return vk::PushConstantRange{.stageFlags = rangeConfig.stages,
                                                             .offset = rangeConfig.offset,
                                                             .size = rangeConfig.size};
                              }) |
                              std::ranges::to<std::vector>();

    // Create pipeline layout
    auto layoutCreateInfo =
        vk::PipelineLayoutCreateInfo{}.setPushConstantRanges(pushConstantRanges);

    auto pipelineLayout = vk::raii::PipelineLayout{device->getVkDevice(), layoutCreateInfo};

    // Create shader stage
    const auto& stageInfo = info.computePipelineInfo->shader;
    auto shaderModule =
        shaderModuleFactory->createShaderModule(stageInfo.stage, stageInfo.shaderFile);
    auto shaderStageCreateInfo =
        vk::PipelineShaderStageCreateInfo{.stage = stageInfo.stage,
                                          .module = *shaderModule,
                                          .pName = stageInfo.entryPoint.c_str()};

    // Create pipeline
    auto pipelineCreateInfo =
        vk::ComputePipelineCreateInfo{.stage = shaderStageCreateInfo, .layout = *pipelineLayout};

    auto pipeline = vk::raii::Pipeline{device->getVkDevice(), VK_NULL_HANDLE, pipelineCreateInfo};

    // Move into final config
    config.computeConfig = ComputePipelineConfig{.pipeline = std::move(pipeline),
                                                 .pipelineLayout = std::move(pipelineLayout)};
  } else {
  }

  std::ranges::copy(config.colorAttachmentConfigs, std::back_inserter(info.colorAttachments));

  if (info.depthAttachment) {
    config.depthAttachmentConfig = info.depthAttachment;
  }

  config.extent = info.extent;

  return std::make_unique<RenderPass>(std::move(config), imageManager);
}

}
