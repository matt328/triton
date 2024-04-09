#include "Pipeline.hpp"
#include "helpers/SpirvHelper.hpp"
#include "GraphicsDevice.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace tr::gfx {
   Pipeline::Pipeline(const GraphicsDevice& graphicsDevice,
                      const vk::PipelineLayoutCreateInfo& pipelineLayoutCreateInfo,
                      const vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo,
                      const vk::PipelineRenderingCreateInfo& renderingCreateInfo,
                      const std::filesystem::path& vertexShaderName,
                      const std::filesystem::path& fragmentShaderName) {

      const auto viewportSize = graphicsDevice.DrawImageExtent2D;

      viewport = vk::Viewport{.x = 0.f,
                              .y = 0.f,
                              .width = static_cast<float>(viewportSize.width),
                              .height = static_cast<float>(viewportSize.height),
                              .minDepth = 0.f,
                              .maxDepth = 1.f};

      scissor = vk::Rect2D{.offset = {0, 0}, .extent = viewportSize};

      // Load up shader modules
      auto helper = std::make_unique<Helpers::SpirvHelper>();

      auto vertexShaderCode = readShaderFile(vertexShaderName);
      auto fragmentShaderCode = readShaderFile(fragmentShaderName);

      const auto vertexSpirv =
          helper->compileShader(vk::ShaderStageFlagBits::eVertex, vertexShaderCode.data());
      Log::debug << "Compiled shader " << vertexShaderName.string() << std::endl;

      const auto fragmentSpirv =
          helper->compileShader(vk::ShaderStageFlagBits::eFragment, fragmentShaderCode.data());
      Log::debug << "Compiled shader " << fragmentShaderName.string() << std::endl;

      auto vertexShaderCreateInfo = vk::ShaderModuleCreateInfo{.codeSize = 4 * vertexSpirv.size(),
                                                               .pCode = vertexSpirv.data()};

      vertexShaderModule = std::make_unique<vk::raii::ShaderModule>(
          graphicsDevice.getVulkanDevice().createShaderModule(vertexShaderCreateInfo));

      auto fragmentShaderCreateInfo =
          vk::ShaderModuleCreateInfo{.codeSize = 4 * fragmentSpirv.size(),
                                     .pCode = fragmentSpirv.data()};

      fragmentShaderModule = std::make_unique<vk::raii::ShaderModule>(
          graphicsDevice.getVulkanDevice().createShaderModule(fragmentShaderCreateInfo));

      auto vertexShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eVertex,
                                            .module = **vertexShaderModule,
                                            .pName = "main"};

      auto fragmentShaderStageInfo =
          vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eFragment,
                                            .module = **fragmentShaderModule,
                                            .pName = "main"};

      auto shaderStages = std::array{vertexShaderStageInfo, fragmentShaderStageInfo};

      pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(
          graphicsDevice.getVulkanDevice().createPipelineLayout(pipelineLayoutCreateInfo));

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

      const auto dynamicStates = std::array<vk::DynamicState, 2>{
          vk::DynamicState::eViewportWithCount,
          vk::DynamicState::eScissorWithCount,
      };

      vk::PipelineDynamicStateCreateInfo dsci{.dynamicStateCount = dynamicStates.size(),
                                              .pDynamicStates = dynamicStates.data()};

      auto viewportInfo = vk::PipelineViewportStateCreateInfo{
          .pViewports = nullptr,
          .pScissors = nullptr,
      };

      auto pipelineCreateInfo =
          vk::GraphicsPipelineCreateInfo{.pNext = &renderingCreateInfo,
                                         .stageCount = static_cast<uint32_t>(shaderStages.size()),
                                         .pStages = shaderStages.data(),
                                         .pVertexInputState = &vertexInputStateCreateInfo,
                                         .pInputAssemblyState = &inputAssembly,
                                         .pViewportState = &viewportInfo,
                                         .pRasterizationState = &rasterizer,
                                         .pMultisampleState = &multisampling,
                                         .pDepthStencilState = &depthStencil,
                                         .pColorBlendState = &colorBlending,
                                         .pDynamicState = &dsci,
                                         .layout = *(*pipelineLayout),
                                         .subpass = 0,
                                         .basePipelineHandle = VK_NULL_HANDLE,
                                         .basePipelineIndex = -1};
      // Finally this is that it's all about
      pipeline = std::make_unique<vk::raii::Pipeline>(
          graphicsDevice.getVulkanDevice().createGraphicsPipeline(VK_NULL_HANDLE,
                                                                  pipelineCreateInfo));
   }

   Pipeline::~Pipeline() {
   }

   void Pipeline::resize(const vk::Extent2D newSize) {
      Log::info << "resizing pipeline: " << newSize.width << ", " << newSize.height << std::endl;
      viewport = vk::Viewport{.x = 0.f,
                              .y = 0.f,
                              .width = static_cast<float>(newSize.width),
                              .height = static_cast<float>(newSize.height),
                              .minDepth = 0.f,
                              .maxDepth = 1.f};

      scissor = vk::Rect2D{.offset = {0, 0}, .extent = newSize};
   }

   /// Binds the Pipeline and sets the viewport and scissor
   void Pipeline::bind(const vk::raii::CommandBuffer& cmd) {
      cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
      cmd.setViewportWithCount(viewport);
      cmd.setScissorWithCount(scissor);
   }

   std::string Pipeline::readShaderFile(const std::filesystem::path& filename) {
      if (std::ifstream file(filename.string().data(), std::ios::binary); file.is_open()) {
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
      ss << "Failed to read shader from file " << filename.string().data();
      throw std::runtime_error(ss.str());
   }
}