#pragma once

#include "cm/ObjectData.hpp"
#include "sb/LayoutFactory.hpp"
#include "sb/ShaderBindingFactory.hpp"

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

namespace tr::gfx::sb {
   class ShaderBindingFactory;
   class ShaderBinding;
}

namespace tr::gfx::mem {
   class Image;
   class Buffer;
}

namespace tr::gfx {

   class GraphicsDevice;
   struct ObjectData;

   class Frame {
    public:
      Frame(const GraphicsDevice& graphicsDevice,
            std::shared_ptr<vk::raii::ImageView> depthImageView,
            sb::ShaderBindingFactory& shaderBindingFactory,
            std::string_view name);
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

      [[nodiscard]] const mem::Buffer& getCameraBuffer() const {
         return *cameraDataBuffer;
      }

      [[nodiscard]] const mem::Buffer& getObjectDataBuffer() const {
         return *objectDataBuffer;
      }

      [[nodiscard]] const mem::Buffer& getAnimationDataBuffer() const {
         return *animationDataBuffer;
      }

      [[nodiscard]] const vk::Image& getDrawImage() const;

      [[nodiscard]] const vk::ImageView& getDrawImageView() const {
         return **drawImageView;
      }

      [[nodiscard]] const sb::ShaderBinding& getPerFrameShaderBinding() const {
         return *perFrameShaderBinding;
      }

      [[nodiscard]] const sb::ShaderBinding& getObjectDataShaderBinding() const {
         return *objectDataShaderBinding;
      }

      [[nodiscard]] const sb::ShaderBinding& getTextureShaderBinding() const {
         return *textureShaderBinding;
      }

      [[nodiscard]] const sb::ShaderBinding& getAnimationShaderBinding() const {
         return *animationDataShaderBinding;
      }

      void updateObjectDataBuffer(const cm::gpu::ObjectData* data, size_t size) const;
      void updatePerFrameDataBuffer(const cm::gpu::CameraData* data, size_t size) const;
      void updateAnimationDataBuffer(const cm::gpu::AnimationData* data, size_t size) const;

      void destroySwapchainResources();
      void createSwapchainResources(const GraphicsDevice& graphicsDevice);

      void updateTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos) const;

      /// Sets up the attachments for renderingInfo and calls cmd.beginRendering()
      void prepareFrame() const;

      void end3D(const vk::Image& swapchainImage, const vk::Extent2D& swapchainExtent) const;
      void renderOverlay(const vk::raii::ImageView& swapchainImageView,
                         const vk::Extent2D& swapchainExtent) const;
      void endFrame(const vk::Image& swapchainImage) const;

    private:
      const vk::raii::Device& graphicsDevice;
      size_t combinedImageSamplerDescriptorSize{};
      std::shared_ptr<vk::raii::ImageView> depthImageView;
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore;
      std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore;
      std::unique_ptr<vk::raii::Fence> inFlightFence;

      sb::ShaderBindingFactory& shaderBindingFactory;

      std::unique_ptr<sb::ShaderBinding> perFrameShaderBinding;
      std::unique_ptr<sb::ShaderBinding> objectDataShaderBinding;
      std::unique_ptr<sb::ShaderBinding> textureShaderBinding;
      std::unique_ptr<sb::ShaderBinding> animationDataShaderBinding;

      std::unique_ptr<mem::Buffer> objectDataBuffer;
      std::unique_ptr<mem::Buffer> cameraDataBuffer;
      std::unique_ptr<mem::Buffer> animationDataBuffer;

      tracy::VkCtx* tracyContext;

      std::unique_ptr<mem::Image> drawImage;
      std::unique_ptr<vk::raii::ImageView> drawImageView;
      vk::Extent2D drawExtent;
   };
}