#include "gfx/RenderContext.hpp"
#include "Frame.hpp"
#include "GraphicsDevice.hpp"
#include "gfx/Handles.hpp"
#include "gfx/RenderObject.hpp"

#include "gfx/ObjectData.hpp"
#include "gfx/ds/DescriptorSetFactory.hpp"
#include "gfx/ds/LayoutFactory.hpp"
#include "gfx/gui/ImguiHelper.hpp"

#include "helpers/Vulkan.hpp"
#include "util/Paths.hpp"
#include "gfx/textures/ResourceManager.hpp"
#include "gfx/VkContext.hpp"
#include "gfx/Pipeline.hpp"
#include "gfx/ds/LayoutFactory.hpp"
#include "gfx/ds/DescriptorSetFactory.hpp"
#include "gfx/ds/DescriptorSet.hpp"

namespace tr::gfx {

   RenderContext::RenderContext(GLFWwindow* window, bool guiEnabled) : guiEnabled{guiEnabled} {
      graphicsDevice = std::make_unique<GraphicsDevice>(window, true);

      layoutFactory = std::make_unique<ds::LayoutFactory>(graphicsDevice->getVulkanDevice());

      dsFactory = std::make_unique<ds::DescriptorSetFactory>(graphicsDevice->getVulkanDevice(),
                                                             *layoutFactory,
                                                             FRAMES_IN_FLIGHT);

      // Create Static Model Pipeline Object
      {
         // How much of this will be the same for all this engine's pipelines?
         // Probably alot of it, and the RenderContext's job is to know about all the different
         // pipelines and descriptors and vertex formats
         /*
            Make all this as expressive as possible. The pipeline should take in only a list of
            Layouts for now.
         */
         const auto setLayouts =
             std::array{layoutFactory->getVkLayout(ds::LayoutHandle::Bindless),
                        layoutFactory->getVkLayout(ds::LayoutHandle::ObjectData),
                        layoutFactory->getVkLayout(ds::LayoutHandle::PerFrame)};

         vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{.setLayoutCount = setLayouts.size(),
                                                               .pSetLayouts = setLayouts.data()};

         // Configure Vertex Attributes
         const auto bindingDescription = Geometry::Vertex::inputBindingDescription(0);
         const auto attributeDescriptions =
             Geometry::Vertex::inputAttributeDescriptions(0,
                                                          {Geometry::VertexComponent::Position,
                                                           Geometry::VertexComponent::Color,
                                                           Geometry::VertexComponent::UV});

         const auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{
             .vertexBindingDescriptionCount = 1,
             .pVertexBindingDescriptions = &bindingDescription,
             .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
             .pVertexAttributeDescriptions = attributeDescriptions.data()};

         const auto colorFormat = vk::Format::eR16G16B16A16Sfloat;
         const auto depthFormat = Helpers::findDepthFormat(graphicsDevice->getPhysicalDevice());

         const auto renderingCreateInfo =
             vk::PipelineRenderingCreateInfo{.colorAttachmentCount = 1,
                                             .pColorAttachmentFormats = &colorFormat,
                                             .depthAttachmentFormat = depthFormat};

         staticModelPipeline = std::make_unique<Pipeline>(*graphicsDevice,
                                                          pipelineLayoutCreateInfo,
                                                          vertexInputStateCreateInfo,
                                                          renderingCreateInfo,
                                                          util::Paths::SHADERS / "shader.vert",
                                                          util::Paths::SHADERS / "shader.frag");
      }

      initDepthResources();

      frames.reserve(FRAMES_IN_FLIGHT);
      for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
         auto name = std::stringstream{};
         name << "Frame " << i;
         frames.push_back(
             std::make_unique<Frame>(*graphicsDevice, depthImageView, *layoutFactory, name.str()));
      }

      if (guiEnabled) {
         imguiHelper = std::make_unique<Gui::ImGuiHelper>(*graphicsDevice, window);
      }

      resourceManager = std::make_unique<tx::ResourceManager>(*graphicsDevice);
   }

   RenderContext::~RenderContext() {
      Log::info << "destroying renderer" << std::endl;
   }

   void RenderContext::initDepthResources() {

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
      depthImageView = std::make_shared<vk::raii::ImageView>(
          graphicsDevice->getVulkanDevice().createImageView(viewInfo));
   }

   void RenderContext::recreateSwapchain() {
      waitIdle();
      for (const auto& fd : frames) {
         fd->destroySwapchainResources();
      }

      graphicsDevice->recreateSwapchain();

      const auto size = graphicsDevice->DrawImageExtent2D;

      if (size.width == 0 || size.height == 0) {
         return;
      }

      staticModelPipeline->resize({size.width, size.height});

      resizeDelegate(graphicsDevice->getCurrentSize());

      for (const auto& fd : frames) {
         fd->createSwapchainResources(*graphicsDevice);
      }
   }

   void RenderContext::drawFrame() {
      ZoneNamedN(render, "Render", true);
      const auto& currentFrameData = frames[currentFrame];
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
         Log::error << "Swapchain needs resized: " << ex.what() << std::endl;
         recreateSwapchain();
         return;
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
         Log::info << "swapchain needs recreated: " << ex.what() << std::endl;
         recreateSwapchain();
      }

      // Finally done, move to the next frame
      currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
   }

   void RenderContext::recordCommandBuffer(Frame& frame, unsigned imageIndex) {
      auto& cmd = frame.getCommandBuffer();

      /*
         These buffers need to be updated at this point in time since we know they're not being used
         by the current frame yet.
         The sources of their data however should be a persistent list that asynchronous updates to
         the game world maintain. These updates should be synced so that each step of the game world
         produces a consistent data set

         If we multithread the game world's systems, they'll all still have to join at a barrier to
         update the state together, so that one tick keeps to one execution of each thread and we
         don't run a fast system 1000 times while a slow system has only run once.

         Each one of these ticks will correspond to an execution of the fixedUpdate() function in
         the timer class.

         Might not implement multithreaded game world just yet, but keep in mind the game world's
         systems should be designed as tasks that run and produce a result. This will make
         orchestration of the thread pool easier since each tick can notify all the threads to
         start, and barrier at them all finishing their task and returning the data. Then, lock the
         data structure, write new data into it, unlock. This will happen as many ticks as it can in
         one frame.  Meanwhile in the render thread, also lock on reading the data, write it into
         the current frame's buffers, then release the lock.
      */
      meshHandlesBuffer.clear();
      resourceManager->accessRenderData([&frame, this](RenderData& renderData) {
         frame.updateObjectDataBuffer(renderData.objectData.data(),
                                      sizeof(ObjectData) * renderData.objectData.size());
         frame.getCameraBuffer().updateBufferValue(&renderData.cameraData, sizeof(CameraData));
         meshHandlesBuffer.reserve(renderData.meshHandles.size());
         std::copy(renderData.meshHandles.begin(),
                   renderData.meshHandles.end(),
                   std::back_inserter(meshHandlesBuffer));
      });

      auto& objectDataSet = dsFactory->getDescriptorSet(ds::SetHandle::ObjectData, currentFrame);
      auto& perFrameSet = dsFactory->getDescriptorSet(ds::SetHandle::PerFrame, currentFrame);
      auto& textureSet = dsFactory->getDescriptorSet(ds::SetHandle::Bindless, currentFrame);

      /* Instead of doing this every frame, maybe be able to create DescriptorSets with buffers
       permanently attached, associated with them at creation time, and the writes updated once on
       init.
       Eventually the Vertex and Index data will go into buffers/descriptors similar to the
       textures, and potentially need to be updated once per frame.

       The PerFrame data won't ever be as dynamic, and can stay in a uniform buffer, ie. compute
       pipeline won't be writing to it like the object buffer (currently a storage buffer) will be
      */

      objectDataSet.writeBuffer(frame.getObjectDataBuffer(), sizeof(ObjectData) * 128);
      perFrameSet.writeBuffer(frame.getCameraBuffer(), sizeof(CameraData));

      resourceManager->accessTextures(
          [&textureSet, this](const std::vector<vk::DescriptorImageInfo>& imageInfoList) {
             ZoneNamedN(a, "Updating Texture DS", true);
             const auto write = vk::WriteDescriptorSet{
                 .dstSet = textureSet.getVkDescriptorSet(),
                 .dstBinding = 3,
                 .dstArrayElement = 0,
                 .descriptorCount = static_cast<uint32_t>(imageInfoList.size()),
                 .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                 .pImageInfo = imageInfoList.data()};
             graphicsDevice->getVulkanDevice().updateDescriptorSets(write, nullptr);
          });

      cmd.begin(
          vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
      {
         auto ds = std::array<vk::DescriptorSet, 3>{textureSet.getVkDescriptorSet(),
                                                    objectDataSet.getVkDescriptorSet(),
                                                    perFrameSet.getVkDescriptorSet()};

         cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                staticModelPipeline->getPipelineLayout(),
                                0,
                                ds,
                                nullptr);

         frame.prepareFrame();
         {
            staticModelPipeline->bind(cmd);
            {
               uint32_t i = 0;
               for (const auto& meshHandle : meshHandlesBuffer) {
                  const auto& mesh = resourceManager->getMesh(meshHandle);

                  cmd.bindVertexBuffers(0, mesh->getVertexBuffer().getBuffer(), {0});
                  cmd.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(),
                                      0,
                                      vk::IndexType::eUint32);

                  cmd.drawIndexed(mesh->getIndicesCount(), 1, 0, 0, i);
                  i++;
               }
            }
         }

         frame.end3D(graphicsDevice->getSwapchainImages()[imageIndex],
                     graphicsDevice->getSwapchainExtent());

         frame.renderOverlay(graphicsDevice->getSwapchainImageViews()[imageIndex],
                             graphicsDevice->getSwapchainExtent());

         frame.endFrame(graphicsDevice->getSwapchainImages()[imageIndex]);
      }
      cmd.end();
   }

   void RenderContext::drawImgui(const vk::raii::CommandBuffer& cmd,
                                 const vk::raii::ImageView& imageView) const {
      const auto colorAttachment = vk::RenderingAttachmentInfo{
          .imageView = *imageView,
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
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

   void RenderContext::render() {
      drawFrame();
      renderObjects.clear();
      objectDataList.clear();
   }

   void RenderContext::waitIdle() {
      graphicsDevice->getVulkanDevice().waitIdle();
   }

   void RenderContext::enqueueRenderObject(RenderObject renderObject) {
      objectDataList.emplace_back(renderObject.modelMatrix, renderObject.textureId);
      renderObjects.push_back(std::move(renderObject));
   }
}