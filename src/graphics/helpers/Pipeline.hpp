#pragma once

#include "SpirvHelper.hpp"
#include "Paths.hpp"
#include "../Vertex.hpp"

namespace Triton::Graphics::Helpers {

   using BasicPipelineData =
       std::tuple<std::unique_ptr<vk::raii::Pipeline>, std::unique_ptr<vk::raii::PipelineLayout>>;

   std::string readShaderFile(const std::string_view& filename) {
      if (std::ifstream file(filename.data(), std::ios::binary); file.is_open()) {
         file.seekg(0, std::ios::end);

         const std::streampos fileSize = file.tellg();

         file.seekg(0, std::ios::beg);

         std::string shaderCode;
         shaderCode.resize(fileSize);
         file.read(shaderCode.data(), fileSize);

         file.close();
         return shaderCode;
      }
      std::stringstream ss;
      ss << "Failed to read shader from file " << filename.data();
      throw std::runtime_error(ss.str());
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createBindlessDescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto textureBinding = vk::DescriptorSetLayoutBinding{
          .binding = 3,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 16,
          .stageFlags = vk::ShaderStageFlagBits::eAll,
          .pImmutableSamplers = nullptr};

      const auto bindlessFlags = vk::DescriptorBindingFlagBits::ePartiallyBound |
                                 vk::DescriptorBindingFlagBits::eUpdateAfterBind;

      const auto extendedInfo =
          vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                           .pBindingFlags = &bindlessFlags};

      const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{
          .pNext = &extendedInfo,
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
          .bindingCount = 1,
          .pBindings = &textureBinding};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(dslCreateInfo));
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createPerFrameDescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo =
          vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &binding};
      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(createInfo));
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createSSBODescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto ssboBinding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo =
          vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &ssboBinding};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(createInfo));
   }

   BasicPipelineData createBasicPipeline(
       const GraphicsDevice& graphicsDevice,
       const vk::raii::RenderPass& renderPass,
       const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
       const vk::raii::DescriptorSetLayout& ssboDescriptorSetLayout,
       const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout) {

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

      auto vertexShaderModule =
          graphicsDevice.getVulkanDevice().createShaderModule(vertexShaderCreateInfo);

      auto fragmentShaderCreateInfo =
          vk::ShaderModuleCreateInfo{.codeSize = 4 * fragmentSpirv.size(),
                                     .pCode = fragmentSpirv.data()};

      auto fragmentShaderModule =
          graphicsDevice.getVulkanDevice().createShaderModule(fragmentShaderCreateInfo);

      auto vertexShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                            .module = *vertexShaderModule,
                                            .pName = "main"};

      auto fragmentShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                            .module = *fragmentShaderModule,
                                            .pName = "main"};

      auto shaderStages = std::array{vertexShaderStageInfo, fragmentShaderStageInfo};

      const auto setLayouts = std::array{*bindlessDescriptorSetLayout,
                                         *ssboDescriptorSetLayout,
                                         *perFrameDescriptorSetLayout};

      vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = setLayouts.size(),
                                                      .pSetLayouts = setLayouts.data()};

      auto pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(
          graphicsDevice.getVulkanDevice().createPipelineLayout(pipelineLayoutInfo));

      // Configure Vertex Attributes
      auto bindingDescription = Vertex::inputBindingDescription(0);
      auto attributeDescriptions = Vertex::inputAttributeDescriptions(
          0,
          {VertexComponent::Position, VertexComponent::Color, VertexComponent::UV});

      vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
          .vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &bindingDescription,
          .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
          .pVertexAttributeDescriptions = attributeDescriptions.data()};

      const auto swapchainExtent = graphicsDevice.getSwapchainExtent();

      const auto viewport = vk::Viewport{.x = 0.f,
                                         .y = 0.f,
                                         .width = static_cast<float>(swapchainExtent.width),
                                         .height = static_cast<float>(swapchainExtent.height),
                                         .minDepth = 0.f,
                                         .maxDepth = 1.f};

      const auto scissor = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent};

      const auto viewportCreateInfo = vk::PipelineViewportStateCreateInfo{.viewportCount = 1,
                                                                          .pViewports = &viewport,
                                                                          .scissorCount = 1,
                                                                          .pScissors = &scissor};

      const vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
          .topology = vk::PrimitiveTopology::eTriangleList,
          .primitiveRestartEnable = VK_FALSE};

      const vk::PipelineRasterizationStateCreateInfo rasterizer{
          .depthClampEnable = false,
          .rasterizerDiscardEnable = VK_FALSE,
          .polygonMode = vk::PolygonMode::eFill,
          .cullMode = vk::CullModeFlagBits::eBack,
          .frontFace = vk::FrontFace::eClockwise,
          .depthBiasEnable = VK_FALSE,
          .lineWidth = 1.f};

      const vk::PipelineMultisampleStateCreateInfo multisampling{
          .rasterizationSamples = vk::SampleCountFlagBits::e1,
          .sampleShadingEnable = VK_FALSE,
      };

      const vk::PipelineColorBlendAttachmentState colorBlendAttachment{
          .blendEnable = VK_FALSE,
          .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

      const vk::PipelineColorBlendStateCreateInfo colorBlending{.logicOpEnable = VK_FALSE,
                                                                .attachmentCount = 1,
                                                                .pAttachments =
                                                                    &colorBlendAttachment};

      const vk::PipelineDepthStencilStateCreateInfo depthStencil{
          .depthTestEnable = VK_TRUE,
          .depthWriteEnable = VK_TRUE,
          .depthCompareOp = vk::CompareOp::eLess,
          .depthBoundsTestEnable = VK_FALSE,
          .stencilTestEnable = VK_FALSE,
      };

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

      auto pipeline = std::make_unique<vk::raii::Pipeline>(
          graphicsDevice.getVulkanDevice().createGraphicsPipeline(VK_NULL_HANDLE,
                                                                  pipelineCreateInfo));

      auto returnValue = std::make_tuple(std::move(pipeline), std::move(pipelineLayout));

      return returnValue;
   }
}