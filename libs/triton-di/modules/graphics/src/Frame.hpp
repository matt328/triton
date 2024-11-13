#pragma once

#include "cm/RenderData.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "gp/IGameplaySystem.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace tr::gfx {

   class IGraphicsDevice;
   enum class AcquireResult;

   namespace mem {
      class Buffer;
   }

   namespace rd {
      class IRenderer;
   }

   namespace sb {
      class IShaderBindingFactory;
   }

   class Frame {
    public:
      Frame(std::shared_ptr<IGraphicsDevice> graphicsDevice,
            std::shared_ptr<vk::raii::ImageView> depthImageView,
            std::shared_ptr<sb::IShaderBindingFactory> shaderBindingFactory,
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
      void render(std::span<std::shared_ptr<rd::IRenderer>> renderers);
      void present();

    private:
      std::unique_ptr<vk::raii::CommandBuffer> commandBuffer;
      TracyContextPtr tracyContext;

      uint32_t swapchainImageIndex{};
      std::shared_ptr<IGraphicsDevice> graphicsDevice;

      std::unique_ptr<vk::raii::Fence> inFlightFence;
      std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore;

      std::vector<cm::gpu::MeshData> staticMeshDataList;
      std::vector<cm::gpu::MeshData> lineDataList;
      std::vector<cm::gpu::MeshData> terrainDataList;
      std::vector<cm::gpu::MeshData> skinnedModelList;
      cm::gpu::PushConstants pushConstants{};

      std::unique_ptr<mem::Buffer> objectDataBuffer;
      std::unique_ptr<mem::Buffer> cameraDataBuffer;
      std::unique_ptr<mem::Buffer> animationDataBuffer;

      void updateObjectDataBuffer(const cm::gpu::ObjectData* data, size_t size) const;
      void updatePerFrameDataBuffer(const cm::gpu::CameraData* data, size_t size) const;
      void updateAnimationDataBuffer(const cm::gpu::AnimationData* data, size_t size) const;
      void prepareFrame();
   };
}
