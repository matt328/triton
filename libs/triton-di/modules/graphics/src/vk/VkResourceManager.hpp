#pragma once
#include "Device.hpp"
#include "IDebugManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "cm/Handles.hpp"
#include "cm/Rando.hpp"
#include "geo/GeometryData.hpp"
#include "geo/Mesh.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/IShaderCompiler.hpp"

namespace tr {

class IPipeline;

struct Vertex {
  glm::vec3 position;
};

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

using BufferHandle = size_t;
using ImageHandle = size_t;
using PipelineHandle = size_t;

class VkResourceManager {
public:
  explicit VkResourceManager(std::shared_ptr<Device> newDevice,
                             std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
                             std::shared_ptr<IShaderCompiler> newShaderCompiler,
                             std::shared_ptr<IDebugManager> newDebugManager,
                             const std::shared_ptr<PhysicalDevice>& physicalDevice,
                             const std::shared_ptr<Instance>& instance);
  ~VkResourceManager();
  VkResourceManager(const VkResourceManager&) = delete;
  VkResourceManager(VkResourceManager&&) = delete;
  auto operator=(const VkResourceManager&) -> VkResourceManager& = delete;
  auto operator=(VkResourceManager&&) -> VkResourceManager& = delete;

  [[nodiscard]] auto createDefaultDescriptorPool() const
      -> std::unique_ptr<vk::raii::DescriptorPool>;

  auto createDrawImageAndView(std::string_view imageName, vk::Extent2D extent) -> ImageHandle;

  auto createDepthImageAndView(std::string_view imageName, vk::Extent2D extent, vk::Format format)
      -> ImageHandle;

  auto createBuffer(size_t size, vk::Flags<vk::BufferUsageFlagBits> flags, std::string_view name)
      -> BufferHandle;

  auto createIndirectBuffer(size_t size) -> BufferHandle;

  auto asyncUpload(const GeometryData& geometryData) -> MeshHandle;

  auto destroyImage(ImageHandle handle) -> void;

  auto createComputePipeline(std::string_view name) -> PipelineHandle;

  [[nodiscard]] auto getImage(ImageHandle handle) const -> const vk::Image&;

  [[nodiscard]] auto getImageView(ImageHandle handle) const -> const vk::ImageView&;

  [[nodiscard]] auto getImageExtent(ImageHandle handle) const -> const vk::Extent2D;

  [[nodiscard]] auto getMesh(MeshHandle handle) -> const ImmutableMesh&;

  [[nodiscard]] auto getBuffer(BufferHandle handle) const -> Buffer&;

  [[nodiscard]] auto getPipeline(PipelineHandle handle) const -> const IPipeline&;

private:
  struct ImageInfo {
    AllocatedImagePtr image;
    vk::raii::ImageView imageView;
    vk::Extent2D extent;
  };

  std::shared_ptr<Device> device;
  std::shared_ptr<ImmediateTransferContext> immediateTransferContext;
  std::shared_ptr<IShaderCompiler> shaderCompiler;
  std::shared_ptr<IDebugManager> debugManager;

  std::shared_ptr<Allocator> allocator;

  std::unordered_map<ImageHandle, ImageInfo> imageInfoMap;
  std::unordered_map<BufferHandle, std::unique_ptr<Buffer>> bufferMap;
  std::unordered_map<PipelineHandle, std::unique_ptr<IPipeline>> pipelineMap;

  MapKey bufferMapKeygen;
  MapKey imageMapKeygen;
  MapKey pipelineMapKeygen;

  std::vector<ImmutableMesh> meshList;
};
}
