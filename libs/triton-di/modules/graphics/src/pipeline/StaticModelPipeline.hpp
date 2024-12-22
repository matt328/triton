#pragma once

#include "IPipeline.hpp"
#include "renderer/RendererConfig.hpp"

namespace tr {
   class IGraphicsDevice;
}

namespace tr {

   class IShaderCompiler;

   class StaticModelPipeline : public IPipeline {
    public:
      StaticModelPipeline(const std::shared_ptr<IGraphicsDevice>& graphicsDevice,
                          const std::shared_ptr<IShaderCompiler>& shaderCompiler,
                          const rd::RendererConfig& rendererConfig);
      ~StaticModelPipeline() override = default;

      StaticModelPipeline(const StaticModelPipeline&) = delete;
      StaticModelPipeline(StaticModelPipeline&&) = delete;
      auto operator=(const StaticModelPipeline&) -> StaticModelPipeline& = delete;
      auto operator=(StaticModelPipeline&&) -> StaticModelPipeline& = delete;

      void bind(const vk::raii::CommandBuffer& cmd) override;
      void applyShaderBinding(
          const ShaderBinding& binding,
          uint32_t setIndex,
          const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) override;

    private:
      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
      std::unique_ptr<vk::raii::Pipeline> pipeline;
   };
}
