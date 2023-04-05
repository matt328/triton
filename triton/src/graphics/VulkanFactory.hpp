#pragma once

#include <vulkan/vulkan_raii.hpp>
namespace Graphics::Utils {

   enum eRenderPassBit : uint8_t {
      eRenderPassBit_First = 0x01,
      eRenderPassBit_Last = 0x02,
   };

   struct RenderPassCreateInfo {
      const vk::raii::Device* device = nullptr;
      const vk::raii::PhysicalDevice* physicalDevice = nullptr;
      vk::Format swapchainFormat = vk::Format::eR8G8B8A8Srgb;
      bool clearColor = false;
      bool clearDepth = false;
      uint8_t flags = 0;
   };

   struct ShaderStage {
      const vk::raii::ShaderModule* shaderModule;
      vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAll;
      std::vector<uint32_t> code;
   };

   struct DescriptorSetLayoutData {
      uint32_t set_number;
      vk::DescriptorSetLayoutCreateInfo create_info;
      std::vector<vk::DescriptorSetLayoutBinding> bindings;
   };

   struct ReflectedBinding {
      uint32_t set;
      uint32_t binding;
      vk::DescriptorType type;
   };

   vk::Format findSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice,
                                  const std::vector<vk::Format>& candidates,
                                  const vk::ImageTiling tiling,
                                  const vk::FormatFeatureFlags features);

   vk::Format findDepthFormat(const vk::raii::PhysicalDevice& physicalDevice);

   vk::raii::RenderPass colorAndDepthRenderPass(const RenderPassCreateInfo& createInfo);

   std::vector<std::unique_ptr<vk::raii::Framebuffer>> createFramebuffers(
       const vk::raii::Device& device,
       const std::vector<vk::raii::ImageView>& swapchainImageViews,
       const vk::ImageView& depthImageView,
       const vk::Extent2D swapchainExtent,
       const vk::raii::RenderPass& renderPass);

   std::unique_ptr<vk::raii::DescriptorSetLayout> createDescriptorSetLayout(
       const vk::raii::Device* device, const std::vector<ShaderStage>& stages);

   void createVertexAttributes(const std::vector<ShaderStage>& shaderStages);

   static void print_resources(const spirv_cross::Compiler& compiler,
                               const char* tag,
                               const spirv_cross::SmallVector<spirv_cross::Resource>& resources);
}
