#include "gfx/QueueTypes.hpp"

namespace tr::gfx::queue {
   Graphics::Graphics(std::unique_ptr<vk::raii::Queue> newQueue) : queue{std::move(newQueue)} {
   }

   auto Graphics::getQueue() const -> vk::raii::Queue& {
      return *queue;
   }

}