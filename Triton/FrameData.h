#pragma once
#include <memory>


namespace vk {
   namespace raii {
      class DescriptorSet;
      class DescriptorSetLayout;
      class Fence;
      class Semaphore;
      class CommandBuffer;
   }
}

struct FrameData {
   std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = nullptr;
   std::unique_ptr<vk::raii::Semaphore> imageAvailableSemaphore = nullptr;
   std::unique_ptr<vk::raii::Semaphore> renderFinishedSemaphore = nullptr;
   std::unique_ptr<vk::raii::Fence> inFlightFence = nullptr;

   std::unique_ptr<AllocatedBuffer> uniformBuffer = nullptr;
   void* uniformBufferData;

   std::unique_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout;
   std::unique_ptr<vk::raii::DescriptorSet> descriptorSet;
};
