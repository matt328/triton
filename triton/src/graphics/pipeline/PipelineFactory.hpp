#pragma once

namespace Graphics::Pipeline {
   const auto defaultInputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
       .topology = vk::PrimitiveTopology::eTriangleList, .primitiveRestartEnable = VK_FALSE};

   const auto defaultRasterizer =
       vk::PipelineRasterizationStateCreateInfo{.depthClampEnable = false,
                                                .rasterizerDiscardEnable = VK_FALSE,
                                                .polygonMode = vk::PolygonMode::eFill,
                                                .cullMode = vk::CullModeFlagBits::eBack,
                                                .frontFace = vk::FrontFace::eClockwise,
                                                .depthBiasEnable = VK_FALSE,
                                                .lineWidth = 1.f};

   const auto disableMultisampling = vk::PipelineMultisampleStateCreateInfo{
       .rasterizationSamples = vk::SampleCountFlagBits::e1,
       .sampleShadingEnable = VK_FALSE,
   };

   const auto defaultColorBlendAttachment = vk::PipelineColorBlendAttachmentState{
       .blendEnable = VK_FALSE,
       .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

   const auto defaultColorBlending =
       vk::PipelineColorBlendStateCreateInfo{.logicOpEnable = VK_FALSE,
                                             .attachmentCount = 1,
                                             .pAttachments = &defaultColorBlendAttachment};

   const auto defaultDepthStencil = vk::PipelineDepthStencilStateCreateInfo{
       .depthTestEnable = VK_TRUE,
       .depthWriteEnable = VK_TRUE,
       .depthCompareOp = vk::CompareOp::eLess,
       .depthBoundsTestEnable = VK_FALSE,
       .stencilTestEnable = VK_FALSE,
   };

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
      throw std::runtime_error(std::format("Failed to read shader from file {}", filename.data()));
   }
}