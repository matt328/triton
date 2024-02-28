#pragma once

#include "gfx/geometry/Vertex.hpp"
#include "gfx/GraphicsDevice.hpp"
#include "Vulkan.hpp"

namespace tr::gfx::Helpers {

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
       const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
       const vk::raii::DescriptorSetLayout& ssboDescriptorSetLayout,
       const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout,
       const vk::raii::ShaderModule& vertexShaderModule,
       const vk::raii::ShaderModule& fragmentShaderModule) {

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
      auto bindingDescription = Geometry::Vertex::inputBindingDescription(0);
      auto attributeDescriptions =
          Geometry::Vertex::inputAttributeDescriptions(0,
                                                       {Geometry::VertexComponent::Position,
                                                        Geometry::VertexComponent::Color,
                                                        Geometry::VertexComponent::UV});

      vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
          .vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &bindingDescription,
          .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
          .pVertexAttributeDescriptions = attributeDescriptions.data()};

      const auto viewport =
          vk::Viewport{.x = 0.f,
                       .y = 0.f,
                       .width = static_cast<float>(graphicsDevice.DrawImageExtent2D.width),
                       .height = static_cast<float>(graphicsDevice.DrawImageExtent2D.height),
                       .minDepth = 0.f,
                       .maxDepth = 1.f};

      const auto scissor = vk::Rect2D{.offset = {0, 0}, .extent = graphicsDevice.DrawImageExtent2D};

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
          .frontFace = vk::FrontFace::eCounterClockwise,
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

      const auto colorFormat = vk::Format::eR16G16B16A16Sfloat;
      const auto depthFormat = Helpers::findDepthFormat(graphicsDevice.getPhysicalDevice());

      const auto renderingInfo =
          vk::PipelineRenderingCreateInfo{.colorAttachmentCount = 1,
                                          .pColorAttachmentFormats = &colorFormat,
                                          .depthAttachmentFormat = depthFormat};

      auto pipelineCreateInfo =
          vk::GraphicsPipelineCreateInfo{.pNext = &renderingInfo,
                                         .stageCount = static_cast<uint32_t>(shaderStages.size()),
                                         .pStages = shaderStages.data(),
                                         .pVertexInputState = &vertexInputInfo,
                                         .pInputAssemblyState = &inputAssembly,
                                         .pViewportState = &viewportCreateInfo,
                                         .pRasterizationState = &rasterizer,
                                         .pMultisampleState = &multisampling,
                                         .pDepthStencilState = &depthStencil,
                                         .pColorBlendState = &colorBlending,
                                         .layout = *(*pipelineLayout),
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