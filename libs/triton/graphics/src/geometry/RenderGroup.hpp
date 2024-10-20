#pragma once

#include "Frame.hpp"
#include "VkContext.hpp"
#include "as/Vertex.hpp"
#include "mem/Allocator.hpp"
#include "mem/MultiBuffer.hpp"

namespace tr::gfx::geo {

   struct InstanceData {
      glm::mat4 modelMatrix;
      bool visible;
   };

   struct MeshData {
      uint32_t vertexOffset;
      uint32_t vertexCount;
      uint32_t instanceCount;
      uint32_t firstInstance;
      std::vector<size_t> instanceDataList;
   };

   struct alignas(16) GpuInstanceData {
      glm::mat4 modelMatrix;
      alignas(4) uint32_t visible;
   };

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

      auto addMesh(const mem::HasDataAndSize auto& vertexData) -> size_t {
         const auto hashValue = vertexListHash(vertexData);

         if (meshDataMap.find(hashValue) == meshDataMap.end()) {
            const auto offset = vertexBuffer->addAndUploadData(vertexData);
            const auto vertexCount = static_cast<uint32_t>(vertexData.size());
            meshDataMap[hashValue] = MeshData{offset, vertexCount, 0, 0};
         }
         return hashValue;
      }

      auto addInstance(size_t meshId, glm::mat4 modelMatrix) -> cm::GroupHandle;

      auto removeInstance(size_t instanceId) -> void;
      auto render(Frame& frame, vk::raii::CommandBuffer& commandBuffer) -> void;

      void registerFrameData(const FrameManager& frameManager) const;

    private:
      std::unique_ptr<mem::MultiBuffer> vertexBuffer;

      std::unordered_map<size_t, MeshData> meshDataMap;
      std::vector<InstanceData> instanceDataList;

      std::shared_ptr<VkContext> transferContext;
      std::shared_ptr<mem::Allocator> allocator;
   };

}
