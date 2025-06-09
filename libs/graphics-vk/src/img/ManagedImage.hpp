#pragma once

namespace tr {

class AllocatedImage {
public:
  AllocatedImage(vk::Image newImage, vma::Allocation newAllocation, vma::Allocator newAllocator)
      : image{newImage}, allocation{newAllocation}, allocator{newAllocator} {
  }

  ~AllocatedImage() {
    allocator.destroyImage(image, allocation);
  }

  AllocatedImage(const AllocatedImage&) = default;
  AllocatedImage(AllocatedImage&&) = delete;
  auto operator=(const AllocatedImage&) -> AllocatedImage& = default;
  auto operator=(AllocatedImage&&) -> AllocatedImage& = delete;

  auto getImage() -> vk::Image& {
    return image;
  }

private:
  vk::Image image;
  vma::Allocation allocation;
  vma::Allocator allocator;
};

class ManagedImage {
public:
  ManagedImage(std::unique_ptr<AllocatedImage> newImage,
               vk::raii::ImageView newImageView,
               vk::Extent2D newExtent,
               vk::Format newFormat,
               vk::ImageUsageFlags newFlags)
      : image(std::move(newImage)),
        imageView(std::move(newImageView)),
        extent(newExtent),
        format(newFormat),
        usageFlags(newFlags) {
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
    return image->getImage();
  }

  [[nodiscard]] auto getFormat() const -> vk::Format {
    return format;
  }

  [[nodiscard]] auto getUsageFlags() const -> vk::ImageUsageFlags {
    return usageFlags;
  }

private:
  std::unique_ptr<AllocatedImage> image;
  vk::raii::ImageView imageView;
  vk::Extent2D extent;
  vk::Format format;
  vk::ImageUsageFlags usageFlags;
};

}
