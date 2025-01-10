#pragma once
#include "vk/core/Device.hpp"
#include "IDebugManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "cm/Handles.hpp"
#include "cm/Rando.hpp"
#include "geo/GeometryData.hpp"
#include "geo/Mesh.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "as/Model.hpp"

#include "ResourceManagerHandles.hpp"
#include "vk/MeshBufferManager.hpp"

namespace tr {

class IPipeline;
class MeshBufferManager;

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

  auto createBuffer(size_t size,
                    vk::Flags<vk::BufferUsageFlagBits> flags,
                    std::string_view name,
                    vma::MemoryUsage usage = vma::MemoryUsage::eCpuToGpu,
                    vk::MemoryPropertyFlags memoryProperties =
                        vk::MemoryPropertyFlagBits::eHostCoherent) -> BufferHandle;

  auto createGpuVertexBuffer(size_t size, std::string_view name) -> BufferHandle;
  auto createGpuIndexBuffer(size_t size, std::string_view name) -> BufferHandle;

  auto createIndirectBuffer(size_t size) -> BufferHandle;

  /// Add a static mesh to the MeshBufferManager for static meshes.
  auto uploadStaticMesh(const GeometryData& geometryData) -> MeshHandle;

  auto asyncUpload2(const GeometryData& geometryData) -> MeshHandle;
  auto uploadImage(const as::ImageData& imageData) -> TextureHandle;

  /// Utility method to only be called by MeshBufferManagers.
  auto addToMesh(const GeometryData& geometryData,
                 BufferHandle vertexBufferHandle,
                 vk::DeviceSize vertexOffset,
                 BufferHandle indexBufferHandle,
                 vk::DeviceSize indexOffset) -> void;

  auto destroyImage(ImageHandle handle) -> void;

  auto createComputePipeline(std::string_view name) -> PipelineHandle;

  [[nodiscard]] auto resizeBuffer(BufferHandle handle, size_t newSize) -> BufferHandle;

  [[nodiscard]] auto getImage(ImageHandle handle) const -> const vk::Image&;

  [[nodiscard]] auto getImageView(ImageHandle handle) const -> const vk::ImageView&;

  [[nodiscard]] auto getImageExtent(ImageHandle handle) const -> const vk::Extent2D;

  [[nodiscard]] auto getMesh(MeshHandle handle) -> const ImmutableMesh&;

  [[nodiscard]] auto getBuffer(BufferHandle handle) const -> Buffer&;

  [[nodiscard]] auto getStaticMeshBuffers() const -> std::tuple<Buffer&, Buffer&>;

  [[nodiscard]] auto getPipeline(PipelineHandle handle) const -> const IPipeline&;

  [[nodiscard]] auto getStaticGpuData(const std::vector<RenderMeshData>& gpuBufferData)
      -> std::vector<GpuBufferEntry>&;

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

  std::unique_ptr<MeshBufferManager> staticMeshBufferManager;
};
}
