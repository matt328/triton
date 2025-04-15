#pragma once

namespace tr {

struct AllocatedImage {
  vk::Image image;
  vma::Allocation allocation;
};

struct ImageDeleter {
  vma::Allocator allocator;

  void operator()(AllocatedImage* image) const {
    if (image != nullptr) {
      allocator.destroyImage(image->image, image->allocation);
      delete image;
    }
  }
};

class ManagedImage {
public:
  ManagedImage(std::unique_ptr<AllocatedImage> newImage,
               vk::raii::ImageView newImageView,
               vk::Extent2D newExtent)
      : image(std::move(newImage)), imageView(std::move(newImageView)), extent(newExtent) {
  }

  ~ManagedImage() = default;

  ManagedImage(const ManagedImage&) = delete;
  ManagedImage(ManagedImage&&) = delete;
  auto operator=(const ManagedImage&) -> ManagedImage& = delete;
  auto operator=(ManagedImage&&) -> ManagedImage& = delete;

  [[nodiscard]] auto getImageView() const -> const vk::ImageView& {
    return *imageView;
  }

  [[nodiscard]] auto getExtent() const -> vk::Extent2D {
    return extent;
  }

  [[nodiscard]] auto getImage() const -> const vk::Image& {
    return image->image;
  }

private:
  std::unique_ptr<AllocatedImage> image;
  vk::raii::ImageView imageView;
  vk::Extent2D extent;
};

}
