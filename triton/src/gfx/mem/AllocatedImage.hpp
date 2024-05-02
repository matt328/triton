#pragma once

namespace tr::gfx::mem {
   class AllocatedImage {
    public:
      AllocatedImage(const AllocatedImage&) = default;
      AllocatedImage(AllocatedImage&&) = delete;
      AllocatedImage& operator=(const AllocatedImage&) = default;
      AllocatedImage& operator=(AllocatedImage&&) = delete;

      AllocatedImage(const vma::Allocator& newAllocator,
                     const vk::Image newImage,
                     const vma::Allocation newAllocation);

      ~AllocatedImage();

      [[nodiscard]] const vk::Image& getImage() const {
         return image;
      }

      [[nodiscard]] const vma::Allocation& getAllocation() const {
         return allocation;
      }

    private:
      vk::Image image;
      vma::Allocation allocation;
      vma::Allocator allocator;
   };
}