#include "NoopDebugManager.hpp"

namespace tr {
auto NoopDebugManager::checkDebugSupport() -> void {
}

auto NoopDebugManager::getAdditionalInstanceExtensions() -> std::vector<char const*> {
  return {};
}

auto NoopDebugManager::getDebugMessengerCreateInfo() -> vk::DebugUtilsMessengerCreateInfoEXT {
  return {};
}

auto NoopDebugManager::addDebugCreateInfo(vk::InstanceCreateInfo& instanceCreateInfo) -> void {
}

auto NoopDebugManager::initializeInstance(vk::raii::Instance& instance) -> void {
}

auto NoopDebugManager::addDeviceConfig(vk::DeviceCreateInfo& deviceCreateInfo) -> void {
}

auto NoopDebugManager::destroyDebugCallbacks() -> void {
}
}
