#pragma once
#include "Device.hpp"
#include "ImmediateTransferContext.hpp"
#include "cm/Handles.hpp"
#include "geo/GeometryData.hpp"
#include "geo/Mesh.hpp"
#include "mem/Allocator.hpp"

namespace tr {

struct Vertex {
   glm::vec3 position;
};

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
                              std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
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
   auto createDepthImageAndView(std::string_view imageName, vk::Extent2D extent, vk::Format format)
       -> void;
   [[nodiscard]] auto getImage(const std::string& id) const -> const vk::Image&;
   [[nodiscard]] auto getImageView(const std::string& id) const -> const vk::ImageView&;
   [[nodiscard]] auto getImageExtent(const std::string& id) const -> const vk::Extent2D;

   [[nodiscard]] auto getMesh(MeshHandle handle) -> const ImmutableMesh&;

   /// For now, VkResourceManager doesn't centrally manage buffers, but gives out unique_ptrs
   /// and they'll clean themselves up if the ptr goes out of scope.
   [[nodiscard]] auto createBuffer(size_t size,
                                   vk::Flags<vk::BufferUsageFlagBits> flags,
                                   std::string_view name) -> std::unique_ptr<Buffer>;

   [[nodiscard]] auto createIndirectBuffer(size_t size) -> std::unique_ptr<Buffer>;

   auto asyncUpload(const GeometryData& geometryData) -> MeshHandle;

   auto destroyImage(const std::string& id) -> void;

 private:
   struct ImageInfo {
      AllocatedImagePtr image;
      vk::raii::ImageView imageView;
      vk::Extent2D extent;
   };
   std::shared_ptr<Device> device;
   std::shared_ptr<ImmediateTransferContext> immediateTransferContext;

   std::shared_ptr<Allocator> allocator;

   std::unordered_map<std::string, ImageInfo> imageInfoMap;

   std::vector<ImmutableMesh> meshList;
};
}
