#include "GraphicsDevice.hpp"
#include "GraphicsHelpers.hpp"
#include "ImmediateContext.hpp"
#include "vma_raii.hpp"

namespace Triton::Game::Graphics {

   const std::vector DESIRED_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

   GraphicsDevice::GraphicsDevice(GLFWwindow* window, const bool validationEnabled)
       : validationEnabled(validationEnabled) {
      glfwGetWindowSize(window, &width, &height);
      context = std::make_unique<vk::raii::Context>();

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

      // Select a PhysicalDevice
      const auto physicalDevices = enumeratePhysicalDevices();

      if (physicalDevices.empty()) {
         throw std::runtime_error("Failed to find any GPUs with Vulkan Support");
      }

      for (const auto& possibleDevice : physicalDevices) {
         if (Helpers::isDeviceSuitable(possibleDevice, *surface.get(), desiredDeviceExtensions)) {
            physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(possibleDevice);
            break;
         }
      }

      if (physicalDevice == nullptr) {
         throw std::runtime_error("Failed to find a suitable GPU");
      }

      Log::info << "Using physical device: " << physicalDevice->getProperties().deviceName.data()
                << std::endl;

      // Select and identify queues Queues
      auto [graphicsFamily, presentFamily, transferFamily, computeFamily] =
          Helpers::findQueueFamilies(*physicalDevice, *surface);

      std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
      std::set uniqueQueueFamilies = {graphicsFamily.value(), presentFamily.value()};

      float queuePriority = 1.f;

      for (auto queueFamily : uniqueQueueFamilies) {
         vk::DeviceQueueCreateInfo queueCreateInfo{.queueFamilyIndex = queueFamily,
                                                   .queueCount = 1,
                                                   .pQueuePriorities = &queuePriority};
         queueCreateInfos.push_back(queueCreateInfo);
      }

      if (validationEnabled) {
         desiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }

      auto features2 =
          physicalDevice->getFeatures2<vk::PhysicalDeviceFeatures2,
                                       vk::PhysicalDevice16BitStorageFeatures,
                                       vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

      auto indexingFeatures = features2.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

      auto drawParamsFeatures =
          vk::PhysicalDeviceShaderDrawParametersFeatures{.pNext = &indexingFeatures,
                                                         .shaderDrawParameters = VK_TRUE};

      const auto bindlessTexturesSupported = indexingFeatures.descriptorBindingPartiallyBound &&
                                             indexingFeatures.runtimeDescriptorArray;

      if (!bindlessTexturesSupported) {
         throw std::runtime_error("GPU does not support bindless textures :(");
      }

      auto physicalFeatures2 = physicalDevice->getFeatures2();
      physicalFeatures2.features.samplerAnisotropy = VK_TRUE;
      physicalFeatures2.pNext = &drawParamsFeatures;

      vk::DeviceCreateInfo createInfo{
          .pNext = &physicalFeatures2,
          .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
          .pQueueCreateInfos = queueCreateInfos.data(),
          .enabledLayerCount = 0,
          .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
          .ppEnabledExtensionNames = desiredDeviceExtensions.data()};

      if (validationEnabled) {
         createInfo.enabledLayerCount = static_cast<uint32_t>(desiredValidationLayers.size());
         createInfo.ppEnabledLayerNames = desiredValidationLayers.data();
      }

      vulkanDevice =
          std::make_unique<vk::raii::Device>(physicalDevice->createDevice(createInfo, nullptr));

      Helpers::setObjectName(**vulkanDevice,
                             *vulkanDevice.get(),
                             vk::raii::Device::debugReportObjectType,
                             "Primary Device");

      Log::trace << "Created Logical Device" << std::endl;

      graphicsQueue =
          std::make_unique<vk::raii::Queue>(vulkanDevice->getQueue(graphicsFamily.value(), 0));
      Helpers::setObjectName(**graphicsQueue,
                             *vulkanDevice.get(),
                             (**graphicsQueue).debugReportObjectType,
                             "Graphics Queue");
      Log::trace << "Created Graphics Queue" << std::endl;

      presentQueue =
          std::make_unique<vk::raii::Queue>(vulkanDevice->getQueue(presentFamily.value(), 0));
      Helpers::setObjectName(**presentQueue,
                             *vulkanDevice.get(),
                             (**presentQueue).debugReportObjectType,
                             "Present Queue");
      Log::trace << "Created Present Queue" << std::endl;

      transferQueue =
          std::make_shared<vk::raii::Queue>(vulkanDevice->getQueue(transferFamily.value(), 0));
      Helpers::setObjectName(**transferQueue,
                             *vulkanDevice.get(),
                             (**transferQueue).debugReportObjectType,
                             "Transfer Queue");
      Log::trace << "Created Transfer Queue" << std::endl;

      computeQueue =
          std::make_unique<vk::raii::Queue>(vulkanDevice->getQueue(computeFamily.value(), 0));
      Helpers::setObjectName(**computeQueue,
                             *vulkanDevice.get(),
                             (**computeQueue).debugReportObjectType,
                             "Compute Queue");
      Log::trace << "Created Compute Queue" << std::endl;

      // Create Swapchain
      auto [capabilities, formats, presentModes] =
          Helpers::querySwapchainSupport(*physicalDevice, *surface);

      const auto surfaceFormat = Helpers::chooseSwapSurfaceFormat(formats);
      const auto presentMode = Helpers::chooseSwapPresentMode(presentModes);
      const auto extent = Helpers::chooseSwapExtent(capabilities, getWindowSize());

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
          .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
          .preTransform = capabilities.currentTransform,
          .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
          .presentMode = presentMode,
          .clipped = VK_TRUE,
          .oldSwapchain = VK_NULL_HANDLE};

      const auto queueFamilyIndices =
          std::array<uint32_t, 2>{graphicsFamily.value(), presentFamily.value()};

      if (graphicsFamily != presentFamily) {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
         swapchainCreateInfo.queueFamilyIndexCount = 2;
         swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
      } else {
         swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
      }

      swapchain = std::make_unique<vk::raii::SwapchainKHR>(*vulkanDevice, swapchainCreateInfo);
      Log::info << "Created Swapchain" << std::endl;

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
      Log::info << "Created " << swapchainImageViews.size() << " swapchain image views"
                << std::endl;

      // Create Command Pools
      auto commandPoolCreateInfo =
          vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                    .queueFamilyIndex = graphicsFamily.value()};

      commandPool = std::make_unique<vk::raii::CommandPool>(
          vulkanDevice->createCommandPool(commandPoolCreateInfo));
      Helpers::setObjectName(**commandPool,
                             *vulkanDevice.get(),
                             (*commandPool).debugReportObjectType,
                             "Graphics Command Pool");

      transferImmediateContext = std::make_unique<ImmediateContext>(*vulkanDevice.get(),
                                                                    *physicalDevice,
                                                                    *transferQueue,
                                                                    transferFamily.value(),
                                                                    "Transfer Immediate Context");

      graphicsImmediateContext = std::make_unique<ImmediateContext>(*vulkanDevice.get(),
                                                                    *physicalDevice,
                                                                    *graphicsQueue,
                                                                    graphicsFamily.value(),
                                                                    "Graphics Immediate Context");

      // Create Descriptor Pools
      const auto poolSize = std::array{
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 10},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 100},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 10},
          vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer,
                                 .descriptorCount = FRAMES_IN_FLIGHT * 10},
      };

      const vk::DescriptorPoolCreateInfo poolInfo{
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                   vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT,
          .maxSets = FRAMES_IN_FLIGHT * 10 * poolSize.size(),
          .poolSizeCount = poolSize.size(),
          .pPoolSizes = poolSize.data()};

      descriptorPool = std::make_unique<vk::raii::DescriptorPool>(
          vulkanDevice->createDescriptorPool(poolInfo, nullptr));

      const auto allocatorCreateInfo = vma::AllocatorCreateInfo{.physicalDevice = **physicalDevice,
                                                                .device = **vulkanDevice,
                                                                .instance = **instance};

      raiillocator = std::make_unique<Allocator>(allocatorCreateInfo);
   }

   std::vector<vk::raii::PhysicalDevice> GraphicsDevice::enumeratePhysicalDevices() const {
      return instance->enumeratePhysicalDevices();
   }

   void GraphicsDevice::resizeWindow(const uint32_t newHeight, const uint32_t newWidth) {
      height = static_cast<int>(newHeight);
      width = static_cast<int>(newWidth);
   }

   bool GraphicsDevice::checkValidationLayerSupport() const {
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

   std::pair<std::vector<const char*>, bool> GraphicsDevice::getRequiredExtensions() const {
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

   VkBool32 GraphicsDevice::debugCallbackFn(
       [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
       [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
       [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
       [[maybe_unused]] void* pUserData) {
      // Log::debug << "Validation Layer: " << pCallbackData->pMessage << std::endl;
      return VK_FALSE;
   }

   VkBool32 GraphicsDevice::vulkanDebugReportCallback(
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