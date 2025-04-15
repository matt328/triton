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

auto ImageManager::createImage(const ImageUsageProfile& profile) -> Handle<ManagedImage> {
  const auto imageCreateInfo = vk::ImageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = profile.format,
      .extent =
          vk::Extent3D{.width = profile.extent.width, .height = profile.extent.height, .depth = 1},
      .mipLevels = profile.mipLevels,
      .arrayLayers = profile.layers,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = profile.usage,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};
  constexpr auto imageAllocateCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  auto [image, allocation] =
      allocator->getAllocator()->createImage(imageCreateInfo, imageAllocateCreateInfo);

  if (profile.debugName) {
    debugManager->setObjectName(image, *profile.debugName);
  }

  const auto imageViewInfo =
      vk::ImageViewCreateInfo{.image = image,
                              .viewType = vk::ImageViewType::e2D,
                              .format = profile.format,
                              .subresourceRange = {
                                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                                  .levelCount = 1,
                                  .layerCount = 1,
                              }};
  const auto key = generator.requestHandle();

  imageMap.emplace(
      key,
      std::make_unique<ManagedImage>(std::make_unique<AllocatedImage>(
                                         AllocatedImage{.image = image, .allocation = allocation}),
                                     device->getVkDevice().createImageView(imageViewInfo),
                                     profile.extent));
  return key;
}

auto ImageManager::getImage(Handle<ManagedImage> imageHandle) -> ManagedImage& {
  assert(imageMap.contains(imageHandle));
  return *imageMap.at(imageHandle);
}

}
