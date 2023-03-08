#pragma once
#include <glslang/Include/glslang_c_interface.h>
#include <memory>
#include <vulkan/vulkan_raii.hpp>

class AbstractPipeline {
 public:
   virtual ~AbstractPipeline() {
   }

   const vk::raii::Pipeline& getPipeline() const {
      return *pipeline;
   };

 protected:
   AbstractPipeline() = default;

   std::string readShaderFile(const std::string_view& filename) const;

   std::unique_ptr<vk::raii::Pipeline> pipeline;

   const vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
       .topology = vk::PrimitiveTopology::eTriangleList, .primitiveRestartEnable = VK_FALSE};

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

   const vk::PipelineColorBlendStateCreateInfo colorBlending{
       .logicOpEnable = VK_FALSE, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

   const vk::PipelineDepthStencilStateCreateInfo depthStencil{
       .depthTestEnable = VK_TRUE,
       .depthWriteEnable = VK_TRUE,
       .depthCompareOp = vk::CompareOp::eLess,
       .depthBoundsTestEnable = VK_FALSE,
       .stencilTestEnable = VK_FALSE,
   };
};
