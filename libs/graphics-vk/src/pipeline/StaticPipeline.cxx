#include "StaticPipeline.hpp"
#include "as/StaticVertex.hpp"
#include "vk/StaticPushConstants.hpp"
#include "VkResourceManager.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace tr {

const auto VertexShaderFile = SHADERS / "static.vert.spv";
const auto FragmentShaderFile = SHADERS / "static.frag.spv";

StaticPipeline::StaticPipeline(const std::shared_ptr<Device>& device,
                               const std::shared_ptr<IShaderModuleFactory>& shaderCompiler,
                               const std::shared_ptr<VkResourceManager>& resourceManager) {
  Log.trace("Constructing StaticPipeline");

  // Pipeline Layout
  const auto pushConstantRange = vk::PushConstantRange{
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0,
      .size = sizeof(StaticPushConstants),
  };

  const auto pipelineLayoutCreateInfo =
      vk::PipelineLayoutCreateInfo{.setLayoutCount = 1,
                                   .pSetLayouts = resourceManager->getTextureDSL(),
                                   .pushConstantRangeCount = 1,
                                   .pPushConstantRanges = &pushConstantRange};
  pipelineLayout =
      std::make_unique<vk::raii::PipelineLayout>(device->getVkDevice(), pipelineLayoutCreateInfo);

  // PipelineRenderingInfo
  constexpr auto colorAttachmentFormat = vk::Format::eR16G16B16A16Sfloat;
  auto pipelineRenderingInfo =
      vk::PipelineRenderingCreateInfo{.colorAttachmentCount = 1,
                                      .pColorAttachmentFormats = &colorAttachmentFormat,
                                      .depthAttachmentFormat = vk::Format::eD32Sfloat};

  // Shader Stages
  auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{};
  auto vsm = shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eVertex, VertexShaderFile);
  auto fsm =
      shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eFragment, FragmentShaderFile);
  shaderStages.emplace_back(
      vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                        .module = *vsm,
                                        .pName = "main"});
  shaderStages.emplace_back(
      vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                        .module = *fsm,
                                        .pName = "main"});

  // Vertex Input State
  const auto vertexAttributeDescriptions = std::array<vk::VertexInputAttributeDescription, 2>{
      vk::VertexInputAttributeDescription{.location = 0,
                                          .binding = 0,
                                          .format = vk::Format::eR32G32B32Sfloat},
      vk::VertexInputAttributeDescription{.location = 1,
                                          .binding = 0,
                                          .format = vk::Format::eR32G32Sfloat,
                                          .offset = offsetof(as::StaticVertex, texCoord)}};

  constexpr auto bindingDescription =
      vk::VertexInputBindingDescription{.binding = 0,
                                        .stride = sizeof(as::StaticVertex),
                                        .inputRate = vk::VertexInputRate::eVertex};

  const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size()),
      .pVertexAttributeDescriptions = vertexAttributeDescriptions.data()};

  // Input Assembly
  constexpr auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
      .topology = vk::PrimitiveTopology::eTriangleList,
      .primitiveRestartEnable = VK_FALSE,
  };

  // Rasterizer
  constexpr auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .lineWidth = 1.f,
  };

  // Multisampling
  constexpr auto multisampling =
      vk::PipelineMultisampleStateCreateInfo{.rasterizationSamples = vk::SampleCountFlagBits::e1,
                                             .sampleShadingEnable = VK_FALSE,
                                             .minSampleShading = 1.f};

  // Depth Stencil
  constexpr auto depthStencil =
      vk::PipelineDepthStencilStateCreateInfo{.depthTestEnable = VK_TRUE,
                                              .depthWriteEnable = VK_TRUE,
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

  pipeline = std::make_unique<vk::raii::Pipeline>(device->getVkDevice(),
                                                  VK_NULL_HANDLE,
                                                  pipelineCreateInfo);
}

auto StaticPipeline::getPipeline() const -> vk::Pipeline {
  return **pipeline;
}
auto StaticPipeline::getPipelineLayout() const -> vk::PipelineLayout {
  return **pipelineLayout;
}

}
