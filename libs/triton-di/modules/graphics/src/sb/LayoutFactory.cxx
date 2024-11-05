#include "sb/LayoutFactory.hpp"
#include "Layout.hpp"

namespace tr::gfx::sb {
   constexpr uint32_t MAX_TEXTURE_COUNT = 16;
   LayoutFactory::LayoutFactory(const std::shared_ptr<gfx::IGraphicsDevice>& graphicsDevice,
                                Config config)
       : config{config} {
      initBindlessLayout(graphicsDevice);
      initPerFrameLayout(graphicsDevice);
      initObjectDataLayout(graphicsDevice);
      initAnimationDataLayout(graphicsDevice);
   }

   LayoutFactory::~LayoutFactory() { // NOLINT(*-use-equals-default)
   }

   [[nodiscard]] auto LayoutFactory::getVkLayout(const LayoutHandle handle) const
       -> const vk::DescriptorSetLayout& {
      return layoutCache.at(handle)->getVkLayout();
   }

   void LayoutFactory::initBindlessLayout(const std::shared_ptr<IGraphicsDevice>& graphicsDevice) {
      constexpr auto binding = vk::DescriptorSetLayoutBinding{
          .binding = 3,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = MAX_TEXTURE_COUNT,
          .stageFlags = vk::ShaderStageFlagBits::eAll};

      static constexpr vk::DescriptorBindingFlags bindlessFlags =
          vk::DescriptorBindingFlagBits::ePartiallyBound;

      constexpr auto extendedInfo =
          vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                           .pBindingFlags = &bindlessFlags};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (config.useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{.pNext = &extendedInfo,
                                                                   .flags = flags,
                                                                   .bindingCount = 1,
                                                                   .pBindings = &binding};

      layoutCache[LayoutHandle::Bindless] =
          std::make_unique<Layout>(graphicsDevice, dslCreateInfo, "Bindless DS Layout");
   }

   void LayoutFactory::initPerFrameLayout(const std::shared_ptr<IGraphicsDevice>& graphicsDevice) {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (config.useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{.flags = flags,
                                                                .bindingCount = 1,
                                                                .pBindings = &binding};

      layoutCache[LayoutHandle::PerFrame] =
          std::make_unique<Layout>(graphicsDevice, createInfo, "Per Frame DS Layout");
   }

   void LayoutFactory::initObjectDataLayout(
       const std::shared_ptr<IGraphicsDevice>& graphicsDevice) {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (config.useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{.flags = flags,
                                                                .bindingCount = 1,
                                                                .pBindings = &binding};

      layoutCache[LayoutHandle::ObjectData] =
          std::make_unique<Layout>(graphicsDevice, createInfo, "Object Data DS Layout");
   }

   void LayoutFactory::initAnimationDataLayout(
       const std::shared_ptr<IGraphicsDevice>& graphicsDevice) {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (config.useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{.flags = flags,
                                                                .bindingCount = 1,
                                                                .pBindings = &binding};
      layoutCache[LayoutHandle::AnimationData] =
          std::make_unique<Layout>(graphicsDevice, createInfo, "Animation Data DS Layout");
   }

}