#include "pch.h"

#include "Instance.h"

#include "Log.h"

const std::vector<const char*> DESIRED_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

Instance::Instance(const bool validationEnabled)
    : validationEnabled(validationEnabled) {
   context = std::make_unique<vk::raii::Context>();

   // Create Instance
   // Log available extensions
   const auto instanceExtensions = context->enumerateInstanceExtensionProperties();
   std::string logString = "Available Instance Extensions\n";
   for (const auto& [extensionName, specVersion] : instanceExtensions) {
      logString.append(fmt::format("   {}: v{}\n", extensionName, specVersion));
   }
   LOG_LDEBUG("{}", logString);

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

   auto debugCreateInfo = createDebugUtilsMessengerCreateInfoExt();

   if (validationEnabled) {
      instanceCreateInfo.enabledLayerCount =
          static_cast<uint32_t>(DESIRED_VALIDATION_LAYERS.size());
      instanceCreateInfo.ppEnabledLayerNames = DESIRED_VALIDATION_LAYERS.data();
      instanceCreateInfo.pNext = &debugCreateInfo;
   }

   instance = std::make_unique<vk::raii::Instance>(*context, instanceCreateInfo);
}

Instance::~Instance() {
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

   return std::make_pair(extensions, portabilityPresent);
}

vk::DebugUtilsMessengerCreateInfoEXT Instance::createDebugUtilsMessengerCreateInfoExt() {
   return vk::DebugUtilsMessengerCreateInfoEXT{
       .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
       .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
       .pfnUserCallback = debugCallbackFn};
}
