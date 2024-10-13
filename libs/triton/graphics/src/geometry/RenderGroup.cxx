#include "RenderGroup.hpp"

namespace tr::gfx::geo {

   RenderGroup::RenderGroup(const std::shared_ptr<VkContext>& context,
                            const std::shared_ptr<mem::Allocator>& allocator,
                            size_t initialBufferSize)
       : transferContext(context), allocator(allocator) {
   }

   auto RenderGroup::addMesh(const std::vector<as::Vertex>& vertexData,
                             const AddMeshCompleteFn& onComplete) {

      auto hashValue = vertexListHash(vertexData);

      auto vertexCount = static_cast<uint32_t>(vertexData.size());

      if (meshDataMap.find(hashValue) != meshDataMap.end()) {
         onComplete(hashValue);
      } else {
         vertexBuffer->addAndUploadData(
             vertexData,
             [this, hashValue, vertexCount, onComplete](uint32_t offset) {
                meshDataMap[hashValue] = MeshData{offset, vertexCount, 0, 0};
                onComplete(hashValue);
             });
      }
   }

   auto RenderGroup::addInstance(cm::EntityType entityId, size_t meshId) -> GroupInfo {
      if (meshDataMap.find(meshId) != meshDataMap.end()) {}
   }

   auto RenderGroup::removeInstance(cm::EntityType entityId, size_t instanceId) -> void {
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
