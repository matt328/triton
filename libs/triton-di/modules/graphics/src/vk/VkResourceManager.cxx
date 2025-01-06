#include "VkResourceManager.hpp"

#include <mem/Allocator.hpp>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_enums.hpp>
#include "IDebugManager.hpp"
#include "ResourceExceptions.hpp"
#include "mem/Buffer.hpp"
#include "pipeline/ComputePipeline.hpp"
#include "pipeline/IPipeline.hpp"
#include "pipeline/IndirectPipeline.hpp"
#include "vk/MeshBufferManager.hpp"

namespace tr {
VkResourceManager::VkResourceManager(
    std::shared_ptr<Device> newDevice,
    std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
    std::shared_ptr<IShaderCompiler> newShaderCompiler,
    std::shared_ptr<IDebugManager> newDebugManager,
    const std::shared_ptr<PhysicalDevice>& physicalDevice,
    const std::shared_ptr<Instance>& instance)
    : device{std::move(newDevice)},
      immediateTransferContext{std::move(newImmediateTransferContext)},
      shaderCompiler{std::move(newShaderCompiler)},
      debugManager{std::move(newDebugManager)} {

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

  allocator = std::make_unique<Allocator>(allocatorCreateInfo, device->getVkDevice(), debugManager);

  staticMeshBufferManager = std::make_unique<MeshBufferManager>(this);
}

VkResourceManager::~VkResourceManager() {
  Log.trace("Destroying VkResourceManager");
}

/*
  Todo(matt) Tomorrow refactor this method to be called just uploadStaticMesh.
  - Add the mesh to the staticMeshBufferManager
  - Refactor the IndirectRenderer to know to get the vertex and index buffers from
  staticMeshBufferManager.

  Create the impl of uploadImage method to use a TextureBufferManager.
  - driven by descriptor buffers. heck any gpu that doesn't support Vulkan 1.3
*/

auto VkResourceManager::asyncUpload(const GeometryData& geometryData) -> MeshHandle {
  // Prepare Vertex Buffer
  const auto vbSize = geometryData.vertexDataSize();
  const auto ibSize = geometryData.indexDataSize();

  try {
    const auto vbStagingBuffer = allocator->createStagingBuffer(vbSize, "Buffer-VertexStaging");
    void* vbData = allocator->mapMemory(*vbStagingBuffer);
    memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
    allocator->unmapMemory(*vbStagingBuffer);

    // Prepare Index Buffer
    const auto ibStagingBuffer = allocator->createStagingBuffer(ibSize, "Buffer-IndexStaging");

    auto* const data = allocator->mapMemory(*ibStagingBuffer);
    memcpy(data, geometryData.indices.data(), ibSize);
    allocator->unmapMemory(*ibStagingBuffer);

    auto vertexBuffer = allocator->createGpuVertexBuffer(vbSize, "Buffer-Vertex");
    auto indexBuffer = allocator->createGpuIndexBuffer(ibSize, "Buffer-Index");
    const auto indicesCount = geometryData.indices.size();

    // Upload Buffers
    immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
      const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = vbSize};
      cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer->getBuffer(), vbCopy);
      const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = ibSize};
      cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
    });

    const auto meshHandle = meshList.size();
    meshList.emplace_back(std::move(vertexBuffer), std::move(indexBuffer), indicesCount);

    return meshHandle;

  } catch (const AllocationException& ex) {
    throw ResourceUploadException(
        fmt::format("Error allocating resources for geometry, {0}", ex.what()));
  }
}

auto VkResourceManager::uploadImage([[maybe_unused]] const as::ImageData& imageData)
    -> TextureHandle {
  return 0L;
}

auto VkResourceManager::createGpuVertexBuffer(size_t size, std::string_view name) -> BufferHandle {
  const auto key = bufferMapKeygen.getKey();
  bufferMap.emplace(key, allocator->createGpuVertexBuffer(size, name));
  return key;
}

auto VkResourceManager::createGpuIndexBuffer(size_t size, std::string_view name) -> BufferHandle {
  const auto key = bufferMapKeygen.getKey();
  bufferMap.emplace(key, allocator->createGpuIndexBuffer(size, name));
  return key;
}

auto VkResourceManager::createBuffer(size_t size,
                                     vk::Flags<vk::BufferUsageFlagBits> flags,
                                     std::string_view name,
                                     vma::MemoryUsage usage,
                                     vk::MemoryPropertyFlags memoryProperties) -> BufferHandle {
  const auto bufferCreateInfo = vk::BufferCreateInfo{.size = size, .usage = flags};

  const auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = usage, .requiredFlags = memoryProperties};

  auto key = bufferMapKeygen.getKey();

  bufferMap.emplace(key, allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, name));
  return key;
}

auto VkResourceManager::createIndirectBuffer(size_t size) -> BufferHandle {
  const auto bufferCreateInfo =
      vk::BufferCreateInfo{.size = size, .usage = vk::BufferUsageFlagBits::eIndirectBuffer};

  constexpr auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  const auto key = bufferMapKeygen.getKey();

  bufferMap.emplace(
      key,
      allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, "Buffer-IndirectDraw"));
  return key;
}

[[nodiscard]] auto VkResourceManager::resizeBuffer(BufferHandle handle, size_t newSize)
    -> BufferHandle {
  ZoneNamedN(var, "Resize Buffer", true);
  auto& oldBuffer = bufferMap.at(handle);

  auto bci = oldBuffer->getBufferCreateInfo();
  const auto oldSize = bci.size;
  bci.size = newSize;

  auto aci = oldBuffer->getAllocationCreateInfo();

  auto newBuffer = allocator->createBuffer(&bci, &aci);

  immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
    ZoneNamedN(var, "Copy Buffer", true);
    const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = oldSize};
    cmd.copyBuffer(oldBuffer->getBuffer(), newBuffer->getBuffer(), vbCopy);
  });

  bufferMap.erase(handle);
  const auto newHandle = bufferMapKeygen.getKey();
  bufferMap.emplace(newHandle, std::move(newBuffer));
  return newHandle;
}

auto VkResourceManager::addToMesh([[maybe_unused]] const GeometryData& geometryData,
                                  [[maybe_unused]] BufferHandle vertexBufferHandle,
                                  [[maybe_unused]] BufferHandle indexBufferHandle) -> void {
}

[[nodiscard]] auto VkResourceManager::getMesh(MeshHandle handle) -> const ImmutableMesh& {
  return meshList[handle];
}

auto VkResourceManager::createDefaultDescriptorPool() const
    -> std::unique_ptr<vk::raii::DescriptorPool> {
  static constexpr auto poolSizes = std::array{
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                             .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampledImage, .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformTexelBuffer,
                             .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageTexelBuffer,
                             .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1000},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000},
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

auto VkResourceManager::createDrawImageAndView(std::string_view imageName,
                                               const vk::Extent2D extent) -> ImageHandle {
  constexpr auto drawImageFormat = vk::Format::eR16G16B16A16Sfloat;

  const auto imageCreateInfo = vk::ImageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = drawImageFormat,
      .extent = vk::Extent3D{extent.width, extent.height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst |
               vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  constexpr auto imageAllocateCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  auto [image, allocation] =
      allocator->getAllocator()->createImage(imageCreateInfo, imageAllocateCreateInfo);

  debugManager->setObjectName(image, imageName.data());

  const auto imageViewInfo =
      vk::ImageViewCreateInfo{.image = image,
                              .viewType = vk::ImageViewType::e2D,
                              .format = drawImageFormat,
                              .subresourceRange = {
                                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                                  .levelCount = 1,
                                  .layerCount = 1,
                              }};

  const auto imageKey = imageMapKeygen.getKey();

  imageInfoMap.emplace(imageKey,
                       ImageInfo{.image = AllocatedImagePtr(
                                     new ImageResource{.image = image, .allocation = allocation},
                                     ImageDeleter{*allocator->getAllocator()}),
                                 .imageView = device->getVkDevice().createImageView(imageViewInfo),
                                 .extent = extent});

  debugManager->setObjectName(*imageInfoMap.at(imageKey).imageView,
                              fmt::format("{}-View", imageName));

  return imageKey;
}
auto VkResourceManager::createDepthImageAndView([[maybe_unused]] std::string_view imageName,
                                                vk::Extent2D extent,
                                                vk::Format format) -> ImageHandle {
  const auto imageCreateInfo = vk::ImageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = vk::Extent3D{.width = extent.width, .height = extent.height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  constexpr auto imageAllocateCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

  auto [image, allocation] =
      allocator->getAllocator()->createImage(imageCreateInfo, imageAllocateCreateInfo);

  const auto imageViewInfo =
      vk::ImageViewCreateInfo{.image = image,
                              .viewType = vk::ImageViewType::e2D,
                              .format = format,
                              .subresourceRange = {
                                  .aspectMask = vk::ImageAspectFlagBits::eDepth,
                                  .levelCount = 1,
                                  .layerCount = 1,
                              }};

  const auto imageKey = imageMapKeygen.getKey();

  imageInfoMap.emplace(imageKey,
                       ImageInfo{.image = AllocatedImagePtr(
                                     new ImageResource{.image = image, .allocation = allocation},
                                     ImageDeleter{*allocator->getAllocator()}),
                                 .imageView = device->getVkDevice().createImageView(imageViewInfo),
                                 .extent = extent});
  return imageKey;
}

auto VkResourceManager::getImage(ImageHandle handle) const -> const vk::Image& {
  return imageInfoMap.at(handle).image->image;
}

auto VkResourceManager::getImageView(ImageHandle handle) const -> const vk::ImageView& {
  return *imageInfoMap.at(handle).imageView;
}

auto VkResourceManager::getImageExtent(ImageHandle handle) const -> const vk::Extent2D {
  return imageInfoMap.at(handle).extent;
}

auto VkResourceManager::getBuffer(const BufferHandle handle) const -> Buffer& {
  return *bufferMap.at(handle);
}

auto VkResourceManager::getStaticMeshBuffers() const -> std::tuple<Buffer&, Buffer&> {
  return {*bufferMap.at(staticMeshBufferManager->getVertexBufferHandle()),
          *bufferMap.at(staticMeshBufferManager->getIndexBufferHandle())};
}

auto VkResourceManager::destroyImage(ImageHandle handle) -> void {
  imageInfoMap.erase(handle);
}

auto VkResourceManager::createComputePipeline([[maybe_unused]] std::string_view name)
    -> PipelineHandle {
  const auto key = pipelineMapKeygen.getKey();
  pipelineMap.emplace(key, std::make_unique<ComputePipeline>(device, shaderCompiler));
  return key;
}

[[nodiscard]] auto VkResourceManager::getPipeline(PipelineHandle handle) const -> const IPipeline& {
  return *pipelineMap.at(handle);
}

/// Gets a list of the GpuBufferEntry that the game world thinks are involved in this frame.
/// These are passed to the compute shader to get turned into DrawCommands and placed in the
/// DrawIndexedIndirect buffer
[[nodiscard]] auto VkResourceManager::getStaticGpuData(
    const std::vector<GpuMeshData>& gpuBufferData) -> std::vector<GpuBufferEntry> {
  return staticMeshBufferManager->getInstanceData(gpuBufferData);
}

}
