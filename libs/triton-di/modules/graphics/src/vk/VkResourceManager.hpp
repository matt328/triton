#pragma once
#include "Device.hpp"

namespace tr::gfx {

   namespace mem {
      class Allocator;
   }

   struct ImageResource {
      vk::Image image;
      vma::Allocation allocation;
   };

   struct ImageDeleter {
      vma::Allocator allocator;

      void operator()(ImageResource* image) const {
         if (image != nullptr) {
            allocator.destroyImage(image->image, image->allocation);
            delete image;
         }
      }
   };

   using AllocatedImagePtr = std::unique_ptr<ImageResource, ImageDeleter>;

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

      auto createDrawImageAndView(std::string_view imageName, vk::Extent2D extent) -> void;
      [[nodiscard]] auto getImage(const std::string& id) const -> const vk::Image&;
      [[nodiscard]] auto getImageView(const std::string& id) const -> const vk::ImageView&;

      auto destroyImage(const std::string& id) -> void;

    private:
      std::shared_ptr<Device> device;

      std::shared_ptr<mem::Allocator> allocator;

      std::unordered_map<std::string, AllocatedImagePtr> images;
      std::unordered_map<std::string, vk::raii::ImageView> imageViews;
   };
}
