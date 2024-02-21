#pragma once

#include "vma_raii.hpp"
#include "ObjectData.hpp"

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

namespace Triton::Graphics {

   class GraphicsDevice;

   class FrameData {
    public:
      FrameData(const GraphicsDevice& graphicsDevice,
                const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
                const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout,
                const std::string_view name);
      ~FrameData();

      FrameData(const FrameData&) = delete;
      FrameData(FrameData&&) = delete;
      FrameData& operator=(const FrameData&) = delete;
      FrameData& operator=(FrameData&&) = delete;

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

      [[nodiscard]] const vk::raii::DescriptorSet& getBindlessDescriptorSet() const {
         return *bindlessDescriptorSet;
      };

      [[nodiscard]] const vk::raii::DescriptorSet& getObjectDescriptorSet() const {
         return *objectDescriptorSet;
      };

      [[nodiscard]] tracy::VkCtx* getTracyContext() const {
         return tracyContext;
      }

      // Can probably do better for managing per-frame items.
      [[nodiscard]] std::vector<uint32_t>& getTexturesToBind() {
         return texturesToBind;
      }

      [[nodiscard]] const AllocatedBuffer& getCameraBuffer() const {
         return *cameraDataBuffer;
      }

      [[nodiscard]] const vk::raii::DescriptorSet& getPerFrameDescriptorSet() const {
         return *perFrameDescriptorSet;
      }

      [[nodiscard]] const vk::Image& getDrawImage() const {
         return drawImage->getImage();
      }

      [[nodiscard]] const vk::ImageView& getDrawImageView() const {
         return **drawImageView;
      }

      void updateObjectDataBuffer(const ObjectData* data, const size_t size);
      void destroySwapchainResources();
      void createSwapchainResources(const GraphicsDevice& graphicsDevice);

    private:
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore;
      std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore;
      std::unique_ptr<vk::raii::Fence> inFlightFence;

      /*
         Keeping one of all of these per frame in flight doubles the memory usage, but also cuts out
         a mess of synchronization that must be done between frames in flight.  Otherwise you would
         have to acquire a lock after updating, and not be able to release it until after the render
         finished fence signals, leaving a very narrow window of time to update any of these
         buffers.  Also me: git gud.

         Yeah, we need to figure this out at some point.  Wait until profiling reveals a problem,
         and deal with optimizing it then.
      */
      std::unique_ptr<AllocatedBuffer> objectDataBuffer;
      std::unique_ptr<AllocatedBuffer> cameraDataBuffer;

      std::unique_ptr<vk::raii::DescriptorSet> bindlessDescriptorSet;
      std::unique_ptr<vk::raii::DescriptorSet> objectDescriptorSet;
      std::unique_ptr<vk::raii::DescriptorSet> perFrameDescriptorSet;

      std::vector<uint32_t> texturesToBind = {};
      tracy::VkCtx* tracyContext;

      std::unique_ptr<AllocatedImage> drawImage;
      std::unique_ptr<vk::raii::ImageView> drawImageView;
      vk::Extent3D drawExtent;
   };
}