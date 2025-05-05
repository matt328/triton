#pragma once
#include "api/gfx/DDGeometryData.hpp"
#include "vk/core/Device.hpp"
#include "gfx/IDebugManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "api/gw/Handles.hpp"
#include "bk/Rando.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/IShaderModuleFactory.hpp"
#include "as/Model.hpp"

#include "vk/ResourceManagerHandles.hpp"
#include "vk/TextureData.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"

namespace tr {

using TransitionBarrierInfo =
    std::tuple<vk::ImageMemoryBarrier, vk::PipelineStageFlagBits, vk::PipelineStageFlagBits>;

class Device;
class IPipeline;
class MeshBufferManager;
class TextureBufferManager;
class BufferManager;
class ImmediateTransferContext;
class IShaderModuleFactory;

struct Vertex {
  glm::vec3 position;
};

class TextureManager;

class VkResourceManager {
public:
  explicit VkResourceManager(std::shared_ptr<Device> newDevice,
                             std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
                             std::shared_ptr<IShaderModuleFactory> newShaderCompiler,
                             std::shared_ptr<IDebugManager> newDebugManager,
                             std::shared_ptr<DSLayoutManager> newLayoutManager,
                             std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                             std::shared_ptr<Allocator> newAllocator,
                             std::shared_ptr<queue::Graphics> newGraphicsQueue,
                             std::shared_ptr<queue::Transfer> newTransferQueue);
  ~VkResourceManager();
  VkResourceManager(const VkResourceManager&) = delete;
  VkResourceManager(VkResourceManager&&) = delete;
  auto operator=(const VkResourceManager&) -> VkResourceManager& = delete;
  auto operator=(VkResourceManager&&) -> VkResourceManager& = delete;

  [[nodiscard]] auto createDefaultDescriptorPool() const
      -> std::unique_ptr<vk::raii::DescriptorPool>;

  auto uploadGeometryData(const DDGeometryData& data) -> MeshHandle;

  auto uploadImage(const as::ImageData& imageData, std::string_view name) -> TextureHandle;

  auto getTextureData(const as::ImageData& imageData, std::string_view name) -> TextureData;

  auto addDescriptorToBuffer(BufferHandle bufferHandle,
                             const vk::DescriptorImageInfo& descriptorImageInfo,
                             size_t slot) -> void;

  auto createComputePipeline(std::string_view name) -> PipelineHandle;

  [[nodiscard]] auto getStaticMeshBuffers() const -> std::tuple<ManagedBuffer&, ManagedBuffer&>;
  [[nodiscard]] auto getDynamicMeshBuffers() const -> std::tuple<ManagedBuffer&, ManagedBuffer&>;
  [[nodiscard]] auto getTerrainMeshBuffers() const -> std::tuple<ManagedBuffer&, ManagedBuffer&>;
  [[nodiscard]] auto getDebugMeshBuffers() const -> std::tuple<ManagedBuffer&, ManagedBuffer&>;

  [[nodiscard]] auto getDescriptorSetLayout() -> const vk::DescriptorSetLayout*;

  [[nodiscard]] auto getPipeline(PipelineHandle handle) const -> const IPipeline&;

  [[nodiscard]] auto getTextureDSL() const -> const vk::DescriptorSetLayout*;
  [[nodiscard]] auto getTextureShaderBinding() const -> IShaderBinding&;

  auto updateShaderBindings() -> void;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<ImmediateTransferContext> immediateTransferContext;
  std::shared_ptr<IShaderModuleFactory> shaderCompiler;
  std::shared_ptr<IDebugManager> debugManager;
  std::shared_ptr<DSLayoutManager> layoutManager;
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<queue::Transfer> transferQueue;

  std::unordered_map<PipelineHandle, std::unique_ptr<IPipeline>> pipelineMap;

  MapKey pipelineMapKeygen;

  std::unique_ptr<TextureManager> textureManager;

  DSLayoutHandle textureDSLHandle;
  ShaderBindingHandle textureShaderBindingHandle;

  static auto createTransitionBarrier(const vk::Image& image,
                                      vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout,
                                      const vk::ImageSubresourceRange& subresourceRange,
                                      uint32_t srcQueueFamily = VK_QUEUE_FAMILY_IGNORED,
                                      uint32_t dstQueueFamily = VK_QUEUE_FAMILY_IGNORED)
      -> TransitionBarrierInfo;

  static auto aligned_size(VkDeviceSize value, VkDeviceSize alignment) -> vk::DeviceSize {
    return (value + alignment - 1) & ~(alignment - 1);
  }
};

}
