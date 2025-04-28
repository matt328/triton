#include "dd/render-pass/RenderPassFactory.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageManager.hpp"
#include "pipeline/IShaderModuleFactory.hpp"
#include "vk/ComputePushConstants.hpp"

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

auto RenderPassFactory::createCullingPass() -> CullingPassInfo {
  const auto pushConstantRange = vk::PushConstantRange{
      .stageFlags = vk::ShaderStageFlagBits::eCompute,
      .offset = 0,
      .size = sizeof(ComputePushConstants),
  };

  const auto pipelineLayoutCreateInfo =
      vk::PipelineLayoutCreateInfo{.pushConstantRangeCount = 1,
                                   .pPushConstantRanges = &pushConstantRange};

  const auto pipelineLayout =
      std::make_unique<vk::raii::PipelineLayout>(device->getVkDevice(), pipelineLayoutCreateInfo);

  auto csm = shaderModuleFactory->createShaderModule(vk::ShaderStageFlagBits::eCompute, ShaderFile);
  auto shaderStage = vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eCompute,
                                                       .module = *csm,
                                                       .pName = "main"};

  const auto pipelineCreateInfo = vk::ComputePipelineCreateInfo{
      .stage = shaderStage,
      .layout = *pipelineLayout,
  };

  const auto pipeline = std::make_unique<vk::raii::Pipeline>(device->getVkDevice(),
                                                             VK_NULL_HANDLE,
                                                             pipelineCreateInfo);

  auto computePassConfig = ComputePassConfig{.pipeline = std::move(pipeline),
                                             .pipelineLayout = std::move(pipelineLayout)};

  const auto cullingHandle = computePassGenerator.requestHandle();
  computePassMap.emplace(cullingHandle,
                         std::make_unique<ComputePass>(std::move(computePassConfig), imageManager));

  const auto objectDataHandle =
      bufferRegistry->

      return CullingPassInfo{.handle = cullingHandle, .objectData = objectDataHandle};
}

auto RenderPassFactory::createGeometryPass() -> GeometryPassInfo {
}

auto RenderPassFactory::createLightingPass() -> LightingPassInfo {
}

auto RenderPassFactory::createCompositePass() -> CompositePassInfo {
}

auto RenderPassFactory::registerDrawContext(const RenderConfig& renderConfig,
                                            DrawContext* drawContext) -> void {
}

auto RenderPassFactory::createRenderPass(GraphicsPassCreateInfo& createInfo)
    -> std::unique_ptr<GraphicsPass> {
  // Pipeline Layout
  const auto& layoutInfo = createInfo.pipelineLayoutInfo;

  // Push Constants
  std::vector<vk::PushConstantRange> pushConstantRanges{};
  pushConstantRanges.reserve(layoutInfo.pushConstantInfoList.size());
  for (const auto& pcrInfo : layoutInfo.pushConstantInfoList) {
    pushConstantRanges.emplace_back(vk::PushConstantRange{
        .stageFlags = pcrInfo.stages,
        .offset = pcrInfo.offset,
        .size = pcrInfo.size,
    });
  }

  // DescriptorSetLayouts are loaded up by the renderer itself
  const auto layoutCreateInfo = vk::PipelineLayoutCreateInfo{
      .setLayoutCount = static_cast<uint32_t>(layoutInfo.descriptorSetLayouts.size()),
      .pSetLayouts = layoutInfo.descriptorSetLayouts.data(),
      .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
      .pPushConstantRanges = pushConstantRanges.data()};

  auto pipelineLayout = vk::raii::PipelineLayout{device->getVkDevice(), layoutCreateInfo};

  auto pipelineRenderingInfo = vk::PipelineRenderingCreateInfo{
      .colorAttachmentCount = static_cast<uint32_t>(createInfo.colorAttachmentFormats.size()),
      .pColorAttachmentFormats = createInfo.colorAttachmentFormats.data()};
  if (createInfo.depthAttachmentFormat) {
    pipelineRenderingInfo.setDepthAttachmentFormat(*createInfo.depthAttachmentFormat);
  }

  // Shader Stages
  auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{};
  for (const auto& stageInfo : createInfo.shaderStageInfo) {
    auto shaderModule =
        shaderModuleFactory->createShaderModule(stageInfo.stage, stageInfo.shaderFile);
    shaderStages.emplace_back(
        vk::PipelineShaderStageCreateInfo{.stage = stageInfo.stage,
                                          .module = *shaderModule,
                                          .pName = stageInfo.entryPoint.c_str()});
  }

  // VertexInputState
  const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
      .vertexBindingDescriptionCount = static_cast<uint32_t>(createInfo.vertexBindings.size()),
      .pVertexBindingDescriptions = createInfo.vertexBindings.data(),
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(createInfo.vertexAttributes.size()),
      .pVertexAttributeDescriptions = createInfo.vertexAttributes.data()};

  // Input Assembly
  const auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
      .topology = createInfo.topology,
      .primitiveRestartEnable = VK_FALSE,
  };

  // Rasterizer
  const auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
      .polygonMode = createInfo.polygonMode,
      .cullMode = createInfo.cullMode,
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
      .depthTestEnable = static_cast<vk::Bool32>(createInfo.enableDepthTest),
      .depthWriteEnable = static_cast<vk::Bool32>(createInfo.enableDepthWrite),
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
  auto graphicsPassConfig = GraphicsPassConfig{.pipeline = std::move(pipeline),
                                               .pipelineLayout = std::move(pipelineLayout)};

  return std::make_unique<GraphicsPass>(std::move(graphicsPassConfig), imageManager);
}

}
