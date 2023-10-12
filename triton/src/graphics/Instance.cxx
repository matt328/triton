#include "Instance.hpp"
#include "Logger.hpp"

namespace Triton {

   const std::vector DESIRED_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

   Instance::Instance(GLFWwindow* window, const bool validationEnabled) :
       validationEnabled(validationEnabled) {
      glfwGetWindowSize(window, &width, &height);
      context = std::make_unique<vk::raii::Context>();

      // Log available extensions
      const auto instanceExtensions = context->enumerateInstanceExtensionProperties();

      if (validationEnabled && !checkValidationLayerSupport()) {
         throw std::runtime_error("Validation layers requested but not available");
      }

      auto [extensions, portabilityRequired] = getRequiredExtensions();

      vk::ApplicationInfo appInfo{.pApplicationName = "Triton",
                                  .applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
                                  .pEngineName = "Triton Engine",
                                  .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
                                  .apiVersion = VK_API_VERSION_1_1};

      vk::InstanceCreateInfo instanceCreateInfo{
          .pApplicationInfo = &appInfo,
          .enabledLayerCount = 0,
          .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
          .ppEnabledExtensionNames = extensions.data(),
      };

      // For some reason, now Win64 decides portability is required
      // Added an override to only even try to detect portability on __APPLE__
      if (portabilityRequired) {
         instanceCreateInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
         desiredDeviceExtensions.push_back("VK_KHR_portability_subset");
      }

      const auto debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{
          .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                         vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                         vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
          .pfnUserCallback = debugCallbackFn};

      if (validationEnabled) {
         instanceCreateInfo.enabledLayerCount =
             static_cast<uint32_t>(DESIRED_VALIDATION_LAYERS.size());
         instanceCreateInfo.ppEnabledLayerNames = DESIRED_VALIDATION_LAYERS.data();
         instanceCreateInfo.pNext = &debugCreateInfo;
      }

      instance = std::make_unique<vk::raii::Instance>(*context, instanceCreateInfo);

      Log::trace << "Created Instance" << std::endl;

      const vk::DebugReportCallbackCreateInfoEXT ci = {
          .pNext = nullptr,
          .flags = vk::DebugReportFlagBitsEXT::eWarning |
                   vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                   vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug,
          .pfnCallback = &vulkanDebugReportCallback,
          .pUserData = nullptr};

      debugCallback = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(
          instance->createDebugUtilsMessengerEXT(debugCreateInfo));

      reportCallback = std::make_unique<vk::raii::DebugReportCallbackEXT>(
          instance->createDebugReportCallbackEXT(ci));

      VkSurfaceKHR tempSurface = nullptr;
      glfwCreateWindowSurface(**instance, window, nullptr, &tempSurface);
      Log::trace << "Created Surface" << std::endl;
      surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, tempSurface);
   }

   std::vector<vk::raii::PhysicalDevice> Instance::enumeratePhysicalDevices() const {
      return instance->enumeratePhysicalDevices();
   }

   void Instance::resizeWindow(const uint32_t newHeight, const uint32_t newWidth) {
      height = static_cast<int>(newHeight);
      width = static_cast<int>(newWidth);
   }

   bool Instance::checkValidationLayerSupport() const {
      const auto availableLayers = context->enumerateInstanceLayerProperties();
      for (const auto& layerProps : availableLayers) {
         Log::trace << layerProps.layerName << std::endl;
      }
      for (const auto layerName : DESIRED_VALIDATION_LAYERS) {
         bool layerFound = false;
         for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
               layerFound = true;
               break;
            }
         }
         if (!layerFound) {
            return false;
         }
      }
      return true;
   }

   std::pair<std::vector<const char*>, bool> Instance::getRequiredExtensions() const {
      uint32_t glfwExtensionCount = 0;
      const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      // NOLINTNEXTLINE This is ok because glfw's C api sucks.
      std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

      if (validationEnabled) {
         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
      }

      const auto exts = context->enumerateInstanceExtensionProperties();

      std::vector<std::string> extNames = {};

      for (auto& ext : exts) {
         extNames.push_back(ext.extensionName);
      }

      auto portabilityPresent = false;

#ifdef __APPLE__
      portabilityPresent = std::ranges::find_if(extNames, [](const std::string& name) {
                              return name == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
                           }) != extNames.end();

      if (portabilityPresent) {
         extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      }
#endif

      return std::make_pair(extensions, portabilityPresent);
   }

   VkBool32 Instance::debugCallbackFn(
       [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
       [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
       [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
       [[maybe_unused]] void* pUserData) {
      // Log::debug << "Validation Layer: " << pCallbackData->pMessage << std::endl;
      return VK_FALSE;
   }

   VkBool32 Instance::vulkanDebugReportCallback(
       [[maybe_unused]] VkDebugReportFlagsEXT flags,
       [[maybe_unused]] VkDebugReportObjectTypeEXT objectType,
       [[maybe_unused]] uint64_t object,
       [[maybe_unused]] size_t location,
       [[maybe_unused]] int32_t messageCode,
       const char* pLayerPrefix,
       const char* pMessage,
       [[maybe_unused]] void* userData) {
      if (!strcmp(pLayerPrefix, "Loader Message")) {
         return VK_FALSE;
      }
      Log::debug << "Debug Callback (" << pLayerPrefix << "): " << pMessage << std::endl;
      return VK_FALSE;
   }
}