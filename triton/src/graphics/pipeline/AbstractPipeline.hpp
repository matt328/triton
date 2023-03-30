#pragma once
#include <memory>
#include <vulkan/vulkan_raii.hpp>

class AbstractPipeline {
 public:
   AbstractPipeline(const AbstractPipeline&) = delete;
   AbstractPipeline(AbstractPipeline&&) = delete;
   AbstractPipeline& operator=(const AbstractPipeline&) = delete;
   AbstractPipeline& operator=(AbstractPipeline&&) = delete;

   virtual ~AbstractPipeline() = default;

   [[nodiscard]] const vk::raii::Pipeline& getPipeline() const {
      return *pipeline;
   };

   [[nodiscard]] const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const {
      return *descriptorSetLayout;
   };

   [[nodiscard]] virtual const vk::raii::PipelineLayout& getPipelineLayout() const = 0;

 protected:
   AbstractPipeline() = default;

   std::string readShaderFile(const std::string_view& filename) const;

   std::unique_ptr<vk::raii::Pipeline> pipeline;

   std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout;

   const vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
       .topology = vk::PrimitiveTopology::eTriangleList, .primitiveRestartEnable = VK_FALSE};

   const vk::PipelineRasterizationStateCreateInfo rasterizer{.depthClampEnable = false,
                                                             .rasterizerDiscardEnable = VK_FALSE,
                                                             .polygonMode = vk::PolygonMode::eFill,
                                                             .cullMode =
                                                                 vk::CullModeFlagBits::eBack,
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
