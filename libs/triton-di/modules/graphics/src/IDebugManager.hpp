#pragma once

namespace tr::gfx {

   struct ObjectHandle {
      enum class Type {
         Semaphore,
         Pipeline
      } type;
      union {
         vk::raii::Semaphore& semaphore;
         VkPipeline pipeline;
      };

      ObjectHandle(vk::raii::Semaphore& s) : type(Type::Semaphore), semaphore{s} {
      }
      ObjectHandle(const VkPipeline p) : type(Type::Pipeline), pipeline(p) {
      }
   };

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

      virtual auto setObjectName(const ObjectHandle& handle, std::string_view name) -> void = 0;
   };
}
