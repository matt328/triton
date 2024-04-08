#pragma once

#include "gfx/ObjectData.hpp"
#include "gfx/GraphicsDevice.hpp"
#include "util/TaskQueue.hpp"

#include <entt/signal/fwd.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx {
   struct RenderObject;
   struct PerFrameData;
   class ImmediateContext;
   class AbstractPipeline;
   class FrameData;
   class RendererBase;
   class Allocator;
   class AllocatedImage;

   namespace Textures {
      class Texture;
      class TextureFactory;
   }

   namespace tx {
      class ResourceManager;
   }

   namespace Geometry {
      template <typename T, typename U>
      class Mesh;
      class MeshFactory;
      struct Vertex;
   }

   namespace Gui {
      class ImGuiHelper;
   }

   using ResizeDelegateType = entt::delegate<void(std::pair<uint32_t, uint32_t>)>;

   class Renderer {
    public:
      Renderer(GLFWwindow* window, bool guiEnabled = false);
      ~Renderer();

      Renderer(const Renderer&) = delete;
      Renderer(Renderer&&) = delete;
      Renderer& operator=(const Renderer&) = delete;
      Renderer& operator=(Renderer&&) = delete;

      void render();
      void waitIdle();

      MeshHandle createMesh(const std::string_view& filename);
      uint32_t createTexture(const std::string_view& filename);

      std::future<ModelHandle> loadModelAsync(const std::filesystem::path& filename);

      void enqueueRenderObject(RenderObject renderObject);

      void setCurrentCameraData(CameraData&& cameraData) {
         this->cameraData = std::move(cameraData);
      }

      template <auto Candidate, typename Type>
      void addResizeListener(Type* valueOrInstance) noexcept {
         resizeDelegate.connect<Candidate>(valueOrInstance);
         resizeDelegate(graphicsDevice->getCurrentSize());
      }

    private:
      static constexpr int FRAMES_IN_FLIGHT = 2;

      bool guiEnabled{};

      std::unique_ptr<GraphicsDevice> graphicsDevice;

      struct QueueFamilyIndices;
      struct SwapchainSupportDetails;

      std::unique_ptr<vk::raii::DescriptorSetLayout> bindlessDescriptorSetLayout;
      std::unique_ptr<vk::raii::DescriptorSetLayout> objectDescriptorSetLayout;
      std::unique_ptr<vk::raii::DescriptorSetLayout> perFrameDescriptorSetLayout;

      std::unique_ptr<vk::raii::ShaderModule> vertexShaderModule;
      std::unique_ptr<vk::raii::ShaderModule> fragmentShaderModule;
      std::unique_ptr<vk::raii::Pipeline> pipeline;
      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;

      std::unique_ptr<AllocatedImage> depthImage;
      std::unique_ptr<vk::raii::ImageView> depthImageView;

      std::vector<std::unique_ptr<FrameData>> frameData;

      std::vector<std::unique_ptr<Geometry::Mesh<Geometry::Vertex, uint32_t>>> meshes;
      std::vector<std::unique_ptr<Textures::Texture>> textureList;

      std::unique_ptr<Gui::ImGuiHelper> imguiHelper;

      std::unique_ptr<util::TaskQueue> modelTaskQueue;

      std::unique_ptr<tx::ResourceManager> resourceManager;

      std::condition_variable_any descriptorSetUpdateCv{};
      TracyLockable(std::mutex, descriptorSetUpdateMtx);

      std::optional<std::vector<vk::DescriptorImageInfo>> imageInfoList;

      boolean canUpdateDS = false;

      std::vector<RenderObject> renderObjects{};
      std::vector<ObjectData> objectDataList{};
      CameraData cameraData{glm::identity<glm::mat4>(),
                            glm::identity<glm::mat4>(),
                            glm::identity<glm::mat4>()};

      uint32_t currentFrame = 0;
      bool framebufferResized = false;

      ResizeDelegateType resizeDelegate{};

      void createSwapchainResources();
      void destroySwapchainResources();

      void recreateSwapchain();
      void drawFrame();
      void recordCommandBuffer(FrameData& frameData, unsigned imageIndex) const;
      void drawImgui(const vk::raii::CommandBuffer& cmd, const vk::raii::ImageView& image) const;

      ModelHandle loadModelInt(const std::filesystem::path& filename);
      void checkDescriptorWrites();
   };
}