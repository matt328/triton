#pragma once

#include "VkContext.hpp"
#include "as/Vertex.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "mem/Allocator.hpp"
#include "mem/Buffer.hpp"

namespace tr::gfx::geo {

   struct InstanceData {
      uint32_t instanceOffset;
   };

   struct MeshData {
      uint32_t vertexCount;
      uint32_t vertexOffset;
      std::vector<InstanceData> instanceData;
   };

   using GroupInfo = std::tuple<size_t, size_t>;
   using AddMeshCompleteFn = std::function<void(size_t)>;

   class GeometryGroup {
    public:
      explicit GeometryGroup(std::shared_ptr<VkContext> context,
                             std::shared_ptr<mem::Allocator> allocator,
                             size_t initialBufferSize = 1024);
      ~GeometryGroup() = default;

      GeometryGroup(const GeometryGroup&) = delete;
      GeometryGroup(GeometryGroup&&) = delete;
      auto operator=(const GeometryGroup&) -> GeometryGroup& = delete;
      auto operator=(GeometryGroup&&) -> GeometryGroup& = delete;

      auto addMesh(std::vector<as::Vertex> vertexData, const AddMeshCompleteFn& onComplete);
      auto addInstance(cm::EntityType entityId, size_t meshId) -> GroupInfo;
      auto removeInstance(cm::EntityType entityId, size_t instanceId) -> void;
      auto render(vk::raii::CommandBuffer& commandBuffer) -> void;

    private:
      std::unique_ptr<mem::Buffer> vertexBuffer;
      std::unordered_map<size_t, MeshData> meshDataMap;
      std::shared_ptr<VkContext> transferContext;
      std::shared_ptr<mem::Allocator> allocator;
   };

}
