#include "graphics/VulkanFactory.hpp"
#include "Log.hpp"
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

   std::unique_ptr<vk::raii::PipelineLayout> createPipelineLayout(
       const std::vector<ShaderStage>& stages) {

      std::unordered_map<std::string, ReflectedBinding> bindings;
      std::vector<DescriptorSetLayoutData> setLayouts;
      std::vector<vk::PushConstantRange> constant_ranges;

      for (const auto& stage : stages) {
         SpvReflectShaderModule spv_module;
         auto result = spvReflectCreateShaderModule(
             stage.code.size() * sizeof(uint32_t), stage.code.data(), &spv_module);

         uint32_t count = 0;
         result = spvReflectEnumerateDescriptorSets(&spv_module, &count, nullptr);
         if (result != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Error reflecting SPV shader code");
         }

         std::vector<SpvReflectDescriptorSet*> sets(count);
         result = spvReflectEnumerateDescriptorSets(&spv_module, &count, sets.data());
         if (result != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Error reflecting SPV shader code");
         }

         for (auto reflSet : sets) {
            DescriptorSetLayoutData layout = {};
            layout.bindings.resize(reflSet->binding_count);
            auto reflBindings = std::span(reflSet->bindings, reflSet->binding_count);

            for (uint32_t i = 0; const auto& reflBinding : reflBindings) {
               layout.bindings[i].binding = reflBinding->binding;
               layout.bindings[i].descriptorType =
                   static_cast<vk::DescriptorType>(reflBinding->descriptor_type);
               layout.bindings[i].descriptorCount = 1;
               for (uint32_t dim = 0; dim < reflBinding->array.dims_count; ++dim) {
                  layout.bindings[i].descriptorCount *= reflBinding->array.dims[dim];
               }
               layout.bindings[i].stageFlags =
                   static_cast<vk::ShaderStageFlagBits>(spv_module.shader_stage);

               ReflectedBinding reflected;
               reflected.binding = layout.bindings[i].binding;
               reflected.set = reflSet->set;
               reflected.type = layout.bindings[i].descriptorType;

               bindings[reflBinding->name] = reflected;
            }
            layout.set_number = reflSet->set;

            const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{
                .bindingCount = reflSet->binding_count, .pBindings = layout.bindings.data()};

            layout.create_info = dslCreateInfo;

            setLayouts.push_back(layout);
         }
      }
      Core::Log::core->debug("SetLayouts: {}", setLayouts.size());

      return std::make_unique<vk::raii::PipelineLayout>(nullptr);
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