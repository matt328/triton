#include "VkResourceManager.hpp"

#include <mem/Allocator.hpp>

namespace tr::gfx {
   VkResourceManager::VkResourceManager(std::shared_ptr<Device> newDevice,
                                        const std::shared_ptr<PhysicalDevice>& physicalDevice,
                                        const std::shared_ptr<Instance>& instance)
       : device{std::move(newDevice)} {

      constexpr auto vulkanFunctions = vma::VulkanFunctions{
          .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
          .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
      };

      const auto allocatorCreateInfo = vma::AllocatorCreateInfo{
          .flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress,
          .physicalDevice = *physicalDevice->getVkPhysicalDevice(),
          .device = *device->getVkDevice(),
          .pVulkanFunctions = &vulkanFunctions,
          .instance = instance->getVkInstance(),
      };

      allocator = std::make_unique<mem::Allocator>(allocatorCreateInfo, device->getVkDevice());
   }
   VkResourceManager::~VkResourceManager() {
      Log.trace("Destroying VkResourceManager");
   }
   auto VkResourceManager::createDefaultDescriptorPool() const
       -> std::unique_ptr<vk::raii::DescriptorPool> {
      static constexpr auto poolSizes = std::array{
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampledImage,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformTexelBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageTexelBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBufferDynamic,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBufferDynamic,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eInputAttachment,
                                 .descriptorCount = 1000}};

      constexpr vk::DescriptorPoolCreateInfo poolInfo{
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
          .maxSets = 1000,
          .poolSizeCount = poolSizes.size(),
          .pPoolSizes = poolSizes.data()};

      return std::make_unique<vk::raii::DescriptorPool>(device->createDescriptorPool(poolInfo));
   }

   auto VkResourceManager::createImageAndView(std::string_view imageName, const vk::Extent2D extent)
       -> void {
      constexpr auto drawImageFormat = vk::Format::eR16G16B16A16Sfloat;

      const auto imageCreateInfo = vk::ImageCreateInfo{
          .imageType = vk::ImageType::e2D,
          .format = drawImageFormat,
          .extent = vk::Extent3D{extent.width, extent.height, 1},
          .mipLevels = 1,
          .arrayLayers = 1,
          .samples = vk::SampleCountFlagBits::e1,
          .tiling = vk::ImageTiling::eOptimal,
          .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
                   vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment,
          .sharingMode = vk::SharingMode::eExclusive,
          .initialLayout = vk::ImageLayout::eUndefined};

      constexpr auto imageAllocateCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

      auto [image, allocation] =
          allocator->getAllocator()->createImage(imageCreateInfo, imageAllocateCreateInfo);

      images.emplace(imageName.data(),
                     AllocatedImagePtr(new ImageResource{.image = image, .allocation = allocation},
                                       ImageDeleter{*allocator->getAllocator()}));

      const auto imageViewInfo =
          vk::ImageViewCreateInfo{.image = getImage(imageName.data()),
                                  .viewType = vk::ImageViewType::e2D,
                                  .format = drawImageFormat,
                                  .subresourceRange = {
                                      .aspectMask = vk::ImageAspectFlagBits::eColor,
                                      .levelCount = 1,
                                      .layerCount = 1,
                                  }};

      imageViews.emplace(imageName.data(), device->getVkDevice().createImageView(imageViewInfo));
   }

   auto VkResourceManager::getImage(const std::string& id) const -> const vk::Image& {
      return images.at(id)->image;
   }

   auto VkResourceManager::getImageView(const std::string& id) const -> const vk::ImageView& {
      return *imageViews.at(id);
   }

   auto VkResourceManager::destroyImage(const std::string& id) -> void {
      imageViews.erase(id);
      images.erase(id);
   }
}
