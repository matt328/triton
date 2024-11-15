#include "DepthResources.hpp"

#include "gfx/IGraphicsDevice.hpp"
#include "mem/Image.hpp"

namespace tr::gfx {
   DepthResources::DepthResources(const std::shared_ptr<IGraphicsDevice>& graphicsDevice) {
      const auto depthFormat = graphicsDevice->findDepthFormat();

      const auto imageCreateInfo =
          vk::ImageCreateInfo{.imageType = vk::ImageType::e2D,
                              .format = depthFormat,
                              .extent = vk::Extent3D{graphicsDevice->DrawImageExtent2D.width,
                                                     graphicsDevice->DrawImageExtent2D.height,
                                                     1},
                              .mipLevels = 1,
                              .arrayLayers = 1,
                              .samples = vk::SampleCountFlagBits::e1,
                              .tiling = vk::ImageTiling::eOptimal,
                              .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
                              .sharingMode = vk::SharingMode::eExclusive,
                              .initialLayout = vk::ImageLayout::eUndefined};
      constexpr auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly};

      depthImage = graphicsDevice->createImage(imageCreateInfo, allocationCreateInfo, "Depth");

      constexpr auto range =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eDepth,
                                    .levelCount = 1,
                                    .layerCount = 1};

      const auto viewInfo = vk::ImageViewCreateInfo{.image = depthImage->getImage(),
                                                    .viewType = vk::ImageViewType::e2D,
                                                    .format = depthFormat,
                                                    .subresourceRange = range};
      // TODO(matt) finish abstracting away vulkan device behind graphicsDevice
      depthImageView = std::make_shared<vk::raii::ImageView>(
          graphicsDevice->getVulkanDevice()->createImageView(viewInfo));
   }

}
