#pragma once
#include "Device.hpp"
#include "ImmediateTransferContext.hpp"
#include "cm/Handles.hpp"
#include "geo/GeometryData.hpp"
#include "geo/Mesh.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/IShaderCompiler.hpp"

namespace tr {

class IPipeline;

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
                             std::shared_ptr<IShaderCompiler> newShaderCompiler,
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

  auto createBuffer(size_t size, vk::Flags<vk::BufferUsageFlagBits> flags, std::string_view name)
      -> void;

  auto createIndirectBuffer(size_t size) -> void;

  [[nodiscard]] auto getBuffer(std::string_view name) const -> Buffer&;

  auto asyncUpload(const GeometryData& geometryData) -> MeshHandle;

  auto destroyImage(const std::string& id) -> void;

  auto createComputePipeline(std::string_view name) -> void;

  [[nodiscard]] auto getPipeline(std::string_view name) const -> const IPipeline&;

private:
  struct ImageInfo {
    AllocatedImagePtr image;
    vk::raii::ImageView imageView;
    vk::Extent2D extent;
  };

  std::shared_ptr<Device> device;
  std::shared_ptr<ImmediateTransferContext> immediateTransferContext;
  std::shared_ptr<IShaderCompiler> shaderCompiler;

  std::shared_ptr<Allocator> allocator;

  std::unordered_map<std::string, ImageInfo> imageInfoMap;
  std::unordered_map<std::string, std::unique_ptr<Buffer>> bufferMap;

  std::vector<ImmutableMesh> meshList;

  std::unordered_map<std::string, std::unique_ptr<IPipeline>> pipelineMap;
};
}
