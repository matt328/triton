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
}
