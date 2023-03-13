#include "DefaultPipeline.h"

#include "Log.h"
#include "SpirvHelper.h"
#include "Utils.h"
#include "VertexFormats.h"

DefaultPipeline::DefaultPipeline(const vk::raii::Device& device,
                                 const vk::raii::RenderPass& renderPass,
                                 const vk::Extent2D& swapchainExtent) {
   // Configure Shader Modules
   auto helper = std::make_unique<SpirvHelper>();

   const auto vertexFilename = (Paths::SHADERS / "shader.vert").string();
   const auto fragmentFilename = (Paths::SHADERS / "shader.frag").string();

   auto vertexShaderCode = readShaderFile(vertexFilename);
   auto fragmentShaderCode = readShaderFile(fragmentFilename);

   const auto vertexSpirv =
       helper->compileShader(vk::ShaderStageFlagBits::eVertex, vertexShaderCode.data());
   Log::core->debug("Compiled shader {}", vertexFilename);

   const auto fragmentSpirv =
       helper->compileShader(vk::ShaderStageFlagBits::eFragment, fragmentShaderCode.data());
   Log::core->debug("Compiled shader {}", fragmentFilename);

   auto vertexShaderCreateInfo =
       vk::ShaderModuleCreateInfo{.codeSize = 4 * vertexSpirv.size(), .pCode = vertexSpirv.data()};

   auto vertexShaderModule = device.createShaderModule(vertexShaderCreateInfo);

   auto fragmentShaderCreateInfo = vk::ShaderModuleCreateInfo{.codeSize = 4 * fragmentSpirv.size(),
                                                              .pCode = fragmentSpirv.data()};

   auto fragmentShaderModule = device.createShaderModule(fragmentShaderCreateInfo);

   auto vertexShaderStageInfo = vk::PipelineShaderStageCreateInfo{
       .stage = vk::ShaderStageFlagBits::eVertex, .module = *vertexShaderModule, .pName = "main"};

   auto fragmentShaderStageInfo =
       vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                         .module = *fragmentShaderModule,
                                         .pName = "main"};

   auto shaderStages = std::array{vertexShaderStageInfo, fragmentShaderStageInfo};

   descriptorSetLayout =
       std::make_unique<vk::raii::DescriptorSetLayout>(createDescriptorSetLayout(device));

   // Configure Vertex Attributes
   auto bindingDescription = VertexFormats::PositionColorTexture::bindingDescription();
   auto attributeDescriptions = VertexFormats::PositionColorTexture::attributeDescriptions();

   vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
       .vertexBindingDescriptionCount = 1,
       .pVertexBindingDescriptions = &bindingDescription,
       .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
       .pVertexAttributeDescriptions = attributeDescriptions.data()};

   vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
       .setLayoutCount = 1,
       .pSetLayouts = &(**descriptorSetLayout)}; // This is (now even more) awkward

   auto pipelineLayout =
       std::make_unique<vk::raii::PipelineLayout>(device.createPipelineLayout(pipelineLayoutInfo));

   const auto viewport = vk::Viewport{.x = 0.f,
                                      .y = 0.f,
                                      .width = static_cast<float>(swapchainExtent.width),
                                      .height = static_cast<float>(swapchainExtent.height),
                                      .minDepth = 0.f,
                                      .maxDepth = 1.f};

   const auto scissor = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent};

   const auto viewportCreateInfo = vk::PipelineViewportStateCreateInfo{
       .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor};

   auto pipelineCreateInfo =
       vk::GraphicsPipelineCreateInfo{.stageCount = static_cast<uint32_t>(shaderStages.size()),
                                      .pStages = shaderStages.data(),
                                      .pVertexInputState = &vertexInputInfo,
                                      .pInputAssemblyState = &inputAssembly,
                                      .pViewportState = &viewportCreateInfo,
                                      .pRasterizationState = &rasterizer,
                                      .pMultisampleState = &multisampling,
                                      .pDepthStencilState = &depthStencil,
                                      .pColorBlendState = &colorBlending,
                                      .layout = *(*pipelineLayout),
                                      .renderPass = *renderPass,
                                      .subpass = 0,
                                      .basePipelineHandle = VK_NULL_HANDLE,
                                      .basePipelineIndex = -1};
   // Finally this is that it's all about
   pipeline = std::make_unique<vk::raii::Pipeline>(
       device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo));
}

DefaultPipeline::~DefaultPipeline() {
}

vk::raii::DescriptorSetLayout DefaultPipeline::createDescriptorSetLayout(
    const vk::raii::Device& device) {
   constexpr auto objectMatricesBinding = vk::DescriptorSetLayoutBinding{
       .binding = 0,
       .descriptorType = vk::DescriptorType::eUniformBuffer,
       .descriptorCount = 1,
       .stageFlags = vk::ShaderStageFlagBits::eVertex,
   };

   constexpr auto textureSamplerBinding =
       vk::DescriptorSetLayoutBinding{.binding = 1,
                                      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                                      .descriptorCount = 1,
                                      .stageFlags = vk::ShaderStageFlagBits::eFragment};

   const auto bindings =
       std::array<vk::DescriptorSetLayoutBinding, 2>{objectMatricesBinding, textureSamplerBinding};

   const auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{
       .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data()};

   return device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}
