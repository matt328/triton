#pragma once

#include "IDebugManager.hpp"

#include <vk/Context.hpp>
#include <vk/Device.hpp>

namespace tr {

class Context;

const auto ValidationLayers = std::vector{"VK_LAYER_KHRONOS_validation"};

class DefaultDebugManager final : public IDebugManager {
public:
  explicit DefaultDebugManager(std::shared_ptr<Context> newContext);
  ~DefaultDebugManager() override;

  DefaultDebugManager(const DefaultDebugManager&) = delete;
  DefaultDebugManager(DefaultDebugManager&&) = delete;
  auto operator=(const DefaultDebugManager&) -> DefaultDebugManager& = delete;
  auto operator=(DefaultDebugManager&&) -> DefaultDebugManager& = delete;

  auto checkDebugSupport() -> void override;
  auto getAdditionalInstanceExtensions() -> std::vector<char const*> override;
  auto getDebugMessengerCreateInfo() -> vk::DebugUtilsMessengerCreateInfoEXT override;
  auto addDebugCreateInfo(vk::InstanceCreateInfo& instanceCreateInfo) -> void override;
  auto initializeInstance(vk::raii::Instance& instance) -> void override;
  auto addDeviceConfig(vk::DeviceCreateInfo& deviceCreateInfo) -> void override;
  auto destroyDebugCallbacks() -> void override;

  auto setDevice(std::shared_ptr<Device> newDevice) -> void override;

  auto setObjectName(const ObjectHandle& handle, std::string_view name) -> void override;

private:
  std::shared_ptr<Context> context;

  std::optional<std::shared_ptr<Device>> device;

  std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
  vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;

  static auto debugCallbackFn(
      [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
      [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      [[maybe_unused]] void* pUserData) -> VkBool32;
};

}
