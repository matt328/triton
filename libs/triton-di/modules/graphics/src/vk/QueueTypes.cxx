#include "gfx/QueueTypes.hpp"

namespace tr::queue {
Graphics::Graphics(const std::shared_ptr<Device>& device)
    : queueFamilyIndex(device->getGraphicsQueueFamily()) {
  queue = device->createGraphicsQueue();
}

auto Graphics::getQueue() const -> vk::raii::Queue& {
  return *queue;
}

auto Graphics::getFamily() const -> uint32_t {
  return queueFamilyIndex;
}

Present::Present(const std::shared_ptr<Device>& device)
    : queueFamilyIndex(device->getPresentQueueFamily()) {
  queue = device->createPresentQueue();
}

auto Present::getQueue() const -> vk::raii::Queue& {
  return *queue;
}

auto Present::getFamily() const -> uint32_t {
  return queueFamilyIndex;
}

Transfer::Transfer(const std::shared_ptr<Device>& device)
    : queueFamilyIndex(device->getTransferQueueFamily()) {
  queue = device->createTransferQueue();
}

auto Transfer::getQueue() const -> vk::raii::Queue& {
  return *queue;
}

auto Transfer::getFamily() const -> uint32_t {
  return queueFamilyIndex;
}

Compute::Compute(const std::shared_ptr<Device>& device)
    : queueFamilyIndex(device->getComputeQueueFamily()) {
  queue = device->createComputeQueue();
}

auto Compute::getQueue() const -> vk::raii::Queue& {
  return *queue;
}

auto Compute::getFamily() const -> uint32_t {
  return queueFamilyIndex;
}

}
