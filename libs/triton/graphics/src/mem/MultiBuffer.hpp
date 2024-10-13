#pragma once

#include "VkContext.hpp"
#include "as/Vertex.hpp"
#include "mem/Buffer.hpp"
#include "mem/Allocator.hpp"

namespace tr::gfx::mem {

   struct BufferEntry {
      size_t offset;
      size_t size;
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

      /// Abstracts away uploading data.
      void addAndUploadData(const std::vector<tr::as::Vertex>& vertexData,
                            const std::function<void(uint32_t)>& onComplete);

      [[nodiscard]] auto getEntries() const {
         return bufferEntries;
      }

    private:
      uint32_t currentOffset = 0;
      std::shared_ptr<Allocator> allocator;
      std::shared_ptr<VkContext> transferContext;
      std::shared_ptr<vk::raii::Device> device;

      std::unique_ptr<mem::Buffer> stagingBuffer;
      std::unique_ptr<mem::Buffer> buffer;

      std::vector<BufferEntry> bufferEntries{};

      void createStagingBuffer();
   };
}