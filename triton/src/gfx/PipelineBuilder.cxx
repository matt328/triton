#include "PipelineBuilder.hpp"
#include "gfx/ObjectData.hpp"
#include "gfx/geometry/Vertex.hpp"

namespace tr::gfx {
   PipelineBuilder::PipelineBuilder(const vk::raii::Device& device)
       : device{device},
         inputAssembly{},
         rasterizer{},
         colorBlendAttachment{},
         multisampling{},
         depthStencil{},
         renderInfo{},
         colorAttachmentformat{vk::Format::eB8G8R8A8Srgb} {
   }

   void PipelineBuilder::setDefaultVertexAttributeDescriptions() {
      auto vec = std::vector{geo::VertexComponent::Position,
                             geo::VertexComponent::Color,
                             geo::VertexComponent::UV,
                             geo::VertexComponent::Normal};
      vertexAttributeDescriptions =
          geo::Vertex::inputAttributeDescriptions(0, std::span(vec.begin(), vec.end()));
   }

   void PipelineBuilder::setVertexAttributeDescriptions(
       std::span<geo::VertexComponent> components) {
      vertexAttributeDescriptions = geo::Vertex::inputAttributeDescriptions(0, components);
   }

   auto PipelineBuilder::buildPipelineLayout(const std::span<vk::DescriptorSetLayout>& layouts)
       -> std::unique_ptr<vk::raii::PipelineLayout> {

      auto pcr = vk::PushConstantRange{.stageFlags = vk::ShaderStageFlagBits::eVertex |
                                                     vk::ShaderStageFlagBits::eFragment,
                                       .offset = 0,
                                       .size = sizeof(PushConstants)};

      vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
          .setLayoutCount = static_cast<uint32_t>(layouts.size()),
          .pSetLayouts = layouts.data(),
          .pushConstantRangeCount = 1,
          .pPushConstantRanges = &pcr};

      return std::make_unique<vk::raii::PipelineLayout>(
          device.createPipelineLayout(pipelineLayoutCreateInfo));
   }

   auto PipelineBuilder::buildPipeline(const vk::raii::PipelineLayout& pipelineLayout)
       -> std::unique_ptr<vk::raii::Pipeline> {
      const auto colorBlending =
          vk::PipelineColorBlendStateCreateInfo{.logicOpEnable = VK_FALSE,
                                                .logicOp = vk::LogicOp::eCopy,
                                                .attachmentCount = 1,
                                                .pAttachments = &colorBlendAttachment};

      const auto bindingDescription = geo::Vertex::inputBindingDescription(0);

      const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
          .vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &bindingDescription,
          .vertexAttributeDescriptionCount =
              static_cast<uint32_t>(vertexAttributeDescriptions.size()),
          .pVertexAttributeDescriptions = vertexAttributeDescriptions.data()};

      auto pipelineCreateInfo =
          vk::GraphicsPipelineCreateInfo{.pNext = &renderInfo,
                                         .stageCount = static_cast<uint32_t>(shaderStages.size()),
                                         .pStages = shaderStages.data(),
                                         .pVertexInputState = &vertexInputStateCreateInfo,
                                         .pInputAssemblyState = &inputAssembly,
                                         .pRasterizationState = &rasterizer,
                                         .pMultisampleState = &multisampling,
                                         .pDepthStencilState = &depthStencil,
                                         .pColorBlendState = &colorBlending,
                                         .pDynamicState = &dsci,
                                         .layout = *pipelineLayout,
                                         .subpass = 0,
                                         .basePipelineHandle = VK_NULL_HANDLE,
                                         .basePipelineIndex = -1};
      // Finally this is that it's all about
      return std::make_unique<vk::raii::Pipeline>(
          device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo));
   }

   void PipelineBuilder::clearShaderStages() {
      shaderStages.clear();
   }

   void PipelineBuilder::setVertexShaderStage(const vk::raii::ShaderModule& vertexShaderModule) {
      shaderStages.emplace_back(
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                            .module = *vertexShaderModule,
                                            .pName = "main"});
   }

   void PipelineBuilder::setFragmentShaderStage(
       const vk::raii::ShaderModule& fragmentShaderModule) {
      shaderStages.emplace_back(
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                            .module = *fragmentShaderModule,
                                            .pName = "main"});
   }

   void PipelineBuilder::setTessellationStages(
       [[maybe_unused]] const vk::raii::ShaderModule& controlModule,
       [[maybe_unused]] const vk::raii::ShaderModule evaluationModule) {
   }

   void PipelineBuilder::setInputTopology(vk::PrimitiveTopology topology) {
      inputAssembly.topology = topology;
      inputAssembly.primitiveRestartEnable = VK_FALSE;
   }

   void PipelineBuilder::setPolygonMode(vk::PolygonMode polygonMode) {
      rasterizer.polygonMode = polygonMode;
      rasterizer.lineWidth = 1.f;
   }

   void PipelineBuilder::setCullMode(vk::CullModeFlagBits cullModeFlagBits,
                                     vk::FrontFace frontFace) {
      rasterizer.cullMode = cullModeFlagBits;
      rasterizer.frontFace = frontFace;
   }

   void PipelineBuilder::setMultisamplingNone() {
      multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.minSampleShading = 1.f;
   }

   void PipelineBuilder::disableBlending() {
      colorBlendAttachment.blendEnable = VK_FALSE;
      colorBlendAttachment.colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
   }

   void PipelineBuilder::setColorAttachmentFormat(vk::Format format) {
      colorAttachmentformat = format;
      renderInfo.colorAttachmentCount = 1;
      renderInfo.pColorAttachmentFormats = &colorAttachmentformat;
   }

   void PipelineBuilder::setDepthFormat(vk::Format format) {
      renderInfo.depthAttachmentFormat = format;
   }

   void PipelineBuilder::setDefaultDepthStencil() {
      depthStencil.depthTestEnable = VK_TRUE;
      depthStencil.depthWriteEnable = VK_TRUE;
      depthStencil.depthCompareOp = vk::CompareOp::eLess;
      depthStencil.depthBoundsTestEnable = VK_FALSE;
      depthStencil.stencilTestEnable = VK_FALSE;
   }

   void PipelineBuilder::clear() {
      inputAssembly = {};
      rasterizer = {};
      colorBlendAttachment = {};
      colorAttachmentformat = {};
      multisampling = {};
      depthStencil = {};
      renderInfo = {};
      shaderStages.clear();
   }
}
