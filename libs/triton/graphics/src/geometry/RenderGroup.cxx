#include "RenderGroup.hpp"
#include "Frame.hpp"

namespace tr::gfx::geo {

   static constexpr auto INSTANCE_DATA = "instance_data";

   RenderGroup::RenderGroup(const std::shared_ptr<VkContext>& context,
                            const std::shared_ptr<mem::Allocator>& allocator,
                            size_t initialBufferSize)
       : transferContext(context), allocator(allocator) {
   }

   void RenderGroup::registerFrameData(const FrameManager& frameManager) {
      frameManager.registerStorageBuffer(INSTANCE_DATA, 512);
   }

   auto RenderGroup::addInstance(const size_t meshId) -> void {
      assert(meshDataMap.contains(meshId));
      meshDataMap.at(meshId).instanceCount++;
   }

   auto RenderGroup::removeInstance(const size_t meshId) -> void {
      assert(meshDataMap.contains(meshId));
      meshDataMap.at(meshId).instanceCount--;
   }

   auto RenderGroup::updateFrameData(const FrameManager& frameManager,
                                     const cm::gpu::RenderData& renderData) -> void {
      size_t totalSize = 0;
      for (const auto& instanceDataList : renderData.instanceData | std::views::values) {
         totalSize += instanceDataList.size();
      }

      auto allInstanceData = std::vector<cm::gpu::GpuInstanceData>{};
      allInstanceData.reserve(totalSize);

      for (const auto& instanceData : renderData.instanceData | std::views::values) {
         std::ranges::copy(instanceData, std::back_inserter(allInstanceData));
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

      // for (const auto& [key, meshData] : meshDataMap) {}

      /*
         for(auto mesh : meshes) {
            commandBuffer.draw(mesh.vertexCount, instanceCount, vertexOffset, instanceOffset);
         }
      */
   }

}
