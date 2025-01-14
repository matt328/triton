#include "VkResourceManager.hpp"

#include "mem/Allocator.hpp"
#include "mem/Image.hpp"
#include "tr/IDebugManager.hpp"
#include "ResourceExceptions.hpp"
#include "mem/Buffer.hpp"
#include "pipeline/ComputePipeline.hpp"
#include "vk/MeshBufferManager.hpp"
#include "vk/TextureBufferManager.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace tr {
VkResourceManager::VkResourceManager(
    std::shared_ptr<Device> newDevice,
    std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
    std::shared_ptr<IShaderCompiler> newShaderCompiler,
    std::shared_ptr<IDebugManager> newDebugManager,
    std::shared_ptr<DSLayoutManager> newLayoutManager,
    std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
    const std::shared_ptr<PhysicalDevice>& physicalDevice,
    const std::shared_ptr<Instance>& instance)
    : device{std::move(newDevice)},
      immediateTransferContext{std::move(newImmediateTransferContext)},
      shaderCompiler{std::move(newShaderCompiler)},
      debugManager{std::move(newDebugManager)},
      layoutManager{std::move(newLayoutManager)},
      shaderBindingFactory{std::move(newShaderBindingFactory)} {

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

  descriptorBufferOffsetAlignment =
      physicalDevice->getDescriptorBufferProperties().descriptorBufferOffsetAlignment;
  descriptorSize =
      physicalDevice->getDescriptorBufferProperties().combinedImageSamplerDescriptorSize;

  constexpr auto binding =
      vk::DescriptorSetLayoutBinding{.binding = 0,
                                     .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                                     .descriptorCount = MaxTextureCount,
                                     .stageFlags = vk::ShaderStageFlagBits::eAll};
  const auto textureDSLHandle = layoutManager->createLayout(binding, "DescriptorSetLayout-Texture");

  [[maybe_unused]] const auto textureShaderBindingHandle =
      shaderBindingFactory->createShaderBinding(ShaderBindingType::Textures, textureDSLHandle);

  createDescriptorSetLayout();

  staticMeshBufferManager = std::make_unique<MeshBufferManager>(this);

  textureBufferManager = std::make_unique<TextureBufferManager>(this);
}

VkResourceManager::~VkResourceManager() {
  Log.trace("Destroying VkResourceManager");
}

auto VkResourceManager::createDescriptorSetLayout() -> void {
  constexpr auto binding =
      vk::DescriptorSetLayoutBinding{.binding = 0,
                                     .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                                     .descriptorCount = MaxTextureCount,
                                     .stageFlags = vk::ShaderStageFlagBits::eAll};

  static constexpr vk::DescriptorBindingFlags bindlessFlags =
      vk::DescriptorBindingFlagBits::ePartiallyBound;

  constexpr auto extendedInfo =
      vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                       .pBindingFlags = &bindlessFlags};

  const auto flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;

  const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{.pNext = &extendedInfo,
                                                               .flags = flags,
                                                               .bindingCount = 1,
                                                               .pBindings = &binding};

  textureDsl = std::make_unique<vk::raii::DescriptorSetLayout>(
      device->getVkDevice().createDescriptorSetLayout(dslCreateInfo));
  textureDslSize = textureDsl->getSizeEXT();

  textureDslSize = aligned_size(textureDslSize, descriptorBufferOffsetAlignment);

  textureDslOffset = textureDsl->getBindingOffsetEXT(0);
}

auto VkResourceManager::uploadStaticMesh(const GeometryData& geometryData) -> MeshHandle {
  return staticMeshBufferManager->addMesh(geometryData);
}

auto VkResourceManager::asyncUpload2(const GeometryData& geometryData) -> MeshHandle {
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

auto VkResourceManager::uploadImage([[maybe_unused]] const as::ImageData& imageData,
                                    std::string_view name) -> TextureHandle {
  return textureBufferManager->addTexture(imageData, name);
}

auto VkResourceManager::addDescriptorToBuffer(BufferHandle bufferHandle,
                                              const vk::DescriptorImageInfo& descriptorImageInfo,
                                              size_t slot) -> void {
  auto getInfo = vk::DescriptorGetInfoEXT{.data = {.pCombinedImageSampler = &descriptorImageInfo}};

  auto& buffer = getBuffer(bufferHandle);

  char* bufPtr = static_cast<char*>(buffer.getData());

  auto* location = bufPtr + slot * textureDslSize + textureDslOffset;

  // Reminder this function gets a descriptor in the device, and places it in the memory location
  // This location happens to be mapped to a buffer.
  device->getVkDevice().getDescriptorEXT(getInfo, descriptorSize, location);
}

auto VkResourceManager::getTextureData(const as::ImageData& imageData,
                                       std::string_view name) -> TextureData {
  auto textureData = TextureData{};
  auto textureSize =
      static_cast<vk::DeviceSize>(imageData.width * imageData.height * imageData.component);

  // Create a Staging Buffer
  const auto bufferCreateInfo = vk::BufferCreateInfo{.size = textureSize,
                                                     .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                                     .sharingMode = vk::SharingMode::eExclusive};
  const auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                 vk::MemoryPropertyFlagBits::eHostCoherent};

  const auto stagingBuffer =
      allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);

  // Copy the texture data into the staging buffer
  auto* const bufferData = allocator->mapMemory(*stagingBuffer);
  memcpy(bufferData, imageData.data.data(), textureSize);
  allocator->unmapMemory(*stagingBuffer);

  // Setup buffer copy regions for each mip level
  std::vector<vk::BufferImageCopy> bufferCopyRegions;

  // hack
  constexpr uint32_t mipLevels = 1;
  const auto uWidth = static_cast<uint32_t>(imageData.width);
  const auto uHeight = static_cast<uint32_t>(imageData.height);

  for (uint32_t i = 0; i < mipLevels; i++) {
    vk::DeviceSize offset{0};

    const auto bufferCopyRegion =
        vk::BufferImageCopy{.bufferOffset = offset,
                            .imageSubresource =
                                vk::ImageSubresourceLayers{
                                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                                    .mipLevel = i,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1,
                                },
                            .imageExtent = vk::Extent3D{.width = std::max(1u, uWidth >> i),
                                                        .height = std::max(1u, uHeight >> i),
                                                        .depth = 1u}};
    bufferCopyRegions.push_back(bufferCopyRegion);
  }
  constexpr auto format = vk::Format::eR8G8B8A8Srgb;
  auto imageCreateInfo = vk::ImageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {.width = uWidth, .height = uHeight, .depth = 1u},
      .mipLevels = mipLevels,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eSampled,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined,
  };
  if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst)) {
    imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
  }

  constexpr auto imageAllocateCreateInfo =
      vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

  // TODO(matt) this has to stick around until removeTexture is called
  textureData.image = allocator->createImage(imageCreateInfo, imageAllocateCreateInfo, name);

  constexpr auto subresourceRange =
      vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                .baseMipLevel = 0,
                                .levelCount = mipLevels,
                                .layerCount = 1};

  // Upload Image
  immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
    {
      const auto [dstBarrier, sourceStage, dstStage] =
          createTransitionBarrier(textureData.image->getImage(),
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eTransferDstOptimal,
                                  subresourceRange);
      cmd.pipelineBarrier(sourceStage,
                          dstStage,
                          vk::DependencyFlagBits::eByRegion,
                          {},
                          {},
                          dstBarrier);
    }
    cmd.copyBufferToImage(stagingBuffer->getBuffer(),
                          textureData.image->getImage(),
                          vk::ImageLayout::eTransferDstOptimal,
                          bufferCopyRegions);
    {
      const auto [dstBarrier, sourceStage, dstStage] =
          createTransitionBarrier(textureData.image->getImage(),
                                  vk::ImageLayout::eTransferDstOptimal,
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  subresourceRange);
      cmd.pipelineBarrier(sourceStage,
                          dstStage,
                          vk::DependencyFlagBits::eByRegion,
                          {},
                          {},
                          dstBarrier);
    }
  });

  constexpr auto samplerInfo =
      vk::SamplerCreateInfo{.magFilter = vk::Filter::eLinear,
                            .minFilter = vk::Filter::eLinear,
                            .mipmapMode = vk::SamplerMipmapMode::eLinear,
                            .addressModeU = vk::SamplerAddressMode::eRepeat,
                            .addressModeV = vk::SamplerAddressMode::eRepeat,
                            .addressModeW = vk::SamplerAddressMode::eRepeat,
                            .mipLodBias = 0.f,
                            .anisotropyEnable = VK_TRUE,
                            .maxAnisotropy = 1, // TODO(matt): look this up
                            .compareEnable = VK_FALSE,
                            .compareOp = vk::CompareOp::eAlways,
                            .minLod = 0.f,
                            .maxLod = 0.f,
                            .borderColor = vk::BorderColor::eIntOpaqueBlack,
                            .unnormalizedCoordinates = VK_FALSE};

  // TODO(matt) this has to stay alive until removeTexture is called
  textureData.sampler = *device->getVkDevice().createSampler(samplerInfo);

  constexpr auto range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                   .baseMipLevel = 0,
                                                   .levelCount = 1,
                                                   .baseArrayLayer = 0,
                                                   .layerCount = 1};

  const auto viewInfo = vk::ImageViewCreateInfo{.image = textureData.image->getImage(),
                                                .viewType = vk::ImageViewType::e2D,
                                                .format = format,
                                                .subresourceRange = range};

  // TODO(matt) this also has to stay alive until removeTexture is called
  textureData.imageView = *device->getVkDevice().createImageView(viewInfo);

  textureData.layout = vk::ImageLayout::eShaderReadOnlyOptimal;

  return textureData;
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

auto VkResourceManager::createDescriptorBuffer(size_t size, std::string_view name) -> BufferHandle {
  const auto flags = vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                     vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT |
                     vk::BufferUsageFlagBits::eShaderDeviceAddress;

  const auto bufferSize = size * textureDslSize;
  const auto memoryUsage = vma::MemoryUsage::eCpuToGpu;
  const auto memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent;

  return createBuffer(bufferSize, flags, name, memoryUsage, memoryProperties, true);
}

auto VkResourceManager::createBuffer(size_t size,
                                     vk::Flags<vk::BufferUsageFlagBits> flags,
                                     std::string_view name,
                                     vma::MemoryUsage usage,
                                     vk::MemoryPropertyFlags memoryProperties,
                                     bool mapped) -> BufferHandle {
  const auto bufferCreateInfo = vk::BufferCreateInfo{.size = size, .usage = flags};

  auto allocationCreateInfo =
      vma::AllocationCreateInfo{.usage = usage, .requiredFlags = memoryProperties};

  if (mapped) {
    allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eMapped;
  }

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

[[nodiscard]] auto VkResourceManager::resizeBuffer(BufferHandle handle,
                                                   size_t newSize) -> BufferHandle {
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
                                  vk::DeviceSize vertexOffset,
                                  [[maybe_unused]] BufferHandle indexBufferHandle,
                                  vk::DeviceSize indexOffset) -> void {
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

    auto& vertexBuffer = getBuffer(vertexBufferHandle);
    auto& indexBuffer = getBuffer(indexBufferHandle);

    immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
      const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = vertexOffset, .size = vbSize};
      cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer.getBuffer(), vbCopy);
      const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = indexOffset, .size = ibSize};
      cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer.getBuffer(), copy);
    });

  } catch (const AllocationException& ex) {
    throw ResourceUploadException(
        fmt::format("Error allocating resources for geometry, {0}", ex.what()));
  }
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

auto VkResourceManager::destroyDrawImageAndView(ImageHandle handle) -> void {
  imageInfoMap.erase(handle);
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

auto VkResourceManager::getDescriptorBuffer() const -> Buffer& {
  return *bufferMap.at(textureBufferManager->getDescriptorBufferHandle());
}

auto VkResourceManager::getDescriptorSetLayout() -> const vk::DescriptorSetLayout* {
  return &**textureDsl;
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
    const std::vector<RenderMeshData>& gpuBufferData) -> std::vector<GpuBufferEntry>& {
  ZoneNamedN(var, "getGpuBufferEntries", true);
  return staticMeshBufferManager->getGpuBufferEntries(gpuBufferData);
}

auto VkResourceManager::createTransitionBarrier(const vk::Image& image,
                                                const vk::ImageLayout oldLayout,
                                                const vk::ImageLayout newLayout,
                                                const vk::ImageSubresourceRange& subresourceRange)
    -> TransitionBarrierInfo {
  vk::ImageMemoryBarrier barrier{
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eNone,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = subresourceRange,
  };

  auto sourceStage = vk::PipelineStageFlagBits::eNone;
  auto destinationStage = vk::PipelineStageFlagBits::eNone;

  if (oldLayout == vk::ImageLayout::eUndefined &&
      newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eNone;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("Unsupported layout transition");
  }
  return {barrier, sourceStage, destinationStage};
}

}
