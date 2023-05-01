#include "graphics/VulkanFactory.hpp"
#include "Log.hpp"
#include "graphics/renderer/RendererBase.hpp"
#include <spirv.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Graphics::Utils {

   vk::Format findSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice,
                                  const std::vector<vk::Format>& candidates,
                                  const vk::ImageTiling tiling,
                                  const vk::FormatFeatureFlags features) {
      for (const auto format : candidates) {
         auto props = physicalDevice.getFormatProperties(format);
         if (tiling == vk::ImageTiling::eLinear &&
                 (props.linearTilingFeatures & features) == features ||
             tiling == vk::ImageTiling::eOptimal &&
                 (props.optimalTilingFeatures & features) == features) {
            return format;
         }
      }
      throw std::runtime_error("Failed to find supported format");
   }

   vk::Format findDepthFormat(const vk::raii::PhysicalDevice& physicalDevice) {
      return findSupportedFormat(
          physicalDevice,
          {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
          vk::ImageTiling::eOptimal,
          vk::FormatFeatureFlagBits::eDepthStencilAttachment);
   }

   std::vector<std::unique_ptr<vk::raii::Framebuffer>> createFramebuffers(
       const FramebufferInfo& framebufferInfo, const vk::raii::RenderPass& renderPass) {

      auto swapchainFramebuffers = std::vector<std::unique_ptr<vk::raii::Framebuffer>>{};
      swapchainFramebuffers.reserve(framebufferInfo.swapchainImageViews.size());
      std::array<vk::ImageView, 2> attachments;

      for (const auto& imageView : framebufferInfo.swapchainImageViews) {
         attachments[0] = *imageView;
         attachments[1] = *framebufferInfo.depthImageView;

         const auto framebufferCreateInfo =
             vk::FramebufferCreateInfo{.renderPass = *renderPass,
                                       .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                       .pAttachments = attachments.data(),
                                       .width = framebufferInfo.swapchainExtent.width,
                                       .height = framebufferInfo.swapchainExtent.height,
                                       .layers = 1};
         swapchainFramebuffers.emplace_back(std::make_unique<vk::raii::Framebuffer>(
             framebufferInfo.device.createFramebuffer(framebufferCreateInfo)));
      }
      return swapchainFramebuffers;
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createDescriptorSetLayout(
       const vk::raii::Device* device, const std::vector<ShaderStage>& stages) {
      auto bindings = std::vector<vk::DescriptorSetLayoutBinding>{};

      for (const auto& stage : stages) {
         spirv_cross::Compiler comp(std::move(stage.code));
         const auto resources = comp.get_shader_resources();

         for (const auto& ubo : resources.uniform_buffers) {
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = comp.get_decoration(ubo.id, spv::DecorationBinding),
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = stage.stages});
         }

         for (const auto& sampledImage : resources.sampled_images) {
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = comp.get_decoration(sampledImage.id, spv::DecorationBinding),
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = stage.stages});
         }
      }

      const auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{
          .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data()};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
   }

   vk::raii::RenderPass colorAndDepthRenderPass(const RenderPassCreateInfo& createInfo) {
      const bool first = createInfo.flags & eRenderPassBit_First;
      const bool last = createInfo.flags & eRenderPassBit_Last;

      const vk::AttachmentDescription colorAttachment{
          .format = createInfo.swapchainFormat,
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp =
              createInfo.clearColor ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout =
              first ? vk::ImageLayout::eUndefined : vk::ImageLayout::eColorAttachmentOptimal,
          .finalLayout =
              last ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eColorAttachmentOptimal};

      constexpr auto colorAttachmentRef =
          vk::AttachmentReference{.attachment = 0, // index into the color attachments array
                                  .layout = vk::ImageLayout::eColorAttachmentOptimal};

      const auto depthAttachment = vk::AttachmentDescription{
          .format = findDepthFormat(*createInfo.physicalDevice),
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp =
              createInfo.clearDepth ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout = createInfo.clearDepth ? vk::ImageLayout::eUndefined
                                                 : vk::ImageLayout::eDepthStencilAttachmentOptimal,
          .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

      constexpr auto depthAttachmentRef = vk::AttachmentReference{
          .attachment = 1, .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

      const auto attachments = std::array{colorAttachment, depthAttachment};

      const auto subpass =
          vk::SubpassDescription{.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                 .colorAttachmentCount = 1,
                                 .pColorAttachments = &colorAttachmentRef,
                                 .pDepthStencilAttachment = &depthAttachmentRef};

      const auto dependency =
          vk::SubpassDependency{.srcSubpass = VK_SUBPASS_EXTERNAL,
                                .dstSubpass = 0,
                                .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                                vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                                vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                .srcAccessMask = vk::AccessFlagBits::eNone,
                                .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
                                                 vk::AccessFlagBits::eColorAttachmentWrite};

      const auto renderPassCreateInfo =
          vk::RenderPassCreateInfo{.attachmentCount = static_cast<uint32_t>(attachments.size()),
                                   .pAttachments = attachments.data(),
                                   .subpassCount = 1,
                                   .pSubpasses = &subpass,
                                   .dependencyCount = 1,
                                   .pDependencies = &dependency};

      return createInfo.device->createRenderPass(renderPassCreateInfo);
   }

   QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& possibleDevice,
                                        const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
      QueueFamilyIndices queueFamilyIndices;

      const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

      for (int i = 0; const auto& queueFamily : queueFamilies) {
         if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;
         }

         if (possibleDevice.getSurfaceSupportKHR(i, **surface)) {
            queueFamilyIndices.presentFamily = i;
         }

         if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            queueFamilyIndices.transferFamily = i;
         }

         if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
            queueFamilyIndices.computeFamily = i;
         }

         if (queueFamilyIndices.isComplete()) {
            break;
         }
         i++;
      }
      return queueFamilyIndices;
   }
}