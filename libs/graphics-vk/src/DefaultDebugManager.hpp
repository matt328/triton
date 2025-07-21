#pragma once

#include "gfx/IDebugManager.hpp"

namespace tr {

class Context;

const auto ValidationLayers = std::vector{"VK_LAYER_KHRONOS_validation"};

class DefaultDebugManager : public IDebugManager {
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

  std::mutex deviceMutex;

  static auto debugCallbackFn(
      [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT messageType,
      [[maybe_unused]] const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
      [[maybe_unused]] void* pUserData) -> VkBool32;
};

}
