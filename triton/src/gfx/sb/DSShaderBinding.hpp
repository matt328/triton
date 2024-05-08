#pragma once

#include "ShaderBinding.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace tr::gfx::sb {
   class DSShaderBinding : public ShaderBinding {
    public:
      DSShaderBinding(const vk::raii::Device& device, vk::DescriptorType descriptorType);
      void bindBuffer(const int binding, const mem::Buffer& buffer, const size_t size) override;
      void update() override;

    private:
      const vk::raii::Device& device;
      std::unique_ptr<vk::raii::DescriptorSet> vkDescriptorSet;
      vk::DescriptorType descriptorType;
   };
}