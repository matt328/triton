#pragma once

namespace tr::gfx {

   class GraphicsDevice;

   class Pipeline {
    public:
      Pipeline(const GraphicsDevice& graphicsDevice,
               const vk::PipelineLayoutCreateInfo& pipelineLayoutCreateInfo,
               const vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo,
               const vk::PipelineRenderingCreateInfo& renderingCreateInfo,
               const std::filesystem::path& vertexShaderName,
               const std::filesystem::path& fragmentShaderName);
      ~Pipeline();

      Pipeline(const Pipeline&) = delete;
      Pipeline(Pipeline&&) = delete;
      Pipeline& operator=(const Pipeline&) = delete;
      Pipeline& operator=(Pipeline&&) = delete;

      void resize(const vk::Extent2D newSize);

      void bind(const vk::raii::CommandBuffer& cmd);

      [[nodiscard]] vk::PipelineLayout getPipelineLayout() const {
         return **pipelineLayout;
      }

    private:
      vk::Rect2D scissor;
      vk::Viewport viewport;
      std::unique_ptr<vk::raii::Pipeline> pipeline;
      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
      std::unique_ptr<vk::raii::ShaderModule> vertexShaderModule;
      std::unique_ptr<vk::raii::ShaderModule> fragmentShaderModule;

      std::string readShaderFile(const std::filesystem::path& filepath);
   };
}
