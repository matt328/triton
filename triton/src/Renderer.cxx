#include "Renderer.hpp"

#include "ResourceFactory.hpp"
#include "TransferData.hpp"
#include "graphics/Instance.hpp"
#include "graphics/FrameData.hpp"
#include "graphics/ImmediateContext.hpp"
#include "graphics/pipeline/DefaultPipeline.hpp"
#include "graphics/renderer/Clear.hpp"
#include "graphics/renderer/Finish.hpp"
#include "graphics/texture/TextureFactory.hpp"
#include "graphics/texture/Texture.hpp"
#include "graphics/geometry/MeshFactory.hpp"
#include "graphics/renderer/RendererBase.hpp"
#include "graphics/VulkanFactory.hpp"
#include "graphics/GraphicsHelpers.hpp"
#include "graphics/DebugHelpers.hpp"
#include "RenderObject.hpp"
#include "Logger.hpp"
#include "RenderObject.hpp"

namespace Triton {

   Renderer::Renderer(GLFWwindow* window) {
      instance = std::make_unique<Instance>(window, true);
      createPhysicalDevice(*instance);
      createLogicalDevice(*instance);
      createSwapchain(*instance);
      createSwapchainImageViews();
      createCommandPools(*instance);
      createDescriptorPool();
      createAllocator(*instance);

      const auto renderPassCreateInfo =
          Utils::RenderPassCreateInfo{.device = device.get(),
                                      .physicalDevice = physicalDevice.get(),
                                      .swapchainFormat = swapchainImageFormat,
                                      .clearColor = false,
                                      .clearDepth = false};

      renderPass = std::make_unique<vk::raii::RenderPass>(
          Utils::colorAndDepthRenderPass(renderPassCreateInfo));

      pipeline = std::make_unique<DefaultPipeline>(*device, *renderPass, swapchainExtent);

      textureFactory = std::make_unique<TextureFactory>(
          *raiillocator, *device, *graphicsImmediateContext, *transferImmediateContext);

      meshFactory =
          std::make_unique<MeshFactory>(raiillocator.get(), transferImmediateContext.get());

      createPerFrameData(pipeline->getBindlessDescriptorSetLayout(),
                         pipeline->getObjectDescriptorSetLayout(),
                         pipeline->getPerFrameDescriptorSetLayout());

      createDepthResources();
      createFramebuffers();

      const auto rendererCreateInfo =
          RendererBaseCreateInfo{.device = *device,
                                 .physicalDevice = *physicalDevice,
                                 .allocator = *raiillocator,
                                 .depthTexture = this->depthImage->getImage(),
                                 .swapchainExtent = swapchainExtent,
                                 .swapchainImages = swapchainImages,
                                 .swapchainImageViews = swapchainImageViews,
                                 .depthImageView = *depthImageView,
                                 .swapchainFormat = swapchainImageFormat};

      renderers.emplace_back(std::make_unique<Clear>(rendererCreateInfo));
      finishRenderer = std::make_unique<Finish>(rendererCreateInfo);
   }

   Renderer::~Renderer() = default;

   // Helpers
   void Renderer::createPhysicalDevice(const Instance& instance) {
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

      Log::info << "Using physical device: " << physicalDevice->getProperties().deviceName.data()
                << std::endl;
   }

   void Renderer::createLogicalDevice(const Instance& instance) {
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

      auto desiredDeviceExtensions = instance.getDesiredDeviceExtensions();
      if (instance.isValidationEnabled()) {
         desiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }

      auto features2 =
          physicalDevice->getFeatures2<vk::PhysicalDeviceFeatures2,
                                       vk::PhysicalDevice16BitStorageFeatures,
                                       vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

      auto indexingFeatures = features2.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

      auto drawParamsFeatures = vk::PhysicalDeviceShaderDrawParametersFeatures{
          .pNext = &indexingFeatures, .shaderDrawParameters = VK_TRUE};

      const auto bindlessTexturesSupported = indexingFeatures.descriptorBindingPartiallyBound &&
                                             indexingFeatures.runtimeDescriptorArray;

      if (!bindlessTexturesSupported) {
         throw std::runtime_error("GPU does not support bindless textures :(");
      }

      auto physicalFeatures2 = physicalDevice->getFeatures2();
      physicalFeatures2.features.samplerAnisotropy = VK_TRUE;
      physicalFeatures2.pNext = &drawParamsFeatures;

      vk::DeviceCreateInfo createInfo{
          .pNext = &physicalFeatures2,
          .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
          .pQueueCreateInfos = queueCreateInfos.data(),
          .enabledLayerCount = 0,
          .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
          .ppEnabledExtensionNames = desiredDeviceExtensions.data()};

      const auto validationLayers = instance.getDesiredValidationLayers();
      if (instance.isValidationEnabled()) {
         createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
         createInfo.ppEnabledLayerNames = validationLayers.data();
      }

      device =
          std::make_unique<vk::raii::Device>(physicalDevice->createDevice(createInfo, nullptr));

      setObjectName(
          **device, *device.get(), vk::raii::Device::debugReportObjectType, "Primary Device");

      Log::trace << "Created Logical Device" << std::endl;

      graphicsQueue =
          std::make_unique<vk::raii::Queue>(device->getQueue(graphicsFamily.value(), 0));
      setObjectName(**graphicsQueue,
                    *device.get(),
                    (**graphicsQueue).debugReportObjectType,
                    "Graphics Queue");
      Log::trace << "Created Graphics Queue" << std::endl;

      presentQueue = std::make_unique<vk::raii::Queue>(device->getQueue(presentFamily.value(), 0));
      setObjectName(
          **presentQueue, *device.get(), (**presentQueue).debugReportObjectType, "Present Queue");
      Log::trace << "Created Present Queue" << std::endl;

      transferQueue =
          std::make_shared<vk::raii::Queue>(device->getQueue(transferFamily.value(), 0));
      setObjectName(**transferQueue,
                    *device.get(),
                    (**transferQueue).debugReportObjectType,
                    "Transfer Queue");
      Log::trace << "Created Transfer Queue" << std::endl;

      computeQueue = std::make_unique<vk::raii::Queue>(device->getQueue(computeFamily.value(), 0));
      setObjectName(
          **computeQueue, *device.get(), (**computeQueue).debugReportObjectType, "Compute Queue");
      Log::trace << "Created Compute Queue" << std::endl;
   }

   void Renderer::createSwapchain(const Instance& instance) {
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
                                            .compositeAlpha =
                                                vk::CompositeAlphaFlagBitsKHR::eOpaque,
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
      Log::info << "Created Swapchain" << std::endl;

      swapchainExtent = extent;
      swapchainImageFormat = surfaceFormat.format;
   }

   void Renderer::createSwapchainImageViews() {
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
      Log::info << "Created " << swapchainImageViews.size() << " swapchain image views"
                << std::endl;
   }

   void Renderer::createCommandPools(const Instance& instance) {
      auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
          findQueueFamilies(*physicalDevice, instance.getSurface());
      auto commandPoolCreateInfo =
          vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                    .queueFamilyIndex = graphicsFamily.value()};

      commandPool =
          std::make_unique<vk::raii::CommandPool>(device->createCommandPool(commandPoolCreateInfo));
      setObjectName(**commandPool,
                    *device.get(),
                    (*commandPool).debugReportObjectType,
                    "Graphics Command Pool");

      transferImmediateContext = std::make_unique<ImmediateContext>(*device.get(),
                                                                    *physicalDevice,
                                                                    *transferQueue,
                                                                    transferFamily.value(),
                                                                    "Transfer Immediate Context");

      graphicsImmediateContext = std::make_unique<ImmediateContext>(*device.get(),
                                                                    *physicalDevice,
                                                                    *graphicsQueue,
                                                                    graphicsFamily.value(),
                                                                    "Graphics Immediate Context");
   }

   void Renderer::createDescriptorPool() {
      const auto poolSize = std::array{
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 10},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 100},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 10},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 10},
      };

      const vk::DescriptorPoolCreateInfo poolInfo{
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                   vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT,
          .maxSets = FRAMES_IN_FLIGHT * 10 * poolSize.size(),
          .poolSizeCount = poolSize.size(),
          .pPoolSizes = poolSize.data()};

      descriptorPool = std::make_unique<vk::raii::DescriptorPool>(
          device->createDescriptorPool(poolInfo, nullptr));
   }

   void Renderer::createAllocator(const Instance& instance) {
      const auto allocatorCreateInfo =
          vma::AllocatorCreateInfo{.physicalDevice = **physicalDevice,
                                   .device = **device,
                                   .instance = **instance.getVkInstance()};

      raiillocator = std::make_unique<Triton::Memory::Allocator>(allocatorCreateInfo);
   }

   void Renderer::createPerFrameData(
       const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
       const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
       const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout) {
      for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
         auto name = std::stringstream{};
         name << "Frame " << i;
         frameData.push_back(std::make_unique<FrameData>(*device,
                                                         *physicalDevice,
                                                         *commandPool,
                                                         *raiillocator,
                                                         *descriptorPool,
                                                         bindlessDescriptorSetLayout,
                                                         objectDescriptorSetLayout,
                                                         perFrameDescriptorSetLayout,
                                                         *graphicsQueue,
                                                         name.str()));
      }
   }

   void Renderer::createDepthResources() {
      const auto depthFormat = Utils::findDepthFormat(*physicalDevice);

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

      constexpr auto range =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eDepth,
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

   void Renderer::createFramebuffers() {
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

   void Renderer::recreateSwapchain() {
   }

   void Renderer::drawFrame() {
      const auto& currentFrameData = frameData[currentFrame];

      /*
         We have multiple 'frames in flight'.  A frame is basically a command buffer, and all the
         resources used by the commands enqueued into it by the cpu each frame.
         Wait for the current frame's in flight fence
         This fence will be signaled when the GPU is done working off
         this frame's command buffer, so it's safe to reset it and start recording another
      */
      if (const auto res =
              device->waitForFences(*currentFrameData->getInFlightFence(), VK_TRUE, UINT64_MAX);
          res != vk::Result::eSuccess) {
         throw std::runtime_error("Error waiting for fences");
      }

      /*
         Tell the swapchain to grab the next image, signaling this semaphore when
         an image has been acquired
      */
      const auto [result, imageIndex] = swapchain->acquireNextImage(
          UINT64_MAX, *currentFrameData->getImageAvailableSemaphore(), nullptr);

      if (result == vk::Result::eErrorOutOfDateKHR) {
         recreateSwapchain();
         return;
      }
      if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
         throw std::runtime_error("Failed to acquire swapchain image");
      }

      {
         ZoneNamedN(updateTextures, "Update Textures", true);
         if (!currentFrameData->getTexturesToBind().empty()) {
            auto writes = std::vector<vk::WriteDescriptorSet>{};
            writes.reserve(currentFrameData->getTexturesToBind().size());
            for (const auto t : currentFrameData->getTexturesToBind()) {
               const auto& texture = textureList[t];
               writes.push_back(vk::WriteDescriptorSet{
                   .dstSet = *currentFrameData->getBindlessDescriptorSet(),
                   .dstBinding = 3,
                   .dstArrayElement = t,
                   .descriptorCount = 1,
                   .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                   .pImageInfo = texture->getImageInfo()});
            }
            device->updateDescriptorSets(writes, nullptr);
            currentFrameData->getTexturesToBind().clear();
         }
      }

      {
         ZoneNamedN(updateCameraData, "Update Camera Data", true);
         // This is awesome feature of C++ idc
         const auto [view, proj, viewProj] = perFrameDataProvider();
         const auto cameraData = CameraData{.view = view, .proj = proj, .viewProj = viewProj};
         currentFrameData->getCameraBuffer().updateBufferValue(&cameraData, sizeof(CameraData));
      }

      // We've already waited on this fence, so we can safely reset it so we can signal it again
      device->resetFences(*currentFrameData->getInFlightFence());

      currentFrameData->getCommandBuffer().reset();

      recordCommandBuffer(*currentFrameData, imageIndex);

      constexpr auto waitStages =
          std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

      const auto renderFinishedSemaphores =
          std::array<vk::Semaphore, 1>{*currentFrameData->getRenderFinishedSemaphore()};

      const auto submitInfo =
          vk::SubmitInfo{.waitSemaphoreCount = 1,
                         .pWaitSemaphores = &*currentFrameData->getImageAvailableSemaphore(),
                         .pWaitDstStageMask = waitStages.data(),
                         .commandBufferCount = 1,
                         .pCommandBuffers = &*currentFrameData->getCommandBuffer(),
                         .signalSemaphoreCount = 1,
                         .pSignalSemaphores = renderFinishedSemaphores.data()};
      /*
         Submit this command buffer, waiting for the acquire image semaphore, and also signaling
         the renderFinished semaphore once it's done. Note rendering here means working off the
         command buffer and drawing to the framebuffer image.  Presenting the framebuffer to the
         screen is another process...
      */
      graphicsQueue->submit(submitInfo, *currentFrameData->getInFlightFence());

      const auto presentInfo =
          vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                             .pWaitSemaphores = renderFinishedSemaphores.data(),
                             .swapchainCount = 1,
                             .pSwapchains = &(*(*swapchain)),
                             .pImageIndices = &imageIndex};

      /*
         Since the submit call is async, the present call needs to wait on the render finished
         semaphore before actually presenting the new image to the screen so it can display it
         at the next vblank period.
      */
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

   void Renderer::recordCommandBuffer(FrameData& frameData, unsigned imageIndex) const {
      constexpr auto beginInfo =
          vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse};
      auto& cmd = frameData.getCommandBuffer();

      // TODO: think more about when and where this should happen from a multi threaded
      // perspective we're looping through objects once here and again down below to bind their
      // index and vertex buffers.  When I finish implementing a fully bindless pipeline that
      // won't matter since the vertices and indices will be accumulated in a giant buffer, and I
      // think all the data handed off between the rendersystem and renderdevice will be copyable
      auto objectDataList = std::vector<ObjectData>{};
      const auto renderObjects = this->renderObjectProvider();
      for (const auto& renderObject : renderObjects) {
         objectDataList.push_back(
             ObjectData{.model = renderObject.modelMatrix,
                        .textureId = static_cast<TextureHandle>(renderObject.textureId)});
      }

      // Profile this and see if it's worth checking if something actually changed or not
      frameData.updateObjectDataBuffer(objectDataList.data(),
                                       sizeof(ObjectData) * objectDataList.size());

      cmd.begin(beginInfo);
      {
         auto ctx = frameData.getTracyContext();

         TracyVkZone(ctx, *cmd, "render room");

         for (const auto& renderer : renderers) {
            renderer->update();
            renderer->fillCommandBuffer(cmd, imageIndex);
         }

         const auto renderArea = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent};

         const auto renderPassInfo =
             vk::RenderPassBeginInfo{.renderPass = *(*renderPass),
                                     .framebuffer = *swapchainFramebuffers[imageIndex],
                                     .renderArea = renderArea};

         cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
         cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->getPipeline());

         const auto& renderObjects = this->renderObjectProvider();

         for (uint32_t i = 0; const auto& renderObject : renderObjects) {
            const auto& mesh = meshes.at(renderObject.meshId);

            cmd.bindVertexBuffers(0, mesh->getVertexBuffer().getBuffer(), {0});
            cmd.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(), 0, vk::IndexType::eUint32);

            const auto set1 = *frameData.getBindlessDescriptorSet();
            const auto set2 = *frameData.getObjectDescriptorSet();
            const auto set3 = *frameData.getPerFrameDescriptorSet();
            const auto allSets = std::array{set1, set2, set3};
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   *pipeline->getPipelineLayout(),
                                   0,
                                   allSets,
                                   nullptr);
            // This is real greasy but it'll do for now
            cmd.drawIndexed(mesh->getIndicesCount(), 1, 0, 0, i);
            i++;
         }

         cmd.endRenderPass();

         finishRenderer->update();
         finishRenderer->fillCommandBuffer(cmd, imageIndex);

         TracyVkCollect(ctx, *cmd);
      }

      cmd.end();
   }

   void Renderer::render() {
      drawFrame();
   }

   void Renderer::waitIdle() {
      device->waitIdle();
   }

   void Renderer::windowResized(const int height, const int width) {
      instance->resizeWindow(height, width);
   }

   std::string Renderer::createMesh(const std::string_view& filename) {
      meshes[filename.data()] = meshFactory->loadMeshFromGltf(filename.data());
      return filename.data();
   }

   uint32_t Renderer::createTexture(const std::string_view& filename) {
      auto handle = textureList.size();
      textureList.push_back(textureFactory->createTexture2D(filename));
      // I think we need to bind the texture once in each framedata
      for (auto& f : frameData) {
         f->getTexturesToBind().push_back(handle);
      }
      Log::debug << "added texture to bind with index" << handle << std::endl;
      return handle;
   }

   const std::tuple<int, int> Renderer::getWindowSize() const {
      return {1, 1};
   }
}