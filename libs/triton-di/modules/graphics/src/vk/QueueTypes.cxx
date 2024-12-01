#include "gfx/QueueTypes.hpp"

namespace tr::gfx::queue {
   Graphics::Graphics(const std::shared_ptr<Device>& device) {
      queue = device->createGraphicsQueue();
   }

   auto Graphics::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

   Present::Present(std::unique_ptr<vk::raii::Queue> newQueue) : queue{std::move(newQueue)} {
   }

   auto Present::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

   Transfer::Transfer(std::unique_ptr<vk::raii::Queue> newQueue) : queue{std::move(newQueue)} {
   }

   auto Transfer::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

   Compute::Compute(std::unique_ptr<vk::raii::Queue> newQueue) : queue{std::move(newQueue)} {
   }

   auto Compute::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

}