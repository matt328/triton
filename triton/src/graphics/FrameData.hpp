#pragma once
#include "core/vma_raii.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace vk::raii {
   class DescriptorPool;
   class Device;
   class CommandPool;
   class DescriptorSet;
   class DescriptorSetLayout;
   class Fence;
   class Semaphore;
   class CommandBuffer;
}

class FrameData {
 public:
   FrameData(const vk::raii::Device& device,
             const vk::raii::PhysicalDevice& physicalDevice,
             const vk::raii::CommandPool& commandPool,
             const vma::raii::Allocator& raiillocator,
             const vk::raii::DescriptorPool& descriptorPool,
             const vk::raii::DescriptorSetLayout& descriptorSetLayout,
             vk::DescriptorImageInfo textureImageInfo,
             const vk::raii::Queue& queue,
             const std::string_view name);
   ~FrameData();

   FrameData(const FrameData&) = delete;
   FrameData(FrameData&&) = delete;
   FrameData& operator=(const FrameData&) = delete;
   FrameData& operator=(FrameData&&) = delete;

   [[nodiscard]] const vk::raii::CommandBuffer& getCommandBuffer() const {
      return *commandBuffer;
   };

   [[nodiscard]] const vk::raii::Semaphore& getImageAvailableSemaphore() const {
      return *imageAvailableSemaphore;
   };

   [[nodiscard]] const vk::raii::Semaphore& getRenderFinishedSemaphore() const {
      return *renderFinishedSemaphore;
   };

   [[nodiscard]] const vk::raii::Fence& getInFlightFence() const {
      return *inFlightFence;
   };

   [[nodiscard]] const vma::raii::AllocatedBuffer& getObjectMatricesBuffer() const {
      return *objectMatricesBuffer;
   };

   [[nodiscard]] const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const {
      return *descriptorSetLayout;
   };

   [[nodiscard]] const vk::raii::DescriptorSet& getDescriptorSet() const {
      return *descriptorSet;
   };

   [[nodiscard]] tracy::VkCtx* getTracyContext() const {
      return tracyContext;
   }

   std::vector<std::string> renderables; // NOLINT this is bad and I feel bad but it's temporary

 private:
   std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;
   std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore = nullptr;
   std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore = nullptr;
   std::unique_ptr<vk::raii::Fence> inFlightFence = nullptr;

   std::unique_ptr<vma::raii::AllocatedBuffer> objectMatricesBuffer = nullptr;

   std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout = nullptr;
   std::unique_ptr<vk::raii::DescriptorSet> descriptorSet = nullptr;
   tracy::VkCtx* tracyContext = nullptr;
};
