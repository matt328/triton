#pragma once

#include "gfx/GraphicsDevice.hpp"
namespace tr::gfx {

   class GraphicsDevice;

   class Pipeline {
    public:
      Pipeline(const GraphicsDevice& graphicsDevice,
               const vk::PipelineLayoutCreateInfo& pipelineLayoutCreateInfo,
               const vk::PipelineVertexInputStateCreateInfo,
               const std::string_view& vertexShaderName,
               const std::string_view& fragmentShaderName);
      ~Pipeline();

      Pipeline(const Pipeline&) = delete;
      Pipeline(Pipeline&&) = delete;
      Pipeline& operator=(const Pipeline&) = delete;
      Pipeline& operator=(Pipeline&&) = delete;

      void recreatePipeline(const GraphicsDevice& graphicsDevice);

      void bind(const vk::raii::CommandBuffer& cmd);

    private:
      std::unique_ptr<vk::raii::Pipeline> pipeline;
      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
      std::unique_ptr<vk::raii::ShaderModule> vertexShaderModule;
      std::unique_ptr<vk::raii::ShaderModule> fragmentShaderModule;
   };
}