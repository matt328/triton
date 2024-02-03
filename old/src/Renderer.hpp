#pragma once

namespace Triton::Memory {
   class Allocator;
   class AllocatedImage;
};

namespace Triton {

   struct RenderObject;
   struct PerFrameData;
   class Instance;
   class ImmediateContext;
   class AbstractPipeline;
   class FrameData;
   class TextureFactory;
   class MeshFactory;
   class Texture;
   class RendererBase;

   template <typename T, typename U>
   class Mesh;

   namespace Models {
      struct Vertex;
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
      void windowResized(const int height, const int width);

      std::string createMesh(const std::string_view& filename);
      uint32_t createTexture(const std::string_view& filename);

      [[nodiscard]] const std::tuple<int, int> getWindowSize() const;

      void registerRenderObjectProvider(RenderObjectProviderFn fn) {
         this->renderObjectProvider = fn;
      }

      void registerPerFrameDataProvider(PerFrameDataProfiderFn fn) {
         this->perFrameDataProvider = fn;
      }

    private:
      RenderObjectProviderFn renderObjectProvider;
      PerFrameDataProfiderFn perFrameDataProvider;

      struct QueueFamilyIndices;
      struct SwapchainSupportDetails;

      static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

      std::unique_ptr<Instance> instance;

      std::unique_ptr<vk::raii::Device> device;

      std::unique_ptr<vk::raii::Queue> graphicsQueue;
      std::unique_ptr<vk::raii::Queue> presentQueue;
      std::shared_ptr<vk::raii::Queue> transferQueue;
      std::unique_ptr<vk::raii::Queue> computeQueue;

      std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;

      std::unique_ptr<vk::raii::SwapchainKHR> swapchain;

      std::vector<vk::Image> swapchainImages;
      std::vector<vk::raii::ImageView> swapchainImageViews;
      vk::Format swapchainImageFormat = vk::Format::eUndefined;
      vk::Extent2D swapchainExtent;

      std::unique_ptr<vk::raii::CommandPool> commandPool;

      std::unique_ptr<vk::raii::DescriptorPool> descriptorPool;

      std::unique_ptr<ImmediateContext> transferImmediateContext;
      std::unique_ptr<ImmediateContext> graphicsImmediateContext;

      std::unique_ptr<vk::raii::RenderPass> renderPass;
      std::unique_ptr<AbstractPipeline> pipeline;

      std::unique_ptr<Triton::Memory::Allocator> raiillocator;

      std::unique_ptr<Triton::Memory::AllocatedImage> depthImage;
      std::unique_ptr<vk::raii::ImageView> depthImageView;
      std::vector<vk::raii::Framebuffer> swapchainFramebuffers;

      std::vector<std::unique_ptr<FrameData>> frameData;

      std::unique_ptr<TextureFactory> textureFactory;
      std::unique_ptr<MeshFactory> meshFactory;

      std::unordered_map<std::string, std::unique_ptr<Mesh<Models::Vertex, uint32_t>>> meshes;
      std::vector<std::unique_ptr<Texture>> textureList;

      std::vector<std::unique_ptr<RendererBase>> renderers;
      std::unique_ptr<RendererBase> finishRenderer;

      uint32_t currentFrame = 0;
      bool framebufferResized = false;

      void createPhysicalDevice(const Instance& instance);
      void createLogicalDevice(const Instance& instance);
      void createSwapchain(const Instance& instance);
      void createSwapchainImageViews();
      void createCommandPools(const Instance& instance);
      void createDescriptorPool();
      void createAllocator(const Instance& instance);
      void createPerFrameData(const vk::raii::DescriptorSetLayout& bindlessDescriptorSetLayout,
                              const vk::raii::DescriptorSetLayout& objectDescriptorSetLayout,
                              const vk::raii::DescriptorSetLayout& perFrameDescriptorSetLayout);
      void createDepthResources();
      void createFramebuffers();
      void recreateSwapchain();
      void drawFrame();
      void recordCommandBuffer(FrameData& frameData, unsigned imageIndex) const;
   };
}