#include "VkResourceManager.hpp"

#include <mem/Allocator.hpp>
#include "ResourceExceptions.hpp"
#include "pipeline/ComputePipeline.hpp"
#include "pipeline/IPipeline.hpp"
#include "pipeline/IndirectPipeline.hpp"

namespace tr {
VkResourceManager::VkResourceManager(
    std::shared_ptr<Device> newDevice,
    std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
    std::shared_ptr<IShaderCompiler> newShaderCompiler,
    const std::shared_ptr<PhysicalDevice>& physicalDevice,
    const std::shared_ptr<Instance>& instance)
    : device{std::move(newDevice)},
      immediateTransferContext{std::move(newImmediateTransferContext)},
      shaderCompiler{std::move(newShaderCompiler)} {

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

  allocator = std::make_unique<Allocator>(allocatorCreateInfo, device->getVkDevice());
}

VkResourceManager::~VkResourceManager() {
  Log.trace("Destroying VkResourceManager");
}

auto VkResourceManager::asyncUpload(const GeometryData& geometryData) -> MeshHandle {
  // Prepare Vertex Buffer
  const auto vbSize = geometryData.vertexDataSize();
  const auto ibSize = geometryData.indexDataSize();

  try {
    const auto vbStagingBuffer = allocator->createStagingBuffer(vbSize, "Vertex Staging Buffer");
    void* vbData = allocator->mapMemory(*vbStagingBuffer);
    memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
    allocator->unmapMemory(*vbStagingBuffer);

    // Prepare Index Buffer
    const auto ibStagingBuffer = allocator->createStagingBuffer(ibSize, "Index Staging Buffer");

    auto* const data = allocator->mapMemory(*ibStagingBuffer);
    memcpy(data, geometryData.indices.data(), ibSize);
    allocator->unmapMemory(*ibStagingBuffer);

    auto vertexBuffer = allocator->createGpuVertexBuffer(vbSize, "GPU Vertex");
    auto indexBuffer = allocator->createGpuIndexBuffer(ibSize, "GPU Index");
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

auto VkResourceManager::createBuffer(size_t size,
                                     vk::Flags<vk::BufferUsageFlagBits> flags,
                                     std::string_view name) -> void {
  const auto bufferCreateInfo = vk::BufferCreateInfo{.size = size, .usage = flags};

  constexpr auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

  bufferMap.emplace(name, allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, name));
}

auto VkResourceManager::createIndirectBuffer(size_t size) -> void {
  const auto bufferCreateInfo =
      vk::BufferCreateInfo{.size = size, .usage = vk::BufferUsageFlagBits::eIndirectBuffer};

  constexpr auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  bufferMap.emplace(
      "IndirectBuffer",
      allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, "IndirectBuffer"));
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
                                               const vk::Extent2D extent) -> void {
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

  const auto imageViewInfo =
      vk::ImageViewCreateInfo{.image = image,
                              .viewType = vk::ImageViewType::e2D,
                              .format = drawImageFormat,
                              .subresourceRange = {
                                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                                  .levelCount = 1,
                                  .layerCount = 1,
                              }};

  imageInfoMap.emplace(imageName.data(),
                       ImageInfo{.image = AllocatedImagePtr(
                                     new ImageResource{.image = image, .allocation = allocation},
                                     ImageDeleter{*allocator->getAllocator()}),
                                 .imageView = device->getVkDevice().createImageView(imageViewInfo),
                                 .extent = extent});
}
auto VkResourceManager::createDepthImageAndView(std::string_view imageName,
                                                vk::Extent2D extent,
                                                vk::Format format) -> void {
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

  imageInfoMap.emplace(imageName.data(),
                       ImageInfo{.image = AllocatedImagePtr(
                                     new ImageResource{.image = image, .allocation = allocation},
                                     ImageDeleter{*allocator->getAllocator()}),
                                 .imageView = device->getVkDevice().createImageView(imageViewInfo),
                                 .extent = extent});
}

auto VkResourceManager::getImage(std::string_view id) const -> const vk::Image& {
  return imageInfoMap.at(id.data()).image->image;
}

auto VkResourceManager::getImageView(std::string_view id) const -> const vk::ImageView& {
  return *imageInfoMap.at(id.data()).imageView;
}

auto VkResourceManager::getImageExtent(std::string_view id) const -> const vk::Extent2D {
  return imageInfoMap.at(id.data()).extent;
}

auto VkResourceManager::getBuffer(std::string_view name) const -> Buffer& {
  return *bufferMap.at(name.data());
}

auto VkResourceManager::destroyImage(const std::string& id) -> void {
  imageInfoMap.erase(id);
}

auto VkResourceManager::createComputePipeline(std::string_view name) -> void {
  pipelineMap.emplace(name.data(), std::make_unique<ComputePipeline>(device, shaderCompiler));
}

[[nodiscard]] auto VkResourceManager::getPipeline(std::string_view name) const -> const IPipeline& {
  return *pipelineMap.at(name.data());
}

}
