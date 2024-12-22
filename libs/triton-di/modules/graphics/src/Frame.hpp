#pragma once

#include "tr/IGuiSystem.hpp"
#include "cm/RenderData.hpp"
#include "cm/TracyDefs.hpp"

namespace tr {

   class IGraphicsDevice;

   enum class AcquireResult;

   namespace mem {
      class Buffer;
      class Image;
   }

   namespace rd {
      class IRenderer;
   }

   namespace sb {
      class IShaderBindingFactory;
      class ShaderBinding;
   }

   class Frame {
    public:
      Frame(std::shared_ptr<IGraphicsDevice> graphicsDevice,
            std::shared_ptr<vk::raii::ImageView> newDepthImageView,
            const std::shared_ptr<IShaderBindingFactory>& shaderBindingFactory,
            std::string_view name);
      ~Frame();

      Frame(const Frame&) = delete;
      Frame(Frame&&) = delete;
      auto operator=(const Frame&) -> Frame& = delete;
      auto operator=(Frame&&) -> Frame& = delete;

      void registerStorageBuffer(const std::string& name, size_t size);
      void destroySwapchainResources();
      void createSwapchainResources();

      void awaitInFlightFence();
      auto acquireSwapchainImage() -> AcquireResult;
      void resetInFlightFence();

      void applyRenderData(const cm::gpu::RenderData& renderData);
      void applyTextures(const std::vector<vk::DescriptorImageInfo>& imageInfo);
      void render(const std::shared_ptr<rd::IRenderer>& renderer,
                  const std::tuple<vk::Viewport, vk::Rect2D>& vpScissor);
      auto end3d() -> void;
      auto renderGuiSystem(const std::shared_ptr<IGuiSystem>& guiSystem) -> void;
      auto present() -> bool;

    private:
      std::string frameName;
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
      cm::TracyContextPtr tracyContext;
      std::shared_ptr<IGraphicsDevice> graphicsDevice;
      std::shared_ptr<vk::raii::ImageView> depthImageView;

      uint32_t swapchainImageIndex{};

      std::unique_ptr<vk::raii::Fence> inFlightFence;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore;
      std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore;

      std::vector<cm::gpu::MeshData> staticMeshDataList;
      std::vector<cm::gpu::MeshData> lineDataList;
      std::vector<cm::gpu::MeshData> terrainDataList;
      std::vector<cm::gpu::MeshData> skinnedModelList;
      cm::gpu::PushConstants pushConstants{};

      std::unique_ptr<Buffer> objectDataBuffer;
      std::unique_ptr<Buffer> cameraDataBuffer;
      std::unique_ptr<Buffer> animationDataBuffer;

      std::unique_ptr<Image> drawImage;
      std::unique_ptr<vk::raii::ImageView> drawImageView;

      std::unique_ptr<ShaderBinding> perFrameShaderBinding;
      std::unique_ptr<ShaderBinding> objectDataShaderBinding;
      std::unique_ptr<ShaderBinding> textureShaderBinding;
      std::unique_ptr<ShaderBinding> animationDataShaderBinding;

      void updateObjectDataBuffer(const cm::gpu::ObjectData* data, size_t size) const;
      void updatePerFrameDataBuffer(const cm::gpu::CameraData* data, size_t size) const;
      void updateAnimationDataBuffer(const cm::gpu::AnimationData* data, size_t size) const;
      void prepareFrame();

      static void transitionImage(const vk::raii::CommandBuffer& cmd,
                                  const vk::Image& image,
                                  vk::ImageLayout currentLayout,
                                  vk::ImageLayout newLayout);
   };
}
