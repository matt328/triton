#include "ImGuiPass.hpp"
#include "api/fx/IWindow.hpp"
#include "gfx/QueueTypes.hpp"
#include "img/ImageManager.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "task/Frame.hpp"
#include "vk/core/Device.hpp"

namespace tr {

ImGuiPass::ImGuiPass(std::shared_ptr<ImageManager> newImageManager,
                     std::shared_ptr<ContextFactory> newDrawContextFactory,
                     std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                     std::shared_ptr<PipelineFactory> newPipelineFactory,
                     std::shared_ptr<Device> newDevice,
                     const std::shared_ptr<IWindow>& window,
                     const std::shared_ptr<Instance>& instance,
                     const std::shared_ptr<PhysicalDevice>& physicalDevice,
                     const std::shared_ptr<queue::Graphics>& graphicsQueue,
                     ImGuiPassCreateInfo createInfo,
                     PassId newPassId)
    : imageManager{std::move(newImageManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      aliasRegistry{std::move(newAliasRegistry)},
      pipelineFactory{std::move(newPipelineFactory)},
      device{std::move(newDevice)},
      colorAlias{createInfo.colorImage},
      id{newPassId} {
  Log.trace("Creating ImGuiPass");

  constexpr auto poolSize = std::array{
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 3 * 10},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                             .descriptorCount = 3 * 100},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = 3 * 10},
      vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 3 * 10},
  };

  const vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
               vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT,
      .maxSets = 3 * 10 * static_cast<uint32_t>(poolSize.size()),
      .poolSizeCount = poolSize.size(),
      .pPoolSizes = poolSize.data()};

  descriptorPool = std::make_unique<vk::raii::DescriptorPool>(
      device->getVkDevice().createDescriptorPool(poolInfo, nullptr));

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

  const auto colorHandle = aliasRegistry->getHandle(colorAlias);
  const auto colorFormat = imageManager->getImageMetadata(colorHandle).format;

  auto pipelineCreateInfo = vk::PipelineRenderingCreateInfo{};
  pipelineCreateInfo.colorAttachmentCount = 1;
  pipelineCreateInfo.pColorAttachmentFormats = &colorFormat;

  initInfo.UseDynamicRendering = true;
  initInfo.PipelineRenderingCreateInfo = pipelineCreateInfo;

  ImGuiIO& io = ImGui::GetIO();
  // NOLINTNEXTLINE
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplVulkan_Init(&initInfo);
}

ImGuiPass::~ImGuiPass() {
  device->waitIdle();
  ImGui_ImplVulkan_DestroyFontsTexture();
  ImGui_ImplVulkan_Shutdown();
}

[[nodiscard]] auto ImGuiPass::getId() const -> PassId {
  return PassId::ImGui;
}

auto ImGuiPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  const auto imageHandle = aliasRegistry->getHandle(colorAlias);
  const auto& image = imageManager->getImage(frame->getLogicalImage(imageHandle));
  const auto extent = image.getExtent();
  const auto colorAttachment = vk::RenderingAttachmentInfo{
      .imageView = image.getImageView(),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = {vk::ClearColorValue{std::array<float, 4>{0.f, 0.f, 0.f, 0.f}}},
  };

  const auto renderInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0}, .extent = extent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
  };

  cmdBuffer.beginRendering(renderInfo);

  for (const auto& contextHandle : drawableContexts) {
    auto& context = drawContextFactory->getDispatchContext(contextHandle);
    context->dispatch(frame, cmdBuffer);
  }

  cmdBuffer.endRendering();
}

auto ImGuiPass::registerDispatchContext(Handle<IDispatchContext> handle) -> void {
  drawableContexts.push_back(handle);
}

[[nodiscard]] auto ImGuiPass::getGraphInfo() const -> PassGraphInfo {
  auto graphInfo = PassGraphInfo{
      .imageWrites = {
          ImageUsageInfo{
              .alias = colorAlias,
              .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
              .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eColorAttachmentOptimal,
              .clearValue = vk::ClearValue{.color = {std::array<float, 4>{0.f, 0.f, 0.f, 1.f}}},
          },
      }};

  for (const auto& handle : drawableContexts) {
    const auto& contextInfo = drawContextFactory->getDispatchContext(handle)->getGraphInfo();
    graphInfo.bufferReads.insert(contextInfo.bufferReads.begin(), contextInfo.bufferReads.end());
    graphInfo.bufferWrites.insert(contextInfo.bufferWrites.begin(), contextInfo.bufferWrites.end());
    graphInfo.imageReads.insert(contextInfo.imageReads.begin(), contextInfo.imageReads.end());
    graphInfo.imageWrites.insert(contextInfo.imageWrites.begin(), contextInfo.imageWrites.end());
  }

  return graphInfo;
}
}
