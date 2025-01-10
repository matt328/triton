#pragma once
#include "tr/IDebugManager.hpp"

namespace tr {

class NoopDebugManager final : public IDebugManager {
public:
  auto checkDebugSupport() -> void override;
  auto getAdditionalInstanceExtensions() -> std::vector<char const*> override;
  auto getDebugMessengerCreateInfo() -> vk::DebugUtilsMessengerCreateInfoEXT override;
  auto addDebugCreateInfo(vk::InstanceCreateInfo& instanceCreateInfo) -> void override;
  auto initializeInstance(vk::raii::Instance& instance) -> void override;
  auto addDeviceConfig(vk::DeviceCreateInfo& deviceCreateInfo) -> void override;
  auto destroyDebugCallbacks() -> void override;
  auto setDevice(std::shared_ptr<Device> newDevice) -> void override;
  auto setObjectName(const ObjectHandle& handle, std::string_view name) -> void override;
};

}
