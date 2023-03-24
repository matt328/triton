#pragma once
#include "vma_raii.hpp"

namespace vk {
   namespace raii {
      class DescriptorPool;
      class Device;
      class CommandPool;
      class DescriptorSet;
      class DescriptorSetLayout;
      class Fence;
      class Semaphore;
      class CommandBuffer;
   }
}

class FrameData {
 public:
   FrameData(const vk::raii::Device& device,
             const vk::raii::CommandPool& commandPool,
             const vma::raii::Allocator& raiillocator,
             const vk::raii::DescriptorPool& descriptorPool,
             const vk::raii::DescriptorSetLayout& descriptorSetLayout,
             const vk::DescriptorImageInfo textureImageInfo);
   ~FrameData();

   const vk::raii::CommandBuffer& getCommandBuffer() const {
      return *commandBuffer;
   };

   const vk::raii::Semaphore& getImageAvailableSemaphore() const {
      return *imageAvailableSemaphore;
   };

   const vk::raii::Semaphore& getRenderFinishedSemaphore() const {
      return *renderFinishedSemaphore;
   };

   const vk::raii::Fence& getInFlightFence() const {
      return *inFlightFence;
   };

   const vma::raii::AllocatedBuffer& getObjectMatricesBuffer() const {
      return *objectMatricesBuffer;
   };

   const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const {
      return *descriptorSetLayout;
   };

   const vk::raii::DescriptorSet& getDescriptorSet() const {
      return *descriptorSet;
   };

   std::vector<std::string> renderables;

 private:
   std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;
   std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore = nullptr;
   std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore = nullptr;
   std::unique_ptr<vk::raii::Fence> inFlightFence = nullptr;

   std::unique_ptr<vma::raii::AllocatedBuffer> objectMatricesBuffer = nullptr;

   std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout = nullptr;
   std::unique_ptr<vk::raii::DescriptorSet> descriptorSet = nullptr;
};
