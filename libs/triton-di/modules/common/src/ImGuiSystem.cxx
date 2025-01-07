#include "cm/ImGuiSystem.hpp"
#include "tr/IWindow.hpp"

#include "vk/Instance.hpp"
#include "vk/Swapchain.hpp"

namespace tr {
ImGuiSystem::ImGuiSystem(const std::shared_ptr<IWindow>& window,
                         const std::shared_ptr<Instance>& instance,
                         std::shared_ptr<Device> newDevice,
                         const std::shared_ptr<PhysicalDevice>& physicalDevice,
                         const std::shared_ptr<queue::Graphics>& graphicsQueue,
                         const std::shared_ptr<Swapchain>& swapchain,
                         std::shared_ptr<VkResourceManager> newResourceManager)
    : resourceManager{std::move(newResourceManager)}, device{std::move(newDevice)} {
  Log.trace("Creating ImGuiSystem");

  descriptorPool = resourceManager->createDefaultDescriptorPool();

  ImGui::CreateContext();

  ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window->getNativeWindow()), true);

  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = instance->getVkInstance();
  initInfo.PhysicalDevice = *physicalDevice->getVkPhysicalDevice();
  initInfo.Device = *device->getVkDevice();
  initInfo.Queue = *graphicsQueue->getQueue();
  initInfo.DescriptorPool = **descriptorPool;
  initInfo.MinImageCount = 3;
  initInfo.ImageCount = 3;
  initInfo.UseDynamicRendering = true;
  initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  auto createInfo = vk::PipelineRenderingCreateInfo{};
  createInfo.colorAttachmentCount = 1;
  const auto format = swapchain->getImageFormat();
  createInfo.pColorAttachmentFormats = &format;

  initInfo.UseDynamicRendering = true;
  initInfo.PipelineRenderingCreateInfo = createInfo;

  ImGui_ImplVulkan_Init(&initInfo);
}

auto ImGuiSystem::setRenderCallback(std::function<void(void)> newRenderFn) -> void {
  renderFn = newRenderFn;
}

auto ImGuiSystem::render(vk::raii::CommandBuffer& commandBuffer,
                         const vk::ImageView& swapchainImageView,
                         const vk::Extent2D& swapchainExtent) -> void {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  renderFn();
  ImGui::Render();

  ZoneNamedN(imguiZone, "Render ImGui", true);
  const auto colorAttachment = vk::RenderingAttachmentInfo{
      .imageView = swapchainImageView,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eLoad,
      .storeOp = vk::AttachmentStoreOp::eStore,
  };

  const auto renderInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0}, .extent = swapchainExtent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
  };

  commandBuffer.beginRendering(renderInfo);

  auto* dd = ImGui::GetDrawData();
  if (dd != nullptr) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);
  }

  commandBuffer.endRendering();
}

ImGuiSystem::~ImGuiSystem() {
  Log.trace("Destroying ImGuiSystem");

  device->waitIdle();
  ImGui_ImplVulkan_DestroyFontsTexture();
  ImGui_ImplVulkan_Shutdown();
}
}
