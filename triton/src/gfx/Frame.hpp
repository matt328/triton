#pragma once

#include "gfx/ds/LayoutFactory.hpp"

namespace vk::raii {
   class DescriptorPool;
   class Device;
   class CommandPool;
   class DescriptorSet;
   class DescriptorSetLayout;
   class Fence;
   class Semaphore;
   class CommandBuffer;
}

namespace tr::gfx::mem {
   class AllocatedImage;
   class AllocatedBuffer;
}

namespace tr::gfx {

   class GraphicsDevice;
   struct ObjectData;

   class Frame {
    public:
      Frame(const GraphicsDevice& graphicsDevice,
            std::shared_ptr<vk::raii::ImageView> depthImageView,
            ds::LayoutFactory& layoutFactory,
            const std::string_view name);
      ~Frame();

      Frame(const Frame&) = delete;
      Frame(Frame&&) = delete;
      Frame& operator=(const Frame&) = delete;
      Frame& operator=(Frame&&) = delete;

      [[nodiscard]] const vk::raii::CommandBuffer& getCommandBuffer() const {
         return *commandBuffer;
      };

      [[nodiscard]] const vk::raii::Semaphore& getImageAvailableSemaphore() const {
         return *imageAvailableSemaphore;
      };

      [[nodiscard]] const vk::raii::Semaphore& getRenderFinishedSemaphore() const {
         return *renderFinishedSemaphore;
      };

      [[nodiscard]] const vk::raii::Fence& getInFlightFence() const {
         return *inFlightFence;
      };

      [[nodiscard]] tracy::VkCtx* getTracyContext() const {
         return tracyContext;
      }

      [[nodiscard]] const mem::AllocatedBuffer& getCameraBuffer() const {
         return *cameraDataBuffer;
      }

      [[nodiscard]] const mem::AllocatedBuffer& getObjectDataBuffer() const {
         return *objectDataBuffer;
      }

      [[nodiscard]] const vk::Image& getDrawImage() const;

      [[nodiscard]] const vk::ImageView& getDrawImageView() const {
         return **drawImageView;
      }

      void updateObjectDataBuffer(const ObjectData* data, const size_t size);
      void destroySwapchainResources();
      void createSwapchainResources(const GraphicsDevice& graphicsDevice);

      /// Sets up the attachments for renderingInfo and calls cmd.beginRendering()
      void prepareFrame();
      void end3D(const vk::Image& swapchainImage, const vk::Extent2D& swapchainExtent);
      void renderOverlay(const vk::raii::ImageView& swapchainImage,
                         const vk::Extent2D& swapchainExtent);
      void endFrame(const vk::Image& swapchainImage);

    private:
      const vk::raii::Device& graphicsDevice;
      std::shared_ptr<vk::raii::ImageView> depthImageView;
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore;
      std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore;
      std::unique_ptr<vk::raii::Fence> inFlightFence;

      ds::LayoutFactory& layoutFactory;

      std::unique_ptr<mem::AllocatedBuffer> objectDataBuffer;
      std::unique_ptr<mem::AllocatedBuffer> cameraDataBuffer;

      tracy::VkCtx* tracyContext;

      std::unique_ptr<mem::AllocatedImage> drawImage;
      std::unique_ptr<vk::raii::ImageView> drawImageView;
      vk::Extent2D drawExtent;
   };
}