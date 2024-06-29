#include "ImguiHelper.hpp"

#include "gfx/GraphicsDevice.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx::Gui {
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
      initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

      auto createInfo = vk::PipelineRenderingCreateInfo{};
      createInfo.colorAttachmentCount = 1;
      const auto format = graphicsDevice.getSwapchainFormat();
      createInfo.pColorAttachmentFormats = &format;

      initInfo.UseDynamicRendering = true;
      initInfo.PipelineRenderingCreateInfo = createInfo;

      ImGui_ImplVulkan_Init(&initInfo);
   }

   ImGuiHelper::~ImGuiHelper() {
      ImGui_ImplVulkan_DestroyFontsTexture();
      ImGui_ImplVulkan_Shutdown();
   }

}
