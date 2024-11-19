#include "cm/ImGuiSystem.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "tr/IWindow.hpp"

namespace tr::cm {
   ImGuiSystem::ImGuiSystem(const std::shared_ptr<gfx::IGraphicsDevice>& graphicsDevice,
                            const std::shared_ptr<IWindow>& window) {
      Log.trace("Creating ImGuiSystem");
      static constexpr auto poolSizes = std::array{
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampledImage,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformTexelBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageTexelBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBufferDynamic,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBufferDynamic,
                                 .descriptorCount = 1000},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eInputAttachment,
                                 .descriptorCount = 1000}};

      constexpr vk::DescriptorPoolCreateInfo poolInfo{
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
          .maxSets = 1000,
          .poolSizeCount = poolSizes.size(),
          .pPoolSizes = poolSizes.data()};

      descriptorPool = std::make_unique<vk::raii::DescriptorPool>(
          graphicsDevice->getVulkanDevice()->createDescriptorPool(poolInfo, nullptr));

      ImGui::CreateContext();

      ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window->getNativeWindow()), true);

      ImGui_ImplVulkan_InitInfo initInfo = {};
      initInfo.Instance = **graphicsDevice->getVulkanInstance();
      initInfo.PhysicalDevice = **graphicsDevice->getPhysicalDevice();
      initInfo.Device = **graphicsDevice->getVulkanDevice();
      initInfo.Queue = **graphicsDevice->getGraphicsQueue();
      initInfo.DescriptorPool = **descriptorPool;
      initInfo.MinImageCount = 3;
      initInfo.ImageCount = 3;
      initInfo.UseDynamicRendering = true;
      initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

      auto createInfo = vk::PipelineRenderingCreateInfo{};
      createInfo.colorAttachmentCount = 1;
      const auto format = graphicsDevice->getSwapchainFormat();
      createInfo.pColorAttachmentFormats = &format;

      initInfo.UseDynamicRendering = true;
      initInfo.PipelineRenderingCreateInfo = createInfo;

      ImGui_ImplVulkan_Init(&initInfo);
   }

   auto ImGuiSystem::setRenderCallback(std::function<void(void)> newRenderFn) -> void {
      renderFn = newRenderFn;
   }

   auto ImGuiSystem::render(const std::unique_ptr<vk::raii::CommandBuffer>& commandBuffer,
                            const vk::raii::ImageView& swapchainImageView,
                            const vk::Extent2D& swapchainExtent) -> void {
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      renderFn();
      ImGui::Render();

      ZoneNamedN(imguiZone, "Render ImGui", true);
      const auto colorAttachment = vk::RenderingAttachmentInfo{
          .imageView = *swapchainImageView,
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eLoad,
          .storeOp = vk::AttachmentStoreOp::eStore,
      };

      const auto renderInfo = vk::RenderingInfo{
          .renderArea = vk::Rect2D{.offset = {0, 0}, .extent = swapchainExtent},
          .layerCount = 1,
          .colorAttachmentCount = 1,
          .pColorAttachments = &colorAttachment,
      };

      commandBuffer->beginRendering(renderInfo);

      auto* dd = ImGui::GetDrawData();
      if (dd != nullptr) {
         ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), **commandBuffer);
      }

      commandBuffer->endRendering();
   }

   ImGuiSystem::~ImGuiSystem() {
      Log.trace("Destroying ImGuiSystem");
      ImGui_ImplVulkan_DestroyFontsTexture();
      ImGui_ImplVulkan_Shutdown();
   }
}