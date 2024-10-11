#include "GraphicsDevice.hpp"
#include "helpers/Vulkan.hpp"
#include "VkContext.hpp"
#include "mem/Allocator.hpp"
#include <vk_mem_alloc_structs.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace tr::gfx {

   const std::vector DESIRED_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

   GraphicsDevice::GraphicsDevice(GLFWwindow* window, const bool validationEnabled)
       : validationEnabled(validationEnabled) {

      {
         ZoneNamedN(zCreateContext, "Create Context", true);
         context = std::make_unique<vk::raii::Context>();
      }

      // Log available extensions
      // const auto instanceExtensions = context->enumerateInstanceExtensionProperties();

      if (validationEnabled && !checkValidationLayerSupport()) {
         throw std::runtime_error("Validation layers requested but not available");
      }

      auto [extensions, portabilityRequired] = getRequiredExtensions();

      vk::ApplicationInfo appInfo{.pApplicationName = "Triton",
                                  .applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
                                  .pEngineName = "Triton Engine",
                                  .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
                                  .apiVersion = VK_API_VERSION_1_3};

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
          .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                         vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                         vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
          .pfnUserCallback = debugCallbackFn};

      if (validationEnabled) {
         instanceCreateInfo.enabledLayerCount =
             static_cast<uint32_t>(DESIRED_VALIDATION_LAYERS.size());
         instanceCreateInfo.ppEnabledLayerNames = DESIRED_VALIDATION_LAYERS.data();
         instanceCreateInfo.pNext = &debugCreateInfo;
      }

      {
         ZoneNamedN(zone, "Create Instance", true);
         instance = std::make_unique<vk::raii::Instance>(*context, instanceCreateInfo);
      }

      Log.trace("Created Instance");

      if (validationEnabled) {
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
      }

      VkSurfaceKHR tempSurface = nullptr;
      // TODO: register a callback to create a surface
      // std::function<std::unique_ptr<vk::raii::SurfaceKHR>(const vk::raii::Instance&)>
      // Should try to remove any glfw from triton, keep all of that in the 'clients'
      glfwCreateWindowSurface(**instance, window, nullptr, &tempSurface);
      Log.trace("Created Surface");
      surface = std::make_unique<vk::raii::SurfaceKHR>(*instance, tempSurface);

      // Select a PhysicalDevice
      const auto physicalDevices = enumeratePhysicalDevices();

      if (physicalDevices.empty()) {
         throw std::runtime_error("Failed to find any GPUs with Vulkan Support");
      }

      for (const auto& possibleDevice : physicalDevices) {
         if (Helpers::isDeviceSuitable(possibleDevice, *surface, desiredDeviceExtensions)) {
            physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(possibleDevice);
            break;
         }
      }

      if (physicalDevice == nullptr) {
         throw std::runtime_error("Failed to find a suitable GPU");
      }

      Log.info("Using physical device: {0}", physicalDevice->getProperties().deviceName.data());

      // Select and identify Queues
      auto queueFamilyIndices = Helpers::findQueueFamilies(*physicalDevice, *surface);
      std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

      // Graphics Queue(s)
      if (queueFamilyIndices.graphicsFamily.has_value() &&
          queueFamilyIndices.graphicsFamilyCount.has_value()) {
         const auto graphicsFamilyCreateInfo = vk::DeviceQueueCreateInfo{
             .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
             .queueCount = queueFamilyIndices.graphicsFamilyCount.value(),
             .pQueuePriorities = queueFamilyIndices.graphicsFamilyPriorities.data()};
         queueCreateInfos.push_back(graphicsFamilyCreateInfo);
      }

      // If present queue family is different from graphics
      if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
         Log.trace("Device supports separate present queue");
         // Present Queue(s)
         if (queueFamilyIndices.presentFamily.has_value() &&
             queueFamilyIndices.presentFamilyCount.has_value()) {
            const auto presentFamilyCreateInfo = vk::DeviceQueueCreateInfo{
                .queueFamilyIndex = queueFamilyIndices.presentFamily.value(),
                .queueCount = queueFamilyIndices.presentFamilyCount.value(),
                .pQueuePriorities = queueFamilyIndices.presentFamilyPriorities.data()};
            queueCreateInfos.push_back(presentFamilyCreateInfo);
         }
      }

      if (validationEnabled) {
         desiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }

      auto deviceProperties =
          physicalDevice->getProperties2KHR<vk::PhysicalDeviceProperties2KHR,
                                            vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();

      descriptorBufferProperties =
          deviceProperties.get<vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();

      auto drfs = vk::PhysicalDeviceDynamicRenderingFeaturesKHR{
          .dynamicRendering = VK_TRUE,
      };

      auto features2 =
          physicalDevice->getFeatures2<vk::PhysicalDeviceFeatures2,
                                       vk::PhysicalDevice16BitStorageFeatures,
                                       vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

      auto indexingFeatures = features2.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

      auto drawParamsFeatures =
          vk::PhysicalDeviceShaderDrawParametersFeatures{.shaderDrawParameters = VK_TRUE};

      if (const auto bindlessTexturesSupported = indexingFeatures.descriptorBindingPartiallyBound &&
                                                 indexingFeatures.runtimeDescriptorArray;
          !bindlessTexturesSupported) {
         throw std::runtime_error("GPU does not support bindless textures :(");
      }

      auto physicalFeatures2 = physicalDevice->getFeatures2();
      physicalFeatures2.features.samplerAnisotropy = VK_TRUE;

      vk::DeviceCreateInfo createInfo{
          .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
          .pQueueCreateInfos = queueCreateInfos.data(),
          .enabledLayerCount = 0,
          .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
          .ppEnabledExtensionNames = desiredDeviceExtensions.data()};

      if (validationEnabled) {
         createInfo.enabledLayerCount = static_cast<uint32_t>(desiredValidationLayers.size());
         createInfo.ppEnabledLayerNames = desiredValidationLayers.data();
      }

      // const auto dbFeatures =
      // vk::PhysicalDeviceDescriptorBufferFeaturesEXT{.descriptorBuffer = true};

      const auto extendedDynamicStateFeatures =
          vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{.extendedDynamicState = true};

      constexpr auto bdaFeatures =
          vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR{.bufferDeviceAddress = true};

      const vk::StructureChain c{createInfo,
                                 physicalFeatures2,
                                 drawParamsFeatures,
                                 indexingFeatures,
                                 drfs,
                                 // dbFeatures,
                                 extendedDynamicStateFeatures,
                                 bdaFeatures};

      vulkanDevice =
          std::make_unique<vk::raii::Device>(physicalDevice->createDevice(c.get(), nullptr));

      // Just don't name this because a bug in vulkan keeps complaining it doesn't match but it does
      // Helpers::setObjectName(**vulkanDevice, *vulkanDevice.get(), "Primary Device");

      Log.trace("Created Logical Device");

      graphicsQueue = std::make_unique<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.graphicsFamily.value(), 0));
      Helpers::setObjectName(**graphicsQueue, *vulkanDevice, "Graphics Queue");
      Log.trace("Created Graphics Queue");

      presentQueue = std::make_unique<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.presentFamily.value(), 0));
      Helpers::setObjectName(**presentQueue, *vulkanDevice, "Present Queue");
      Log.trace("Created Present Queue");

      transferQueue = std::make_shared<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.transferFamily.value(), 0));
      Helpers::setObjectName(**transferQueue, *vulkanDevice, "Transfer Queue");
      Log.trace("Created Transfer Queue");

      computeQueue = std::make_unique<vk::raii::Queue>(
          vulkanDevice->getQueue(queueFamilyIndices.computeFamily.value(), 0));
      Helpers::setObjectName(**computeQueue, *vulkanDevice, "Compute Queue");
      Log.trace("Created Compute Queue");

      createSwapchain();

      asyncTransferContext = std::make_shared<VkContext>(*vulkanDevice,
                                                         *physicalDevice,
                                                         0,
                                                         queueFamilyIndices.transferFamily.value(),
                                                         "Async Transfer Context");

      constexpr auto vulkanFunctions = vma::VulkanFunctions{
          .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
          .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
      };

      const auto allocatorCreateInfo = vma::AllocatorCreateInfo{
          .flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress,
          .physicalDevice = **physicalDevice,
          .device = **vulkanDevice,
          .pVulkanFunctions = &vulkanFunctions,
          .instance = **instance,
      };

      {
         ZoneNamedN(zone, "Create Allocator", true);
         raiillocator = std::make_unique<mem::Allocator>(allocatorCreateInfo, *vulkanDevice);
      }
   }

   GraphicsDevice::~GraphicsDevice() {
      Log.info("destroying graphicsDevice");
      vulkanDevice->waitIdle();
   }

   auto GraphicsDevice::getCurrentSize() const -> std::pair<uint32_t, uint32_t> {
      const auto surfaceCaps = physicalDevice->getSurfaceCapabilitiesKHR(**surface);
      return std::make_pair(surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height);
   }

   void GraphicsDevice::createSwapchain() {
      if (oldSwapchain != nullptr) {
         commandPool.reset();
         swapchainImages.clear();
         swapchainImageViews.clear();
      }
      // Create Swapchain
      auto queueFamilyIndicesInfo = Helpers::findQueueFamilies(*physicalDevice, *surface);

      auto [capabilities, formats, presentModes] =
          Helpers::querySwapchainSupport(*physicalDevice, *surface);

      const auto surfaceFormat = Helpers::chooseSwapSurfaceFormat(formats);
      const auto presentMode = Helpers::chooseSwapPresentMode(presentModes);
      const auto extent = Helpers::chooseSwapExtent(capabilities, getCurrentSize());

      uint32_t imageCount = capabilities.minImageCount + 1;

      if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
         imageCount = capabilities.maxImageCount;
      }

      vk::SwapchainCreateInfoKHR swapchainCreateInfo{
          .surface = **surface,
          .minImageCount = imageCount,
          .imageFormat = surfaceFormat.format,
          .imageColorSpace = surfaceFormat.colorSpace,
          .imageExtent = extent,
          .imageArrayLayers = 1,
          .imageUsage =
              vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
          .preTransform = capabilities.currentTransform,
          .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
          .presentMode = presentMode,
          .clipped = VK_TRUE,
          .oldSwapchain = VK_NULL_HANDLE};

      if (oldSwapchain != nullptr) {
         swapchainCreateInfo.oldSwapchain = **oldSwapchain;
      }

      const auto queueFamilyIndices = std::array{queueFamilyIndicesInfo.graphicsFamily.value(),
                                                 queueFamilyIndicesInfo.presentFamily.value()};

      if (queueFamilyIndicesInfo.graphicsFamily != queueFamilyIndicesInfo.presentFamily) {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
         swapchainCreateInfo.queueFamilyIndexCount = 2;
         swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
      } else {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
      }

      swapchain = std::make_unique<vk::raii::SwapchainKHR>(*vulkanDevice, swapchainCreateInfo);
      Log.info("Created Swapchain");

      swapchainExtent = extent;
      swapchainImageFormat = surfaceFormat.format;

      swapchainImages = swapchain->getImages();
      swapchainImageViews.reserve(swapchainImages.size());

      constexpr vk::ComponentMapping components{.r = vk::ComponentSwizzle::eIdentity,
                                                .g = vk::ComponentSwizzle::eIdentity,
                                                .b = vk::ComponentSwizzle::eIdentity,
                                                .a = vk::ComponentSwizzle::eIdentity};

      constexpr vk::ImageSubresourceRange subresourceRange{.aspectMask =
                                                               vk::ImageAspectFlagBits::eColor,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1};

      for (const auto& image : swapchainImages) {
         vk::ImageViewCreateInfo createInfo{.image = image,
                                            .viewType = vk::ImageViewType::e2D,
                                            .format = swapchainImageFormat,
                                            .components = components,
                                            .subresourceRange = subresourceRange};

         swapchainImageViews.emplace_back(*vulkanDevice, createInfo);
      }
      Log.info("Created {0} swapchain image views", swapchainImageViews.size());

      // Create Command Pools
      auto commandPoolCreateInfo = vk::CommandPoolCreateInfo{
          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
          .queueFamilyIndex = queueFamilyIndicesInfo.graphicsFamily.value()};

      commandPool = std::make_unique<vk::raii::CommandPool>(
          vulkanDevice->createCommandPool(commandPoolCreateInfo));
      Helpers::setObjectName(**commandPool, *vulkanDevice, "Graphics Command Pool");
   }

   void GraphicsDevice::recreateSwapchain() {
      oldSwapchain = std::move(swapchain);
      createSwapchain();
   }

   std::vector<vk::raii::PhysicalDevice> GraphicsDevice::enumeratePhysicalDevices() const {
      return instance->enumeratePhysicalDevices();
   }

   bool GraphicsDevice::checkValidationLayerSupport() const {
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

   std::pair<std::vector<const char*>, bool> GraphicsDevice::getRequiredExtensions() const {
      uint32_t glfwExtensionCount = 0;
      const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      // NOLINTNEXTLINE This is ok because glfw's C api sucks.
      std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

      extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

      if (validationEnabled) {
         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
         extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
      }

      const auto exts = context->enumerateInstanceExtensionProperties();

      std::vector<std::string> extNames = {};

      extNames.reserve(exts.size());
      for (auto& ext : exts) {
         extNames.push_back(ext.extensionName);
      }

      extNames.emplace_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);

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

   VkBool32 GraphicsDevice::debugCallbackFn(
       [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
       [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
       [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
       [[maybe_unused]] void* pUserData) {

      Log.trace("Validation Layer: {0}", pCallbackData->pMessage);
      return VK_FALSE;
   }

   auto GraphicsDevice::vulkanDebugReportCallback(
       [[maybe_unused]] VkDebugReportFlagsEXT flags,
       [[maybe_unused]] VkDebugReportObjectTypeEXT objectType,
       [[maybe_unused]] uint64_t object,
       [[maybe_unused]] size_t location,
       [[maybe_unused]] int32_t messageCode,
       const char* pLayerPrefix,
       const char* pMessage,
       [[maybe_unused]] void* userData) -> VkBool32 {
      // if (!strcmp(pLayerPrefix, "Loader Message")) {
      //    return VK_FALSE;
      // }
      Log.debug("Debug Callback ({0}): {1}", pLayerPrefix, pMessage);
      return VK_TRUE;
   }
}
