#include "gfx/QueueTypes.hpp"

namespace tr::gfx::queue {
   Graphics::Graphics(const std::shared_ptr<Device>& device) {
      queue = device->createGraphicsQueue();
   }

   auto Graphics::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

   Present::Present(const std::shared_ptr<Device>& device) {
      queue = device->createPresentQueue();
   }

   auto Present::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

   Transfer::Transfer(const std::shared_ptr<Device>& device) {
      queue = device->createTransferQueue();
   }

   auto Transfer::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

   Compute::Compute(const std::shared_ptr<Device>& device) {
      queue = device->createComputeQueue();
   }

   auto Compute::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

}