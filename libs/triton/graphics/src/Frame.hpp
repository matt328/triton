#pragma once

#include "GraphicsDevice.hpp"
#include "cm/ObjectData.hpp"
#include "mem/Allocator.hpp"
#include "mem/Buffer.hpp"
#include "sb/LayoutFactory.hpp"
#include "sb/ShaderBindingFactory.hpp"
#include <vulkan/vulkan_raii.hpp>

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
      Frame(std::shared_ptr<GraphicsDevice> graphicsDevice,
            std::shared_ptr<vk::raii::ImageView> depthImageView,
            std::shared_ptr<sb::ShaderBindingFactory> shaderBindingFactory,
            std::string_view name);
      ~Frame();

      Frame(const Frame&) = delete;
      Frame(Frame&&) = delete;
      auto operator=(const Frame&) -> Frame& = delete;
      auto operator=(Frame&&) -> Frame& = delete;

      [[nodiscard]] auto getCommandBuffer() const -> const vk::raii::CommandBuffer& {
         return *commandBuffer;
      };

      [[nodiscard]] auto getImageAvailableSemaphore() const -> const vk::raii::Semaphore& {
         return *imageAvailableSemaphore;
      };

      [[nodiscard]] auto getRenderFinishedSemaphore() const -> const vk::raii::Semaphore& {
         return *renderFinishedSemaphore;
      };

      [[nodiscard]] auto getInFlightFence() const -> const vk::raii::Fence& {
         return *inFlightFence;
      };

      [[nodiscard]] auto getTracyContext() const -> tracy::VkCtx* {
         return tracyContext;
      }

      [[nodiscard]] auto getCameraBuffer() const -> const mem::Buffer& {
         return *cameraDataBuffer;
      }

      [[nodiscard]] auto getObjectDataBuffer() const -> const mem::Buffer& {
         return *objectDataBuffer;
      }

      [[nodiscard]] auto getAnimationDataBuffer() const -> const mem::Buffer& {
         return *animationDataBuffer;
      }

      [[nodiscard]] auto getDrawImage() const -> const vk::Image&;

      [[nodiscard]] auto getDrawImageView() const -> const vk::ImageView& {
         return **drawImageView;
      }

      [[nodiscard]] auto getPerFrameShaderBinding() const -> const sb::ShaderBinding& {
         return *perFrameShaderBinding;
      }

      [[nodiscard]] auto getObjectDataShaderBinding() const -> const sb::ShaderBinding& {
         return *objectDataShaderBinding;
      }

      [[nodiscard]] auto getTextureShaderBinding() const -> const sb::ShaderBinding& {
         return *textureShaderBinding;
      }

      [[nodiscard]] auto getAnimationShaderBinding() const -> const sb::ShaderBinding& {
         return *animationDataShaderBinding;
      }

      void updateObjectDataBuffer(const cm::gpu::ObjectData* data, size_t size) const;
      void updatePerFrameDataBuffer(const cm::gpu::CameraData* data, size_t size) const;
      void updateAnimationDataBuffer(const cm::gpu::AnimationData* data, size_t size) const;

      void destroySwapchainResources();
      void createSwapchainResources();

      void updateTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos) const;

      /// Sets up the attachments for renderingInfo and calls cmd.beginRendering()
      void prepareFrame() const;

      void end3D(const vk::Image& swapchainImage, const vk::Extent2D& swapchainExtent) const;
      void renderOverlay(const vk::raii::ImageView& swapchainImageView,
                         const vk::Extent2D& swapchainExtent) const;
      void endFrame(const vk::Image& swapchainImage) const;

      void registerStorageBuffer(const std::string& name, size_t size);
      void updateStorageBuffer(const std::string& name, const void* data, size_t size) const;

    private:
      std::shared_ptr<GraphicsDevice> graphicsDevice2;
      size_t combinedImageSamplerDescriptorSize{};
      std::shared_ptr<vk::raii::ImageView> depthImageView;
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore;
      std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore;
      std::unique_ptr<vk::raii::Fence> inFlightFence;

      std::shared_ptr<sb::ShaderBindingFactory> shaderBindingFactory;

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

      std::unordered_map<std::string, std::unique_ptr<mem::Buffer>> buffers;
   };

   class FrameManager {
    public:
      explicit FrameManager(size_t numFrames,
                            const std::shared_ptr<GraphicsDevice>& graphicsDevice,
                            const std::shared_ptr<vk::raii::ImageView>& depthImageView,
                            const std::shared_ptr<sb::ShaderBindingFactory>& sbFactory)
          : numFrames(numFrames) {
         for (size_t i = 0; i < numFrames; ++i) {
            auto name = std::stringstream{};
            name << "Frame " << i;
            frames.push_back(
                std::make_unique<Frame>(graphicsDevice, depthImageView, sbFactory, name.str()));
         }
      }

      [[nodiscard]] auto getCurrentFrame() const -> Frame& {
         return *frames[currentFrame];
      }

      void nextFrame() {
         currentFrame = (currentFrame + 1) % numFrames;
      }

      void registerStorageBuffer(const std::string& name, size_t size) const {
         for (const auto& frame : frames) {
            frame->registerStorageBuffer(name, size);
         }
      }

      void destroySwapchainResources() {
         for (const auto& frame : frames) {
            frame->destroySwapchainResources();
         }
      }

      void createSwapchainResources() {
         for (const auto& frame : frames) {
            frame->createSwapchainResources();
         }
      }

    private:
      size_t currentFrame = 0;
      size_t numFrames;
      std::vector<std::unique_ptr<Frame>> frames;
   };
}