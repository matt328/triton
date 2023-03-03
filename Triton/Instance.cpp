#include "Instance.h"
#include "Log.h"

#include <GLFW/glfw3.h>

const std::vector DESIRED_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

Instance::Instance(GLFWwindow* window,
                   const bool validationEnabled,
                   const uint32_t initialHeight,
                   const uint32_t initialWidth)
    : validationEnabled(validationEnabled)
    , height(initialHeight)
    , width(initialWidth) {
   context = std::make_unique<vk::raii::Context>();

   // Log available extensions
   const auto instanceExtensions = context->enumerateInstanceExtensionProperties();
   std::string logString = "Available Instance Extensions\n";
   for (const auto& [extensionName, specVersion] : instanceExtensions) {
      logString.append(fmt::format("   {}: v{}\n", extensionName, specVersion));
   }
   Log::core->debug("{}", logString);

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

   VkSurfaceKHR tempSurface;
   glfwCreateWindowSurface(**instance, window, nullptr, &tempSurface);
   surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, tempSurface);
}

Instance::~Instance() {
}

std::vector<vk::raii::PhysicalDevice> Instance::enumeratePhysicalDevices() const {
   return instance->enumeratePhysicalDevices();
}

void Instance::resizeWindow(const uint32_t newHeight, const uint32_t newWidth) {
   height = newHeight;
   width = newWidth;
}

bool Instance::checkValidationLayerSupport() const {
   const auto availableLayers = context->enumerateInstanceLayerProperties();
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
   uint32_t glfwExtensionCount;
   const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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

   auto portabilityPresent = std::ranges::find_if(extNames, [](const std::string& name) {
                                return name == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
                             }) != extNames.end();

   if (portabilityPresent) {
      extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
   }

   for (const auto& ext : extensions) {
      Log::core->debug("Extension: {}", ext);
   }

   return std::make_pair(extensions, portabilityPresent);
}

VkBool32 Instance::debugCallbackFn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                   VkDebugUtilsMessageTypeFlagsEXT messageType,
                                   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                   void* pUserData) {
   Log::core->debug("Validation Layer: {}", pCallbackData->pMessage);
   return VK_FALSE;
}

VkBool32 Instance::vulkanDebugReportCallback(VkDebugReportFlagsEXT flags,
                                             VkDebugReportObjectTypeEXT objectType,
                                             uint64_t object,
                                             size_t location,
                                             int32_t messageCode,
                                             const char* pLayerPrefix,
                                             const char* pMessage,
                                             void* userData) {
   if (!strcmp(pLayerPrefix, "Loader Message")) {
      return VK_FALSE;
   }
   Log::core->debug("Debug Callback ({}): {}", pLayerPrefix, pMessage);
   return VK_FALSE;
}
