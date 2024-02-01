#pragma once

#include "../GraphicsDevice.hpp"
#include "Vulkan.hpp"

namespace Triton::Game::Graphics::Helpers {
   std::unique_ptr<vk::raii::RenderPass> createBasicRenderPass(
       const GraphicsDevice& graphicsDevice) {
      const vk::AttachmentDescription colorAttachment{
          .format = graphicsDevice.getSwapchainFormat(),
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout = vk::ImageLayout::eUndefined,
          .finalLayout = vk::ImageLayout::ePresentSrcKHR,
      };

      constexpr auto colorAttachmentRef =
          vk::AttachmentReference{.attachment = 0, // index into the color attachments array
                                  .layout = vk::ImageLayout::eColorAttachmentOptimal};

      const auto depthAttachment = vk::AttachmentDescription{
          .format = Helpers::findDepthFormat(graphicsDevice.getPhysicalDevice()),
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout = vk::ImageLayout::eUndefined,
          .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

      constexpr auto depthAttachmentRef =
          vk::AttachmentReference{.attachment = 1,
                                  .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};

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

      return std::make_unique<vk::raii::RenderPass>(
          graphicsDevice.getVulkanDevice().createRenderPass(renderPassCreateInfo));
   }
}