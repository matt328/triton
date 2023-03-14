#include "RenderDevice.h"

#include "DefaultPipeline.h"
#include "vma_raii.h"

#include "ImmediateContext.h"
#include "Log.h"
#include "ObjectMatrices.h"
#include "Texture.h"
#include "TextureFactory.h"
#include "Utils.h"

#include <GLFW/glfw3.h>
#include <ranges>
#include <set>

RenderDevice::RenderDevice(const Instance& instance) {
   createPhysicalDevice(instance);
   createLogicalDevice(instance);
   createSwapchain(instance);
   createSwapchainImageViews();
   createCommandPools(instance);
   createDescriptorPool();
   createAllocator(instance);

   renderPass = std::make_unique<vk::raii::RenderPass>(defaultRenderPass());
   pipeline = std::make_unique<DefaultPipeline>(*device, *renderPass, swapchainExtent);

   textureFactory = std::make_unique<TextureFactory>(
       *raiillocator, *device, *graphicsImmediateContext, *transferImmediateContext);

   const auto textureFilename = Paths::TEXTURES / "viking_room_2.ktx";

   textures["texture1"] = textureFactory->createTexture2D(textureFilename.string());

   createPerFrameData(pipeline->getDescriptorSetLayout());

   createDepthResources();
   createFramebuffers();
}

RenderDevice::~RenderDevice() {
}

void RenderDevice::waitIdle() const {
   device->waitIdle();
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

   const auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
       findQueueFamilies(*physicalDevice, surface);
   const uint32_t queueFamilyIndices[] = {graphicsFamily.value(), presentFamily.value()};

   if (graphicsFamily != presentFamily) {
      createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
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
   const auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
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
                                      textures["texture1"]->getDescriptorImageInfo()));
   }

   auto objectMatrices =
       ObjectMatrices{.model = glm::mat4(), .view = glm::mat4(), .proj = glm::mat4()};

   frameData[0]->getObjectMatricesBuffer().updateBufferValue(&objectMatrices,
                                                             sizeof(ObjectMatrices));
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

   constexpr auto subpass =
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
   const auto res =
       device->waitForFences(*currentFrameData->getInFlightFence(), VK_TRUE, UINT64_MAX);

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

   recordCommandBuffer(currentFrameData->getCommandBuffer(), imageIndex);

   constexpr vk::PipelineStageFlags waitStages[] = {
       vk::PipelineStageFlagBits::eColorAttachmentOutput};

   const vk::Semaphore signalSemaphores[] = {*currentFrameData->getRenderFinishedSemaphore()};

   // updateUniformBuffer(currentFrame);

   const auto submitInfo =
       vk::SubmitInfo{.waitSemaphoreCount = 1,
                      .pWaitSemaphores = &*currentFrameData->getImageAvailableSemaphore(),
                      .pWaitDstStageMask = waitStages,
                      .commandBufferCount = 1,
                      .pCommandBuffers = &*currentFrameData->getCommandBuffer(),
                      .signalSemaphoreCount = 1,
                      .pSignalSemaphores = signalSemaphores};

   graphicsQueue->submit(submitInfo, *currentFrameData->getInFlightFence());

   const auto presentInfo = vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                                               .pWaitSemaphores = signalSemaphores,
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

void RenderDevice::recordCommandBuffer(const vk::raii::CommandBuffer& cmd, unsigned imageIndex) {
   std::array<vk::ClearValue, 2> clearValues;
   std::array<float, 4> clearColor = {0.39f, 0.58f, 0.93f, 1.f};
   clearValues[0].color = vk::ClearColorValue(clearColor);
   clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

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

   // cmd.bindVertexBuffers(0, *vertexBuffer->buffer, {0});
   // cmd.bindIndexBuffer(*indexBuffer->buffer, 0, vk::IndexType::eUint32);
   // cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
   //                        *pipeline->getPipelineLayout(),
   //                         0,
   //                         *frameData[currentFrame]->getDescriptorSet(),
   //                         nullptr);
   // cmd.drawIndexed(static_cast<uint32_t>(this->model.indices.size()), 1, 0, 0, 0);
   cmd.endRenderPass();
   cmd.end();
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

      if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
          !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
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
