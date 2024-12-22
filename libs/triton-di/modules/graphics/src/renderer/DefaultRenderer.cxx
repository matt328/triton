#include "DefaultRenderer.hpp"
#include "cm/RenderData.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "pipeline/StaticModelPipeline.hpp"
#include "geo/Mesh.hpp"

namespace tr {
   DefaultRenderer::DefaultRenderer(const RendererConfig& config,
                                    std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                                    const std::shared_ptr<IShaderCompiler>& shaderCompiler)
       : graphicsDevice{std::move(newGraphicsDevice)} {
      Log.trace("Constructing DefaultRenderer");
      pipeline =
          std::make_shared<StaticModelPipeline>(graphicsDevice, shaderCompiler, config);
   }

   void DefaultRenderer::bindPipeline(
       const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) {
      pipeline->bind(*commandBuffer);
   }

   void DefaultRenderer::applyShaderBinding(
       const std::unique_ptr<ShaderBinding>& binding,
       uint32_t setIndex,
       const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer) {
      pipeline->applyShaderBinding(*binding, setIndex, commandBuffer);
   }

   void DefaultRenderer::render(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer,
                                std::span<cm::gpu::MeshData> meshDataList,
                                const std::tuple<vk::Viewport, vk::Rect2D>& vpScissor) {
      const auto [vp, scissor] = vpScissor;
      commandBuffer->setViewportWithCount(vp);
      commandBuffer->setScissorWithCount(scissor);
      {
         ZoneNamedN(zone3, "Render Static Meshes", true);
         for (const auto& meshData : meshDataList) {
            const auto& mesh = graphicsDevice->getMesh(meshData.handle);

            commandBuffer->bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
            commandBuffer->bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(),
                                           0,
                                           vk::IndexType::eUint32);

            // instanceId becomes gl_BaseInstance in the shader
            commandBuffer->drawIndexed(mesh.getIndicesCount(), 1, 0, 0, meshData.objectDataId);
         }
      }
   }
}
