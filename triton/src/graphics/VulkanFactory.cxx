#include "graphics/VulkanFactory.hpp"
#include "Log.hpp"
#include <spirv.hpp>
#include <vulkan/vulkan_enums.hpp>
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
       const vk::raii::Device& device,
       const std::vector<vk::raii::ImageView>& swapchainImageViews,
       const vk::ImageView& depthImageView,
       const vk::Extent2D swapchainExtent,
       const vk::raii::RenderPass& renderPass) {

      auto swapchainFramebuffers = std::vector<std::unique_ptr<vk::raii::Framebuffer>>{};
      swapchainFramebuffers.reserve(swapchainImageViews.size());
      std::array<vk::ImageView, 2> attachments;

      for (const auto& imageView : swapchainImageViews) {
         attachments[0] = *imageView;
         attachments[1] = depthImageView;

         const auto framebufferCreateInfo =
             vk::FramebufferCreateInfo{.renderPass = *renderPass,
                                       .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                       .pAttachments = attachments.data(),
                                       .width = swapchainExtent.width,
                                       .height = swapchainExtent.height,
                                       .layers = 1};
         swapchainFramebuffers.emplace_back(std::make_unique<vk::raii::Framebuffer>(
             device.createFramebuffer(framebufferCreateInfo)));
      }
      return swapchainFramebuffers;
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createSSBODescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto ssboBinding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo =
          vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &ssboBinding};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(createInfo));
   }

   std::unique_ptr<vk::raii::DescriptorSetLayout> createBindlessDescriptorSetLayout(
       const vk::raii::Device& device) {
      const auto textureBinding = vk::DescriptorSetLayoutBinding{
          .binding = 3,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 16,
          .stageFlags = vk::ShaderStageFlagBits::eAll,
          .pImmutableSamplers = nullptr};

      const auto bindlessFlags = vk::DescriptorBindingFlagBits::ePartiallyBound |
                                 vk::DescriptorBindingFlagBits::eUpdateAfterBind;

      const auto extendedInfo = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{
          .bindingCount = 1, .pBindingFlags = &bindlessFlags};

      const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{
          .pNext = &extendedInfo,
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
          .bindingCount = 1,
          .pBindings = &textureBinding};

      return std::make_unique<vk::raii::DescriptorSetLayout>(
          device.createDescriptorSetLayout(dslCreateInfo));
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
}