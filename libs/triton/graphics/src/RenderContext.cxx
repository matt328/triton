#include "RenderContext.hpp"
#include "Frame.hpp"
#include "GraphicsDevice.hpp"
#include "RenderObject.hpp"

#include "cm/Handles.hpp"
#include "cm/ObjectData.hpp"
#include "cm/RenderData.hpp"
#include "sb/LayoutFactory.hpp"
#include "geometry/Vertex.hpp"
#include "gui/ImguiHelper.hpp"

#include "mem/Buffer.hpp"
#include "mem/Image.hpp"
#include "sb/ShaderBindingFactory.hpp"
#include "helpers/Vulkan.hpp"
#include "helpers/SpirvHelper.hpp"
#include "Paths.hpp"
#include "textures/ResourceManager.hpp"
#include "PipelineBuilder.hpp"
#include "mem/Allocator.hpp"
#include "sb/ShaderBinding.hpp"

namespace tr::gfx {

   /*
      TODO: This thing is a monolith. Look at all those #includes.
   */

   class RenderContext::Impl {
    public:
      Impl(GLFWwindow* window, bool guiEnabled, bool validationEnabled) {
         graphicsDevice = std::make_unique<GraphicsDevice>(window, validationEnabled);

         layoutFactory = std::make_unique<ds::LayoutFactory>(*graphicsDevice);

         sbFactory = std::make_unique<sb::ShaderBindingFactory>(*graphicsDevice, *layoutFactory);

         const auto viewportSize = graphicsDevice->getSwapchainExtent();
         mainViewport = vk::Viewport{.x = 0.f,
                                     .y = 0.f,
                                     .width = static_cast<float>(viewportSize.width),
                                     .height = static_cast<float>(viewportSize.height),
                                     .minDepth = 0.f,
                                     .maxDepth = 1.f};
         mainScissor = vk::Rect2D{.offset = {0, 0}, .extent = viewportSize};

         auto helper = std::make_unique<Helpers::SpirvHelper>(graphicsDevice->getVulkanDevice());

         auto vsm = helper->createShaderModule(vk::ShaderStageFlagBits::eVertex,
                                               util::Paths::SHADERS / "shader.vert");
         auto fsm = helper->createShaderModule(vk::ShaderStageFlagBits::eFragment,
                                               util::Paths::SHADERS / "shader.frag");

         auto setLayouts = std::array{layoutFactory->getVkLayout(ds::LayoutHandle::Bindless),
                                      layoutFactory->getVkLayout(ds::LayoutHandle::ObjectData),
                                      layoutFactory->getVkLayout(ds::LayoutHandle::PerFrame)};

         pb = std::make_unique<PipelineBuilder>(graphicsDevice->getVulkanDevice());

         pb->setDefaultVertexAttributeDescriptions();
         pb->setVertexShaderStage(vsm);
         pb->setFragmentShaderStage(fsm);
         pb->setInputTopology(vk::PrimitiveTopology::eTriangleList);
         pb->setPolygonMode(vk::PolygonMode::eFill);
         pb->setCullMode(vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise);
         pb->setMultisamplingNone();
         pb->disableBlending();
         pb->setColorAttachmentFormat(vk::Format::eR16G16B16A16Sfloat);
         pb->setDepthFormat(Helpers::findDepthFormat(graphicsDevice->getPhysicalDevice()));
         pb->setDefaultDepthStencil();

         staticModelPipelineLayout =
             pb->buildPipelineLayout(setLayouts, "Static Model Pipeline Layout");
         staticModelPipeline =
             pb->buildPipeline(*staticModelPipelineLayout, "Static Model Pipeline");

         auto terrainVertex = helper->createShaderModule(vk::ShaderStageFlagBits::eVertex,
                                                         util::Paths::SHADERS / "terrain.vert");
         auto terrainFragment = helper->createShaderModule(vk::ShaderStageFlagBits::eFragment,
                                                           util::Paths::SHADERS / "terrain.frag");

         pb->clearShaderStages();
         pb->setVertexShaderStage(terrainVertex);
         pb->setFragmentShaderStage(terrainFragment);
         pb->setPolygonMode(vk::PolygonMode::eFill);

         terrainPipelineLayout = pb->buildPipelineLayout(setLayouts, "Terrain Pipeline Layout");
         terrainPipeline = pb->buildPipeline(*terrainPipelineLayout, "Terrain Pipeline");

         auto debugVertex = helper->createShaderModule(vk::ShaderStageFlagBits::eVertex,
                                                       util::Paths::SHADERS / "debug.vert");
         auto debugFragment = helper->createShaderModule(vk::ShaderStageFlagBits::eFragment,
                                                         util::Paths::SHADERS / "debug.frag");

         pb->clearShaderStages();
         pb->setVertexShaderStage(debugVertex);
         pb->setFragmentShaderStage(debugFragment);
         pb->setPolygonMode(vk::PolygonMode::eLine);

         debugPipelineLayout = pb->buildPipelineLayout(setLayouts, "Debug Pipeline Layout");
         debugPipeline = pb->buildPipeline(*debugPipelineLayout, "Debug Pipeline");

         auto skinnedVertex = helper->createShaderModule(vk::ShaderStageFlagBits::eVertex,
                                                         util::Paths::SHADERS / "skinned.vert");
         auto skinnedFragment = helper->createShaderModule(vk::ShaderStageFlagBits::eFragment,
                                                           util::Paths::SHADERS / "skinned.frag");

         auto vec = std::vector{geo::VertexComponent::Position,
                                geo::VertexComponent::Color,
                                geo::VertexComponent::UV,
                                geo::VertexComponent::Normal,
                                geo::VertexComponent::Joint0,
                                geo::VertexComponent::Weight0};

         pb->clearShaderStages();
         pb->setVertexShaderStage(skinnedVertex);
         pb->setFragmentShaderStage(skinnedFragment);
         pb->setPolygonMode(vk::PolygonMode::eFill);
         pb->setVertexAttributeDescriptions(std::span(vec.begin(), vec.end()));

         auto skinnedSetLayouts =
             std::array{layoutFactory->getVkLayout(ds::LayoutHandle::Bindless),
                        layoutFactory->getVkLayout(ds::LayoutHandle::ObjectData),
                        layoutFactory->getVkLayout(ds::LayoutHandle::PerFrame),
                        layoutFactory->getVkLayout(ds::LayoutHandle::AnimationData)};

         skinnedModelPipelineLayout =
             pb->buildPipelineLayout(skinnedSetLayouts, "Skinned Model Pipeline Layout");
         skinnedModelPipeline =
             pb->buildPipeline(*skinnedModelPipelineLayout, "Skinned Model Pipeline");

         initDepthResources();

         frames.reserve(FRAMES_IN_FLIGHT);
         for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
            auto name = std::stringstream{};
            name << "Frame " << i;
            frames.push_back(
                std::make_unique<Frame>(*graphicsDevice, depthImageView, *sbFactory, name.str()));
         }

         if (guiEnabled) {
            imguiHelper = std::make_unique<Gui::ImGuiHelper>(*graphicsDevice, window);
         }

         resourceManager = std::make_unique<tx::ResourceManager>(*graphicsDevice);
      }

      void render() {
         drawFrame();
         renderObjects.clear();
         objectDataList.clear();
      }

      void waitIdle() const {
         graphicsDevice->getVulkanDevice().waitIdle();
      }

      void enqueueRenderObject(RenderObject renderObject) {
         objectDataList.emplace_back(renderObject.modelMatrix, renderObject.textureId);
         renderObjects.push_back(std::move(renderObject));
      }

      [[nodiscard]] auto& getResourceManager() const {
         return *resourceManager;
      }

      void setCurrentCameraData(cm::gpu::CameraData&& cameraData) {
         this->cameraData = cameraData;
      }

      void setDebugRendering(const bool wireframeEnabled) {
         debugRendering = wireframeEnabled;
      }

      void setResizeListener(const std::function<void(std::pair<uint32_t, uint32_t>)>& fn) {
         resizeFn = fn;
         resizeFn(graphicsDevice->getCurrentSize());
      }

      void setRenderData(const cm::gpu::RenderData& renderData) const {
         resourceManager->setRenderData(renderData);
      }

      [[nodiscard]] auto createStaticModel(const std::filesystem::path& modelPath) const noexcept {
         return resourceManager->createModel(modelPath);
      }

      [[nodiscard]] auto createTerrain(const uint32_t size) const {
         return resourceManager->createTerrain(size);
      }

      [[nodiscard]] auto createSkinnedModel(const std::filesystem::path& modelPath) const noexcept {
         return resourceManager->createModel(modelPath);
      }

    private:
      void initDepthResources() {
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

      void recreateSwapchain() {
         waitIdle();
         for (const auto& fd : frames) {
            fd->destroySwapchainResources();
         }

         graphicsDevice->recreateSwapchain();

         constexpr auto size = GraphicsDevice::DrawImageExtent2D;

         // resize Viewport and Scissor Rect
         mainViewport = vk::Viewport{.x = 0.f,
                                     .y = 0.f,
                                     .width = static_cast<float>(size.width),
                                     .height = static_cast<float>(size.height),
                                     .minDepth = 0.f,
                                     .maxDepth = 1.f};

         mainScissor = vk::Rect2D{.offset = {0, 0}, .extent = size};

         resizeFn(graphicsDevice->getCurrentSize());

         for (const auto& fd : frames) {
            fd->createSwapchainResources(*graphicsDevice);
         }
      }

      void drawFrame() {
         ZoneNamedN(render, "Render", true);
         const auto& currentFrameData = frames[currentFrame];
         // Wait for this frame's fence so we can be sure the gpu is finished with this frame's
         // command buffer.  Which it should be since it was submitted a frame or two ago.
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

         // Ask the swapchain to move to the next image. This call is async, and will signal the
         // given semaphore when it's completed.
         vk::Result result{};
         unsigned int imageIndex{};
         try {
            ZoneNamedN(acquire, "Acquire Swapchain Image", true);
            std::tie(result, imageIndex) = graphicsDevice->getSwapchain().acquireNextImage(
                UINT64_MAX,
                *currentFrameData->getImageAvailableSemaphore(),
                nullptr);
         } catch (const std::exception& ex) {
            Log.warn("Swapchain needs resized: {0}", ex.what());
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

         // Build out the struct and submit the command buffer, signaling the in flight fence when
         // it can be recorded to again
         constexpr auto waitStages = std::array<vk::PipelineStageFlags, 1>{
             vk::PipelineStageFlagBits::eColorAttachmentOutput};

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

         graphicsDevice->getGraphicsQueue().submit(submitInfo,
                                                   *currentFrameData->getInFlightFence());

         // Ask the GPU to present the image once the submit semaphore is signaled. Also trap errors
         // here and recreate (resize) the swapchain
         try {

            if (const auto result2 = graphicsDevice->getGraphicsQueue().presentKHR(
                    vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                                       .pWaitSemaphores = renderFinishedSemaphores.data(),
                                       .swapchainCount = 1,
                                       .pSwapchains = &*graphicsDevice->getSwapchain(),
                                       .pImageIndices = &imageIndex});
                result2 == vk::Result::eSuboptimalKHR) {
               recreateSwapchain();
            }
         } catch (const std::exception& ex) {
            Log.info("Swapchain needs recreated: {0}", ex.what());
            recreateSwapchain();
         }

         // Finally done, move to the next frame
         currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
      }

      void recordCommandBuffer(Frame& frame, const unsigned imageIndex) {
         auto& cmd = frame.getCommandBuffer();

         /*
            These buffers need to be updated at this point in time since we know they're not being
            used by the current frame yet. The sources of their data however should be a persistent
            list that asynchronous updates to the game world maintain. These updates should be
            synced so that each step of the game world produces a consistent data set

            If we multithread the game world's systems, they'll all still have to join at a barrier
            to update the state together, so that one tick keeps to one execution of each thread and
            we don't run a fast system 1000 times while a slow system has only run once.

            Each one of these ticks will correspond to an execution of the fixedUpdate() function in
            the timer class.

            Might not implement multithreaded game world just yet, but keep in mind the game world's
            systems should be designed as tasks that run and produce a result. This will make
            orchestration of the thread pool easier since each tick can notify all the threads to
            start, and barrier at them all finishing their task and returning the data. Then, lock
            the data structure, write new data into it, unlock. This will happen as many ticks as it
            can in one frame.  Meanwhile in the render thread, also lock on reading the data, write
            it into the current frame's buffers, then release the lock.
         */
         staticMeshDataList.clear();
         terrainDataList.clear();
         skinnedModelList.clear();

         resourceManager->accessRenderData([&frame, this](cm::gpu::RenderData& renderData) {
            frame.updateObjectDataBuffer(renderData.objectData.data(),
                                         sizeof(cm::gpu::ObjectData) *
                                             renderData.objectData.size());

            frame.updatePerFrameDataBuffer(&renderData.cameraData, sizeof(cm::gpu::CameraData));

            // TODO Rename this AnimationData class
            frame.updateAnimationDataBuffer(renderData.animationData.data(),
                                            sizeof(cm::gpu::AnimationData) *
                                                renderData.animationData.size());

            staticMeshDataList.reserve(renderData.staticMeshData.size());
            std::ranges::copy(renderData.staticMeshData, std::back_inserter(staticMeshDataList));

            terrainDataList.reserve(renderData.terrainMeshData.size());
            std::ranges::copy(renderData.terrainMeshData, std::back_inserter(terrainDataList));

            skinnedModelList.reserve(renderData.skinnedMeshData.size());
            std::ranges::copy(renderData.skinnedMeshData, std::back_inserter(skinnedModelList));

            pushConstants = renderData.pushConstants;
         });

         resourceManager->accessTextures(
             [&frame](const std::vector<vk::DescriptorImageInfo>& imageInfoList) {
                ZoneNamedN(a, "Updating Texture DB", true);
                frame.updateTextures(imageInfoList);
             });

         cmd.begin(
             vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
         {
            frame.prepareFrame();

            // Static Models
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **staticModelPipeline);

            // Bind ShaderBindings to Pipeline
            frame.getTextureShaderBinding().bindToPipeline(cmd,
                                                           vk::PipelineBindPoint::eGraphics,
                                                           0,
                                                           **staticModelPipelineLayout);
            frame.getObjectDataShaderBinding().bindToPipeline(cmd,
                                                              vk::PipelineBindPoint::eGraphics,
                                                              1,
                                                              **staticModelPipelineLayout);
            frame.getPerFrameShaderBinding().bindToPipeline(cmd,
                                                            vk::PipelineBindPoint::eGraphics,
                                                            2,
                                                            **staticModelPipelineLayout);

            cmd.setViewportWithCount(mainViewport);
            cmd.setScissorWithCount(mainScissor);
            {
               for (const auto& meshData : staticMeshDataList) {
                  const auto& mesh = resourceManager->getMesh(meshData.handle);

                  cmd.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
                  cmd.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(),
                                      0,
                                      vk::IndexType::eUint32);

                  // instanceId becomes gl_BaseInstance in the shader
                  cmd.drawIndexed(mesh.getIndicesCount(), 1, 0, 0, meshData.objectDataId);
               }
            }

            // Terrain
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **terrainPipeline);
            {
               cmd.pushConstants<cm::gpu::PushConstants>(**terrainPipelineLayout,
                                                         vk::ShaderStageFlagBits::eVertex |
                                                             vk::ShaderStageFlagBits::eFragment,
                                                         0,
                                                         pushConstants);
               for (const auto& meshData : terrainDataList) {
                  const auto& mesh = resourceManager->getMesh(meshData.handle);

                  cmd.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
                  cmd.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(),
                                      0,
                                      vk::IndexType::eUint32);

                  // instanceId becomes gl_BaseInstance in the shader
                  cmd.drawIndexed(mesh.getIndicesCount(), 1, 0, 0, meshData.objectDataId);
               }
            }

            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **skinnedModelPipeline);
            {
               frame.getAnimationShaderBinding().bindToPipeline(cmd,
                                                                vk::PipelineBindPoint::eGraphics,
                                                                3,
                                                                **skinnedModelPipelineLayout);
               for (const auto& meshData : skinnedModelList) {
                  const auto& mesh = resourceManager->getMesh(meshData.handle);

                  cmd.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
                  cmd.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(),
                                      0,
                                      vk::IndexType::eUint32);

                  // instanceId becomes gl_BaseInstance in the shader
                  cmd.drawIndexed(mesh.getIndicesCount(), 1, 0, 0, meshData.objectDataId);
               }
            }

            // Wireframe
            if (debugRendering) {
               cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **debugPipeline);
               {
                  for (const auto& meshData : terrainDataList) {
                     const auto& mesh = resourceManager->getMesh(meshData.handle);

                     cmd.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
                     cmd.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(),
                                         0,
                                         vk::IndexType::eUint32);

                     // instanceId becomes gl_BaseInstance in the shader
                     cmd.drawIndexed(mesh.getIndicesCount(), 1, 0, 0, meshData.objectDataId);
                  }

                  for (const auto& meshData : staticMeshDataList) {
                     const auto& mesh = resourceManager->getMesh(meshData.handle);

                     cmd.bindVertexBuffers(0, mesh.getVertexBuffer()->getBuffer(), {0});
                     cmd.bindIndexBuffer(mesh.getIndexBuffer()->getBuffer(),
                                         0,
                                         vk::IndexType::eUint32);

                     // instanceId becomes gl_BaseInstance in the shader
                     cmd.drawIndexed(mesh.getIndicesCount(), 1, 0, 0, meshData.objectDataId);
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

      void drawImgui(const vk::raii::CommandBuffer& cmd,
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

      static constexpr int FRAMES_IN_FLIGHT = 2;

      bool debugRendering{false};

      std::unique_ptr<GraphicsDevice> graphicsDevice;

      std::unique_ptr<PipelineBuilder> pb;

      vk::Viewport mainViewport;
      vk::Rect2D mainScissor;

      std::unique_ptr<vk::raii::Pipeline> staticModelPipeline;
      std::unique_ptr<vk::raii::PipelineLayout> staticModelPipelineLayout;

      std::unique_ptr<vk::raii::Pipeline> terrainPipeline;
      std::unique_ptr<vk::raii::PipelineLayout> terrainPipelineLayout;

      std::unique_ptr<vk::raii::Pipeline> debugPipeline;
      std::unique_ptr<vk::raii::PipelineLayout> debugPipelineLayout;

      std::unique_ptr<vk::raii::Pipeline> skinnedModelPipeline;
      std::unique_ptr<vk::raii::PipelineLayout> skinnedModelPipelineLayout;

      std::unique_ptr<mem::Image> depthImage;
      std::shared_ptr<vk::raii::ImageView> depthImageView;

      std::unique_ptr<ds::LayoutFactory> layoutFactory;
      std::unique_ptr<sb::ShaderBindingFactory> sbFactory;

      std::vector<std::unique_ptr<Frame>> frames;

      std::unique_ptr<Gui::ImGuiHelper> imguiHelper;

      std::unique_ptr<tx::ResourceManager> resourceManager;

      std::vector<RenderObject> renderObjects{};
      std::vector<cm::gpu::ObjectData> objectDataList{};
      cm::gpu::CameraData cameraData{glm::identity<glm::mat4>(),
                                     glm::identity<glm::mat4>(),
                                     glm::identity<glm::mat4>()};

      uint32_t currentFrame = 0;

      std::function<void(std::pair<uint32_t, uint32_t>)> resizeFn;

      std::vector<cm::gpu::MeshData> staticMeshDataList;
      std::vector<cm::gpu::MeshData> terrainDataList;
      std::vector<cm::gpu::MeshData> skinnedModelList;
      cm::gpu::PushConstants pushConstants{};
   };

   RenderContext::RenderContext(GLFWwindow* window, bool guiEnabled, bool validationEnabled) {
      impl = std::make_unique<Impl>(window, guiEnabled, validationEnabled);
   }

   RenderContext::~RenderContext() {
      Log.info("Destroying RenderContext");
   }

   void RenderContext::setResizeListener(
       const std::function<void(std::pair<uint32_t, uint32_t>)>& fn) const {
      impl->setResizeListener(fn);
   }

   void RenderContext::render() const {
      impl->render();
   }

   void RenderContext::waitIdle() const {
      impl->waitIdle();
   }

   void RenderContext::enqueueRenderObject(RenderObject renderObject) const {
      impl->enqueueRenderObject(std::move(renderObject));
   }

   // TODO: Profile this and see if we can eliminate this move here
   void RenderContext::setCurrentCameraData(cm::gpu::CameraData&& cameraData) const {
      impl->setCurrentCameraData(std::move(cameraData));
   }

   void RenderContext::setRenderData(const cm::gpu::RenderData& renderData) const {
      impl->setRenderData(renderData);
   }

   void RenderContext::setDebugRendering(const bool wireframeEnabled) const {
      impl->setDebugRendering(wireframeEnabled);
   }

   auto RenderContext::createTerrain(const uint32_t size) const -> cm::ModelData {
      return impl->createTerrain(size);
   }

   auto RenderContext::createStaticModel(const std::filesystem::path& modelPath) const
       -> cm::ModelData {
      return impl->createStaticModel(modelPath);
   }

   auto RenderContext::createSkinnedModel(const std::filesystem::path& modelPath) const
       -> cm::ModelData {
      return impl->createSkinnedModel(modelPath);
   }

}