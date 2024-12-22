#pragma once

namespace tr {
   class Image {
    public:
      Image(const Image&) = default;
      Image(Image&&) = delete;
      auto operator=(const Image&) -> Image& = default;
      auto operator=(Image&&) -> Image& = delete;

      Image(const vma::Allocator& newAllocator, vk::Image newImage, vma::Allocation newAllocation);

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