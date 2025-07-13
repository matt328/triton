#include "ImageProcessor.hpp"
#include "img/ImageManager.hpp"
#include "resources/TransferSystem.hpp"

namespace tr {

ImageProcessor::ImageProcessor(std::shared_ptr<ImageManager> newImageManager,
                               std::shared_ptr<TransferSystem> newTransferSystem)
    : imageManager{std::move(newImageManager)}, transferSystem{std::move(newTransferSystem)} {
}

auto ImageProcessor::processImageData(const as::ImageData& imageData, uint64_t requestId)
    -> ImageUploadData {
  auto uploadDataList = std::vector<ImageUploadData>{};

  // Transform into a byte vector
  auto data = std::vector<std::byte>{};
  data.resize(imageData.data.size());
  std::ranges::transform(imageData.data, data.begin(), [](unsigned char c) {
    return static_cast<std::byte>(c);
  });

  // Create a destination image
  const auto imageHandle = imageManager->createImage({
      .logicalName = "ModelTexture",
      .format = getVkFormat(imageData.bits, imageData.component),
      .extent =
          vk::Extent2D{
              .width = static_cast<uint32_t>(imageData.width),
              .height = static_cast<uint32_t>(imageData.height),
          },
      .usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      .aspectFlags = vk::ImageAspectFlagBits::eColor,
      .debugName = "ModelTexture",
  });

  // Allocate in staging buffer
  auto stagingBufferOffset =
      transferSystem->getTransferContext().imageStagingAllocator->allocate({.size = data.size()});

  // Create bookkeeping data
  return ImageUploadData{
      .data = std::make_shared<std::vector<std::byte>>(data),
      .dataSize = data.size(),
      .dstImage = imageHandle,
      .subresource = vk::ImageSubresourceLayers{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                .mipLevel = 0,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1},
      .imageExtent = vk::Extent3D{.width = static_cast<uint32_t>(imageData.width),
                                  .height = static_cast<uint32_t>(imageData.height),
                                  .depth = 1},
      .stagingBufferOffset = stagingBufferOffset->offset,
      .requestId = requestId};
}

auto ImageProcessor::getVkFormat(int bits, int component) -> vk::Format {
  if (bits == 8) {
    switch (component) {
      case 1:
        return vk::Format::eR8Unorm;
      case 2:
        return vk::Format::eR8G8Unorm;
      case 3:
        return vk::Format::eR8G8B8Unorm;
      case 4:
        return vk::Format::eR8G8B8A8Unorm;
      default:
        return vk::Format::eR8Unorm;
    }
  } else if (bits == 16) {
    switch (component) {
      case 1:
        return vk::Format::eR16Unorm;
      case 2:
        return vk::Format::eR16G16Unorm;
      case 3:
        return vk::Format::eR16G16B16Unorm;
      case 4:
        return vk::Format::eR16G16B16A16Unorm;
      default:
        return vk::Format::eR16Unorm;
    }
  } else if (bits == 32) {
    switch (component) {
      case 1:
        return vk::Format::eR32Sfloat;
      case 2:
        return vk::Format::eR32G32Sfloat;
      case 3:
        return vk::Format::eR32G32B32Sfloat;
      case 4:
        return vk::Format::eR32G32B32A32Sfloat;
      default:
        return vk::Format::eR32Sfloat;
    }
  }

  throw std::runtime_error("Unsupported image format: component=" + std::to_string(component) +
                           ", bits=" + std::to_string(bits));
}

}
