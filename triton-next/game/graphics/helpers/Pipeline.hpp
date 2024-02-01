#pragma once

#include "SpirvHelper.hpp"
#include "Paths.hpp"

namespace Triton::Game::Graphics::Helpers {

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

   std::tuple<int, float, std::string> getThings() {
      return {34, 34.f, "hello"};
   }

   std::unique_ptr<vk::raii::Pipeline> createBasicPipeline(const vk::raii::Device& device) {

      auto [anInt, aFloat, aString] = getThings();

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

      auto fragmentShaderCreateInfo =
          vk::ShaderModuleCreateInfo{.codeSize = 4 * fragmentSpirv.size(),
                                     .pCode = fragmentSpirv.data()};

      auto fragmentShaderModule = device.createShaderModule(fragmentShaderCreateInfo);

      auto vertexShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                            .module = *vertexShaderModule,
                                            .pName = "main"};

      auto fragmentShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                            .module = *fragmentShaderModule,
                                            .pName = "main"};

      auto shaderStages = std::array{vertexShaderStageInfo, fragmentShaderStageInfo};

      auto bindlessDescriptorSetLayout = createBindlessDescriptorSetLayout(device);
      auto objectDescriptorSetLayout = createSSBODescriptorSetLayout(device);
      auto perFrameDescriptorSetLayout = createPerFrameDescriptorSetLayout(device);

      const auto setLayouts = std::array{*(*bindlessDescriptorSetLayout),
                                         *(*objectDescriptorSetLayout),
                                         *(*perFrameDescriptorSetLayout)};

      vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = setLayouts.size(),
                                                      .pSetLayouts = setLayouts.data()};

      auto pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(
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

      const auto viewportCreateInfo = vk::PipelineViewportStateCreateInfo{.viewportCount = 1,
                                                                          .pViewports = &viewport,
                                                                          .scissorCount = 1,
                                                                          .pScissors = &scissor};

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
      return std::make_unique<vk::raii::Pipeline>(
          device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo));
   }
}