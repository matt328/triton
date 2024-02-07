#pragma once

#include "graphics/ObjectData.hpp"

namespace Triton::Graphics {
   struct RenderObject;
   struct PerFrameData;
   class GraphicsDevice;
   class ImmediateContext;
   class AbstractPipeline;
   class FrameData;
   class RendererBase;
   class Allocator;
   class AllocatedImage;
   struct Vertex;

   namespace Textures {
      class Texture;
      class TextureFactory;
   }

   namespace Geometry {
      template <typename T, typename U>
      class Mesh;
      class MeshFactory;
   }

   using RenderObjectProviderFn = std::function<std::vector<RenderObject>()>;
   using PerFrameDataProfiderFn = std::function<PerFrameData()>;

   class Renderer {
    public:
      Renderer(GLFWwindow* window);
      ~Renderer();

      Renderer(const Renderer&) = delete;
      Renderer(Renderer&&) = delete;
      Renderer& operator=(const Renderer&) = delete;
      Renderer& operator=(Renderer&&) = delete;

      void render();
      void waitIdle();
      void windowResized(const int width, const int height);

      MeshHandle createMesh(const std::string_view& filename);
      uint32_t createTexture(const std::string_view& filename);

      void enqueueRenderObject(RenderObject renderObject);

      void setCurrentCameraData(const CameraData&& cameraData) {
         this->cameraData = cameraData;
      }

      [[nodiscard]] const std::tuple<int, int> getWindowSize() const;

      void registerPerFrameDataProvider(PerFrameDataProfiderFn fn) {
         this->perFrameDataProvider = fn;
      }

    private:
      static constexpr int FRAMES_IN_FLIGHT = 2;

      std::unique_ptr<GraphicsDevice> graphicsDevice;

      PerFrameDataProfiderFn perFrameDataProvider;

      struct QueueFamilyIndices;
      struct SwapchainSupportDetails;

      std::unique_ptr<vk::raii::DescriptorSetLayout> bindlessDescriptorSetLayout;
      std::unique_ptr<vk::raii::DescriptorSetLayout> objectDescriptorSetLayout;
      std::unique_ptr<vk::raii::DescriptorSetLayout> perFrameDescriptorSetLayout;

      std::unique_ptr<vk::raii::RenderPass> renderPass;
      std::unique_ptr<vk::raii::Pipeline> pipeline;
      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;

      std::unique_ptr<AllocatedImage> depthImage;
      std::unique_ptr<vk::raii::ImageView> depthImageView;
      std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

      std::vector<std::unique_ptr<FrameData>> frameData;

      std::vector<std::unique_ptr<Geometry::Mesh<Vertex, uint32_t>>> meshes;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      std::vector<RenderObject> renderObjects{};
      std::vector<ObjectData> objectDataList{};
      CameraData cameraData{};

      uint32_t currentFrame = 0;
      bool framebufferResized = false;

      void recreateSwapchain();
      void drawFrame();
      void recordCommandBuffer(FrameData& frameData, unsigned imageIndex) const;
   };
}