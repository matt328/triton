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
      [[nodiscard]] auto getImageExtent(const std::string& id) const -> const vk::Extent2D;

      auto destroyImage(const std::string& id) -> void;

    private:
      struct ImageInfo {
         AllocatedImagePtr image;
         vk::raii::ImageView imageView;
         vk::Extent2D extent;
      };
      std::shared_ptr<Device> device;

      std::shared_ptr<mem::Allocator> allocator;

      std::unordered_map<std::string, ImageInfo> imageInfoMap;
   };
}
