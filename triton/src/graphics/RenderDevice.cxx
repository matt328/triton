#include "RenderDevice.hpp"
#include "Instance.hpp"
#include "ImmediateContext.hpp"
#include "Log.hpp"

#include "core/Utils.hpp"
#include "core/vma_raii.hpp"
#include "graphics/renderer/Clear.hpp"
#include "graphics/pipeline/DefaultPipeline.hpp"
#include "graphics/pipeline/ObjectMatrices.hpp"
#include "graphics/renderer/RendererBase.hpp"
#include "graphics/texture/Texture.hpp"
#include "graphics/texture/TextureFactory.hpp"

using Core::Log;

RenderDevice::RenderDevice(const Instance& instance) {
   createPhysicalDevice(instance);
   createLogicalDevice(instance);
   createSwapchain(instance);
   createSwapchainImageViews();
   createCommandPools(instance);
   createDescriptorPool();
   createAllocator(instance);

   {
      const auto a = 5;
      const auto c = a + 3;
   }

   renderPass = std::make_unique<vk::raii::RenderPass>(defaultRenderPass());
   pipeline = std::make_unique<DefaultPipeline>(*device, *renderPass, swapchainExtent);

   textureFactory = std::make_unique<TextureFactory>(
       *raiillocator, *device, *graphicsImmediateContext, *transferImmediateContext);

   meshFactory = std::make_unique<MeshFactory>(raiillocator.get(), transferImmediateContext.get());

   const auto textureFilename = Core::Paths::TEXTURES / "viking_room.png";

   tempTextureId = createTexture(textureFilename.string());

   createPerFrameData(pipeline->getDescriptorSetLayout());

   createDepthResources();
   createFramebuffers();

   const auto rendererCreateInfo =
       RendererBaseCreateInfo{.device = *device,
                              .allocator = *raiillocator,
                              .depthTexture = this->depthImage->getImage(),
                              .swapchainExtent = swapchainExtent,
                              .swapchainImages = swapchainImages};

   const auto renderer = std::make_unique<Clear>(rendererCreateInfo);
}

RenderDevice::~RenderDevice() = default;

void RenderDevice::waitIdle() const {
   device->waitIdle();
}

std::string RenderDevice::createMesh(const std::string_view& filename) {
   meshes[filename.data()] = meshFactory->loadMeshFromGltf(filename.data());
   return filename.data();
}

std::string RenderDevice::createTexture(const std::string_view& filename) {
   if (const auto it = textures.find(filename.data()); it != textures.end()) {
      return filename.data();
   }
   textures[filename.data()] = textureFactory->createTexture2D(filename);
   return filename.data();
}

void RenderDevice::enqueue(const Renderable& renderable) const {
   frameData[currentFrame]->renderables.push_back(renderable.getMeshId());
}

void RenderDevice::createPhysicalDevice(const Instance& instance) {
   const auto physicalDevices = instance.enumeratePhysicalDevices();

   if (physicalDevices.empty()) {
      throw std::runtime_error("Failed to find any GPUs with Vulkan Support");
   }

   for (const auto& possibleDevice : physicalDevices) {
      if (isDeviceSuitable(possibleDevice, instance)) {
         physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(possibleDevice);
         break;
      }
   }

   if (physicalDevice == nullptr) {
      throw std::runtime_error("Failed to find a suitable GPU");
   }

   Log::core->debug("Using physical device: {}", physicalDevice->getProperties().deviceName);
}

void RenderDevice::createLogicalDevice(const Instance& instance) {
   auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
       findQueueFamilies(*physicalDevice, instance.getSurface());

   std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
   std::set uniqueQueueFamilies = {graphicsFamily.value(), presentFamily.value()};

   float queuePriority = 1.f;

   for (auto queueFamily : uniqueQueueFamilies) {
      vk::DeviceQueueCreateInfo queueCreateInfo{
          .queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &queuePriority};
      queueCreateInfos.push_back(queueCreateInfo);
   }

   vk::PhysicalDeviceFeatures deviceFeatures{.samplerAnisotropy = VK_TRUE};

   auto desiredDeviceExtensions = instance.getDesiredDeviceExtensions();
   desiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

   vk::DeviceCreateInfo createInfo{
       .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
       .pQueueCreateInfos = queueCreateInfos.data(),
       .enabledLayerCount = 0,
       .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
       .ppEnabledExtensionNames = desiredDeviceExtensions.data(),
       .pEnabledFeatures = &deviceFeatures};

   const auto validationLayers = instance.getDesiredValidationLayers();
   if (instance.isValidationEnabled()) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
   }

   device = std::make_unique<vk::raii::Device>(physicalDevice->createDevice(createInfo, nullptr));

   setObjectName(
       **device, *device.get(), vk::raii::Device::debugReportObjectType, "Primary Device");

   Log::core->info("Created Logical Device");

   graphicsQueue = std::make_unique<vk::raii::Queue>(device->getQueue(graphicsFamily.value(), 0));
   setObjectName(
       **graphicsQueue, *device.get(), (**graphicsQueue).debugReportObjectType, "Graphics Queue");
   Log::core->info("Created Graphics Queue");

   presentQueue = std::make_unique<vk::raii::Queue>(device->getQueue(presentFamily.value(), 0));
   setObjectName(
       **presentQueue, *device.get(), (**presentQueue).debugReportObjectType, "Present Queue");
   Log::core->info("Created Present Queue");

   transferQueue = std::make_shared<vk::raii::Queue>(device->getQueue(transferFamily.value(), 0));
   setObjectName(
       **transferQueue, *device.get(), (**transferQueue).debugReportObjectType, "Transfer Queue");
   Log::core->info("Created Transfer Queue");

   computeQueue = std::make_unique<vk::raii::Queue>(device->getQueue(computeFamily.value(), 0));
   setObjectName(
       **computeQueue, *device.get(), (**computeQueue).debugReportObjectType, "Compute Queue");
   Log::core->info("Created Compute Queue");
}

void RenderDevice::createSwapchain(const Instance& instance) {
   const auto& surface = instance.getSurface();
   auto [capabilities, formats, presentModes] = querySwapchainSupport(*physicalDevice, surface);

   const auto surfaceFormat = chooseSwapSurfaceFormat(formats);
   const auto presentMode = chooseSwapPresentMode(presentModes);
   const auto extent = chooseSwapExtent(capabilities, instance);

   uint32_t imageCount = capabilities.minImageCount + 1;

   if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
      imageCount = capabilities.maxImageCount;
   }

   vk::SwapchainCreateInfoKHR createInfo{.surface = **surface,
                                         .minImageCount = imageCount,
                                         .imageFormat = surfaceFormat.format,
                                         .imageColorSpace = surfaceFormat.colorSpace,
                                         .imageExtent = extent,
                                         .imageArrayLayers = 1,
                                         .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                                         .preTransform = capabilities.currentTransform,
                                         .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                         .presentMode = presentMode,
                                         .clipped = VK_TRUE,
                                         .oldSwapchain = VK_NULL_HANDLE};

   auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
       findQueueFamilies(*physicalDevice, surface);

   const auto queueFamilyIndices =
       std::array<uint32_t, 2>{graphicsFamily.value(), presentFamily.value()};

   if (graphicsFamily != presentFamily) {
      createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
   } else {
      createInfo.imageSharingMode = vk::SharingMode::eExclusive;
   }

   swapchain = std::make_unique<vk::raii::SwapchainKHR>(*device, createInfo);
   Log::core->info("Created Swapchain");

   swapchainExtent = extent;
   swapchainImageFormat = surfaceFormat.format;
}

void RenderDevice::createSwapchainImageViews() {
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

      swapchainImageViews.emplace_back(*device, createInfo);
   }
   Log::core->info("Created {} swapchain image views", swapchainImageViews.size());
}

void RenderDevice::createCommandPools(const Instance& instance) {
   auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
       findQueueFamilies(*physicalDevice, instance.getSurface());
   auto commandPoolCreateInfo =
       vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                 .queueFamilyIndex = graphicsFamily.value()};

   commandPool =
       std::make_unique<vk::raii::CommandPool>(device->createCommandPool(commandPoolCreateInfo));
   setObjectName(
       **commandPool, *device.get(), (*commandPool).debugReportObjectType, "Graphics Command Pool");

   commandPoolCreateInfo.queueFamilyIndex = computeFamily.value();
   computeCommandPool =
       std::make_unique<vk::raii::CommandPool>(device->createCommandPool(commandPoolCreateInfo));
   setObjectName(**computeCommandPool,
                 *device.get(),
                 (*computeCommandPool).debugReportObjectType,
                 "Compute Command Pool");

   transferImmediateContext =
       std::make_unique<ImmediateContext>(*device.get(), *transferQueue, transferFamily.value());

   graphicsImmediateContext =
       std::make_unique<ImmediateContext>(*device.get(), *graphicsQueue, graphicsFamily.value());
}

void RenderDevice::createDescriptorPool() {
   const auto poolSize =
       std::array{vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = FRAMES_IN_FLIGHT * 10},
                  vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                         .descriptorCount = FRAMES_IN_FLIGHT * 10}};

   const vk::DescriptorPoolCreateInfo poolInfo{
       .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
       .maxSets = FRAMES_IN_FLIGHT * 2,
       .poolSizeCount = 2,
       .pPoolSizes = poolSize.data()};

   descriptorPool =
       std::make_unique<vk::raii::DescriptorPool>(device->createDescriptorPool(poolInfo, nullptr));

   const auto poolSizesBindless = std::array{
       vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                              .descriptorCount = 1024},
       vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = 1024}};

   const auto bindlessPoolInfo =
       vk::DescriptorPoolCreateInfo{.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
                                    .maxSets = 1024,
                                    .pPoolSizes = poolSizesBindless.data()};

   const auto bindlessDescriptorPool = std::make_unique<vk::raii::DescriptorPool>(
       device->createDescriptorPool(bindlessPoolInfo, nullptr));
}

void RenderDevice::createAllocator(const Instance& instance) {
   const auto allocatorCreateInfo =
       vma::AllocatorCreateInfo{.physicalDevice = **physicalDevice,
                                .device = **device,
                                .instance = **instance.getVkInstance()};

   raiillocator = std::make_unique<vma::raii::Allocator>(allocatorCreateInfo);
}

void RenderDevice::createPerFrameData(const vk::raii::DescriptorSetLayout& descriptorSetLayout) {
   for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
      frameData.push_back(
          std::make_unique<FrameData>(*device,
                                      *commandPool,
                                      *raiillocator,
                                      *descriptorPool,
                                      descriptorSetLayout,
                                      textures[tempTextureId]->getDescriptorImageInfo()));
   }
}

void RenderDevice::createDepthResources() {
   const auto depthFormat = findDepthFormat();

   const auto imageCreateInfo =
       vk::ImageCreateInfo{.imageType = vk::ImageType::e2D,
                           .format = depthFormat,
                           .extent = vk::Extent3D{.width = swapchainExtent.width,
                                                  .height = swapchainExtent.height,
                                                  .depth = 1},
                           .mipLevels = 1,
                           .arrayLayers = 1,
                           .samples = vk::SampleCountFlagBits::e1,
                           .tiling = vk::ImageTiling::eOptimal,
                           .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                           .sharingMode = vk::SharingMode::eExclusive,
                           .initialLayout = vk::ImageLayout::eUndefined};

   constexpr auto allocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

   depthImage = raiillocator->createImage(imageCreateInfo, allocationCreateInfo);

   constexpr auto range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eDepth,
                                                    .baseMipLevel = 0,
                                                    .levelCount = 1,
                                                    .baseArrayLayer = 0,
                                                    .layerCount = 1};

   const auto viewInfo = vk::ImageViewCreateInfo{.image = depthImage->getImage(),
                                                 .viewType = vk::ImageViewType::e2D,
                                                 .format = depthFormat,
                                                 .subresourceRange = range};
   depthImageView = std::make_unique<vk::raii::ImageView>(device->createImageView(viewInfo));
}

void RenderDevice::createFramebuffers() {
   swapchainFramebuffers.reserve(swapchainImageViews.size());
   std::array<vk::ImageView, 2> attachments;

   for (const auto& imageView : swapchainImageViews) {
      attachments[0] = *imageView;
      attachments[1] = **depthImageView;

      const auto framebufferCreateInfo =
          vk::FramebufferCreateInfo{.renderPass = **renderPass,
                                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                    .pAttachments = attachments.data(),
                                    .width = swapchainExtent.width,
                                    .height = swapchainExtent.height,
                                    .layers = 1};
      swapchainFramebuffers.emplace_back(device->createFramebuffer(framebufferCreateInfo));
   }
}

vk::raii::RenderPass RenderDevice::defaultRenderPass() const {
   const vk::AttachmentDescription colorAttachment{.format = swapchainImageFormat,
                                                   .samples = vk::SampleCountFlagBits::e1,
                                                   .loadOp = vk::AttachmentLoadOp::eClear,
                                                   .storeOp = vk::AttachmentStoreOp::eStore,
                                                   .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                                   .stencilStoreOp =
                                                       vk::AttachmentStoreOp::eDontCare,
                                                   .initialLayout = vk::ImageLayout::eUndefined,
                                                   .finalLayout = vk::ImageLayout::ePresentSrcKHR};

   constexpr auto colorAttachmentRef =
       vk::AttachmentReference{.attachment = 0, // index into the color attachments array
                               .layout = vk::ImageLayout::eColorAttachmentOptimal};

   const auto depthAttachment =
       vk::AttachmentDescription{.format = findDepthFormat(),
                                 .samples = vk::SampleCountFlagBits::e1,
                                 .loadOp = vk::AttachmentLoadOp::eClear,
                                 .storeOp = vk::AttachmentStoreOp::eDontCare,
                                 .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                 .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                 .initialLayout = vk::ImageLayout::eUndefined,
                                 .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

   constexpr auto depthAttachmentRef = vk::AttachmentReference{
       .attachment = 1, .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

   const auto attachments = std::array{colorAttachment, depthAttachment};

   const auto subpass =
       vk::SubpassDescription{.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                              .colorAttachmentCount = 1,
                              .pColorAttachments = &colorAttachmentRef,
                              .pDepthStencilAttachment = &depthAttachmentRef};

   const auto dependency =
       vk::SubpassDependency{.srcSubpass = VK_SUBPASS_EXTERNAL,
                             .dstSubpass = 0,
                             .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                             vk::PipelineStageFlagBits::eEarlyFragmentTests,
                             .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                             vk::PipelineStageFlagBits::eEarlyFragmentTests,
                             .srcAccessMask = vk::AccessFlagBits::eNone,
                             .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                                              vk::AccessFlagBits::eDepthStencilAttachmentWrite};

   const auto renderPassCreateInfo =
       vk::RenderPassCreateInfo{.attachmentCount = static_cast<uint32_t>(attachments.size()),
                                .pAttachments = attachments.data(),
                                .subpassCount = 1,
                                .pSubpasses = &subpass,
                                .dependencyCount = 1,
                                .pDependencies = &dependency};

   return device->createRenderPass(renderPassCreateInfo);
}

vk::Format RenderDevice::findDepthFormat() const {
   return findSupportedFormat(
       {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
       vk::ImageTiling::eOptimal,
       vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format RenderDevice::findSupportedFormat(const std::vector<vk::Format>& candidates,
                                             const vk::ImageTiling tiling,
                                             const vk::FormatFeatureFlags features) const {
   for (const auto format : candidates) {
      auto props = physicalDevice->getFormatProperties(format);
      if (tiling == vk::ImageTiling::eLinear &&
              (props.linearTilingFeatures & features) == features ||
          tiling == vk::ImageTiling::eOptimal &&
              (props.optimalTilingFeatures & features) == features) {
         return format;
      }
   }
   throw std::runtime_error("Failed to find supported format");
}

void RenderDevice::recreateSwapchain() {
}

void RenderDevice::drawFrame() {
   const auto& currentFrameData = frameData[currentFrame];

   if (const auto res =
           device->waitForFences(*currentFrameData->getInFlightFence(), VK_TRUE, UINT64_MAX);
       res != vk::Result::eSuccess) {
      throw std::runtime_error("Error waiting for fences");
   }

   const auto [result, imageIndex] = swapchain->acquireNextImage(
       UINT64_MAX, *currentFrameData->getImageAvailableSemaphore(), nullptr);

   if (result == vk::Result::eErrorOutOfDateKHR) {
      recreateSwapchain();
      return;
   }
   if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
      throw std::runtime_error("Failed to acquire swapchain image");
   }

   device->resetFences(*currentFrameData->getInFlightFence());

   currentFrameData->getCommandBuffer().reset();

   const auto cameraPosition = glm::vec3{0.f, 0.f, 5.f};
   const auto cameraTarget = glm::vec3{0.f, 0.f, 0.f};
   const auto cameraUp = glm::vec3{0.f, 1.f, 0.f};
   const auto view = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

   constexpr float fov = glm::radians(60.f);
   const float aspectRatio =
       static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height);
   constexpr float nearPlane = 0.1f;
   constexpr float farPlane = 1000.f;

   const auto projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);

   auto objectMatrices = ObjectMatrices{.model = glm::mat4{1.f}, .view = view, .proj = projection};

   currentFrameData->getObjectMatricesBuffer().updateBufferValue(&objectMatrices,
                                                                 sizeof(ObjectMatrices));

   recordCommandBuffer(currentFrameData->getCommandBuffer(), imageIndex);

   constexpr auto waitStages =
       std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

   const auto signalSemaphores =
       std::array<vk::Semaphore, 1>{*currentFrameData->getRenderFinishedSemaphore()};

   updateUniformBuffer(currentFrame);

   const auto submitInfo =
       vk::SubmitInfo{.waitSemaphoreCount = 1,
                      .pWaitSemaphores = &*currentFrameData->getImageAvailableSemaphore(),
                      .pWaitDstStageMask = waitStages.data(),
                      .commandBufferCount = 1,
                      .pCommandBuffers = &*currentFrameData->getCommandBuffer(),
                      .signalSemaphoreCount = 1,
                      .pSignalSemaphores = signalSemaphores.data()};

   graphicsQueue->submit(submitInfo, *currentFrameData->getInFlightFence());

   const auto presentInfo = vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                                               .pWaitSemaphores = signalSemaphores.data(),
                                               .swapchainCount = 1,
                                               .pSwapchains = &(*(*swapchain)),
                                               .pImageIndices = &imageIndex};

   if (const auto pResult = graphicsQueue->presentKHR(presentInfo);
       pResult == vk::Result::eErrorOutOfDateKHR || pResult == vk::Result::eSuboptimalKHR ||
       framebufferResized) {
      framebufferResized = false;
      recreateSwapchain();
   } else if (result != vk::Result::eSuccess) {
      throw std::runtime_error("Failed to present swapchain image");
   }

   currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
}

void RenderDevice::recordCommandBuffer(const vk::raii::CommandBuffer& cmd,
                                       const unsigned imageIndex) const {
   const std::array<vk::ClearValue, 2> clearValues{
       vk::ClearValue{.color =
                          vk::ClearColorValue{std::array<float, 4>({{0.39f, 0.58f, 0.93f, 1.f}})}},
       vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

   constexpr auto beginInfo = vk::CommandBufferBeginInfo{};
   cmd.begin(beginInfo);

   const auto renderArea = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent};

   const auto renderPassInfo =
       vk::RenderPassBeginInfo{.renderPass = *(*renderPass),
                               .framebuffer = *swapchainFramebuffers[imageIndex],
                               .renderArea = renderArea,
                               .clearValueCount = static_cast<uint32_t>(clearValues.size()),
                               .pClearValues = clearValues.data()};

   cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
   cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->getPipeline());

   for (const auto& mesh : meshes | std::views::values) {
      cmd.bindVertexBuffers(0, mesh->getVertexBuffer().getBuffer(), {0});
      cmd.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(), 0, vk::IndexType::eUint32);
      cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                             *pipeline->getPipelineLayout(),
                             0,
                             *frameData[currentFrame]->getDescriptorSet(),
                             nullptr);
      cmd.drawIndexed(mesh->getIndicesCount(), 1, 0, 0, 0);
   }

   frameData[currentFrame]->renderables.clear();

   cmd.endRenderPass();
   cmd.end();
}

void RenderDevice::updateUniformBuffer(const uint32_t currentFrame) const {
   static auto startTime = std::chrono::high_resolution_clock::now();
   const auto currentTime = std::chrono::high_resolution_clock::now();
   const float time =
       std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

   //      time = 0; // stop rotation

   const ObjectMatrices ubo{
       .model =
           glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
       .view = glm::lookAt(
           glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
       .proj = glm::perspective(glm::radians(60.0f),
                                static_cast<float>(swapchainExtent.width) /
                                    static_cast<float>(swapchainExtent.height),
                                0.1f,
                                100.0f)};
   // ubo.proj[1][1] *= -1;

   const auto dest = raiillocator->mapMemory(frameData[currentFrame]->getObjectMatricesBuffer());
   memcpy(dest, &ubo, sizeof(ubo));
   raiillocator->unmapMemory(frameData[currentFrame]->getObjectMatricesBuffer());
}

vk::PresentModeKHR RenderDevice::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) {
   for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
         return availablePresentMode;
      }
   }
   return vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR RenderDevice::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
   for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
          availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
         return availableFormat;
      }
   }
   return availableFormats[0];
}

vk::Extent2D RenderDevice::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                            const Instance& instance) const {
   if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
   } else {
      const auto& [width, height] = instance.getWindowSize();

      vk::Extent2D actualExtent = {width, height};

      actualExtent.width = std::clamp(
          actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height,
                                       capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height);

      return actualExtent;
   }
}

bool RenderDevice::isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                                    const Instance& instance) {
   const QueueFamilyIndices queueFamilyIndices =
       findQueueFamilies(possibleDevice, instance.getSurface());

   const bool extensionsSupported =
       checkDeviceExtensionSupport(possibleDevice, instance.getDesiredDeviceExtensions());

   bool swapchainAdequate = false;
   if (extensionsSupported) {
      auto [capabilities, formats, presentModes] =
          querySwapchainSupport(possibleDevice, instance.getSurface());
      swapchainAdequate = !formats.empty() && !presentModes.empty();
   }

   const auto features = possibleDevice.getFeatures();

   return queueFamilyIndices.isComplete() && extensionsSupported && swapchainAdequate &&
          features.samplerAnisotropy && features.tessellationShader;
}

RenderDevice::QueueFamilyIndices RenderDevice::findQueueFamilies(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
   QueueFamilyIndices queueFamilyIndices;

   const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

   for (int i = 0; const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
         queueFamilyIndices.graphicsFamily = i;
      }

      if (possibleDevice.getSurfaceSupportKHR(i, **surface)) {
         queueFamilyIndices.presentFamily = i;
      }

      if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
         queueFamilyIndices.transferFamily = i;
      }

      if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
         queueFamilyIndices.computeFamily = i;
      }

      if (queueFamilyIndices.isComplete()) {
         break;
      }
      i++;
   }
   return queueFamilyIndices;
}

bool RenderDevice::checkDeviceExtensionSupport(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::vector<const char*> desiredDeviceExtensions) {
   const auto availableExtensions = possibleDevice.enumerateDeviceExtensionProperties();

   for (const auto& ext : availableExtensions) {
      Log::core->debug("Physical Device Extension: {}", ext.extensionName);
   }

   std::set<std::string> requiredExtensions(desiredDeviceExtensions.begin(),
                                            desiredDeviceExtensions.end());

   for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
   }

   return requiredExtensions.empty();
}

RenderDevice::SwapchainSupportDetails RenderDevice::querySwapchainSupport(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
   SwapchainSupportDetails details;
   details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(**surface);
   details.formats = possibleDevice.getSurfaceFormatsKHR(**surface);
   details.presentModes = possibleDevice.getSurfacePresentModesKHR(**surface);
   return details;
}
