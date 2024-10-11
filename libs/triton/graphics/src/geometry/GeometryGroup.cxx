#include "GeometryGroup.hpp"

namespace tr::gfx::geo {

   GeometryGroup::GeometryGroup(std::shared_ptr<VkContext> context,
                                std::shared_ptr<mem::Allocator> allocator,
                                size_t initialBufferSize)
       : transferContext(std::move(context)), allocator(std::move(allocator)) {
      // Create a staging buffer
      // Create the dst buffer

      // keep track of the staging buffer offset after adding each meshInstance
      // reset the offset after copying the staging to the dst
      // need to keep track of the offset into the dst?

      // what about removing?
   }

   auto GeometryGroup::addMeshInstance(cm::EntityType entityId, std::vector<as::Vertex> vertexData)
       -> std::tuple<size_t, size_t> {
      return {0, 0};
   }

   auto GeometryGroup::removeInstance(cm::EntityType entityId, size_t instanceId) -> void {
   }

   auto GeometryGroup::render(vk::raii::CommandBuffer& commandBuffer) -> void {
   }

} // namespace tr::gfx::geo
