#include "IndirectPipeline.hpp"
#include "IShaderCompiler.hpp"
#include "geo/VertexAttributes.hpp"
#include "cm/IndirectPushConstants.hpp"

namespace tr::gfx::pipe {

   const auto VertexShaderFile = SHADERS / "indirect.vert";
   const auto FragmentShaderFile = SHADERS / "indirect.frag";

   IndirectPipeline::IndirectPipeline(const std::shared_ptr<Device>& device,
                                      const std::shared_ptr<IShaderCompiler>& shaderCompiler) {
      Log.trace("Constructing IndirectPipeline");

      const auto pushConstantRange = vk::PushConstantRange{
          .stageFlags = vk::ShaderStageFlagBits::eVertex,
          .offset = 0,
          .size = sizeof(cm::gpu::IndirectPushConstants),
      };

      const auto pipelineLayoutCreateInfo =
          vk::PipelineLayoutCreateInfo{.pushConstantRangeCount = 1,
                                       .pPushConstantRanges = &pushConstantRange};

      pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(device->getVkDevice(),
                                                                  pipelineLayoutCreateInfo);

      // PipelineRenderingInfo
      constexpr auto colorAttachmentFormat = vk::Format::eR16G16B16A16Sfloat;
      auto pipelineRenderingInfo =
          vk::PipelineRenderingCreateInfo{.colorAttachmentCount = 1,
                                          .pColorAttachmentFormats = &colorAttachmentFormat,
                                          .depthAttachmentFormat = vk::Format::eD32Sfloat};

      // Stages
      auto shaderStages = std::vector<vk::PipelineShaderStageCreateInfo>{};
      auto vsm =
          shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eVertex, VertexShaderFile);
      auto fsm = shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eFragment,
                                                    FragmentShaderFile);
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
                             geo::VertexComponent::Normal,
                             geo::VertexComponent::UV,
                             geo::VertexComponent::Color,
                             geo::VertexComponent::Joint0,
                             geo::VertexComponent::Weight0,
                             geo::VertexComponent::Tangent};

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
      constexpr auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
          .topology = vk::PrimitiveTopology::eTriangleList,
          .primitiveRestartEnable = VK_FALSE,
      };

      // Rasterizer
      constexpr auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
          .polygonMode = vk::PolygonMode::eFill,
          .cullMode = vk::CullModeFlagBits::eBack,
          .lineWidth = 1.f,
      };

      // Multisampling
      constexpr auto multisampling = vk::PipelineMultisampleStateCreateInfo{
          .rasterizationSamples = vk::SampleCountFlagBits::e1,
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

   auto IndirectPipeline::getPipeline() -> vk::Pipeline {
      return **pipeline;
   }
   auto IndirectPipeline::getPipelineLayout() -> vk::PipelineLayout {
      return **pipelineLayout;
   }

   void IndirectPipeline::bind(const vk::raii::CommandBuffer& cmd) {
   }

   void IndirectPipeline::applyShaderBinding(
       const sb::ShaderBinding& binding,
       uint32_t setIndex,
       const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) {
   }
}