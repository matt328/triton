#include "gfx/QueueTypes.hpp"

namespace tr::queue {
Graphics::Graphics(const std::shared_ptr<Device>& device)
    : queueFamilyIndex(device->getGraphicsQueueFamily()) {
  queue = device->createGraphicsQueue();
  Log.trace("Graphics queue family: {}", queueFamilyIndex);
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
  Log.trace("Present queue family: {}", queueFamilyIndex);
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
  Log.trace("Transfer queue family: {}", queueFamilyIndex);
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
  Log.trace("Compute queue family: {}", queueFamilyIndex);
}

auto Compute::getQueue() const -> vk::raii::Queue& {
  return *queue;
}

auto Compute::getFamily() const -> uint32_t {
  return queueFamilyIndex;
}

}
