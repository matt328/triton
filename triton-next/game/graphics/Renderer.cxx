#include "Renderer.hpp"
#include "GraphicsDevice.hpp"
#include "GraphicsHelpers.hpp"

namespace Triton::Game::Graphics {

   Renderer::Renderer(GLFWwindow* window) {
      graphicsDevice = std::make_unique<GraphicsDevice>(window, true);

      // All the Above is boilerplate and should move into the GraphicsDevice class
      // If anything needs something inside GraphicsDevice, just pass a const
      // std::unique_ptr<GraphicsDevice>& and move on with your life

      const auto renderPassCreateInfo =
          Utils::RenderPassCreateInfo{.device = device.get(),
                                      .physicalDevice = physicalDevice.get(),
                                      .swapchainFormat = swapchainImageFormat,
                                      .clearColor = false,
                                      .clearDepth = false};

      renderPass = std::make_unique<vk::raii::RenderPass>(
          Utils::colorAndDepthRenderPass(renderPassCreateInfo));

      pipeline = std::make_unique<DefaultPipeline>(*device, *renderPass, swapchainExtent);

      textureFactory = std::make_unique<TextureFactory>(*raiillocator,
                                                        *device,
                                                        *graphicsImmediateContext,
                                                        *transferImmediateContext);

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
      const auto [result, imageIndex] =
          swapchain->acquireNextImage(UINT64_MAX,
                                      *currentFrameData->getImageAvailableSemaphore(),
                                      nullptr);

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
         if (perFrameDataProvider != nullptr) {
            const auto [view, proj, viewProj] = perFrameDataProvider();
            const auto cameraData = CameraData{.view = view, .proj = proj, .viewProj = viewProj};
            currentFrameData->getCameraBuffer().updateBufferValue(&cameraData, sizeof(CameraData));
         }
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
      if (this->renderObjectProvider != nullptr) {
         const auto renderObjects = this->renderObjectProvider();
         for (const auto& renderObject : renderObjects) {
            objectDataList.push_back(
                ObjectData{.model = renderObject.modelMatrix,
                           .textureId = static_cast<TextureHandle>(renderObject.textureId)});
         }
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

         if (this->renderObjectProvider != nullptr) {
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
      device->resizeWindow(height, width);
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