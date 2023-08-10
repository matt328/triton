#include "graphics/renderer/Terrain.hpp"

#include "graphics/renderer/RendererBase.hpp"
#include "graphics/pipeline/SpirvHelper.hpp"
#include "core/Utils.hpp"
#include "graphics/pipeline/Vertex.hpp"
#include "graphics/VulkanFactory.hpp"
#include "graphics/pipeline/PipelineFactory.hpp"

using Core::Log;

Terrain::Terrain(const RendererBaseCreateInfo& createInfo) {
   createRenderPass(&createInfo.device, &createInfo.physicalDevice, createInfo.swapchainFormat);
   createPipeline(createInfo.swapchainExtent, &createInfo.device);
}

void Terrain::fillCommandBuffer(const vk::raii::CommandBuffer&, size_t currentImage) {
}

void Terrain::update() {
}

void Terrain::createRenderPass(const vk::raii::Device* device,
                               const vk::raii::PhysicalDevice* physicalDevice,
                               const vk::Format swapchainFormat) {
   const auto renderPassCreateInfo =
       Graphics::Utils::RenderPassCreateInfo{.device = device,
                                             .physicalDevice = physicalDevice,
                                             .swapchainFormat = swapchainFormat,
                                             .clearColor = false,
                                             .clearDepth = false};

   renderPass = std::make_unique<vk::raii::RenderPass>(
       Graphics::Utils::colorAndDepthRenderPass(renderPassCreateInfo));
}

void Terrain::createPipeline(const vk::Extent2D swapchainExtent, const vk::raii::Device* device) {
   auto helper = std::make_unique<SpirvHelper>();

   const auto vertexFilename = (Core::Paths::SHADERS / "shader.vert").string();
   const auto fragmentFilename = (Core::Paths::SHADERS / "shader.frag").string();

   auto vertexShaderCode = Graphics::Pipeline::readShaderFile(vertexFilename);
   auto fragmentShaderCode = Graphics::Pipeline::readShaderFile(fragmentFilename);

   const auto vertexSpirv =
       helper->compileShader(vk::ShaderStageFlagBits::eVertex, vertexShaderCode.data());
   Log::core->debug("Compiled shader {}", vertexFilename);

   const auto fragmentSpirv =
       helper->compileShader(vk::ShaderStageFlagBits::eFragment, fragmentShaderCode.data());
   Log::core->debug("Compiled shader {}", fragmentFilename);

   auto vertexShaderCreateInfo =
       vk::ShaderModuleCreateInfo{.codeSize = 4 * vertexSpirv.size(), .pCode = vertexSpirv.data()};

   auto vertexShaderModule = device->createShaderModule(vertexShaderCreateInfo);

   auto fragmentShaderCreateInfo = vk::ShaderModuleCreateInfo{.codeSize = 4 * fragmentSpirv.size(),
                                                              .pCode = fragmentSpirv.data()};

   auto fragmentShaderModule = device->createShaderModule(fragmentShaderCreateInfo);

   const auto vertStage = Graphics::Utils::ShaderStage{.shaderModule = &vertexShaderModule,
                                                       .stages = vk::ShaderStageFlagBits::eVertex,
                                                       .code = vertexSpirv};
   const auto fragStage =
       Graphics::Utils::ShaderStage({.shaderModule = &fragmentShaderModule,
                                     .stages = vk::ShaderStageFlagBits::eFragment,
                                     .code = fragmentSpirv});

   const auto shaderLibraryInfo = std::vector<Graphics::Utils::ShaderStage>{vertStage, fragStage};

   auto vertexShaderStageInfo = vk::PipelineShaderStageCreateInfo{
       .stage = vk::ShaderStageFlagBits::eVertex, .module = *vertexShaderModule, .pName = "main"};

   auto fragmentShaderStageInfo =
       vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                         .module = *fragmentShaderModule,
                                         .pName = "main"};

   auto shaderStages = std::array{vertexShaderStageInfo, fragmentShaderStageInfo};

   const auto descriptorSetLayout =
       Graphics::Utils::createDescriptorSetLayout(device, shaderLibraryInfo);

   vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
       .setLayoutCount = 1,
       .pSetLayouts = &(**descriptorSetLayout)}; // This is (now even more) awkward

   pipelineLayout =
       std::make_unique<vk::raii::PipelineLayout>(device->createPipelineLayout(pipelineLayoutInfo));

   // Configure Vertex Attributes
   auto bindingDescription = Models::Vertex::inputBindingDescription(0);
   auto attributeDescriptions =
       Models::Vertex::inputAttributeDescriptions(0,
                                                  {Models::VertexComponent::Position,
                                                   Models::VertexComponent::Color,
                                                   Models::VertexComponent::UV});

   vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
       .vertexBindingDescriptionCount = 1,
       .pVertexBindingDescriptions = &bindingDescription,
       .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
       .pVertexAttributeDescriptions = attributeDescriptions.data()};

   const auto viewport = vk::Viewport{.x = 0.f,
                                      .y = 0.f,
                                      .width = static_cast<float>(swapchainExtent.width),
                                      .height = static_cast<float>(swapchainExtent.height),
                                      .minDepth = 0.f,
                                      .maxDepth = 1.f};

   const auto scissor = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent};

   const auto viewportCreateInfo = vk::PipelineViewportStateCreateInfo{
       .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor};

   auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo{
       .stageCount = static_cast<uint32_t>(shaderStages.size()),
       .pStages = shaderStages.data(),
       .pVertexInputState = &vertexInputInfo,
       .pInputAssemblyState = &Graphics::Pipeline::defaultInputAssembly,
       .pViewportState = &viewportCreateInfo,
       .pRasterizationState = &Graphics::Pipeline::defaultRasterizer,
       .pMultisampleState = &Graphics::Pipeline::disableMultisampling,
       .pDepthStencilState = &Graphics::Pipeline::defaultDepthStencil,
       .pColorBlendState = &Graphics::Pipeline::defaultColorBlending,
       .layout = **pipelineLayout,
       .renderPass = **renderPass,
       .subpass = 0,
       .basePipelineHandle = VK_NULL_HANDLE,
       .basePipelineIndex = -1};
   // Finally this is what it's all about
   pipeline = std::make_unique<vk::raii::Pipeline>(
       device->createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo));
}
