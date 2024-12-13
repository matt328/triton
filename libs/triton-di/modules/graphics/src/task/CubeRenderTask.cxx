#include "CubeRenderTask.hpp"

namespace tr::gfx::task {
   auto CubeRenderTask::record(vk::raii::CommandBuffer& commandBuffer) -> void {
      // Define vertex data for a unit cube
      std::vector<float> const vertices = {
          // Positions         // Colors (optional)
          -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // Vertex 0
          0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 1
          0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, // Vertex 2
          -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f, // Vertex 3
          -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, // Vertex 4
          0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, // Vertex 5
          0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, // Vertex 6
          -0.5f, 0.5f,  0.5f,  0.5f, 0.5f, 0.5f  // Vertex 7
      };

      // Define indices for the cube
      std::vector<uint16_t> const indices = {
          0, 1, 2, 2, 3, 0, // Back face
          4, 5, 6, 6, 7, 4, // Front face
          0, 4, 7, 7, 3, 0, // Left face
          1, 5, 6, 6, 2, 1, // Right face
          3, 2, 6, 6, 7, 3, // Top face
          0, 1, 5, 5, 4, 0  // Bottom face
      };

      // Bind the graphics pipeline
      // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
      // commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
      // commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
      //
      // commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
      //                                  *pipelineLayout,
      //                                  0,
      //                                  *descriptorSet,
      //                                  nullptr);
      // // Draw the cube
      // commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
   }
}
