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

auto NoopDebugManager::addDebugCreateInfo(
    [[maybe_unused]] vk::InstanceCreateInfo& instanceCreateInfo) -> void {
}

auto NoopDebugManager::initializeInstance([[maybe_unused]] vk::raii::Instance& instance) -> void {
}

auto NoopDebugManager::addDeviceConfig([[maybe_unused]] vk::DeviceCreateInfo& deviceCreateInfo)
    -> void {
}

auto NoopDebugManager::destroyDebugCallbacks() -> void {
}

auto NoopDebugManager::setDevice([[maybe_unused]] std::shared_ptr<Device> newDevice) -> void {
}

auto NoopDebugManager::setObjectName([[maybe_unused]] const ObjectHandle& handle,
                                     [[maybe_unused]] std::string_view name) -> void {
}

}
