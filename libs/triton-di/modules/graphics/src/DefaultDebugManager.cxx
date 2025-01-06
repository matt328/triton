#include "DefaultDebugManager.hpp"

#include "vk/Context.hpp"
#include "vk/Device.hpp"

namespace tr {

DefaultDebugManager::DefaultDebugManager(std::shared_ptr<Context> newContext)
    : context{std::move(newContext)} {
  Log.trace("Constructing DefaultDebugManager");
}

DefaultDebugManager::~DefaultDebugManager() {
  Log.trace("Destroying DefaultDebugManager");
}

auto DefaultDebugManager::checkDebugSupport() -> void {
  const auto availableLayers = context->getLayerProperties();

  for (const auto* const layerName : ValidationLayers) {
    bool layerFound = false;
    for (const auto& layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      throw std::runtime_error(fmt::format("Layer {} requested but not found", layerName));
    }
  }
}

auto DefaultDebugManager::getAdditionalInstanceExtensions() -> std::vector<char const*> {
  return {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
}

auto DefaultDebugManager::getDebugMessengerCreateInfo() -> vk::DebugUtilsMessengerCreateInfoEXT {
  return vk::DebugUtilsMessengerCreateInfoEXT{
      .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
      .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      .pfnUserCallback = debugCallbackFn};
}

auto DefaultDebugManager::addDebugCreateInfo(vk::InstanceCreateInfo& instanceCreateInfo) -> void {
  debugCreateInfo = getDebugMessengerCreateInfo();
  instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
  instanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
  instanceCreateInfo.pNext = &debugCreateInfo;
}

auto DefaultDebugManager::initializeInstance(vk::raii::Instance& instance) -> void {
  debugCallback = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(
      instance.createDebugUtilsMessengerEXT(debugCreateInfo));
}
auto DefaultDebugManager::addDeviceConfig(vk::DeviceCreateInfo& deviceCreateInfo) -> void {
  deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
  deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
}
auto DefaultDebugManager::destroyDebugCallbacks() -> void {
  debugCallback = nullptr;
}
auto DefaultDebugManager::setDevice(std::shared_ptr<Device> newDevice) -> void {
  device.emplace(std::move(newDevice));
}

auto DefaultDebugManager::setObjectName(const ObjectHandle& handle, std::string_view name) -> void {
  if (!device.has_value()) {
    Log.warn("Attempted to set object name before Device was initialized");
    return;
  }

  switch (handle.type) {
    case ObjectHandle::Type::Semaphore: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkSemaphore>(handle.semaphore));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::eSemaphore,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    case ObjectHandle::Type::Pipeline: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkPipeline>(handle.pipeline));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::ePipeline,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    case ObjectHandle::Type::CommandPool: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkCommandPool>(handle.commandPool));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::eCommandPool,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    case ObjectHandle::Type::Buffer: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkBuffer>(handle.buffer));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::eBuffer,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    case ObjectHandle::Type::Image: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkImage>(handle.image));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::eImage,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    case ObjectHandle::Type::ImageView: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkImageView>(handle.imageView));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::eImageView,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    case ObjectHandle::Type::CommandBuffer: {
      const auto l = reinterpret_cast<uint64_t>(static_cast<VkCommandBuffer>(handle.commandBuffer));
      const auto debugNameInfo =
          vk::DebugUtilsObjectNameInfoEXT{.objectType = vk::ObjectType::eCommandBuffer,
                                          .objectHandle = l,
                                          .pObjectName = name.data()};
      device->get()->getVkDevice().setDebugUtilsObjectNameEXT(debugNameInfo);
    } break;
    default:
      Log.warn("Object type not supported for setting name");
  }
}

auto DefaultDebugManager::debugCallbackFn(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData) -> VkBool32 {

  Log.trace("Validation Layer: {0}", pCallbackData->pMessage);
  return VK_FALSE;
}
}

// Send rick a chg thing.
