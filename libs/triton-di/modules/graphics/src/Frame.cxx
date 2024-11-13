#include "Frame.hpp"

#include "cm/RenderData.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "mem/Image.hpp"
#include "renderer/IRenderer.hpp"
#include "mem/Buffer.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx {
   Frame::Frame(std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
                std::shared_ptr<vk::raii::ImageView> newDepthImageView,
                std::shared_ptr<sb::IShaderBindingFactory> shaderBindingFactory,
                std::string_view name)
       : commandBuffer{newGraphicsDevice->createCommandBuffer()},
         tracyContext{newGraphicsDevice->createTracyContext(name, *commandBuffer)},
         graphicsDevice{std::move(newGraphicsDevice)},
         depthImageView{std::move(newDepthImageView)} {

      constexpr auto fenceCreateInfo =
          vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled};
      inFlightFence =
          std::make_unique<vk::raii::Fence>(*graphicsDevice->getVulkanDevice(), fenceCreateInfo);
      imageAvailableSemaphore =
          std::make_unique<vk::raii::Semaphore>(*graphicsDevice->getVulkanDevice(),
                                                vk::SemaphoreCreateInfo{});

      objectDataBuffer =
          graphicsDevice->createStorageBuffer(sizeof(cm::gpu::ObjectData) * cm::gpu::MAX_OBJECTS,
                                              "Object Data");
      objectDataBuffer->mapBuffer();

      cameraDataBuffer =
          graphicsDevice->createUniformBuffer(sizeof(cm::gpu::CameraData), "Camera Data");

      animationDataBuffer =
          graphicsDevice->createStorageBuffer(sizeof(cm::AnimationData) * cm::gpu::MAX_OBJECTS,
                                              "Animation Data");

      std::tie(drawImage, drawImageView) = graphicsDevice->createDrawImage("Draw Image");
   }

   Frame::~Frame() {
      objectDataBuffer->unmapBuffer();
   }

   void Frame::registerStorageBuffer(const std::string& name, size_t size) {
   }

   void Frame::destroySwapchainResources() {
      commandBuffer.reset();
   }

   void Frame::createSwapchainResources() {
      commandBuffer = graphicsDevice->createCommandBuffer();
   }

   void Frame::awaitInFlightFence() {
      if (const auto res = graphicsDevice->getVulkanDevice()->waitForFences(**inFlightFence,
                                                                            VK_TRUE,
                                                                            UINT64_MAX);
          res != vk::Result::eSuccess) {
         throw std::runtime_error("Error waiting for fences");
      }
   }

   auto Frame::acquireSwapchainImage() -> AcquireResult {
      ZoneNamedN(acquire, "Acquire Swapchain Image", true);
      const auto result = graphicsDevice->acquireNextSwapchainImage(*imageAvailableSemaphore);
      if (std::holds_alternative<uint32_t>(result)) {
         swapchainImageIndex = std::get<uint32_t>(result);
      } else {
         return std::get<AcquireResult>(result);
      }

      return AcquireResult::Error;
   }

   void Frame::resetInFlightFence() {
      graphicsDevice->getVulkanDevice()->resetFences(**inFlightFence);
   }

   void Frame::applyRenderData(const cm::gpu::RenderData& renderData) {
      staticMeshDataList.clear();
      lineDataList.clear();
      terrainDataList.clear();
      skinnedModelList.clear();
      {
         ZoneNamedN(zone, "Copying RenderData", true);
         updateObjectDataBuffer(renderData.objectData.data(),
                                sizeof(cm::gpu::ObjectData) * renderData.objectData.size());

         updatePerFrameDataBuffer(&renderData.cameraData, sizeof(cm::gpu::CameraData));

         // TODO(Matt): Rename this AnimationData class
         updateAnimationDataBuffer(renderData.animationData.data(),
                                   sizeof(cm::gpu::AnimationData) *
                                       renderData.animationData.size());

         auto meshData = renderData.staticMeshData;

         auto it = std::partition(meshData.begin(), meshData.end(), [](auto& meshData) {
            return meshData.topology == cm::Topology::Triangles;
         });
         staticMeshDataList.assign(meshData.begin(), it);
         lineDataList.assign(it, meshData.end());

         terrainDataList.reserve(renderData.terrainMeshData.size());
         std::ranges::copy(renderData.terrainMeshData, std::back_inserter(terrainDataList));

         skinnedModelList.reserve(renderData.skinnedMeshData.size());
         std::ranges::copy(renderData.skinnedMeshData, std::back_inserter(skinnedModelList));

         pushConstants = renderData.pushConstants;
      }

      {
         const auto imageInfoList = resourceManager->getTextures();
         ZoneNamedN(zone, "Updating Texture DB", true);
         updateTextures(imageInfoList.get());
      }
   }

   void Frame::render(std::shared_ptr<rd::IRenderer> renderer) {
      commandBuffer->begin(
          vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});
      {
         TracyVkZone(tracyContext.get(), **commandBuffer, "Prep Frame");
         prepareFrame();

         {
            ZoneNamedN(zone2, "Apply Shader Bindings", true);
            // Static Models

            commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, **staticModelPipeline);

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
         }
      }
   }
   void Frame::prepareFrame() {
      Helpers::transitionImage(*commandBuffer,
                               drawImage->getImage(),
                               vk::ImageLayout::eUndefined,
                               vk::ImageLayout::eColorAttachmentOptimal);

      const auto colorAttachmentInfo = vk::RenderingAttachmentInfo{
          .imageView = **drawImageView,
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
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
          .clearValue = vk::ClearValue{.depthStencil =
                                           vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
      };

      const auto renderingInfo =
          vk::RenderingInfo{.renderArea = vk::Rect2D{.offset = {0, 0}, .extent = drawExtent},
                            .layerCount = 1,
                            .colorAttachmentCount = 1,
                            .pColorAttachments = &colorAttachmentInfo,
                            .pDepthAttachment = &depthAttachmentInfo};

      commandBuffer->beginRendering(renderingInfo);
   }

   void Frame::present() {
   }

   void Frame::updateObjectDataBuffer(const cm::gpu::ObjectData* data, size_t size) const {
      objectDataBuffer->updateMappedBufferValue(data, size);
   }
   void Frame::updatePerFrameDataBuffer(const cm::gpu::CameraData* data, size_t size) const {
      cameraDataBuffer->updateMappedBufferValue(data, size);
   }
   void Frame::updateAnimationDataBuffer(const cm::gpu::AnimationData* data, size_t size) const {
      animationDataBuffer->updateMappedBufferValue(data, size);
   }
}
