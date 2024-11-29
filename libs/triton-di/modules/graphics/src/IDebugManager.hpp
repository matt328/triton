#pragma once

namespace tr::gfx {
   class IDebugManager {
    public:
      IDebugManager() = default;
      virtual ~IDebugManager() = default;

      IDebugManager(const IDebugManager&) = delete;
      IDebugManager(IDebugManager&&) = delete;
      auto operator=(const IDebugManager&) -> IDebugManager& = delete;
      auto operator=(IDebugManager&&) -> IDebugManager& = delete;

      /// @brief Ensures required validation layers are available
      /// @throws std::runtime_error if a requested layer is not available.
      virtual auto checkDebugSupport() -> void = 0;

      /// @brief Returns a std::vector of additional extensions required by the Debug Manager.
      virtual auto getAdditionalInstanceExtensions() -> std::vector<const char*> = 0;

      virtual auto getDebugMessengerCreateInfo() -> vk::DebugUtilsMessengerCreateInfoEXT = 0;

      virtual auto addDebugCreateInfo(vk::InstanceCreateInfo& instanceCreateInfo) -> void = 0;

      virtual auto initializeInstance(vk::raii::Instance& instance) -> void = 0;

      virtual auto addDeviceConfig(vk::DeviceCreateInfo& deviceCreateInfo) -> void = 0;

      virtual auto destroyDebugCallbacks() -> void = 0;
   };
}
