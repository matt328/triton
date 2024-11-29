#include "VkGraphicsDevice.hpp"
#include "Vulkan.hpp"
#include "VkContext.hpp"
#include "as/Model.hpp"
#include "cm/Handles.hpp"
#include "geo/GeometryData.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "mem/Allocator.hpp"
#include "mem/Image.hpp"
#include "ResourceExceptions.hpp"
#include "tex/Texture.hpp"
#include "geo/Mesh.hpp"
#include "vk/PhysicalDevice.hpp"

/*
 * Have PhysicalDevice create a vk::raii::Device, and wrap that with a Device injectable.
 * Create an injectable abstraction over a queue, and have 4 types, but whatever the system supports
 * There will need to be a factory that uses the Device class to create queues.
 * Some queues may just fall back to using others underneath, eg if a separate transfer queue isn't
 * available, still present a transfer queue api, but just use the graphics queue underneath
 *
 *
 */

/*
 * - extract ResourceManager into a DI component
 *    - All buffers, images, image views, samplers, pipelines/layouts, descriptor sets and pools
 *      should be created by the ResourceManager and only handles given out.
 * - create RenderScheduler
 * - implement what's needed for a StaticGeometryRenderJob
 * - add PBR and some kind of lighting?
 * - Add an AnimatedModelRenderJob
 * - Add TerrainRenderJob
 */

namespace tr::gfx {

   VkGraphicsDevice::VkGraphicsDevice(std::shared_ptr<tr::IWindow> newWindow,
                                      std::shared_ptr<Context> newContext,
                                      std::shared_ptr<IDebugManager> newDebugManager,
                                      std::shared_ptr<Instance> newInstance,
                                      std::shared_ptr<PhysicalDevice> newPhysicalDevice)
       : window{std::move(newWindow)},
         context{std::move(newContext)},
         debugManager{std::move(newDebugManager)},
         instance{std::move(newInstance)},
         physicalDevice{std::move(newPhysicalDevice)} {

      debugManager->checkDebugSupport();

      // Select and identify Queues
      auto queueFamilyIndices = Helpers::findQueueFamilies(*physicalDevice, *surface);
      std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

      // Graphics Queue(s)
      if (queueFamilyIndices.graphicsFamily.has_value() &&
          queueFamilyIndices.graphicsFamilyCount.has_value()) {
         const auto graphicsFamilyCreateInfo = vk::DeviceQueueCreateInfo{
             .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
             .queueCount = queueFamilyIndices.graphicsFamilyCount.value(),
             .pQueuePriorities = queueFamilyIndices.graphicsFamilyPriorities.data()};
         queueCreateInfos.push_back(graphicsFamilyCreateInfo);
      }

      // If present queue family is different from graphics
      if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
         Log.trace("Device supports separate present queue");
         // Present Queue(s)
         if (queueFamilyIndices.presentFamily.has_value() &&
             queueFamilyIndices.presentFamilyCount.has_value()) {
            const auto presentFamilyCreateInfo = vk::DeviceQueueCreateInfo{
                .queueFamilyIndex = queueFamilyIndices.presentFamily.value(),
                .queueCount = queueFamilyIndices.presentFamilyCount.value(),
                .pQueuePriorities = queueFamilyIndices.presentFamilyPriorities.data()};
            queueCreateInfos.push_back(presentFamilyCreateInfo);
         }
      }

      auto deviceProperties =
          physicalDevice->getProperties2KHR<vk::PhysicalDeviceProperties2KHR,
                                            vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();

      descriptorBufferProperties =
          deviceProperties.get<vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();

      auto drfs = vk::PhysicalDeviceDynamicRenderingFeaturesKHR{
          .dynamicRendering = VK_TRUE,
      };

      auto features2 =
          physicalDevice->getFeatures2<vk::PhysicalDeviceFeatures2,
                                       vk::PhysicalDevice16BitStorageFeatures,
                                       vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

      auto indexingFeatures = features2.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

      auto drawParamsFeatures =
          vk::PhysicalDeviceShaderDrawParametersFeatures{.shaderDrawParameters = VK_TRUE};

      if (const auto bindlessTexturesSupported = indexingFeatures.descriptorBindingPartiallyBound &&
                                                 indexingFeatures.runtimeDescriptorArray;
          !bindlessTexturesSupported) {
         throw std::runtime_error("GPU does not support bindless textures :(");
      }

      auto physicalFeatures2 = physicalDevice->getFeatures2();
      physicalFeatures2.features.samplerAnisotropy = VK_TRUE;

      vk::DeviceCreateInfo createInfo{
          .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
          .pQueueCreateInfos = queueCreateInfos.data(),
          .enabledLayerCount = 0,
          .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
          .ppEnabledExtensionNames = desiredDeviceExtensions.data()};

      debugManager->addDeviceConfig(createInfo);

      // const auto dbFeatures =
      // vk::PhysicalDeviceDescriptorBufferFeaturesEXT{.descriptorBuffer = true};

      const auto extendedDynamicStateFeatures =
          vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{.extendedDynamicState = true};

      constexpr auto bdaFeatures =
          vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR{.bufferDeviceAddress = true};

      const vk::StructureChain c{createInfo,
                                 physicalFeatures2,
                                 drawParamsFeatures,
                                 indexingFeatures,
                                 drfs,
                                 // dbFeatures,
                                 extendedDynamicStateFeatures,
                                 bdaFeatures};

      vulkanDevice =
          std::make_shared<vk::raii::Device>(physicalDevice->createDevice(c.get(), nullptr));

      // Just don't name this because a bug in vulkan keeps complaining it doesn't match but it does
      // Helpers::setObjectName(**vulkanDevice, *vulkanDevice.get(), "Primary Device");

      Log.trace("Created Logical Device");

      graphicsQueue = std::make_shared<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.graphicsFamily.value(), 0));
      Helpers::setObjectName(**graphicsQueue, *vulkanDevice, "Graphics Queue");
      Log.trace("Created Graphics Queue");

      presentQueue = std::make_unique<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.presentFamily.value(), 0));
      Helpers::setObjectName(**presentQueue, *vulkanDevice, "Present Queue");
      Log.trace("Created Present Queue");

      transferQueue = std::make_shared<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.transferFamily.value(), 0));
      Helpers::setObjectName(**transferQueue, *vulkanDevice, "Transfer Queue");
      Log.trace("Created Transfer Queue");

      computeQueue = std::make_unique<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.computeFamily.value(), 0));
      Helpers::setObjectName(**computeQueue, *vulkanDevice, "Compute Queue");
      Log.trace("Created Compute Queue");

      createSwapchain();

      asyncTransferContext = std::make_shared<VkContext>(*vulkanDevice,
                                                         *physicalDevice,
                                                         0,
                                                         queueFamilyIndices.transferFamily.value(),
                                                         "Async Transfer Context");

      constexpr auto vulkanFunctions = vma::VulkanFunctions{
          .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
          .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
      };

      const auto allocatorCreateInfo = vma::AllocatorCreateInfo{
          .flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress,
          .physicalDevice = **physicalDevice,
          .device = **vulkanDevice,
          .pVulkanFunctions = &vulkanFunctions,
          .instance = instance->getVkInstance(),
      };

      {
         ZoneNamedN(zone, "Create Allocator", true);
         allocator = std::make_unique<mem::Allocator>(allocatorCreateInfo, *vulkanDevice);
      }
   }

   VkGraphicsDevice::~VkGraphicsDevice() {
      Log.trace("Destroying VkGraphicsDevice");
      debugManager->destroyDebugCallbacks();
   }

   auto VkGraphicsDevice::submit(const vk::SubmitInfo& submitInfo,
                                 const std::unique_ptr<vk::raii::Fence>& fence) -> void {
      graphicsQueue->submit(submitInfo, **fence);
   }

   auto VkGraphicsDevice::present(const std::unique_ptr<vk::raii::Semaphore>& semaphore,
                                  uint32_t imageIndex) -> vk::Result {
      try {
         const auto sphore = std::array<vk::Semaphore, 1>{*semaphore};
         return graphicsQueue->presentKHR(vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                                                             .pWaitSemaphores = sphore.data(),
                                                             .swapchainCount = 1,
                                                             .pSwapchains = &**swapchain,
                                                             .pImageIndices = &imageIndex});
      } catch (const std::exception& ex) {
         Log.trace("presentKHR error {}, recreating swapchain", ex.what());
         return vk::Result::eSuboptimalKHR;
      }
   }

   auto VkGraphicsDevice::createPipelineLayout(const vk::PipelineLayoutCreateInfo& createInfo,
                                               const std::string& name)
       -> std::unique_ptr<vk::raii::PipelineLayout> {
      auto layout = std::make_unique<vk::raii::PipelineLayout>(*vulkanDevice, createInfo);
      setObjectName(**layout, name);
      return layout;
   }

   auto VkGraphicsDevice::createPipeline(const vk::GraphicsPipelineCreateInfo& createInfo,
                                         const std::string& name)
       -> std::unique_ptr<vk::raii::Pipeline> {

      auto pipeline =
          std::make_unique<vk::raii::Pipeline>(*vulkanDevice, VK_NULL_HANDLE, createInfo);

      setObjectName(**pipeline, name);

      return pipeline;
   }

   auto VkGraphicsDevice::createImage(const vk::ImageCreateInfo& imageCreateInfo,
                                      const vma::AllocationCreateInfo& allocationCreateInfo,
                                      const std::string_view& newName) const
       -> std::unique_ptr<mem::Image> {
      return allocator->createImage(imageCreateInfo, allocationCreateInfo, newName);
   }

   auto VkGraphicsDevice::createDrawImage(std::string_view newName) const
       -> std::pair<std::unique_ptr<mem::Image>, std::unique_ptr<vk::raii::ImageView>> {
      constexpr auto drawImageFormat = vk::Format::eR16G16B16A16Sfloat;
      const auto [width, height] = IGraphicsDevice::DrawImageExtent2D;

      const auto imageCreateInfo = vk::ImageCreateInfo{
          .imageType = vk::ImageType::e2D,
          .format = drawImageFormat,
          .extent = vk::Extent3D{width, height, 1},
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
      auto drawImage = allocator->createImage(imageCreateInfo, imageAllocateCreateInfo, newName);

      const auto imageViewCreateInfo =
          vk::ImageViewCreateInfo{.image = drawImage->getImage(),
                                  .viewType = vk::ImageViewType::e2D,
                                  .format = drawImageFormat,
                                  .subresourceRange = {
                                      .aspectMask = vk::ImageAspectFlagBits::eColor,
                                      .levelCount = 1,
                                      .layerCount = 1,
                                  }};
      auto drawImageView =
          std::make_unique<vk::raii::ImageView>(vulkanDevice->createImageView(imageViewCreateInfo));

      return std::make_pair(std::move(drawImage), std::move(drawImageView));
   }

   auto VkGraphicsDevice::createStorageBuffer(vk::DeviceSize size, const std::string& name)
       -> std::unique_ptr<mem::Buffer> {
      const auto bufferCreateInfo =
          vk::BufferCreateInfo{.size = size,
                               .usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      return allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   auto VkGraphicsDevice::createUniformBuffer(vk::DeviceSize size, const std::string& name)
       -> std::unique_ptr<mem::Buffer> {
      const auto bufferCreateInfo =
          vk::BufferCreateInfo{.size = size,
                               .usage = vk::BufferUsageFlagBits::eUniformBuffer |
                                        vk::BufferUsageFlagBits::eShaderDeviceAddress};

      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuToGpu,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

      return allocator->createBuffer(&bufferCreateInfo, &allocationCreateInfo, name);
   }

   auto VkGraphicsDevice::createCommandBuffer() -> std::unique_ptr<vk::raii::CommandBuffer> {
      const auto allocInfo =
          vk::CommandBufferAllocateInfo{.commandPool = *commandPool,
                                        .level = vk::CommandBufferLevel::ePrimary,
                                        .commandBufferCount = 1};
      auto commandBuffers = vulkanDevice->allocateCommandBuffers(allocInfo);
      return std::make_unique<vk::raii::CommandBuffer>(std::move(commandBuffers[0]));
   }

   auto VkGraphicsDevice::createTracyContext(std::string_view name,
                                             const vk::raii::CommandBuffer& commandBuffer)
       -> cm::TracyContextPtr {
      auto ctx = cm::TracyContextPtr{tracy::CreateVkContext(**physicalDevice,
                                                            **vulkanDevice,
                                                            **graphicsQueue,
                                                            *commandBuffer,
                                                            nullptr,
                                                            nullptr),
                                     tracy::DestroyVkContext};
      ctx->Name(name.data(), name.length());
      return ctx;
   }

   auto VkGraphicsDevice::acquireNextSwapchainImage(const vk::Semaphore& semaphore)
       -> std::variant<uint32_t, AcquireResult> {
      try {
         ZoneNamedN(acquire, "Acquire Swapchain Image", true);
         auto [result, imageIndex] = swapchain->acquireNextImage(UINT64_MAX, semaphore, nullptr);
         if (result == vk::Result::eSuccess) {
            return imageIndex;
         }
         if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) {
            return AcquireResult::NeedsResize;
         }
         return AcquireResult::Error;
      } catch (const std::exception& ex) {
         Log.warn("Swapchain needs resized: {0}", ex.what());
         return AcquireResult::NeedsResize;
      }
   }

   auto VkGraphicsDevice::uploadVertexData(const geo::GeometryData& geometryData)
       -> cm::MeshHandle {

      // Prepare Vertex Buffer
      const auto vbSize = geometryData.vertexDataSize();
      const auto ibSize = geometryData.indexDataSize();

      try {
         const auto vbStagingBuffer =
             allocator->createStagingBuffer(vbSize, "Vertex Staging Buffer");
         void* vbData = allocator->mapMemory(*vbStagingBuffer);
         memcpy(vbData, geometryData.vertices.data(), static_cast<size_t>(vbSize));
         allocator->unmapMemory(*vbStagingBuffer);

         // Prepare Index Buffer
         const auto ibStagingBuffer =
             allocator->createStagingBuffer(ibSize, "Index Staging Buffer");

         auto* const data = allocator->mapMemory(*ibStagingBuffer);
         memcpy(data, geometryData.indices.data(), ibSize);
         allocator->unmapMemory(*ibStagingBuffer);

         auto vertexBuffer = allocator->createGpuVertexBuffer(vbSize, "GPU Vertex");
         auto indexBuffer = allocator->createGpuIndexBuffer(ibSize, "GPU Index");
         const auto indicesCount = geometryData.indices.size();

         // Upload Buffers
         asyncTransferContext->submit([&](const vk::raii::CommandBuffer& cmd) {
            const auto vbCopy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = vbSize};
            cmd.copyBuffer(vbStagingBuffer->getBuffer(), vertexBuffer->getBuffer(), vbCopy);
            const auto copy = vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = ibSize};
            cmd.copyBuffer(ibStagingBuffer->getBuffer(), indexBuffer->getBuffer(), copy);
         });

         const auto meshHandle = meshList.size();
         meshList.emplace_back(std::move(vertexBuffer), std::move(indexBuffer), indicesCount);

         return meshHandle;

      } catch (const mem::AllocationException& ex) {
         throw ResourceUploadException(
             fmt::format("Error allocating resources for geometry, {0}", ex.what()));
      }
   }

   auto VkGraphicsDevice::uploadImageData(const as::ImageData& imageData) -> cm::TextureHandle {
      const auto textureHandle = textureList.size();
      textureList.push_back(
          std::make_unique<tex::Texture>(static_cast<const void*>(imageData.data.data()),
                                         imageData.width,
                                         imageData.height,
                                         imageData.component,
                                         *allocator,
                                         *vulkanDevice,
                                         *asyncTransferContext));

      {
         ZoneNamedN(zn1, "Update TextureInfoList", true);
         std::lock_guard lock(textureListMutex);
         LockMark(textureListMutex);
         LockableName(textureListMutex, "Mutate", 6);
         textureInfoList.emplace_back(textureList[textureHandle]->getImageInfo());
      }
      return textureHandle;
   }

   auto VkGraphicsDevice::getTextures() const
       -> cm::LockableResource<const std::vector<vk::DescriptorImageInfo>> {
      return {textureInfoList, textureListMutex};
   }

   auto VkGraphicsDevice::getMesh(cm::MeshHandle meshHandle) -> geo::ImmutableMesh& {
      return meshList.at(meshHandle);
   }

   // Utility Functions

   auto VkGraphicsDevice::findDepthFormat() -> vk::Format {
      constexpr auto candidates = std::array<vk::Format, 3>{
          {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}};
      for (const auto format : candidates) {
         if (auto props = physicalDevice->getFormatProperties(format);
             (props.linearTilingFeatures | props.optimalTilingFeatures) &
             vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            return format;
         }
      }
      throw std::runtime_error("Failed to find supported format");
   }

   auto VkGraphicsDevice::getDescriptorBufferProperties()
       -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT {
      return descriptorBufferProperties;
   }

   auto VkGraphicsDevice::getRequiredExtensions() const
       -> std::pair<std::vector<const char*>, bool> {
      uint32_t glfwExtensionCount = 0;
      auto* const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      // NOLINTNEXTLINE This is ok because glfw's C api sucks.
      std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

      extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

      const auto exts = context->getExtensionProperties();

      std::vector<std::string> extNames = {};

      extNames.reserve(exts.size());
      for (const auto& ext : exts) {
         extNames.push_back(ext.extensionName);
      }

      extNames.emplace_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);

      auto portabilityPresent = false;

#ifdef __APPLE__
      portabilityPresent = std::ranges::find_if(extNames, [](const std::string& name) {
                              return name == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
                           }) != extNames.end();

      if (portabilityPresent) {
         extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      }
#endif

      return std::make_pair(extensions, portabilityPresent);
   }

   auto VkGraphicsDevice::enumeratePhysicalDevices() const
       -> std::vector<vk::raii::PhysicalDevice> {
      return instance->enumeratePhysicalDevices();
   }

   void VkGraphicsDevice::recreateSwapchain() {
      oldSwapchain = std::move(swapchain);
      createSwapchain();
   }

   auto VkGraphicsDevice::waitIdle() -> void {
      vulkanDevice->waitIdle();
   }

   void VkGraphicsDevice::createSwapchain() {
      if (oldSwapchain != nullptr) {
         commandPool.reset();
         swapchainImages.clear();
         swapchainImageViews.clear();
      }
      auto queueFamilyIndicesInfo = Helpers::findQueueFamilies(*physicalDevice, *surface);

      auto [capabilities, formats, presentModes] =
          Helpers::querySwapchainSupport(*physicalDevice, *surface);

      const auto surfaceFormat = Helpers::chooseSwapSurfaceFormat(formats);
      const auto presentMode = Helpers::chooseSwapPresentMode(presentModes);
      const auto extent = Helpers::chooseSwapExtent(capabilities, getCurrentSize());

      uint32_t imageCount = capabilities.minImageCount + 1;

      if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
         imageCount = capabilities.maxImageCount;
      }

      vk::SwapchainCreateInfoKHR swapchainCreateInfo{
          .surface = **surface,
          .minImageCount = imageCount,
          .imageFormat = surfaceFormat.format,
          .imageColorSpace = surfaceFormat.colorSpace,
          .imageExtent = extent,
          .imageArrayLayers = 1,
          .imageUsage =
              vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
          .preTransform = capabilities.currentTransform,
          .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
          .presentMode = presentMode,
          .clipped = VK_TRUE,
          .oldSwapchain = VK_NULL_HANDLE};

      if (oldSwapchain != nullptr) {
         swapchainCreateInfo.oldSwapchain = **oldSwapchain;
      }

      const auto queueFamilyIndices = std::array{queueFamilyIndicesInfo.graphicsFamily.value(),
                                                 queueFamilyIndicesInfo.presentFamily.value()};

      if (queueFamilyIndicesInfo.graphicsFamily != queueFamilyIndicesInfo.presentFamily) {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
         swapchainCreateInfo.queueFamilyIndexCount = 2;
         swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
      } else {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
      }

      swapchain = std::make_unique<vk::raii::SwapchainKHR>(*vulkanDevice, swapchainCreateInfo);
      Log.info("Created Swapchain");

      swapchainExtent = extent;
      swapchainImageFormat = surfaceFormat.format;

      swapchainImages = swapchain->getImages();
      swapchainImageViews.reserve(swapchainImages.size());

      constexpr vk::ComponentMapping components{.r = vk::ComponentSwizzle::eIdentity,
                                                .g = vk::ComponentSwizzle::eIdentity,
                                                .b = vk::ComponentSwizzle::eIdentity,
                                                .a = vk::ComponentSwizzle::eIdentity};

      constexpr vk::ImageSubresourceRange subresourceRange{.aspectMask =
                                                               vk::ImageAspectFlagBits::eColor,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1};

      for (const auto& image : swapchainImages) {
         vk::ImageViewCreateInfo createInfo{.image = image,
                                            .viewType = vk::ImageViewType::e2D,
                                            .format = swapchainImageFormat,
                                            .components = components,
                                            .subresourceRange = subresourceRange};

         swapchainImageViews.emplace_back(*vulkanDevice, createInfo);
      }
      Log.info("Created {0} swapchain image views", swapchainImageViews.size());

      // Create Command Pools
      auto commandPoolCreateInfo = vk::CommandPoolCreateInfo{
          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
          .queueFamilyIndex = queueFamilyIndicesInfo.graphicsFamily.value()};

      commandPool = std::make_unique<vk::raii::CommandPool>(
          vulkanDevice->createCommandPool(commandPoolCreateInfo));
      Helpers::setObjectName(**commandPool, *vulkanDevice, "Graphics Command Pool");
   }

   auto VkGraphicsDevice::getCurrentSize() const -> std::pair<uint32_t, uint32_t> {
      const auto surfaceCaps = physicalDevice->getSurfaceCapabilitiesKHR(**surface);
      return std::make_pair(surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height);
   }

   [[nodiscard]] auto VkGraphicsDevice::getSwapchainExtent() -> vk::Extent2D {
      return swapchainExtent;
   }

   auto VkGraphicsDevice::getSwapchainImage(uint32_t swapchainImageIndex) -> const vk::Image& {
      return swapchainImages[swapchainImageIndex];
   }

   auto VkGraphicsDevice::getSwapchainImageView(uint32_t swapchainImageIndex)
       -> const vk::raii::ImageView& {
      return swapchainImageViews[swapchainImageIndex];
   }

   void VkGraphicsDevice::transitionImage(const vk::raii::CommandBuffer& cmd,
                                          const vk::Image& image,
                                          const vk::ImageLayout& currentLayout,
                                          const vk::ImageLayout& newLayout) {
      const auto barrier = vk::ImageMemoryBarrier{
          .srcAccessMask = vk::AccessFlagBits::eMemoryWrite,
          .dstAccessMask = vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
          .oldLayout = currentLayout,
          .newLayout = newLayout,
          .image = image,
          .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                               .levelCount = 1,
                               .layerCount = 1}};

      cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                          vk::PipelineStageFlagBits::eAllCommands,
                          vk::DependencyFlagBits{}, // None
                          {},
                          {},
                          barrier);
   }

   void VkGraphicsDevice::copyImageToImage(const vk::raii::CommandBuffer& cmd,
                                           const vk::Image& source,
                                           const vk::Image& destination,
                                           const vk::Extent2D& srcSize,
                                           const vk::Extent2D& dstSize) {

      const auto blitRegion = vk::ImageBlit2{
          .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
          .srcOffsets = std::array{vk::Offset3D{},
                                   vk::Offset3D{.x = static_cast<int>(srcSize.width),
                                                .y = static_cast<int>(srcSize.height),
                                                .z = 1}},
          .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
          .dstOffsets = std::array{vk::Offset3D{},
                                   vk::Offset3D{.x = static_cast<int>(dstSize.width),
                                                .y = static_cast<int>(dstSize.height),
                                                .z = 1}},
      };

      const auto blitInfo = vk::BlitImageInfo2{
          .srcImage = source,
          .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
          .dstImage = destination,
          .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
          .regionCount = 1,
          .pRegions = &blitRegion,
          .filter = vk::Filter::eLinear,
      };

      cmd.blitImage2(blitInfo);
   }
}
