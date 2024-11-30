#pragma once
#include "Device.hpp"

namespace tr::gfx {

   namespace mem {
      class Allocator;
   }

   class VkResourceManager {
    public:
      explicit VkResourceManager(std::shared_ptr<Device> newDevice,
                                 const std::shared_ptr<PhysicalDevice>& physicalDevice,
                                 const std::shared_ptr<Instance>& instance);
      ~VkResourceManager();
      VkResourceManager(const VkResourceManager&) = delete;
      VkResourceManager(VkResourceManager&&) = delete;
      auto operator=(const VkResourceManager&) -> VkResourceManager& = delete;
      auto operator=(VkResourceManager&&) -> VkResourceManager& = delete;

      [[nodiscard]] auto createDefaultDescriptorPool() const
          -> std::unique_ptr<vk::raii::DescriptorPool>;

    private:
      std::shared_ptr<Device> device;

      std::shared_ptr<mem::Allocator> allocator;
   };
}
