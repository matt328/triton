#pragma once

#include "Allocator.hpp"
#include "VkContext.hpp"
#include "as/Vertex.hpp"
#include <vk_mem_alloc_handles.hpp>

namespace tr::gfx::mem {
   class MultiBuffer {
    public:
      MultiBuffer(std::shared_ptr<mem::Allocator> allocator,
                  std::shared_ptr<VkContext> transferContext,
                  std::shared_ptr<vk::raii::Device> device);
      ~MultiBuffer();

      MultiBuffer(const MultiBuffer&) = delete;
      MultiBuffer(MultiBuffer&&) = delete;
      auto operator=(const MultiBuffer&) -> MultiBuffer& = delete;
      auto operator=(MultiBuffer&&) -> MultiBuffer& = delete;

      void addAndUploadData(const std::vector<tr::as::Vertex>& vertexData,
                            const std::function<void(uint32_t)>& onComplete);

    private:
      std::shared_ptr<mem::Allocator> allocator;
      std::shared_ptr<VkContext> transferContext;
      std::shared_ptr<vk::raii::Device> device;

      std::unique_ptr<vk::Buffer> buffer;
      std::unique_ptr<vma::Allocation> allocation;
      vma::AllocationInfo allocationInfo;
   };
}