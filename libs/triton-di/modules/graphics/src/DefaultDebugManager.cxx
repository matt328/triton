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
      const vk::DebugReportCallbackCreateInfoEXT ci = {
          .pNext = nullptr,
          .flags = vk::DebugReportFlagBitsEXT::eWarning |
                   vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                   vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug,
          .pfnCallback = &vulkanDebugReportCallback,
          .pUserData = nullptr};

      debugCallback = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(
          instance.createDebugUtilsMessengerEXT(debugCreateInfo));

      reportCallback = std::make_unique<vk::raii::DebugReportCallbackEXT>(
          instance.createDebugReportCallbackEXT(ci));
   }
   auto DefaultDebugManager::addDeviceConfig(vk::DeviceCreateInfo& deviceCreateInfo) -> void {
      deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
      deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
   }
   auto DefaultDebugManager::destroyDebugCallbacks() -> void {
      debugCallback = nullptr;
      reportCallback = nullptr;
   }

   auto DefaultDebugManager::debugCallbackFn(
       [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
       [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
       [[maybe_unused]] void* pUserData) -> VkBool32 {
      Log.trace("Validation Layer: {0}", pCallbackData->pMessage);
      return VK_FALSE;
   }
   auto DefaultDebugManager::vulkanDebugReportCallback(
       [[maybe_unused]] VkDebugReportFlagsEXT flags,
       [[maybe_unused]] VkDebugReportObjectTypeEXT objectType,
       [[maybe_unused]] uint64_t object,
       [[maybe_unused]] size_t location,
       [[maybe_unused]] int32_t messageCode,
       const char* pLayerPrefix,
       const char* pMessage,
       [[maybe_unused]] void* userData) -> VkBool32 {
      Log.debug("Debug Callback ({0}): {1}", pLayerPrefix, pMessage);
      return VK_TRUE;
   }

}