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
    // Pipeline Layout
    const auto& layoutInfo = info.graphicsPipelineInfo->layout;

    // Push Constants
    auto pushConstantRanges = layoutInfo.pushConstantRanges |
                              std::views::transform([](const auto& rangeConfig) {
                                return vk::PushConstantRange{.stageFlags = rangeConfig.stages,
                                                             .offset = rangeConfig.offset,
                                                             .size = rangeConfig.size};
                              }) |
                              std::ranges::to<std::vector>();

    // DescriptorSetLayouts are loaded up by the renderer itself
    const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo{
        .setLayoutCount = static_cast<uint32_t>(layoutInfo.descriptorSetLayouts.size()),
        .pSetLayouts = layoutInfo.descriptorSetLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
        .pPushConstantRanges = pushConstantRanges.data()};
    auto pipelineLayout = vk::raii::PipelineLayout{device->getVkDevice(), layoutCreateInfo};

    const auto& attachmentInfo = info.graphicsPipelineInfo->attachmentInfo;
    auto pipelineRenderingInfo = vk::PipelineRenderingCreateInfo{
        .colorAttachmentCount = static_cast<uint32_t>(attachmentInfo.colorAttachmentFormats.size()),
        .pColorAttachmentFormats = attachmentInfo.colorAttachmentFormats.data()};
    if (attachmentInfo.depthAttachmentFormat) {
      pipelineRenderingInfo.setDepthAttachmentFormat(*attachmentInfo.depthAttachmentFormat);
    }

    // Shader Stages
    auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{};
    for (const auto& stageInfo : info.graphicsPipelineInfo->shaderStageInfo) {
      auto shaderModule =
          shaderModuleFactory->createShaderModule(stageInfo.stage, stageInfo.shaderFile);
      shaderStages.emplace_back(
          vk::PipelineShaderStageCreateInfo{.stage = stageInfo.stage,
                                            .module = *shaderModule,
                                            .pName = stageInfo.entryPoint.c_str()});
    }

    // VertexInputState
    const auto& pipelineInfo = *info.graphicsPipelineInfo;
    const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
        .vertexBindingDescriptionCount = static_cast<uint32_t>(pipelineInfo.vertexBindings.size()),
        .pVertexBindingDescriptions = pipelineInfo.vertexBindings.data(),
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(pipelineInfo.vertexAttributes.size()),
        .pVertexAttributeDescriptions = pipelineInfo.vertexAttributes.data()};

    // Input Assembly
    const auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
        .topology = pipelineInfo.topology,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Rasterizer
    const auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
        .polygonMode = pipelineInfo.polygonMode,
        .cullMode = pipelineInfo.cullMode,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .lineWidth = 1.f,
    };

    // Multisampling
    constexpr auto multisampling =
        vk::PipelineMultisampleStateCreateInfo{.rasterizationSamples = vk::SampleCountFlagBits::e1,
                                               .sampleShadingEnable = VK_FALSE,
                                               .minSampleShading = 1.f};

    // Depth Stencil
    const auto depthStencil = vk::PipelineDepthStencilStateCreateInfo{
        .depthTestEnable = static_cast<vk::Bool32>(pipelineInfo.enableDepthTest),
        .depthWriteEnable = static_cast<vk::Bool32>(pipelineInfo.enableDepthWrite),
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE};

    // Color Blending
    constexpr auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
    const auto colorBlending =
        vk::PipelineColorBlendStateCreateInfo{.logicOpEnable = VK_FALSE,
                                              .logicOp = vk::LogicOp::eCopy,
                                              .attachmentCount = 1,
                                              .pAttachments = &colorBlendAttachment};

    // Dynamic State
    constexpr auto dynamicStates = std::array<vk::DynamicState, 2>{
        vk::DynamicState::eViewportWithCount,
        vk::DynamicState::eScissorWithCount,
    };

    const auto dynamicStateInfo =
        vk::PipelineDynamicStateCreateInfo{.dynamicStateCount = 2,
                                           .pDynamicStates = dynamicStates.data()};

    // Pipeline Itself
    const auto pipelineCreateInfo =
        vk::GraphicsPipelineCreateInfo{.pNext = &pipelineRenderingInfo,
                                       .stageCount = static_cast<uint32_t>(shaderStages.size()),
                                       .pStages = shaderStages.data(),
                                       .pVertexInputState = &vertexInputStateCreateInfo,
                                       .pInputAssemblyState = &inputAssembly,
                                       .pRasterizationState = &rasterizer,
                                       .pMultisampleState = &multisampling,
                                       .pDepthStencilState = &depthStencil,
                                       .pColorBlendState = &colorBlending,
                                       .pDynamicState = &dynamicStateInfo,
                                       .layout = *pipelineLayout,
                                       .subpass = 0,
                                       .basePipelineHandle = VK_NULL_HANDLE,
                                       .basePipelineIndex = -1};
    auto pipeline = vk::raii::Pipeline{device->getVkDevice(), VK_NULL_HANDLE, pipelineCreateInfo};

    // Move into final config
    config.graphicsConfig = GraphicsPipelineConfig{.pipeline = std::move(pipeline),
                                                   .pipelineLayout = std::move(pipelineLayout)};
  }

  return std::make_unique<RenderPass>(std::move(config), imageManager);
}

}
