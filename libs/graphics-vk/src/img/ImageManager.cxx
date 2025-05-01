#include "img/ImageManager.hpp"

#include "img/ManagedImage.hpp"
#include "mem/Allocator.hpp"
#include "vk/core/Device.hpp"

namespace tr {

ImageManager::ImageManager(std::shared_ptr<Allocator> newAllocator,
                           std::shared_ptr<IDebugManager> newDebugManager,
                           std::shared_ptr<Device> newDevice)
    : allocator{std::move(newAllocator)},
      debugManager{std::move(newDebugManager)},
      device{std::move(newDevice)} {
}

ImageManager::~ImageManager() {
  Log.trace("Destroying ImageManager");
  imageMap.clear();
}

auto ImageManager::createImage(const ImageRequest& request) -> Handle<ManagedImage> {
  const auto imageCreateInfo = vk::ImageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = request.format,
      .extent =
          vk::Extent3D{.width = request.extent.width, .height = request.extent.height, .depth = 1},
      .mipLevels = request.mipLevels,
      .arrayLayers = request.layers,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = request.usageFlags,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};
  constexpr auto imageAllocateCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  auto [image, allocation] =
      allocator->getAllocator()->createImage(imageCreateInfo, imageAllocateCreateInfo);

  if (request.debugName) {
    debugManager->setObjectName(image, *request.debugName);
  }

  const auto imageViewInfo = vk::ImageViewCreateInfo{.image = image,
                                                     .viewType = vk::ImageViewType::e2D,
                                                     .format = request.format,
                                                     .subresourceRange = {
                                                         .aspectMask = request.aspectFlags,
                                                         .levelCount = 1,
                                                         .layerCount = 1,
                                                     }};
  const auto key = generator.requestHandle();

  imageMap.emplace(
      key,
      std::make_unique<ManagedImage>(
          std::make_unique<AllocatedImage>(image, allocation, *allocator->getAllocator()),
          device->getVkDevice().createImageView(imageViewInfo),
          request.extent));
  return key;
}

auto ImageManager::getImage(Handle<ManagedImage> imageHandle) -> ManagedImage& {
  assert(imageMap.contains(imageHandle));
  return *imageMap.at(imageHandle);
}

}
