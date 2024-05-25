#pragma once

#include "gfx/ObjectData.hpp"
#include "gfx/GraphicsDevice.hpp"
#include "gfx/PipelineBuilder.hpp"
#include "gfx/sb/ShaderBindingFactory.hpp"
#include "gfx/textures/ResourceManager.hpp"

#include <entt/signal/fwd.hpp>

namespace tr::gfx {
   struct RenderObject;
   struct PerFrameData;
   class ImmediateContext;
   class AbstractPipeline;
   class Frame;
   class RendererBase;
   class Pipeline;
   class PipelineBuilder;

   namespace tx {
      class ResourceManager;
   }

   namespace ds {
      class LayoutFactory;
      class DescriptorSetFactory;
   }

   namespace Gui {
      class ImGuiHelper;
   }

   namespace mem {
      class Allocator;
      class Image;
   }

   using ResizeDelegateType = entt::delegate<void(std::pair<uint32_t, uint32_t>)>;

   class RenderContext {
    public:
      RenderContext(GLFWwindow* window, bool guiEnabled = false);
      ~RenderContext();

      RenderContext(const RenderContext&) = delete;
      RenderContext& operator=(const RenderContext&) = delete;

      RenderContext(RenderContext&&) = delete;
      RenderContext& operator=(RenderContext&&) = delete;

      [[nodiscard]] tx::ResourceManager& getResourceManager() const {
         return *resourceManager;
      }

      void render();
      void waitIdle();

      void enqueueRenderObject(RenderObject renderObject);

      void setCurrentCameraData(CameraData&& cameraData) {
         this->cameraData = std::move(cameraData);
      }

      template <auto Candidate, typename Type>
      void addResizeListener(Type* valueOrInstance) noexcept {
         resizeDelegate.connect<Candidate>(valueOrInstance);
         resizeDelegate(graphicsDevice->getCurrentSize());
      }

      void setDebugRendering(bool wireframeEnabled) {
         debugRendering = wireframeEnabled;
      }

    private:
      static constexpr int FRAMES_IN_FLIGHT = 2;

      bool guiEnabled{};
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
      std::vector<ObjectData> objectDataList{};
      CameraData cameraData{glm::identity<glm::mat4>(),
                            glm::identity<glm::mat4>(),
                            glm::identity<glm::mat4>()};

      uint32_t currentFrame = 0;
      bool framebufferResized = false;

      ResizeDelegateType resizeDelegate{};

      std::vector<MeshData> staticMeshDataList;
      std::vector<MeshData> terrainDataList;
      std::vector<MeshData> skinnedModelList;
      PushConstants pushConstants;

      void initDepthResources();

      void recreateSwapchain();
      void drawFrame();
      void recordCommandBuffer(Frame& frame, unsigned imageIndex);
      void drawImgui(const vk::raii::CommandBuffer& cmd, const vk::raii::ImageView& image) const;
   };
}