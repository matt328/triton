#pragma once
#include "graphics/ImmediateContext.hpp"
#include "core/vma_raii.hpp"

template <typename T, typename R>
class Mesh {
 public:
   Mesh(vma::raii::Allocator* allocator,
        const std::vector<T>& vertexData,
        const std::vector<R>& indexData,
        ImmediateContext* transferContext) :
       indicesCount(indexData.size()) {

      const auto a = 5; // This has to be here or else clang-format flips out
      {
         auto size = sizeof(vertexData[0]) * vertexData.size();

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

      {
         auto size = sizeof(indexData[0]) * indexData.size();

         const auto stagingBuffer = allocator->createStagingBuffer(size, "Mesh Index Staging");
         auto data = allocator->mapMemory(*stagingBuffer);
         memcpy(data, indexData.data(), static_cast<size_t>(size));
         allocator->unmapMemory(*stagingBuffer);

         indexBuffer = allocator->createGpuIndexBuffer(size, "GPU Index");

         transferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
            const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = size};
            cmd.copyBuffer(stagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
         });
      }
   };

   Mesh(const Mesh&) = delete;
   Mesh(Mesh&&) = delete;
   Mesh& operator=(const Mesh&) = delete;
   Mesh& operator=(Mesh&&) = delete;

   [[nodiscard]] const vma::raii::AllocatedBuffer& getVertexBuffer() const {
      return *vertexBuffer;
   }

   [[nodiscard]] const vma::raii::AllocatedBuffer& getIndexBuffer() const {
      return *indexBuffer;
   }

   [[nodiscard]] uint32_t getIndicesCount() const {
      return indicesCount;
   }

   ~Mesh() = default;

 private:
   std::unique_ptr<vma::raii::AllocatedBuffer> vertexBuffer;
   std::unique_ptr<vma::raii::AllocatedBuffer> indexBuffer;
   uint32_t indicesCount;
};