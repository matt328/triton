#pragma once

#include "IDebugManager.hpp"

namespace tr::gfx {

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

    private:
      std::shared_ptr<Context> context;

      std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugCallback;
      std::unique_ptr<vk::raii::DebugReportCallbackEXT> reportCallback;

      vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;

      static auto debugCallbackFn(
          [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
          [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
          [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
          [[maybe_unused]] void* pUserData) -> VkBool32;
      static auto vulkanDebugReportCallback([[maybe_unused]] VkDebugReportFlagsEXT flags,
                                            [[maybe_unused]] VkDebugReportObjectTypeEXT objectType,
                                            [[maybe_unused]] uint64_t object,
                                            [[maybe_unused]] size_t location,
                                            [[maybe_unused]] int32_t messageCode,
                                            [[maybe_unused]] const char* pLayerPrefix,
                                            [[maybe_unused]] const char* pMessage,
                                            [[maybe_unused]] void* userData) -> VkBool32;
   };

}