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

// TODO(Images) This class can have images or external images, and its intent could be
// expressed more clearly.
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

  ManagedImage(vk::Image externalImage,
               vk::ImageView newImageView,
               vk::Extent2D newExtent,
               vk::Format newFormat,
               vk::ImageUsageFlags newFlags)
      : image(nullptr), // no AllocatedImage
        externalImage(externalImage),
        imageView(nullptr),
        externalImageView(newImageView),
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
    return image ? *imageView : externalImageView;
  }

  [[nodiscard]] auto getExtent() const -> vk::Extent2D {
    return extent;
  }

  [[nodiscard]] auto getImage() const -> const vk::Image& {
    return image ? image->getImage() : externalImage;
  }

  [[nodiscard]] auto getFormat() const -> vk::Format {
    return format;
  }

  [[nodiscard]] auto getUsageFlags() const -> vk::ImageUsageFlags {
    return usageFlags;
  }

  auto setExternalImage(vk::Image newImage) -> void {
    this->externalImage = newImage;
  }

  auto setExternalImageView(vk::ImageView newImageView) -> void {
    this->externalImageView = newImageView;
  }

private:
  std::unique_ptr<AllocatedImage> image = nullptr;
  vk::Image externalImage;
  vk::raii::ImageView imageView;
  vk::ImageView externalImageView;
  vk::Extent2D extent;
  vk::Format format;
  vk::ImageUsageFlags usageFlags;
};

}
