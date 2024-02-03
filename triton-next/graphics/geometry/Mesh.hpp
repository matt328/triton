#pragma once
#include "../ImmediateContext.hpp"
#include "../vma_raii.hpp"

namespace Triton::Graphics::Geometry {

   template <typename T, typename R>
   class Mesh {
    public:
      Mesh(Allocator* allocator,
           const std::vector<T>& vertexData,
           const std::vector<R>& indexData,
           ImmediateContext* transferContext)
          : indicesCount(indexData.size()) {

         auto size = sizeof(vertexData[0]) * vertexData.size();

         {
            const auto stagingBuffer = allocator->createStagingBuffer(size, "Mesh Vertex Staging");

            void* data = allocator->mapMemory(*stagingBuffer);
            memcpy(data, vertexData.data(), static_cast<size_t>(size));
            allocator->unmapMemory(*stagingBuffer);

            vertexBuffer = allocator->createGpuVertexBuffer(size, "GPU Vertex");

            transferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
               const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = size};
               cmd.copyBuffer(stagingBuffer->getBuffer(), vertexBuffer->getBuffer(), copy);
            });
         }

         auto indexSize = sizeof(indexData[0]) * indexData.size();

         {
            const auto stagingBuffer =
                allocator->createStagingBuffer(indexSize, "Mesh Index Staging");
            auto data = allocator->mapMemory(*stagingBuffer);
            memcpy(data, indexData.data(), static_cast<size_t>(indexSize));
            allocator->unmapMemory(*stagingBuffer);

            indexBuffer = allocator->createGpuIndexBuffer(indexSize, "GPU Index");

            transferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
               const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = indexSize};
               cmd.copyBuffer(stagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
            });
         }
      };

      Mesh(const Mesh&) = delete;
      Mesh(Mesh&&) = delete;
      Mesh& operator=(const Mesh&) = delete;
      Mesh& operator=(Mesh&&) = delete;

      [[nodiscard]] const AllocatedBuffer& getVertexBuffer() const {
         return *vertexBuffer;
      }

      [[nodiscard]] const AllocatedBuffer& getIndexBuffer() const {
         return *indexBuffer;
      }

      [[nodiscard]] uint32_t getIndicesCount() const {
         return indicesCount;
      }

      ~Mesh() = default;

    private:
      std::unique_ptr<AllocatedBuffer> vertexBuffer;
      std::unique_ptr<AllocatedBuffer> indexBuffer;
      uint32_t indicesCount;
   };
}