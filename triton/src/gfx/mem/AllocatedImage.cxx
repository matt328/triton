#include "AllocatedImage.hpp"

namespace tr::gfx::mem {
   AllocatedImage::AllocatedImage(const vma::Allocator& newAllocator,
                                  const vk::Image newImage,
                                  const vma::Allocation newAllocation)
       : image(newImage), allocation(newAllocation), allocator(newAllocator) {
   }

   AllocatedImage::~AllocatedImage() {
      allocator.destroyImage(image, allocation);
   }
}