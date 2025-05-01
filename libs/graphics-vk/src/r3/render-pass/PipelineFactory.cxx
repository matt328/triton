#include "PipelineFactory.hpp"
#include "pipeline/IShaderModuleFactory.hpp"
#include "vk/core/Device.hpp"

namespace tr {

PipelineFactory::PipelineFactory(std::shared_ptr<Device> newDevice,
                                 std::shared_ptr<IShaderModuleFactory> newShaderModuleFactory)
    : device{std::move(newDevice)}, shaderModuleFactory{std::move(newShaderModuleFactory)} {
}

auto PipelineFactory::createPipeline(const PipelineCreateInfo& createInfo)
    -> std::tuple<vk::raii::PipelineLayout, vk::raii::Pipeline> {
  if (createInfo.pipelineType == PipelineType::Graphics) {
    return createGraphicsPipeline(createInfo);
  }
  return createComputePipeline(createInfo);
}

auto PipelineFactory::createGraphicsPipeline(const PipelineCreateInfo& createInfo)
    -> std::tuple<vk::raii::PipelineLayout, vk::raii::Pipeline> {
  const auto& layoutInfo = createInfo.pipelineLayoutInfo;

  // Push Constants
  std::vector<vk::PushConstantRange> pushConstantRanges{};
  pushConstantRanges.reserve(layoutInfo.pushConstantInfoList.size());
  for (const auto& pcrInfo : layoutInfo.pushConstantInfoList) {
    pushConstantRanges.emplace_back(vk::PushConstantRange{
        .stageFlags = pcrInfo.stageFlags,
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
  auto shaderModules = std::vector<vk::raii::ShaderModule>{};
  auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{};
  for (const auto& stageInfo : createInfo.shaderStageInfo) {
    shaderModules.emplace_back(
        shaderModuleFactory->createShaderModule(stageInfo.stage, stageInfo.shaderFile));
    shaderStages.emplace_back(
        vk::PipelineShaderStageCreateInfo{.stage = stageInfo.stage,
                                          .module = *shaderModules.back(),
                                          .pName = stageInfo.entryPoint.c_str()});
  }

  // VertexInputState
  const auto vertexInputStateCreateInfo =
      vk::PipelineVertexInputStateCreateInfo{.vertexBindingDescriptionCount = 0,
                                             .pVertexBindingDescriptions = nullptr,
                                             .vertexAttributeDescriptionCount = 0,
                                             .pVertexAttributeDescriptions = nullptr};

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

  return {nullptr, nullptr};
}

auto PipelineFactory::createComputePipeline(const PipelineCreateInfo& createInfo)
    -> std::tuple<vk::raii::PipelineLayout, vk::raii::Pipeline> {
  return {nullptr, nullptr};
}
}
