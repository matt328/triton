#include "DefaultPipeline.hpp"

#include "Logger.hpp"
#include "SpirvHelper.hpp"
#include "core/Utils.hpp"
#include "Vertex.hpp"
#include "graphics/VulkanFactory.hpp"

namespace Triton {

   DefaultPipeline::DefaultPipeline(const vk::raii::Device& device,
                                    const vk::raii::RenderPass& renderPass,
                                    const vk::Extent2D& swapchainExtent) {
      // Configure Shader Modules // TODO pull out shader module creation into it's own thing
      auto helper = std::make_unique<SpirvHelper>();

      const auto vertexFilename = (Core::Paths::SHADERS / "shader.vert").string();
      const auto fragmentFilename = (Core::Paths::SHADERS / "shader.frag").string();

      auto vertexShaderCode = readShaderFile(vertexFilename);
      auto fragmentShaderCode = readShaderFile(fragmentFilename);

      const auto vertexSpirv =
          helper->compileShader(vk::ShaderStageFlagBits::eVertex, vertexShaderCode.data());
      Log::debug << "Compiled shader " << vertexFilename << std::endl;

      const auto fragmentSpirv =
          helper->compileShader(vk::ShaderStageFlagBits::eFragment, fragmentShaderCode.data());
      Log::debug << "Compiled shader " << fragmentFilename << std::endl;

      auto vertexShaderCreateInfo = vk::ShaderModuleCreateInfo{.codeSize = 4 * vertexSpirv.size(),
                                                               .pCode = vertexSpirv.data()};

      auto vertexShaderModule = device.createShaderModule(vertexShaderCreateInfo);

      auto fragmentShaderCreateInfo = vk::ShaderModuleCreateInfo{
          .codeSize = 4 * fragmentSpirv.size(), .pCode = fragmentSpirv.data()};

      auto fragmentShaderModule = device.createShaderModule(fragmentShaderCreateInfo);

      const auto vertStage = Utils::ShaderStage{.shaderModule = &vertexShaderModule,
                                                .stages = vk::ShaderStageFlagBits::eVertex,
                                                .code = vertexSpirv};
      const auto fragStage = Utils::ShaderStage({.shaderModule = &fragmentShaderModule,
                                                 .stages = vk::ShaderStageFlagBits::eFragment,
                                                 .code = fragmentSpirv});

      const auto shaderLibraryInfo = std::vector<Utils::ShaderStage>{vertStage, fragStage};

      auto vertexShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                            .module = *vertexShaderModule,
                                            .pName = "main"};

      auto fragmentShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                            .module = *fragmentShaderModule,
                                            .pName = "main"};

      auto shaderStages = std::array{vertexShaderStageInfo, fragmentShaderStageInfo};

      bindlessDescriptorSetLayout = Utils::createBindlessDescriptorSetLayout(device);
      objectDescriptorSetLayout = Utils::createSSBODescriptorSetLayout(device);
      perFrameDescriptorSetLayout = Utils::createPerFrameDescriptorSetLayout(device);

      const auto setLayouts = std::array{*(*bindlessDescriptorSetLayout),
                                         *(*objectDescriptorSetLayout),
                                         *(*perFrameDescriptorSetLayout)};

      vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = setLayouts.size(),
                                                      .pSetLayouts = setLayouts.data()};

      pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(
          device.createPipelineLayout(pipelineLayoutInfo));

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
}