#pragma once
#include "vk/ArenaGeometryBuffer.hpp"
#include "vk/core/Device.hpp"
#include "tr/IDebugManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "cm/Handles.hpp"
#include "cm/Rando.hpp"
#include "geo/Mesh.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "as/Model.hpp"

#include "vk/ResourceManagerHandles.hpp"
#include "TextureData.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"

namespace tr {

using TransitionBarrierInfo =
    std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlagBits, vk::PipelineStageFlagBits>;

class IPipeline;
class MeshBufferManager;
class TextureBufferManager;
class BufferManager;

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

class TextureManager;

class VkResourceManager {
public:
  explicit VkResourceManager(std::shared_ptr<Device> newDevice,
                             std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
                             std::shared_ptr<IShaderCompiler> newShaderCompiler,
                             std::shared_ptr<IDebugManager> newDebugManager,
                             std::shared_ptr<DSLayoutManager> newLayoutManager,
                             std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                             std::shared_ptr<Allocator> newAllocator,
                             std::shared_ptr<IBufferManager> newBufferManager);
  ~VkResourceManager();
  VkResourceManager(const VkResourceManager&) = delete;
  VkResourceManager(VkResourceManager&&) = delete;
  auto operator=(const VkResourceManager&) -> VkResourceManager& = delete;
  auto operator=(VkResourceManager&&) -> VkResourceManager& = delete;

  [[nodiscard]] auto createDefaultDescriptorPool() const
      -> std::unique_ptr<vk::raii::DescriptorPool>;

  auto createDrawImageAndView(std::string_view imageName, vk::Extent2D extent) -> ImageHandle;
  auto destroyDrawImageAndView(ImageHandle handle) -> void;

  auto createDepthImageAndView(std::string_view imageName, vk::Extent2D extent, vk::Format format)
      -> ImageHandle;

  /// Add a static mesh to the MeshBufferManager for static meshes.
  auto uploadStaticMesh(const IGeometryData& geometryData) -> MeshHandle;

  /// Add a Dynamic mesh to the MeshBufferManager for dynamic meshes.
  auto uploadDynamicMesh(const IGeometryData& geometryData) -> MeshHandle;

  auto asyncUpload2(const IGeometryData& geometryData) -> MeshHandle;
  auto uploadImage(const as::ImageData& imageData, std::string_view name) -> TextureHandle;

  /// Utility method to only be called by MeshBufferManagers.
  auto addToMesh(const IGeometryData& geometryData,
                 BufferHandle vertexBufferHandle,
                 vk::DeviceSize vertexOffset,
                 BufferHandle indexBufferHandle,
                 vk::DeviceSize indexOffset) -> void;

  auto getTextureData(const as::ImageData& imageData, std::string_view name) -> TextureData;

  auto addDescriptorToBuffer(BufferHandle bufferHandle,
                             const vk::DescriptorImageInfo& descriptorImageInfo,
                             size_t slot) -> void;

  auto destroyImage(ImageHandle handle) -> void;

  auto createComputePipeline(std::string_view name) -> PipelineHandle;

  [[nodiscard]] auto getImage(ImageHandle handle) const -> const vk::Image&;

  [[nodiscard]] auto getImageView(ImageHandle handle) const -> const vk::ImageView&;

  [[nodiscard]] auto getImageExtent(ImageHandle handle) const -> const vk::Extent2D;

  [[nodiscard]] auto getMesh(MeshHandle handle) -> const ImmutableMesh&;

  [[nodiscard]] auto getStaticMeshBuffers() const -> std::tuple<Buffer&, Buffer&>;
  [[nodiscard]] auto getDynamicMeshBuffers() const -> std::tuple<Buffer&, Buffer&>;

  [[nodiscard]] auto getDescriptorSetLayout() -> const vk::DescriptorSetLayout*;

  [[nodiscard]] auto getPipeline(PipelineHandle handle) const -> const IPipeline&;

  [[nodiscard]] auto getTextureDSL() const -> const vk::DescriptorSetLayout*;
  [[nodiscard]] auto getTextureShaderBinding() const -> IShaderBinding&;

  [[nodiscard]] auto getStaticGpuData(const std::vector<RenderMeshData>& gpuBufferData)
      -> std::vector<GpuBufferEntry>&;
  [[nodiscard]] auto getDynamicGpuData(const std::vector<RenderMeshData>& gpuBufferData)
      -> std::vector<GpuBufferEntry>&;

  auto updateShaderBindings() -> void;

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
  std::shared_ptr<DSLayoutManager> layoutManager;
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<IBufferManager> bufferManager;

  std::unordered_map<ImageHandle, ImageInfo> imageInfoMap;
  std::unordered_map<PipelineHandle, std::unique_ptr<IPipeline>> pipelineMap;

  MapKey imageMapKeygen;
  MapKey pipelineMapKeygen;

  std::vector<ImmutableMesh> meshList;

  std::unique_ptr<ArenaGeometryBuffer> staticMeshBuffer;
  std::unique_ptr<ArenaGeometryBuffer> dynamicMeshBuffer;

  std::unique_ptr<TextureManager> textureManager;

  DSLayoutHandle textureDSLHandle;
  ShaderBindingHandle textureShaderBindingHandle;

  auto createTransitionBarrier(const vk::Image& image,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout,
                               const vk::ImageSubresourceRange& subresourceRange)
      -> TransitionBarrierInfo;

  static inline auto aligned_size(VkDeviceSize value, VkDeviceSize alignment) -> vk::DeviceSize {
    return (value + alignment - 1) & ~(alignment - 1);
  }
};

}
