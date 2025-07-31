#include "img/ImageManager.hpp"

#include "gfx/IFrameManager.hpp"
#include "img/ManagedImage.hpp"
#include "mem/Allocator.hpp"
#include "task/Frame.hpp"
#include "vk/core/Device.hpp"

namespace tr {

ImageManager::ImageManager(std::shared_ptr<Allocator> newAllocator,
                           std::shared_ptr<IDebugManager> newDebugManager,
                           std::shared_ptr<Device> newDevice,
                           std::shared_ptr<IFrameManager> newFrameManager,
                           std::shared_ptr<Swapchain> newSwapchain)
    : allocator{std::move(newAllocator)},
      debugManager{std::move(newDebugManager)},
      device{std::move(newDevice)},
      frameManager{std::move(newFrameManager)},
      swapchain{std::move(newSwapchain)} {
  Log.trace("Creating ImageManager");
  registerSwapchainImages();
  Log.trace("ImageManager Created");
}

ImageManager::~ImageManager() {
  Log.trace("Destroying ImageManager");
  imageMap.clear();
}

auto ImageManager::createImage(ImageRequest request) -> Handle<ManagedImage> {
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
          request.extent,
          request.format,
          request.usageFlags,
          request.debugName));
  return key;
}

auto ImageManager::createPerFrameImage(ImageRequest request) -> LogicalHandle<ManagedImage> {
  auto logicalHandle = generator.requestLogicalHandle();
  for (const auto& frame : frameManager->getFrames()) {
    if (request.debugName) {
      request.debugName = std::make_optional(
          std::format("{}-Frame-{}", request.debugName.value(), frame->getIndex()));
    }
    const auto handle = createImage(request);
    frame->addLogicalImage(logicalHandle, handle);
  }
  return logicalHandle;
}

auto ImageManager::registerSwapchainImages() -> void {
  swapchainLogicalHandle = generator.requestLogicalHandle();

  for (const auto& frame : frameManager->getFrames()) {
    for (uint32_t index = 0; index < swapchain->getImages().size(); ++index) {
      frame->registerSwapchainLogicalHandle(swapchainLogicalHandle);
      const auto handle = registerSwapchainImage(index);
      handleToSwapchainIndex.emplace(handle, index);
      frame->addSwapchainImage(handle, index);
    }
  }
}

auto ImageManager::registerSwapchainImage(uint32_t index) -> Handle<ManagedImage> {
  const auto handle = generator.requestHandle();

  Log.trace("Registering swapchain image index={}, handle={}", index, handle.id);

  imageMap.emplace(handle,
                   std::make_unique<ManagedImage>(swapchain->getSwapchainImage(index),
                                                  swapchain->getSwapchainImageView(index),
                                                  swapchain->getImageExtent(),
                                                  swapchain->getImageFormat(),
                                                  vk::ImageUsageFlagBits::eColorAttachment,
                                                  "SwapchainImage"));

  return handle;
}

auto ImageManager::registerDefaultSampler() -> Handle<vk::raii::Sampler> {
  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vk::SamplerAddressMode::eRepeat,
      .addressModeV = vk::SamplerAddressMode::eRepeat,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = 16.0f,
      .compareEnable = VK_FALSE,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.0f,
      .maxLod = VK_LOD_CLAMP_NONE,
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = VK_FALSE,
  };
  defaultSampler = samplerGenerator.requestHandle();

  samplers.emplace(defaultSampler, device->getVkDevice().createSampler(samplerInfo));

  return defaultSampler;
}

auto ImageManager::getDefaultSampler() const -> Handle<vk::raii::Sampler> {
  return defaultSampler;
}

auto ImageManager::getSwapchainImageHandle() const -> LogicalHandle<ManagedImage> {
  return swapchainLogicalHandle;
}

auto ImageManager::getImage(Handle<ManagedImage> imageHandle) -> ManagedImage& {
  if (handleToSwapchainIndex.contains(imageHandle)) {
    const auto imageIndex = handleToSwapchainIndex.at(imageHandle);
    auto& managedImage = imageMap.at(imageHandle);
    managedImage->setExternalImage(swapchain->getSwapchainImage(imageIndex));
    managedImage->setExternalImageView(swapchain->getSwapchainImageView(imageIndex));
    return *managedImage;
  }
  assert(imageMap.contains(imageHandle));
  return *imageMap.at(imageHandle);
}

auto ImageManager::getImageMetadata(LogicalHandle<ManagedImage> logicalHandle) -> ImageMetadata {
  const auto& frame = frameManager->getFrames().front();
  const auto handle = frame->getLogicalImage(logicalHandle);
  return getImageMetadata(handle);
}

auto ImageManager::getImageMetadata(Handle<ManagedImage> handle) -> ImageMetadata {
  assert(imageMap.contains(handle));
  const auto& image = imageMap.at(handle);
  return ImageMetadata{
      .format = image->getFormat(),
      .extent = image->getExtent(),
      .imageUsage = image->getUsageFlags(),
  };
}

[[nodiscard]] auto ImageManager::getSampler(Handle<vk::raii::Sampler> handle) const
    -> const vk::Sampler& {
  assert(samplers.contains(handle));
  return *samplers.at(handle);
}

}
