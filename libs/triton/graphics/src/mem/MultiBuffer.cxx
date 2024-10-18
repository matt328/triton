#include "MultiBuffer.hpp"
#include "as/Vertex.hpp"
#include "mem/Allocator.hpp"

namespace tr::gfx::mem {

   constexpr size_t StagingVertexCount = 12;
   constexpr size_t MaxVertexCount = 256;

   MultiBuffer::MultiBuffer(std::shared_ptr<mem::Allocator> allocator,
                            std::shared_ptr<VkContext> transferContext,
                            std::shared_ptr<vk::raii::Device> device)
       : allocator(std::move(allocator)),
         transferContext(std::move(transferContext)),
         device(std::move(device)) {

      stagingBuffer = allocator->createStagingBuffer(sizeof(as::Vertex) * StagingVertexCount);
      stagingBuffer->mapBuffer();
      buffer = allocator->createGpuVertexBuffer(sizeof(as::Vertex) * MaxVertexCount);
   }

   MultiBuffer::~MultiBuffer() {
   }

   auto MultiBuffer::addAndUploadData(const std::vector<tr::as::Vertex>& vertexData) -> uint32_t {

      const auto size = sizeof(as::Vertex) * vertexData.size();

      stagingBuffer->updateMappedBufferValue(vertexData.data(), size);

      transferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
         const auto vbCopy =
             vk::BufferCopy{.srcOffset = 0, .dstOffset = currentOffset, .size = size};
         cmd.copyBuffer(stagingBuffer->getBuffer(), buffer->getBuffer(), vbCopy);
      });

      bufferEntries.emplace_back(currentOffset, size);

      currentOffset = +size;
      return currentOffset;
   }
}