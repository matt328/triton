#include "RenderDevice.h"
#include "Log.h"
#include "vulkan-memory-allocator-hpp/vk_mem_alloc.hpp"

#include <GLFW/glfw3.h>
#include <set>

RenderDevice::RenderDevice(const std::unique_ptr<Instance>& instance) {
   createPhysicalDevice(instance);
   createLogicalDevice(instance);
   createSwapchain(instance);
   createSwapchainImageViews();
   createAllocator(instance);
}

RenderDevice::~RenderDevice() {
}

void RenderDevice::createPhysicalDevice(const std::unique_ptr<Instance>& instance) {
   const auto physicalDevices = instance->enumeratePhysicalDevices();

   if (physicalDevices.empty()) {
      throw std::runtime_error("Failed to find any GPUs with Vulkan Support");
   }

   for (const auto& possibleDevice : physicalDevices) {
      if (isDeviceSuitable(possibleDevice, instance)) {
         physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(possibleDevice);
         break;
      }
   }

   if (physicalDevice == nullptr) {
      throw std::runtime_error("Failed to find a suitable GPU");
   }

   Log::core->debug("Using physical device: {}", physicalDevice->getProperties().deviceName);
}

void RenderDevice::createAllocator(const std::unique_ptr<Instance>& instance) {
   const auto getInstanceProcAddrfn = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
       instance->getVkInstance()->getProcAddr("vkGetInstanceProcAddr"));
   const auto getDeviceProcAddrfn = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
       instance->getVkInstance()->getProcAddr("vkGetDeviceProcAddr"));

   const auto fns = vma::VulkanFunctions{.vkGetInstanceProcAddr = getInstanceProcAddrfn,
                                         .vkGetDeviceProcAddr = getDeviceProcAddrfn};

   const auto aci = vma::AllocatorCreateInfo{.physicalDevice = (*(*physicalDevice)),
                                             .device = *(*device),
                                             .pVulkanFunctions = &fns,
                                             .instance = *(*instance->getVkInstance())};

   allocator = std::make_unique<vma::Allocator>(vma::createAllocator(aci));
}

void RenderDevice::createLogicalDevice(const std::unique_ptr<Instance>& instance) {
   auto [graphicsFamily, presentFamily, transferFamily] =
       findQueueFamilies(*physicalDevice, instance->getSurface());

   std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
   std::set uniqueQueueFamilies = {graphicsFamily.value(), presentFamily.value()};

   float queuePriority = 1.f;

   for (auto queueFamily : uniqueQueueFamilies) {
      vk::DeviceQueueCreateInfo queueCreateInfo{
          .queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &queuePriority};
      queueCreateInfos.push_back(queueCreateInfo);
   }

   vk::PhysicalDeviceFeatures deviceFeatures{.samplerAnisotropy = VK_TRUE};

   auto desiredDeviceExtensions = instance->getDesiredDeviceExtensions();
   desiredDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

   vk::DeviceCreateInfo createInfo{
       .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
       .pQueueCreateInfos = queueCreateInfos.data(),
       .enabledLayerCount = 0,
       .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
       .ppEnabledExtensionNames = desiredDeviceExtensions.data(),
       .pEnabledFeatures = &deviceFeatures};

   const auto validationLayers = instance->getDesiredValidationLayers();
   if (instance->isValidationEnabled()) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
   }

   device = std::make_unique<vk::raii::Device>(physicalDevice->createDevice(createInfo, nullptr));

   Log::core->info("Created Logical Device");

   graphicsQueue = std::make_unique<vk::raii::Queue>(device->getQueue(graphicsFamily.value(), 0));
   setObjectName(
       *(*graphicsQueue), device, (*(*graphicsQueue)).debugReportObjectType, "Graphics Queue");
   Log::core->info("Created Graphics Queue");

   presentQueue = std::make_unique<vk::raii::Queue>(device->getQueue(presentFamily.value(), 0));
   setObjectName(
       *(*presentQueue), device, (*(*presentQueue)).debugReportObjectType, "Present Queue");
   Log::core->info("Created Present Queue");
}

void RenderDevice::createSwapchain(const std::unique_ptr<Instance>& instance) {
   const auto& surface = instance->getSurface();
   auto [capabilities, formats, presentModes] = querySwapchainSupport(*physicalDevice, surface);

   const auto surfaceFormat = chooseSwapSurfaceFormat(formats);
   const auto presentMode = chooseSwapPresentMode(presentModes);
   const auto extent = chooseSwapExtent(capabilities, instance);

   uint32_t imageCount = capabilities.minImageCount + 1;

   if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
      imageCount = capabilities.maxImageCount;
   }

   vk::SwapchainCreateInfoKHR createInfo{.surface = *(*surface),
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

   const auto [graphicsFamily, presentFamily, transferFamily] =
       findQueueFamilies(*physicalDevice, surface);
   const uint32_t queueFamilyIndices[] = {graphicsFamily.value(), presentFamily.value()};

   if (graphicsFamily != presentFamily) {
      createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
   } else {
      createInfo.imageSharingMode = vk::SharingMode::eExclusive;
   }

   swapchain = std::make_unique<vk::raii::SwapchainKHR>(*device, createInfo);
   Log::core->info("Created Swapchain");

   swapchainExtent = extent;
   swapchainImageFormat = surfaceFormat.format;
}

void RenderDevice::createSwapchainImageViews() {
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

      swapchainImageViews.emplace_back(*device, createInfo);
   }
   Log::core->info("Created {} swapchain image views", swapchainImageViews.size());
}

vk::PresentModeKHR RenderDevice::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR>& availablePresentModes) {
   for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
         return availablePresentMode;
      }
   }
   return vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR RenderDevice::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
   for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
          availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
         return availableFormat;
      }
   }
   return availableFormats[0];
}

vk::Extent2D RenderDevice::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                            const std::unique_ptr<Instance>& instance) const {
   if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
   } else {
      int width, height;
      const auto& window = instance->getWindow();
      glfwGetFramebufferSize(*window, &width, &height);

      vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

      actualExtent.width = std::clamp(
          actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height,
                                       capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height);

      return actualExtent;
   }
}

bool RenderDevice::isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                                    const std::unique_ptr<Instance>& instance) {
   const QueueFamilyIndices queueFamilyIndices =
       findQueueFamilies(possibleDevice, instance->getSurface());

   const bool extensionsSupported =
       checkDeviceExtensionSupport(possibleDevice, instance->getDesiredDeviceExtensions());

   bool swapchainAdequate = false;
   if (extensionsSupported) {
      auto [capabilities, formats, presentModes] =
          querySwapchainSupport(possibleDevice, instance->getSurface());
      swapchainAdequate = !formats.empty() && !presentModes.empty();
   }

   const auto features = possibleDevice.getFeatures();

   return queueFamilyIndices.isComplete() && extensionsSupported && swapchainAdequate &&
          features.samplerAnisotropy && features.tessellationShader;
}

RenderDevice::QueueFamilyIndices RenderDevice::findQueueFamilies(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
   QueueFamilyIndices queueFamilyIndices;

   const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

   for (int i = 0; const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
         queueFamilyIndices.graphicsFamily = i;
      }

      if (possibleDevice.getSurfaceSupportKHR(i, *(*surface))) {
         queueFamilyIndices.presentFamily = i;
      }

      if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
         queueFamilyIndices.transferFamily = i;
      }

      if (queueFamilyIndices.isComplete()) {
         break;
      }
      i++;
   }
   return queueFamilyIndices;
}

bool RenderDevice::checkDeviceExtensionSupport(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::vector<const char*> desiredDeviceExtensions) {
   const auto availableExtensions = possibleDevice.enumerateDeviceExtensionProperties();

   for (const auto& ext : availableExtensions) {
      Log::core->debug("Physical Device Extension: {}", ext.extensionName);
   }

   std::set<std::string> requiredExtensions(desiredDeviceExtensions.begin(),
                                            desiredDeviceExtensions.end());

   for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
   }

   return requiredExtensions.empty();
}

RenderDevice::SwapchainSupportDetails RenderDevice::querySwapchainSupport(
    const vk::raii::PhysicalDevice& possibleDevice,
    const std::unique_ptr<vk::raii::SurfaceKHR>& surface) {
   SwapchainSupportDetails details;
   details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(*(*surface));
   details.formats = possibleDevice.getSurfaceFormatsKHR(*(*surface));
   details.presentModes = possibleDevice.getSurfacePresentModesKHR(*(*surface));
   return details;
}
