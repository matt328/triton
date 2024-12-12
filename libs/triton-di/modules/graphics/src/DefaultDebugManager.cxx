#include "DefaultDebugManager.hpp"

#include "vk/Context.hpp"

namespace tr::gfx {

   DefaultDebugManager::DefaultDebugManager(std::shared_ptr<Context> newContext)
       : context{std::move(newContext)} {
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
      return {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME};
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

   auto DefaultDebugManager::addDebugCreateInfo(vk::InstanceCreateInfo& instanceCreateInfo)
       -> void {
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
   auto DefaultDebugManager::setObjectName(const ObjectHandle& handle, std::string_view name)
       -> void {
      if (!device.has_value()) {
         Log.warn("Attempted to set object name before Device was initialized");
         return;
      }
      const auto fn =
          (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(*device->getVkDevice(),
                                                                 "vkDebugMarkerSetObjectNameEXT");
      if (fn != nullptr) {
         switch (handle.type) {
            case ObjectHandle::Type::Semaphore:
               auto l = reinterpret_cast<uint64_t>(static_cast<VkSemaphore>(*handle.semaphore));
               const auto debugNameInfo = vk::DebugMarkerObjectNameInfoEXT{
                   .objectType = vk::raii::Semaphore::debugReportObjectType,
                   .object = l,
                   .pObjectName = name.data()};
               device->getVkDevice().debugMarkerSetObjectNameEXT(debugNameInfo);
            default:;
         }
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
