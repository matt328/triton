#include "Renderer.hpp"
#include "FrameData.hpp"
#include "GraphicsDevice.hpp"
#include "RenderObject.hpp"

#include "graphics/ObjectData.hpp"
#include "graphics/gui/ImguiHelper.hpp"
#include "helpers/Pipeline.hpp"
#include "helpers/Rendering.hpp"

#include "textures/Texture.hpp"
#include "textures/TextureFactory.hpp"

#include "geometry/MeshFactory.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_structs.hpp>

namespace Triton::Graphics {

   Renderer::Renderer(GLFWwindow* window) {
      glfwGetWindowSize(window, &width, &height);
      graphicsDevice = std::make_unique<GraphicsDevice>(window, true);

      init();

      imguiHelper = std::make_unique<Gui::ImGuiHelper>(*graphicsDevice, window);
   }

   Renderer::~Renderer() {
      Log::info << "destroying renderer" << std::endl;
      meshes.clear();
      textureList.clear();
   }

   void Renderer::init() {
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

      frameData.clear();
      frameData.reserve(FRAMES_IN_FLIGHT);
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

      const auto imageCreateInfo =
          vk::ImageCreateInfo{.imageType = vk::ImageType::e2D,
                              .format = depthFormat,
                              .extent = vk::Extent3D{graphicsDevice->DrawImageExtent2D.width,
                                                     graphicsDevice->DrawImageExtent2D.height,
                                                     1},
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

   void Renderer::recreateSwapchain() {
      waitIdle();
      frameData.clear();
      graphicsDevice->recreateSwapchain();
      init();
   }

   void Renderer::drawFrame() {
      ZoneNamedN(render, "Render", true);
      const auto& currentFrameData = frameData[currentFrame];

      // Wait for this frame's fence so we can be sure the gpu is finished with this frame's command
      // buffer.  Which it should be since it was submitted a frame or two ago.
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

      // Ask the swapchain to move to the next image. This call is async, and will signal the given
      // semaphore when it's completed.
      vk::Result result{};
      unsigned int imageIndex{};
      try {
         ZoneNamedN(acquire, "Acquire Swapchain Image", true);
         std::tie(result, imageIndex) = graphicsDevice->getSwapchain().acquireNextImage(
             UINT64_MAX,
             *currentFrameData->getImageAvailableSemaphore(),
             nullptr);
      } catch (const std::exception& ex) {
         Log::error << "Exception acquiring: " << ex.what() << std::endl;
         recreateSwapchain();
         return;
      }

      // Check for, and add any new textures into the bindless texture descriptor
      {
         // TODO: Move this into another thread eventually
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

      // Update the once-per-frame data.
      {
         ZoneNamedN(updateCameraData, "Update Camera Data", true);
         currentFrameData->getCameraBuffer().updateBufferValue(&cameraData, sizeof(CameraData));
      }

      // Reset this fence
      graphicsDevice->getVulkanDevice().resetFences(*currentFrameData->getInFlightFence());

      // Reset and record the current frame's command buffer.
      currentFrameData->getCommandBuffer().reset();
      {
         ZoneNamedN(cmdBuffer, "Recording CommandBuffer", true);
         recordCommandBuffer(*currentFrameData, imageIndex);
      }

      // Build out the struct and submit the command buffer, signaling the in flight fence when it
      // can be recorded to again
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

      graphicsDevice->getGraphicsQueue().submit(submitInfo, *currentFrameData->getInFlightFence());

      // Ask the GPU to present the image once the submit semaphore is signaled. Also trap errors
      // here and recreate (resize) the swapchain
      try {
         const auto result = graphicsDevice->getGraphicsQueue().presentKHR(
             vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                                .pWaitSemaphores = renderFinishedSemaphores.data(),
                                .swapchainCount = 1,
                                .pSwapchains = &(*graphicsDevice->getSwapchain()),
                                .pImageIndices = &imageIndex});

         if (result == vk::Result::eSuboptimalKHR) {
            recreateSwapchain();
         }
      } catch (const std::exception& ex) {
         Log::error << "Exception Presenting: " << ex.what() << std::endl;
         recreateSwapchain();
      }

      // Finally done, move to the next frame
      currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
   }

   void Renderer::recordCommandBuffer(FrameData& frameData, unsigned imageIndex) const {
      auto& cmd = frameData.getCommandBuffer();

      // Update the ObjectData buffer
      frameData.updateObjectDataBuffer(objectDataList.data(),
                                       sizeof(ObjectData) * objectDataList.size());
      cmd.begin(
          vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
      {
         auto ctx = frameData.getTracyContext();
         TracyVkZone(ctx, *cmd, "render room");

         // With dynamic rendering, have to manually insert image barriers into the command buffer
         // at certain points in time. This barrier cannot be passed until the swapchain image has
         // transitioned into the ColorAttachmentOptimal layout.
         Helpers::transitionImage(cmd,
                                  frameData.getDrawImage(),
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eColorAttachmentOptimal);

         // TODO: specify framedata.drawImageView as the color attachment here?
         const auto colorAttachmentInfo = vk::RenderingAttachmentInfo{
             .imageView = frameData.getDrawImageView(),
             .imageLayout = vk::ImageLayout::eAttachmentOptimal,
             .loadOp = vk::AttachmentLoadOp::eClear,
             .storeOp = vk::AttachmentStoreOp::eStore,
             .clearValue = vk::ClearValue{.color = vk::ClearColorValue{std::array<float, 4>(
                                              {{0.39f, 0.58f, 0.93f, 1.f}})}},
         };

         const auto depthAttachmentInfo = vk::RenderingAttachmentInfo{
             .imageView = **depthImageView,
             .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
             .loadOp = vk::AttachmentLoadOp::eClear,
             .storeOp = vk::AttachmentStoreOp::eStore,
             .clearValue = vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f,
                                                                                     .stencil = 0}},
         };

         const auto renderingInfo = vk::RenderingInfo{
             .renderArea =
                 vk::Rect2D{.offset = {0, 0}, .extent = graphicsDevice->DrawImageExtent2D},
             .layerCount = 1,
             .colorAttachmentCount = 1,
             .pColorAttachments = &colorAttachmentInfo,
             .pDepthAttachment = &depthAttachmentInfo};

         cmd.beginRendering(renderingInfo);
         {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
            // For each object, bind the vertex and index buffers, descriptor sets, and finally draw
            // the object.
            // TODO: move the vertices and indices into a single giant buffer.
            // Then switch this call from drawIndexed to draw*Indirect
            for (uint32_t i = 0; const auto& renderObject : renderObjects) {
               const auto& mesh = meshes.at(renderObject.meshId);

               cmd.bindVertexBuffers(0, mesh->getVertexBuffer().getBuffer(), {0});
               cmd.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(), 0, vk::IndexType::eUint32);

               cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                      **pipelineLayout,
                                      0,
                                      {*frameData.getBindlessDescriptorSet(),
                                       *frameData.getObjectDescriptorSet(),
                                       *frameData.getPerFrameDescriptorSet()},
                                      nullptr);
               // This is real greasy but it'll do for now
               // Change this to draw*Indirect
               cmd.drawIndexed(mesh->getIndicesCount(), 1, 0, 0, i);
               i++;
            }
         }

         cmd.endRendering();

         Helpers::transitionImage(cmd,
                                  frameData.getDrawImage(),
                                  vk::ImageLayout::eColorAttachmentOptimal,
                                  vk::ImageLayout::eTransferSrcOptimal);
         Helpers::transitionImage(cmd,
                                  graphicsDevice->getSwapchainImages()[imageIndex],
                                  vk::ImageLayout::eUndefined,
                                  vk::ImageLayout::eTransferDstOptimal);

         Helpers::copyImageToImage(cmd,
                                   frameData.getDrawImage(),
                                   graphicsDevice->getSwapchainImages()[imageIndex],
                                   graphicsDevice->DrawImageExtent2D,
                                   graphicsDevice->getSwapchainExtent());

         Helpers::transitionImage(cmd,
                                  graphicsDevice->getSwapchainImages()[imageIndex],
                                  vk::ImageLayout::eTransferDstOptimal,
                                  vk::ImageLayout::eColorAttachmentOptimal);

         drawImgui(cmd, graphicsDevice->getSwapchainImageViews()[imageIndex]);

         Helpers::transitionImage(cmd,
                                  graphicsDevice->getSwapchainImages()[imageIndex],
                                  vk::ImageLayout::eColorAttachmentOptimal,
                                  vk::ImageLayout::ePresentSrcKHR);

         TracyVkCollect(ctx, *cmd);
      }
      cmd.end();
   }

   void Renderer::drawImgui(const vk::raii::CommandBuffer& cmd,
                            const vk::raii::ImageView& imageView) const {
      const auto colorAttachment = vk::RenderingAttachmentInfo{
          .imageView = *imageView,
          .imageLayout = vk::ImageLayout::eAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eLoad,
          .storeOp = vk::AttachmentStoreOp::eStore,
      };

      const auto renderInfo = vk::RenderingInfo{
          .renderArea =
              vk::Rect2D{.offset = {0, 0}, .extent = graphicsDevice->getSwapchainExtent()},
          .layerCount = 1,
          .colorAttachmentCount = 1,
          .pColorAttachments = &colorAttachment,
      };

      cmd.beginRendering(renderInfo);

      ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);

      cmd.endRendering();
   }

   void Renderer::render() {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::ShowDemoWindow();

      ImGui::Render();

      drawFrame();
      renderObjects.clear();
      objectDataList.clear();
   }

   void Renderer::waitIdle() {
      graphicsDevice->getVulkanDevice().waitIdle();
   }

   void Renderer::windowResized(const int width, const int height) {
      this->width = width;
      this->height = height;
      TracyMessageL("WindowResized");
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
      return {width, height};
   }

   void Renderer::enqueueRenderObject(RenderObject renderObject) {
      objectDataList.emplace_back(renderObject.modelMatrix, renderObject.textureId);
      renderObjects.push_back(std::move(renderObject));
   }
}