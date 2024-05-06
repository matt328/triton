#include "LayoutFactory.hpp"
#include "Layout.hpp"
#include "gfx/GraphicsDevice.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace tr::gfx::ds {
   LayoutFactory::LayoutFactory(const GraphicsDevice& device) {
      initBindlessLayout(device);
      initPerFrameLayout(device);
      initObjectDataLayout(device);
      initAnimationDataLayout(device);
   }

   LayoutFactory::~LayoutFactory() {
   }

   [[nodiscard]] const vk::DescriptorSetLayout& LayoutFactory::getVkLayout(
       LayoutHandle handle) const {
      return layoutCache.at(handle)->getVkLayout();
   }

   void LayoutFactory::initBindlessLayout(const GraphicsDevice& device) {
      const auto binding = vk::DescriptorSetLayoutBinding{
          .binding = 3,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 128,
          .stageFlags = vk::ShaderStageFlagBits::eAll};

      const vk::DescriptorBindingFlags bindlessFlags =
          vk::DescriptorBindingFlagBits::ePartiallyBound;

      const auto extendedInfo =
          vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                           .pBindingFlags = &bindlessFlags};

      const auto dslCreateInfo = vk::DescriptorSetLayoutCreateInfo{
          .pNext = &extendedInfo,
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
          .bindingCount = 1,
          .pBindings = &binding};

      layoutCache[LayoutHandle::Bindless] = std::make_unique<Layout>(device, dslCreateInfo);
   }

   void LayoutFactory::initPerFrameLayout(const GraphicsDevice& device) {
      const auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};

      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
          .bindingCount = 1,
          .pBindings = &binding};

      layoutCache[LayoutHandle::PerFrame] = std::make_unique<Layout>(device, createInfo);
   }

   void LayoutFactory::initObjectDataLayout(const GraphicsDevice& device) {
      const auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
          .bindingCount = 1,
          .pBindings = &binding};

      layoutCache[LayoutHandle::ObjectData] = std::make_unique<Layout>(device, createInfo);
   }

   void LayoutFactory::initAnimationDataLayout(const GraphicsDevice& device) {
      const auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      const auto createInfo = vk::DescriptorSetLayoutCreateInfo{
          .flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
          .bindingCount = 1,
          .pBindings = &binding};
      layoutCache[LayoutHandle::AnimationData] = std::make_unique<Layout>(device, createInfo);
   }

}