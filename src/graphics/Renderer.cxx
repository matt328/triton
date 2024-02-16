#include "Renderer.hpp"
#include "FrameData.hpp"
#include "GraphicsDevice.hpp"
#include "RenderObject.hpp"

#include "graphics/ObjectData.hpp"
#include "helpers/Pipeline.hpp"

#include "textures/Texture.hpp"
#include "textures/TextureFactory.hpp"

#include "geometry/MeshFactory.hpp"

namespace Triton::Graphics {

   Renderer::Renderer(GLFWwindow* window) {
      graphicsDevice = std::make_unique<GraphicsDevice>(window, true);

      bindlessDescriptorSetLayout =
          Helpers::createBindlessDescriptorSetLayout(graphicsDevice->getVulkanDevice());
      objectDescriptorSetLayout =
          Helpers::createSSBODescriptorSetLayout(graphicsDevice->getVulkanDevice());
      perFrameDescriptorSetLayout =
          Helpers::createPerFrameDescriptorSetLayout(graphicsDevice->getVulkanDevice());

      std::tie(pipeline, pipelineLayout) =
          Helpers::createBasicPipeline(*graphicsDevice,
                                       *bindlessDescriptorSetLayout,
                                       *objectDescriptorSetLayout,
                                       *perFrameDescriptorSetLayout);

      for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
         auto name = std::stringstream{};
         name << "Frame " << i;
         frameData.push_back(std::make_unique<FrameData>(*graphicsDevice,
                                                         *bindlessDescriptorSetLayout,
                                                         *objectDescriptorSetLayout,
                                                         *perFrameDescriptorSetLayout,
                                                         name.str()));
      }

      const auto depthFormat = Helpers::findDepthFormat(graphicsDevice->getPhysicalDevice());

      auto swapchainExtent = graphicsDevice->getSwapchainExtent();

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

      depthImage =
          graphicsDevice->getAllocator().createImage(imageCreateInfo, allocationCreateInfo);

      constexpr auto range =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eDepth,
                                    .levelCount = 1,
                                    .layerCount = 1};

      const auto viewInfo = vk::ImageViewCreateInfo{.image = depthImage->getImage(),
                                                    .viewType = vk::ImageViewType::e2D,
                                                    .format = depthFormat,
                                                    .subresourceRange = range};
      depthImageView = std::make_unique<vk::raii::ImageView>(
          graphicsDevice->getVulkanDevice().createImageView(viewInfo));
   }

   Renderer::~Renderer() {
      Log::info << "destroying renderer" << std::endl;
      meshes.clear();
      textureList.clear();
   }

   void Renderer::recreateSwapchain() {
      waitIdle();
   }

   void Renderer::drawFrame() {
      ZoneNamedN(render, "Render", true);
      const auto& currentFrameData = frameData[currentFrame];

      /*
         We have multiple 'frames in flight'.  A frame is basically a command buffer, and all the
         resources used by the commands enqueued into it by the cpu each frame.
         Wait for the current frame's in flight fence
         This fence will be signaled when the GPU is done working off
         this frame's command buffer, so it's safe to reset it and start recording another
      */

      {
         ZoneNamedN(fences, "Awaiting Fences", true);
         if (const auto res = graphicsDevice->getVulkanDevice().waitForFences(
                 *currentFrameData->getInFlightFence(),
                 VK_TRUE,
                 UINT64_MAX);
             res != vk::Result::eSuccess) {
            throw std::runtime_error("Error waiting for fences");
         }
      }

      /*
         Tell the swapchain to grab the next image, signaling this semaphore when
         an image has been acquired
      */
      TracyCZoneN(acquire, "Acquire Image", true);
      const auto [result, imageIndex] = graphicsDevice->getSwapchain().acquireNextImage(
          UINT64_MAX,
          *currentFrameData->getImageAvailableSemaphore(),
          nullptr);

      if (result == vk::Result::eErrorOutOfDateKHR) {
         recreateSwapchain();
         return;
      }
      if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
         throw std::runtime_error("Failed to acquire swapchain image");
      }
      TracyCZoneEnd(acquire);
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
            graphicsDevice->getVulkanDevice().updateDescriptorSets(writes, nullptr);
            currentFrameData->getTexturesToBind().clear();
         }
      }

      {
         ZoneNamedN(updateCameraData, "Update Camera Data", true);
         currentFrameData->getCameraBuffer().updateBufferValue(&cameraData, sizeof(CameraData));
      }

      // We've already waited on this fence, so we can safely reset it so we can signal it again
      graphicsDevice->getVulkanDevice().resetFences(*currentFrameData->getInFlightFence());

      currentFrameData->getCommandBuffer().reset();

      {
         ZoneNamedN(cmdBuffer, "Recording CommandBuffer", true);
         recordCommandBuffer(*currentFrameData, imageIndex);
      }

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
      graphicsDevice->getGraphicsQueue().submit(submitInfo, *currentFrameData->getInFlightFence());

      const auto presentInfo =
          vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                             .pWaitSemaphores = renderFinishedSemaphores.data(),
                             .swapchainCount = 1,
                             .pSwapchains = &(*graphicsDevice->getSwapchain()),
                             .pImageIndices = &imageIndex};

      /*
         Since the submit call is async, the present call needs to wait on the render finished
         semaphore before actually presenting the new image to the screen so it can display it
         at the next vblank period.
      */
      if (const auto pResult = graphicsDevice->getGraphicsQueue().presentKHR(presentInfo);
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

      frameData.updateObjectDataBuffer(objectDataList.data(),
                                       sizeof(ObjectData) * objectDataList.size());

      cmd.begin(beginInfo);
      {
         auto ctx = frameData.getTracyContext();

         TracyVkZone(ctx, *cmd, "render room");

         // TODO: transition depth image as well
         const auto b2 = vk::ImageMemoryBarrier{
             .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
             .oldLayout = vk::ImageLayout::eUndefined,
             .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
             .image = graphicsDevice->getSwapchainImages()[imageIndex],
             .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                  .levelCount = 1,
                                  .layerCount = 1}};

         cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                             vk::PipelineStageFlagBits::eColorAttachmentOutput,
                             vk::DependencyFlagBits{}, // None
                             {},
                             {},
                             b2);

         const auto clearValues = std::array<vk::ClearValue, 2>{
             vk::ClearValue{
                 .color = vk::ClearColorValue{std::array<float, 4>({{0.39f, 0.58f, 0.93f, 1.f}})}},
             vk::ClearValue{.depthStencil =
                                vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

         const auto renderArea =
             vk::Rect2D{.offset = {0, 0}, .extent = graphicsDevice->getSwapchainExtent()};

         const auto colorAttachmentInfo = vk::RenderingAttachmentInfo{
             .imageView = *graphicsDevice->getSwapchainImageViews()[imageIndex],
             .imageLayout = vk::ImageLayout::eAttachmentOptimal,
             .loadOp = vk::AttachmentLoadOp::eClear,
             .storeOp = vk::AttachmentStoreOp::eStore,
             .clearValue = clearValues[0],
         };

         const auto depthAttachmentInfo = vk::RenderingAttachmentInfo{
             .imageView = **depthImageView,
             .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
             .loadOp = vk::AttachmentLoadOp::eClear,
             .storeOp = vk::AttachmentStoreOp::eStore,
             .clearValue = clearValues[1],
         };

         const auto renderingInfo = vk::RenderingInfo{.renderArea = renderArea,
                                                      .layerCount = 1,
                                                      .colorAttachmentCount = 1,
                                                      .pColorAttachments = &colorAttachmentInfo,
                                                      .pDepthAttachment = &depthAttachmentInfo};

         cmd.beginRendering(renderingInfo);
         {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);

            for (uint32_t i = 0; const auto& renderObject : renderObjects) {
               const auto& mesh = meshes.at(renderObject.meshId);

               cmd.bindVertexBuffers(0, mesh->getVertexBuffer().getBuffer(), {0});
               cmd.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(), 0, vk::IndexType::eUint32);

               const auto set1 = *frameData.getBindlessDescriptorSet();
               const auto set2 = *frameData.getObjectDescriptorSet();
               const auto set3 = *frameData.getPerFrameDescriptorSet();
               const auto allSets = std::array{set1, set2, set3};
               cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                      **pipelineLayout,
                                      0,
                                      allSets,
                                      nullptr);
               // This is real greasy but it'll do for now
               cmd.drawIndexed(mesh->getIndicesCount(), 1, 0, 0, i);
               i++;
            }
         }

         cmd.endRendering();

         const auto b = vk::ImageMemoryBarrier{
             .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
             .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
             .newLayout = vk::ImageLayout::ePresentSrcKHR,
             .image = graphicsDevice->getSwapchainImages()[imageIndex],
             .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                                  .levelCount = 1,
                                  .layerCount = 1}};

         cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                             vk::PipelineStageFlagBits::eBottomOfPipe,
                             vk::DependencyFlagBits{}, // None
                             {},
                             {},
                             b);
         TracyVkCollect(ctx, *cmd);
      }
      cmd.end();
   }

   void Renderer::render() {
      drawFrame();
      renderObjects.clear();
      objectDataList.clear();
   }

   void Renderer::waitIdle() {
      graphicsDevice->getVulkanDevice().waitIdle();
   }

   void Renderer::windowResized(const int width, const int height) {
      graphicsDevice->resizeWindow(width, height);
   }

   // TODO: Should the renderer be what creates these?
   // I guess it delegates to the factory, which is created and owned by the device
   // and the renderer is just indexing the Mesh so it knows how to access it consistently
   // Change this around so that the meshes are indexed by their index into an ObjectData
   // Buffer so we can leverage bindless design.
   MeshHandle Renderer::createMesh(const std::string_view& filename) {
      auto handle = meshes.size();
      meshes.push_back(graphicsDevice->getMeshFactory().loadMeshFromGltf(filename.data()));
      return handle;
   }

   uint32_t Renderer::createTexture(const std::string_view& filename) {
      auto handle = textureList.size();
      textureList.push_back(graphicsDevice->getTextureFactory().createTexture2D(filename));
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

   void Renderer::enqueueRenderObject(RenderObject renderObject) {
      objectDataList.emplace_back(renderObject.modelMatrix, renderObject.textureId);
      renderObjects.push_back(std::move(renderObject));
   }
}