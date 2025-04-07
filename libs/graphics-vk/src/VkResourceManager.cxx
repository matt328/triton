#include "VkResourceManager.hpp"

#include "as/ColorVertex.hpp"
#include "as/StaticVertex.hpp"
#include "as/TerrainVertex.hpp"
#include "gfx/IGeometryData.hpp"
#include "mem/Allocator.hpp"
#include "mem/Image.hpp"
#include "gfx/IDebugManager.hpp"
#include "ResourceExceptions.hpp"
#include "mem/Buffer.hpp"
#include "pipeline/ComputePipeline.hpp"
#include "vk/ArenaGeometryBuffer.hpp"
#include "vk/sb/DSLayout.hpp"
#include "vk/sb/IShaderBinding.hpp"
#include "vk/sb/IShaderBindingFactory.hpp"
#include "vk/TextureManager.hpp"

namespace tr {

VkResourceManager::VkResourceManager(
    std::shared_ptr<Device> newDevice,
    std::shared_ptr<ImmediateTransferContext> newImmediateTransferContext,
    std::shared_ptr<IShaderModuleFactory> newShaderCompiler,
    std::shared_ptr<IDebugManager> newDebugManager,
    std::shared_ptr<DSLayoutManager> newLayoutManager,
    std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
    std::shared_ptr<Allocator> newAllocator,
    std::shared_ptr<IBufferManager> newBufferManager,
    std::shared_ptr<queue::Graphics> newGraphicsQueue,
    std::shared_ptr<queue::Transfer> newTransferQueue)
    : device{std::move(newDevice)},
      immediateTransferContext{std::move(newImmediateTransferContext)},
      shaderCompiler{std::move(newShaderCompiler)},
      debugManager{std::move(newDebugManager)},
      layoutManager{std::move(newLayoutManager)},
      shaderBindingFactory{std::move(newShaderBindingFactory)},
      allocator{std::move(newAllocator)},
      bufferManager{std::move(newBufferManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      transferQueue{std::move(newTransferQueue)} {

  constexpr auto binding =
      vk::DescriptorSetLayoutBinding{.binding = 0,
                                     .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                                     .descriptorCount = MaxTextureCount,
                                     .stageFlags = vk::ShaderStageFlagBits::eAll};
  textureDSLHandle = layoutManager->createLayout(binding, "DescriptorSetLayout-Texture");

  // NOLINTNEXTLINE
  textureShaderBindingHandle =
      shaderBindingFactory->createShaderBinding(ShaderBindingType::Textures, textureDSLHandle);

  const auto smbci = ArenaGeometryBufferCreateInfo{.vertexSize = sizeof(as::StaticVertex),
                                                   .indexSize = sizeof(uint32_t),
                                                   .bufferName = "StaticGeometry"};
  staticMeshBuffer = std::make_unique<ArenaGeometryBuffer>(bufferManager, smbci);

  const auto dmbci = ArenaGeometryBufferCreateInfo{.vertexSize = sizeof(as::DynamicVertex),
                                                   .indexSize = sizeof(uint32_t),
                                                   .bufferName = "DynamicGeometry"};

  dynamicMeshBuffer = std::make_unique<ArenaGeometryBuffer>(bufferManager, dmbci);

  const auto terrainBufferCreateInfo =
      ArenaGeometryBufferCreateInfo{.vertexSize = sizeof(as::TerrainVertex),
                                    .indexSize = sizeof(uint32_t),
                                    .bufferName = "TerrainGeometry"};

  terrainMeshBuffer = std::make_unique<ArenaGeometryBuffer>(bufferManager, terrainBufferCreateInfo);

  const auto dbmbci = ArenaGeometryBufferCreateInfo{.vertexSize = sizeof(as::ColorVertex),
                                                    .indexSize = sizeof(uint32_t),
                                                    .bufferName = "DebugGeometry"};
  debugMeshBuffer = std::make_unique<ArenaGeometryBuffer>(bufferManager, dbmbci);

  textureManager = std::make_unique<TextureManager>(this);
}

VkResourceManager::~VkResourceManager() {
  Log.trace("Destroying VkResourceManager");
}

auto VkResourceManager::uploadGeometryData([[maybe_unused]] const DDGeometryData& data)
    -> MeshHandle {
  // TODO(matt): The vertex list needs to contain enough data to find/create a buffer
  /*
    What will drive the creation/finding of renderers and pipelines? Probably more augmentations to
    the DDGeometryData struct.
    GeometryData can have a topology and some other params to create the pipeline
    How to determine which renderer to use? Maybe there are no different renderers, maybe they'll
    all do the same thing just with different pipelines and buffers?
    There may be more than 1 pipeline for objects in a given buffer, possibly MeshHandle needs to be
    augmented to be able to know which buffer to find the Mesh?
    Could there be only one GpuBufferEntry/ObjectDataBuffer? Some of those buffers could be
    condensed into a single global buffer, but the IndirectCommandBuffer and ObjectCountBuffer would
    need to exist for each pipeline.
    Also some pipelines may need additional buffers like AnimationData
  */
  return static_cast<MeshHandle>(0L);
}

auto VkResourceManager::uploadStaticMesh(const IGeometryData& geometryData) -> MeshHandle {
  return staticMeshBuffer->addMesh(geometryData);
}

auto VkResourceManager::uploadDynamicMesh(const IGeometryData& geometryData) -> MeshHandle {
  return dynamicMeshBuffer->addMesh(geometryData);
}

auto VkResourceManager::uploadTerrainMesh(const IGeometryData& geometryData) -> MeshHandle {
  return terrainMeshBuffer->addMesh(geometryData);
}

/*
  The process should be upload an image to the GPU in a background task which waits till the upload
  is finished, creates a vk::DescriptorImageInfo and stores it in a host side list and returns a
  TextureHandle, which is an index into this list. Also set a 'dirty' flag in the TextureManager.
  At the beginning of each frame, check the dirty flag, and if it's dirty, perform the
  WriteDescriptorSet.

  Should have a TextureManager that's just a part of the ResourceManager. ResourceManager will just
  delegate to it.
*/
auto VkResourceManager::uploadImage([[maybe_unused]] const as::ImageData& imageData,
                                    std::string_view name) -> TextureHandle {
  return textureManager->addTexture(imageData, name);
}

/// Synchronously uploads the texturedata to the device, waiting until it is uploaded and available
/// before creating and returning an image, view, and default sampler.
auto VkResourceManager::getTextureData(const as::ImageData& imageData, std::string_view name)
    -> TextureData {
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

  textureData.image = allocator->createImage(imageCreateInfo, imageAllocateCreateInfo, name);

  constexpr auto subresourceRange =
      vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                .baseMipLevel = 0,
                                .levelCount = mipLevels,
                                .layerCount = 1};

  /* TODO(matt) Need the transfer and graphics queues up in here to make sure we transition the
     ownership of the image we're uploading from the transfer queue to the graphics queue
  */

  // Upload Image
  immediateTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
    {
      const auto [dstBarrier, sourceStage, dstStage] =
          createTransitionBarrier(textureData.image->getImage(),
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eTransferDstOptimal,
                                  subresourceRange);

      cmd.pipelineBarrier(sourceStage,
                          vk::PipelineStageFlagBits::eTransfer,
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
                                  subresourceRange,
                                  transferQueue->getFamily(),
                                  graphicsQueue->getFamily());
      cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                          vk::PipelineStageFlagBits::eBottomOfPipe,
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
  textureData.sampler =
      std::make_unique<vk::raii::Sampler>(device->getVkDevice().createSampler(samplerInfo));

  constexpr auto range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                   .baseMipLevel = 0,
                                                   .levelCount = 1,
                                                   .baseArrayLayer = 0,
                                                   .layerCount = 1};

  const auto viewInfo = vk::ImageViewCreateInfo{.image = textureData.image->getImage(),
                                                .viewType = vk::ImageViewType::e2D,
                                                .format = format,
                                                .subresourceRange = range};

  textureData.imageView =
      std::make_unique<vk::raii::ImageView>(device->getVkDevice().createImageView(viewInfo));

  textureData.layout = vk::ImageLayout::eShaderReadOnlyOptimal;

  return textureData;
}

[[nodiscard]] auto VkResourceManager::getMesh(MeshHandle handle) -> const ImmutableMesh& {
  return meshList[handle];
}

[[nodiscard]] auto VkResourceManager::getStaticMeshBuffers() const -> std::tuple<Buffer&, Buffer&> {
  return staticMeshBuffer->getBuffers();
}

[[nodiscard]] auto VkResourceManager::getDynamicMeshBuffers() const
    -> std::tuple<Buffer&, Buffer&> {
  return dynamicMeshBuffer->getBuffers();
}

[[nodiscard]] auto VkResourceManager::getTerrainMeshBuffers() const
    -> std::tuple<Buffer&, Buffer&> {
  return terrainMeshBuffer->getBuffers();
}

[[nodiscard]] auto VkResourceManager::getDebugMeshBuffers() const -> std::tuple<Buffer&, Buffer&> {
  return debugMeshBuffer->getBuffers();
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
      .extent = vk::Extent3D{.width = extent.width, .height = extent.height, .depth = 1},
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
  assert(imageCreateInfo.extent.height != 0 && imageCreateInfo.extent.width != 0);
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

auto VkResourceManager::getTextureDSL() const -> const vk::DescriptorSetLayout* {
  return layoutManager->getLayout(textureDSLHandle).getVkLayout();
}

auto VkResourceManager::getTextureShaderBinding() const -> IShaderBinding& {
  return shaderBindingFactory->getShaderBinding(textureShaderBindingHandle);
}

auto VkResourceManager::updateShaderBindings() -> void {
  if (textureManager->isDirty()) {
    auto& sb = shaderBindingFactory->getShaderBinding(textureShaderBindingHandle);
    sb.bindImageSamplers(0, textureManager->getDescriptorImageInfoList());
  }
}

/// Gets a list of the GpuBufferEntry that the game world thinks are involved in this frame.
/// These are passed to the compute shader to get turned into DrawCommands and placed in the
/// DrawIndexedIndirect buffer
[[nodiscard]] auto VkResourceManager::getStaticGpuData(
    const std::vector<RenderMeshData>& gpuBufferData) -> std::vector<GpuBufferEntry>& {
  ZoneNamedN(var, "getGpuBufferEntries", true);
  return staticMeshBuffer->getGpuBufferEntries(gpuBufferData);
}

[[nodiscard]] auto VkResourceManager::getTerrainGpuData(
    const std::vector<RenderMeshData>& gpuBufferData) -> std::vector<GpuBufferEntry>& {
  ZoneNamedN(var, "getGpuBufferEntries", true);
  return terrainMeshBuffer->getGpuBufferEntries(gpuBufferData);
}

[[nodiscard]] auto VkResourceManager::getDynamicGpuData(
    const std::vector<RenderMeshData>& gpuBufferData) -> std::vector<GpuBufferEntry>& {
  ZoneNamedN(var, "getDynamicGpuBufferEntries", true);
  return dynamicMeshBuffer->getGpuBufferEntries(gpuBufferData);
}

auto VkResourceManager::createTransitionBarrier(const vk::Image& image,
                                                const vk::ImageLayout oldLayout,
                                                const vk::ImageLayout newLayout,
                                                const vk::ImageSubresourceRange& subresourceRange,
                                                uint32_t srcQueueFamily,
                                                uint32_t dstQueueFamily) -> TransitionBarrierInfo {
  vk::ImageMemoryBarrier barrier{
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eNone,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = srcQueueFamily,
      .dstQueueFamilyIndex = dstQueueFamily,
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
