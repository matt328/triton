#pragma once

#include "VkContext.hpp"
#include "as/Vertex.hpp"
#include "mem/Buffer.hpp"
#include "mem/Allocator.hpp"

namespace tr {

   struct BufferEntry {
      size_t offset;
      size_t size;
   };

   template <typename T>
   concept HasDataAndSize = requires(T t) {
      { t.data() } -> std::convertible_to<const void*>;
      { t.size() } -> std::convertible_to<std::size_t>;
   };

   class MultiBuffer {
    public:
      MultiBuffer(std::shared_ptr<Allocator> allocator,
                  std::shared_ptr<VkContext> transferContext,
                  std::shared_ptr<vk::raii::Device> device);
      ~MultiBuffer();

      MultiBuffer(const MultiBuffer&) = delete;
      MultiBuffer(MultiBuffer&&) = delete;
      auto operator=(const MultiBuffer&) -> MultiBuffer& = delete;
      auto operator=(MultiBuffer&&) -> MultiBuffer& = delete;

      auto addAndUploadData(const HasDataAndSize auto& vertexData) -> uint32_t {
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

      [[nodiscard]] auto getEntries() const {
         return bufferEntries;
      }

    private:
      uint32_t currentOffset = 0;
      std::shared_ptr<Allocator> allocator;
      std::shared_ptr<VkContext> transferContext;
      std::shared_ptr<vk::raii::Device> device;

      std::unique_ptr<Buffer> stagingBuffer;
      std::unique_ptr<Buffer> buffer;

      std::vector<BufferEntry> bufferEntries{};

      void createStagingBuffer();
   };
}