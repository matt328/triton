#include "StaticModelPipeline.hpp"
#include "geo/VertexAttributes.hpp"
#include "renderer/RendererConfig.hpp"

namespace tr::gfx::pipe {
   StaticModelPipeline::StaticModelPipeline(const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
                                            const std::shared_ptr<IShaderCompiler>& shaderCompiler,
                                            const rd::RendererConfig& rendererConfig) {
      Log.trace("Constructing StaticModelPipeline");

      // Create Pipeline Layout
      auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{
          .setLayoutCount = static_cast<uint32_t>(rendererConfig.setLayouts.size()),
          .pSetLayouts = rendererConfig.setLayouts.data(),
      };

      if (!rendererConfig.pushConstantRanges.empty()) {
         pipelineLayoutCreateInfo.pushConstantRangeCount =
             static_cast<uint32_t>(rendererConfig.pushConstantRanges.size());
         pipelineLayoutCreateInfo.pPushConstantRanges = rendererConfig.pushConstantRanges.data();
      }

      pipelineLayout =
          graphicsDevice->createPipelineLayout(pipelineLayoutCreateInfo, "StaticModel");

      // PipelineRenderingInfo
      const vk::Format colorAttachmentFormat = vk::Format::eB8G8R8A8Srgb;
      auto pipelineRenderingInfo = vk::PipelineRenderingCreateInfo{
          .colorAttachmentCount = 1,
          .pColorAttachmentFormats = &colorAttachmentFormat,
      };

      // Stages
      auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{};
      auto vsm = shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eVertex,
                                                    SHADERS / "shader.vert");
      auto fsm = shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eFragment,
                                                    SHADERS / "shader.frag");
      shaderStages.emplace_back(
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                            .module = *vsm,
                                            .pName = "main"});
      shaderStages.emplace_back(
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                            .module = *fsm,
                                            .pName = "main"});

      // Vertex Input State
      auto vec = std::vector{geo::VertexComponent::Position,
                             geo::VertexComponent::Color,
                             geo::VertexComponent::UV,
                             geo::VertexComponent::Normal};
      const auto vertexAttributeDescriptions =
          geo::VertexBuilder::inputAttributeDescriptions(0, std::span(vec.begin(), vec.end()));

      const auto bindingDescription = geo::VertexBuilder::inputBindingDescription(0);

      const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
          .vertexBindingDescriptionCount = 1,
          .pVertexBindingDescriptions = &bindingDescription,
          .vertexAttributeDescriptionCount =
              static_cast<uint32_t>(vertexAttributeDescriptions.size()),
          .pVertexAttributeDescriptions = vertexAttributeDescriptions.data()};

      // Input Assembly
      const auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
          .topology = vk::PrimitiveTopology::eTriangleList,
          .primitiveRestartEnable = VK_FALSE,
      };

      // Rasterizer
      const auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
          .polygonMode = vk::PolygonMode::eFill,
      };

      // Multisampling
      const auto multisampling = vk::PipelineMultisampleStateCreateInfo{
          .rasterizationSamples = vk::SampleCountFlagBits::e1,
          .sampleShadingEnable = VK_FALSE,
          .minSampleShading = 1.f};

      // Depth Stencil
      const auto depthStencil =
          vk::PipelineDepthStencilStateCreateInfo{.depthTestEnable = VK_TRUE,
                                                  .depthWriteEnable = VK_TRUE,
                                                  .depthCompareOp = vk::CompareOp::eLess,
                                                  .depthBoundsTestEnable = VK_FALSE,
                                                  .stencilTestEnable = VK_FALSE};

      // Color Blending
      const auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{};
      const auto colorBlending =
          vk::PipelineColorBlendStateCreateInfo{.logicOpEnable = VK_FALSE,
                                                .logicOp = vk::LogicOp::eCopy,
                                                .attachmentCount = 1,
                                                .pAttachments = &colorBlendAttachment};

      // Dynamic State
      const auto dynamicStates = std::array<vk::DynamicState, 2>{
          vk::DynamicState::eViewportWithCount,
          vk::DynamicState::eScissorWithCount,
      };

      const auto dynamicStateInfo =
          vk::PipelineDynamicStateCreateInfo{.dynamicStateCount = 2,
                                             .pDynamicStates = dynamicStates.data()};

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

      pipeline = graphicsDevice->createPipeline(pipelineCreateInfo, "StaticModel");
   }

   void StaticModelPipeline::bind(const vk::raii::CommandBuffer& cmd) {
   }

   void StaticModelPipeline::applyShaderBinding(const sb::ShaderBinding& binding) {
   }
}