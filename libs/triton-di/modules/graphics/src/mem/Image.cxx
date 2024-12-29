#include "Image.hpp"

namespace tr {
Image::Image(const vma::Allocator& newAllocator,
             const vk::Image newImage,
             const vma::Allocation newAllocation)
    : image(newImage), allocation(newAllocation), allocator(newAllocator) {
}

Image::~Image() {
  allocator.destroyImage(image, allocation);
}
}
