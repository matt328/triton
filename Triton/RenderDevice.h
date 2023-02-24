#pragma once
class RenderDevice {
 public:
   RenderDevice();
   ~RenderDevice();

 private:
   uint32_t framebufferWidth = 0;
   uint32_t framebufferHeight = 0;

   std::unique_ptr<vk::raii::Device> device;
   std::unique_ptr<vk::raii::Queue> graphicsQueue;
   std::unique_ptr<vk::raii::PhysicalDevice> physicalDevice;

   uint32_t graphicsFamily = 0;

   std::unique_ptr<vk::raii::SwapchainKHR> swapchain;
   std::unique_ptr<vk::raii::Semaphore> semaphore;
   std::unique_ptr<vk::raii::Semaphore> renderSemaphore;

   std::vector<std::unique_ptr<vk::raii::Image>> swapchainImages;
   std::vector<std::unique_ptr<vk::raii::ImageView>> swapchainImageViews;

   std::unique_ptr<vk::raii::CommandPool> commandPool;
   std::vector<std::unique_ptr<vk::raii::CommandBuffer>> commandBuffers;

   bool useCompute = false;

   uint32_t computeFamily = 0;
   std::unique_ptr<vk::raii::Queue> computeQueue;

   std::vector<uint32_t> deviceQueueIndices;
   std::vector<std::unique_ptr<vk::raii::Queue>> deviceQueues;

   std::unique_ptr<vk::raii::CommandBuffer> computeCommandBuffer;
   std::unique_ptr<vk::raii::CommandPool> computeCommandPool;
};
