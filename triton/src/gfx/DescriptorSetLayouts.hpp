#pragma once

namespace tr::gfx {
   namespace PerFrameDescriptorSet {
      constexpr auto binding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      extern const vk::DescriptorSetLayoutCreateInfo LayoutCreateInfo;
   }

   namespace ObjectDataDescriptorSet {
      constexpr auto ssboBinding =
          vk::DescriptorSetLayoutBinding{.binding = 0,
                                         .descriptorType = vk::DescriptorType::eStorageBuffer,
                                         .descriptorCount = 1,
                                         .stageFlags = vk::ShaderStageFlagBits::eVertex};
      extern const vk::DescriptorSetLayoutCreateInfo LayoutCreateInfo;
   }

   namespace TextureDescriptorSet {
      const auto textureBinding = vk::DescriptorSetLayoutBinding{
          .binding = 3,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 128,
          .stageFlags = vk::ShaderStageFlagBits::eAll,
          .pImmutableSamplers = nullptr};

      const auto bindlessFlags = vk::DescriptorBindingFlagBits::ePartiallyBound |
                                 vk::DescriptorBindingFlagBits::eUpdateAfterBind;

      const auto extendedInfo =
          vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT{.bindingCount = 1,
                                                           .pBindingFlags = &bindlessFlags};

      extern const vk::DescriptorSetLayoutCreateInfo LayoutCreateInfo;
   }
}

const auto tr::gfx::PerFrameDescriptorSet::LayoutCreateInfo =
    vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &binding};

const auto tr::gfx::ObjectDataDescriptorSet::LayoutCreateInfo =
    vk::DescriptorSetLayoutCreateInfo{.bindingCount = 1, .pBindings = &ssboBinding};

const auto tr::gfx::TextureDescriptorSet::LayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{
    .pNext = &extendedInfo,
    .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
    .bindingCount = 1,
    .pBindings = &textureBinding};