#pragma once

namespace tr {

class Device;

struct ObjectHandle {
  enum class Type {
    Semaphore,
    Pipeline,
    CommandPool,
    Buffer,
    Image,
    ImageView,
    CommandBuffer,
  } type;
  union {
    vk::Semaphore semaphore;
    vk::Pipeline pipeline;
    vk::CommandPool commandPool;
    vk::Buffer buffer;
    vk::Image image;
    vk::ImageView imageView;
    vk::CommandBuffer commandBuffer;
  };

  ObjectHandle(const vk::Semaphore s) : type(Type::Semaphore), semaphore{s} {
  }
  ObjectHandle(vk::Pipeline p) : type(Type::Pipeline), pipeline(p) {
  }
  ObjectHandle(vk::CommandPool cp) : type(Type::CommandPool), commandPool{cp} {
  }
  ObjectHandle(vk::Buffer b) : type(Type::Buffer), buffer{b} {
  }
  ObjectHandle(vk::Image i) : type(Type::Image), image{i} {
  }
  ObjectHandle(vk::ImageView i) : type(Type::ImageView), imageView{i} {
  }
  ObjectHandle(vk::CommandBuffer i) : type(Type::CommandBuffer), commandBuffer{i} {
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

  virtual auto setDevice(std::shared_ptr<Device> newDevice) -> void = 0;

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
