#pragma once

#include "Frame.hpp"
#include "VkContext.hpp"
#include "as/Vertex.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "mem/Allocator.hpp"
#include "mem/MultiBuffer.hpp"

namespace tr::gfx::geo {

   struct InstanceData {
      uint32_t instanceOffset;
   };

   struct MeshData {
      uint32_t vertexOffset;
      uint32_t vertexCount;
      uint32_t instanceCount;
      uint32_t firstInstance;
   };

   struct alignas(16) GpuInstanceData {
      glm::mat4 modelMatrix;
      alignas(4) uint32_t visible;
   };

   struct GroupHandle {
      size_t meshId;
      size_t instanceId;
   };

   using GroupInfo = std::tuple<size_t, size_t>;
   using AddMeshCompleteFn = std::function<void(size_t)>;

   class RenderGroup {
    public:
      explicit RenderGroup(const std::shared_ptr<VkContext>& context,
                           const std::shared_ptr<mem::Allocator>& allocator,
                           size_t initialBufferSize = 1024);
      ~RenderGroup() = default;

      RenderGroup(const RenderGroup&) = delete;
      RenderGroup(RenderGroup&&) = delete;
      auto operator=(const RenderGroup&) -> RenderGroup& = delete;
      auto operator=(RenderGroup&&) -> RenderGroup& = delete;

      auto addMesh(const std::vector<as::Vertex>& vertexData, const AddMeshCompleteFn& onComplete);
      auto addInstance(cm::EntityType entityId, size_t meshId, glm::mat4 modelMatrix) -> GroupInfo;

      auto removeInstance(cm::EntityType entityId, size_t instanceId) -> void;
      auto render(Frame& frame, vk::raii::CommandBuffer& commandBuffer) -> void;

    private:
      std::unique_ptr<mem::MultiBuffer> vertexBuffer;
      std::unordered_map<size_t, MeshData> meshDataMap;
      std::shared_ptr<VkContext> transferContext;
      std::shared_ptr<mem::Allocator> allocator;
   };

}
