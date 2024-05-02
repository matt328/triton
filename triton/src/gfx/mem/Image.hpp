#pragma once

namespace tr::gfx::mem {
   class Image {
    public:
      Image(const Image&) = default;
      Image(Image&&) = delete;
      Image& operator=(const Image&) = default;
      Image& operator=(Image&&) = delete;

      Image(const vma::Allocator& newAllocator,
            const vk::Image newImage,
            const vma::Allocation newAllocation);

      ~Image();

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