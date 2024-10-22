#include "RenderGroup.hpp"
#include "cm/Handles.hpp"
#include "Frame.hpp"
#include <basetsd.h>
#include <unordered_map>

namespace tr::gfx::geo {

   static constexpr auto INSTANCE_DATA = "instance_data";

   RenderGroup::RenderGroup(const std::shared_ptr<VkContext>& context,
                            const std::shared_ptr<mem::Allocator>& allocator,
                            size_t initialBufferSize)
       : transferContext(context), allocator(allocator) {
   }

   void RenderGroup::registerFrameData(const FrameManager& frameManager) const {
      frameManager.registerStorageBuffer(INSTANCE_DATA, 512);
   }

   auto RenderGroup::addInstance(size_t meshId, glm::mat4 modelMatrix) -> cm::GroupHandle {
      assert(meshDataMap.find(meshId) != meshDataMap.end());
      meshDataMap.at(meshId).instanceCount++;
   }

   auto RenderGroup::removeInstance(size_t instanceId) -> void {
   }

   auto RenderGroup::updateFrameData(const FrameManager& frameManager,
                                     const cm::gpu::RenderData& renderData) const -> void {
      size_t totalSize = 0;
      for (const auto& pair : renderData.instanceData) {
         totalSize += pair.second.size();
      }

      auto allInstanceData = std::vector<cm::gpu::GpuInstanceData>{};
      allInstanceData.reserve(totalSize);

      for (const auto& meshData : renderData.instanceData) {
         std::ranges::copy(meshData.second, std::back_inserter(allInstanceData));
      }

      frameManager.getCurrentFrame().updateStorageBuffer(INSTANCE_DATA,
                                                         allInstanceData.data(),
                                                         sizeof(cm::gpu::GpuInstanceData) *
                                                             allInstanceData.size());
   }

   auto RenderGroup::render(Frame& frame, vk::raii::CommandBuffer& commandBuffer) -> void {

      // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

      // frame.getObjectDataShaderBinding().bindToPipeline(commandBuffer,
      //                                                   vk::PipelineBindPoint::eGraphics,
      //                                                   0,
      //                                                   pipelineLayout);

      // vertexBuffer.bind(commandBuffer);

      for (const auto& [key, meshData] : meshDataMap) {
         meshData.
      }

      /*
         for(auto mesh : meshes) {
            commandBuffer.draw(mesh.vertexCount, instanceCount, vertexOffset, instanceOffset);
         }
      */
   }

}
