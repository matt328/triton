#include "RenderGroup.hpp"
#include "cm/Handles.hpp"
#include "Frame.hpp"

namespace tr::gfx::geo {

   RenderGroup::RenderGroup(const std::shared_ptr<VkContext>& context,
                            const std::shared_ptr<mem::Allocator>& allocator,
                            size_t initialBufferSize)
       : transferContext(context), allocator(allocator) {
   }

   void RenderGroup::registerFrameData(const FrameManager& frameManager) const {
      frameManager.registerStorageBuffer("buffer", 512);
   }

   auto RenderGroup::addInstance(size_t meshId, glm::mat4 modelMatrix) -> cm::GroupHandle {
      assert(meshDataMap.find(meshId) != meshDataMap.end());
      const auto instanceId = instanceDataList.size();
      instanceDataList.emplace_back(modelMatrix, true);
      return {meshId, instanceId};
   }

   auto RenderGroup::removeInstance(size_t instanceId) -> void {
   }

   auto RenderGroup::render(Frame& frame, vk::raii::CommandBuffer& commandBuffer) -> void {

      frame.updateStorageBuffer("buffer", data);

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
