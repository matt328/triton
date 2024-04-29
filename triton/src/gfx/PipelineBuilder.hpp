#pragma once

#include "geometry/Vertex.hpp"

namespace tr::gfx {
   class PipelineBuilder {
    public:
      PipelineBuilder(const vk::raii::Device& device);

      void clear();

      void setDefaultVertexAttributeDescriptions();
      void setVertexAttributeDescriptions(std::span<geo::VertexComponent> components);

      void clearShaderStages();
      void setVertexShaderStage(const vk::raii::ShaderModule& vertexShaderModule);
      void setFragmentShaderStage(const vk::raii::ShaderModule& fragmentShaderModule);
      void setTessellationStages(const vk::raii::ShaderModule& controlModule,
                                 const vk::raii::ShaderModule evaluationModule);
      void setInputTopology(vk::PrimitiveTopology topology);
      void setPolygonMode(vk::PolygonMode polygonMode);
      void setCullMode(vk::CullModeFlagBits cullModeFlagBits, vk::FrontFace frontFace);
      void setMultisamplingNone();
      void disableBlending();
      void setColorAttachmentFormat(vk::Format format);
      void setDepthFormat(vk::Format format);
      void setDefaultDepthStencil();
      void setSetLayouts(const std::span<vk::DescriptorSetLayout>& layouts);

      auto buildPipelineLayout(const std::span<vk::DescriptorSetLayout>& layouts)
          -> std::unique_ptr<vk::raii::PipelineLayout>;
      auto buildPipeline(const vk::raii::PipelineLayout& pipelineLayout)
          -> std::unique_ptr<vk::raii::Pipeline>;

    private:
      const vk::raii::Device& device;

      std::array<vk::DynamicState, 2> dynamicStates = std::array<vk::DynamicState, 2>{
          vk::DynamicState::eViewportWithCount,
          vk::DynamicState::eScissorWithCount,
      };

      vk::PipelineDynamicStateCreateInfo dsci{.dynamicStateCount = 2,
                                              .pDynamicStates = dynamicStates.data()};

      std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
      std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
      vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
      vk::PipelineRasterizationStateCreateInfo rasterizer;
      vk::PipelineColorBlendAttachmentState colorBlendAttachment;
      vk::PipelineMultisampleStateCreateInfo multisampling;
      vk::PipelineDepthStencilStateCreateInfo depthStencil;
      vk::PipelineRenderingCreateInfo renderInfo;
      vk::Format colorAttachmentformat;

      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
   };
}
