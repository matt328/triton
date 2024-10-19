#include "RenderGroup.hpp"

namespace tr::gfx::geo {

   RenderGroup::RenderGroup(const std::shared_ptr<VkContext>& context,
                            const std::shared_ptr<mem::Allocator>& allocator,
                            size_t initialBufferSize)
       : transferContext(context), allocator(allocator) {
   }

   auto RenderGroup::addMesh(const std::vector<as::Vertex>& vertexData) -> size_t {
      const auto hashValue = vertexListHash(vertexData);

      if (meshDataMap.find(hashValue) == meshDataMap.end()) {
         const auto offset = vertexBuffer->addAndUploadData(vertexData);
         const auto vertexCount = static_cast<uint32_t>(vertexData.size());
         meshDataMap[hashValue] = MeshData{offset, vertexCount, 0, 0};
      }
      return hashValue;
   }

   auto RenderGroup::addInstance(size_t meshId, glm::mat4 modelMatrix) -> GroupInfo {
      if (meshDataMap.find(meshId) != meshDataMap.end()) {}
   }

   auto RenderGroup::removeInstance(size_t instanceId) -> void {
   }

   auto RenderGroup::render(Frame& frame, vk::raii::CommandBuffer& commandBuffer) -> void {

      // frame.updateObjectDataBuffer(nullptr, 0);
      // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

      // frame.getObjectDataShaderBinding().bindToPipeline(commandBuffer,
      //                                                   vk::PipelineBindPoint::eGraphics,
      //                                                   0,
      //                                                   pipelineLayout);

      // vertexBuffer.bind(commandBuffer);

      /*
         for(auto mesh : meshes) {
            commandBuffer.draw(mesh.vertexCount, instanceCount, vertexOffset, instanceOffset);
         }
      */
   }

}
