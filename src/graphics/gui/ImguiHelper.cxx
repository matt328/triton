#include "ImguiHelper.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Triton::Graphics::Gui {
   ImGuiHelper::ImGuiHelper(const GraphicsDevice& graphicsDevice, GLFWwindow* window) {
      const auto poolSizes = std::array{
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

      const vk::DescriptorPoolCreateInfo poolInfo{
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
          .maxSets = 1000,
          .poolSizeCount = poolSizes.size(),
          .pPoolSizes = poolSizes.data()};

      descriptorPool = std::make_unique<vk::raii::DescriptorPool>(
          graphicsDevice.getVulkanDevice().createDescriptorPool(poolInfo, nullptr));

      ImGui::CreateContext();

      ImGui_ImplGlfw_InitForVulkan(window, true);

      ImGui_ImplVulkan_InitInfo initInfo = {};
      initInfo.Instance = graphicsDevice.getVkInstance();
      initInfo.PhysicalDevice = *graphicsDevice.getPhysicalDevice();
      initInfo.Device = *graphicsDevice.getVulkanDevice();
      initInfo.Queue = *graphicsDevice.getGraphicsQueue();
      initInfo.DescriptorPool = **descriptorPool;
      initInfo.MinImageCount = 3;
      initInfo.ImageCount = 3;
      initInfo.UseDynamicRendering = true;
      initInfo.ColorAttachmentFormat = static_cast<VkFormat>(graphicsDevice.getSwapchainFormat());
      initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

      ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE);

      auto& io = ImGui::GetIO();
      io.Fonts->AddFontFromFileTTF("assets\\fonts\\Roboto-Regular.ttf", 18);

      ImGui_ImplVulkan_CreateFontsTexture();
   }

   ImGuiHelper::~ImGuiHelper() {
      ImGui_ImplVulkan_DestroyFontsTexture();
      ImGui_ImplVulkan_Shutdown();
   }

}