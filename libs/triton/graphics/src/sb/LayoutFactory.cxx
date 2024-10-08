#include "LayoutFactory.hpp"
#include "Layout.hpp"
#include "GraphicsDevice.hpp"

namespace tr::gfx::ds {
   constexpr uint32_t MAX_TEXTURE_COUNT = 16;
   LayoutFactory::LayoutFactory(const GraphicsDevice& device, const bool useDescriptorBuffers)
       : useDescriptorBuffers{useDescriptorBuffers} {
      initBindlessLayout(device);
      initPerFrameLayout(device);
      initObjectDataLayout(device);
      initAnimationDataLayout(device);
   }

   LayoutFactory::~LayoutFactory() { // NOLINT(*-use-equals-default)
   }

   [[nodiscard]] const vk::DescriptorSetLayout& LayoutFactory::getVkLayout(
       const LayoutHandle handle) const {
      return layoutCache.at(handle)->getVkLayout();
   }

   void LayoutFactory::initBindlessLayout(const GraphicsDevice& device) {
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
      if (useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{.pNext = &extendedInfo,
                                                                   .flags = flags,
                                                                   .bindingCount = 1,
                                                                   .pBindings = &binding};

      layoutCache[LayoutHandle::Bindless] =
          std::make_unique<Layout>(device, dslCreateInfo, "Bindless DS Layout");
   }

   void LayoutFactory::initPerFrameLayout(const GraphicsDevice& device) {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{.flags = flags,
                                                                .bindingCount = 1,
                                                                .pBindings = &binding};

      layoutCache[LayoutHandle::PerFrame] =
          std::make_unique<Layout>(device, createInfo, "Per Frame DS Layout");
   }

   void LayoutFactory::initObjectDataLayout(const GraphicsDevice& device) {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{.flags = flags,
                                                                .bindingCount = 1,
                                                                .pBindings = &binding};

      layoutCache[LayoutHandle::ObjectData] =
          std::make_unique<Layout>(device, createInfo, "Object Data DS Layout");
   }

   void LayoutFactory::initAnimationDataLayout(const GraphicsDevice& device) {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      vk::DescriptorSetLayoutCreateFlags flags{};
      if (useDescriptorBuffers) {
         flags |= vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
      }

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{.flags = flags,
                                                                .bindingCount = 1,
                                                                .pBindings = &binding};
      layoutCache[LayoutHandle::AnimationData] =
          std::make_unique<Layout>(device, createInfo, "Animation Data DS Layout");
   }

}