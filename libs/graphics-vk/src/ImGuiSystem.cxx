#include "api/gfx/ImGuiSystem.hpp"
#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "imgui.h"
#include "api/fx/IWindow.hpp"

#include "vk/core/Instance.hpp"
#include "vk/core/Swapchain.hpp"

namespace tr {
ImGuiSystem::ImGuiSystem([[maybe_unused]] const std::shared_ptr<IWindow>& window,
                         const std::shared_ptr<Instance>& instance,
                         std::shared_ptr<Device> newDevice,
                         const std::shared_ptr<PhysicalDevice>& physicalDevice,
                         const std::shared_ptr<queue::Graphics>& graphicsQueue,
                         const std::shared_ptr<Swapchain>& swapchain,
                         std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar)
    : device{std::move(newDevice)}, guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)} {
  Log.trace("Creating ImGuiSystem");

  // descriptorPool = resourceManager->createDefaultDescriptorPool();

  ImGui::CreateContext();

  // ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(window->getNativeWindow()), true);

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

  ImGuiIO& io = ImGui::GetIO();
  // NOLINTNEXTLINE
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplVulkan_Init(&initInfo);
}

auto ImGuiSystem::render(vk::raii::CommandBuffer& commandBuffer,
                         const vk::ImageView& swapchainImageView,
                         const vk::Extent2D& swapchainExtent) -> void {
  ZoneNamedN(imguiZone, "Render ImGui", true);

  {
    ZoneNamedN(var, "ImGui::NewFrame", true);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  guiCallbackRegistrar->render();

  {
    ZoneNamedN(var, "ImGui::Render", true);
    ImGui::Render();
  }

  {
    ZoneNamedN(var, "ImGuiRecordCommandBuffer", true);
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
      ImGui_ImplVulkan_RenderDrawData(dd, *commandBuffer);
    }

    commandBuffer.endRendering();
  }
}

ImGuiSystem::~ImGuiSystem() {
  Log.trace("Destroying ImGuiSystem");

  device->waitIdle();
  ImGui_ImplVulkan_DestroyFontsTexture();
  ImGui_ImplVulkan_Shutdown();
}
}
