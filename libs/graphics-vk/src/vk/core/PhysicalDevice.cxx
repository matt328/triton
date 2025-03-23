#include "PhysicalDevice.hpp"

#include "Surface.hpp"

namespace tr {

PhysicalDevice::PhysicalDevice(const std::shared_ptr<Instance>& instance,
                               std::shared_ptr<Surface> newSurface,
                               std::shared_ptr<IDebugManager> newDebugManager)
    : surface{std::move(newSurface)}, debugManager{std::move(newDebugManager)} {
  const auto physicalDevices = instance->getPhysicalDevices();

  if (physicalDevices.empty()) {
    throw std::runtime_error("Failed to find any GPUs with Vulkan Support");
  }

  for (const auto& possibleDevice : physicalDevices) {
    if (isDeviceSuitable(possibleDevice, surface->getVkSurface(), desiredDeviceExtensions)) {
      physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(possibleDevice);
      Log.info("Using physical device: {}", physicalDevice->getProperties().deviceName.data());
      break;
    }
  }
  if (physicalDevice == nullptr) {
    Log.error("Failed to select a suitable physical device");
  }

  auto deviceProperties =
      physicalDevice->getProperties2KHR<vk::PhysicalDeviceProperties2KHR,
                                        vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();

  descriptorBufferProperties =
      deviceProperties.get<vk::PhysicalDeviceDescriptorBufferPropertiesEXT>();
}

PhysicalDevice::~PhysicalDevice() {
  Log.trace("Destroying PhysicalDevice");
}

auto PhysicalDevice::getDescriptorBufferProperties() const
    -> vk::PhysicalDeviceDescriptorBufferPropertiesEXT {
  return descriptorBufferProperties;
}

auto PhysicalDevice::createDevice() -> std::unique_ptr<vk::raii::Device> {
  deviceQueueFamilyIndices = findQueueFamilies(*physicalDevice, surface->getVkSurface());
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

  // Graphics Queue(s)
  if (deviceQueueFamilyIndices.graphicsFamily.has_value() &&
      deviceQueueFamilyIndices.graphicsFamilyCount.has_value()) {
    const auto graphicsFamilyCreateInfo = vk::DeviceQueueCreateInfo{
        .queueFamilyIndex = deviceQueueFamilyIndices.graphicsFamily.value(),
        .queueCount = deviceQueueFamilyIndices.graphicsFamilyCount.value(),
        .pQueuePriorities = deviceQueueFamilyIndices.graphicsFamilyPriorities.data()};
    queueCreateInfos.push_back(graphicsFamilyCreateInfo);
  }

  // If present queue family is different from graphics
  if (deviceQueueFamilyIndices.graphicsFamily.value() !=
      deviceQueueFamilyIndices.presentFamily.value()) {
    Log.trace("Device supports separate present queue");
    // Present Queue(s)
    if (deviceQueueFamilyIndices.presentFamily.has_value() &&
        deviceQueueFamilyIndices.presentFamilyCount.has_value()) {
      const auto presentFamilyCreateInfo = vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = deviceQueueFamilyIndices.presentFamily.value(),
          .queueCount = deviceQueueFamilyIndices.presentFamilyCount.value(),
          .pQueuePriorities = deviceQueueFamilyIndices.presentFamilyPriorities.data()};
      queueCreateInfos.push_back(presentFamilyCreateInfo);
    }
  }

  // If Transfer queue family is different from graphics
  if (deviceQueueFamilyIndices.graphicsFamily.value() !=
      deviceQueueFamilyIndices.transferFamily.value()) {
    Log.trace("Creating Device with transfer queue family: {}",
              deviceQueueFamilyIndices.transferFamily.value());
    // Transfer Queue(s)
    if (deviceQueueFamilyIndices.transferFamily.has_value() &&
        deviceQueueFamilyIndices.transferFamilyCount.has_value()) {

      const auto transferFamilyCreateInfo = vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = deviceQueueFamilyIndices.transferFamily.value(),
          .queueCount = deviceQueueFamilyIndices.transferFamilyCount.value(),
          .pQueuePriorities = deviceQueueFamilyIndices.transferFamilyPriorities.data()};

      queueCreateInfos.push_back(transferFamilyCreateInfo);
    }
  }

  auto dynamicRenderingFeatures = vk::PhysicalDeviceDynamicRenderingFeaturesKHR{
      .dynamicRendering = VK_TRUE,
  };

  auto features2 = physicalDevice->getFeatures2<vk::PhysicalDeviceFeatures2,
                                                vk::PhysicalDevice16BitStorageFeatures,
                                                vk::PhysicalDeviceDescriptorIndexingFeaturesEXT>();

  auto indexingFeatures = features2.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();

  auto drawParamsFeatures =
      vk::PhysicalDeviceShaderDrawParametersFeatures{.shaderDrawParameters = VK_TRUE};

  if (const auto bindlessTexturesSupported =
          (indexingFeatures.descriptorBindingPartiallyBound != 0u) &&
          (indexingFeatures.runtimeDescriptorArray != 0u);
      !bindlessTexturesSupported) {
    throw std::runtime_error("GPU does not support bindless textures :(");
  }

  auto vk12Features =
      physicalDevice
          ->getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features>();

  auto physicalFeatures2 = vk12Features.get<vk::PhysicalDeviceFeatures2>();
  physicalFeatures2.features.samplerAnisotropy = VK_TRUE;

  auto physicalVulkan12Features = vk12Features.get<vk::PhysicalDeviceVulkan12Features>();
  physicalVulkan12Features.drawIndirectCount = VK_TRUE;
  physicalVulkan12Features.bufferDeviceAddress = VK_TRUE;

  vk::DeviceCreateInfo createInfo{
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledLayerCount = 0,
      .enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size()),
      .ppEnabledExtensionNames = desiredDeviceExtensions.data()};

  debugManager->addDeviceConfig(createInfo);

  // const auto dbFeatures =
  //     vk::PhysicalDeviceDescriptorBufferFeaturesEXT{.descriptorBuffer = VK_TRUE};

  constexpr auto extendedDynamicStateFeatures =
      vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{.extendedDynamicState = VK_TRUE};

  const vk::StructureChain c{createInfo,
                             physicalFeatures2,
                             physicalVulkan12Features,
                             drawParamsFeatures,
                             dynamicRenderingFeatures,
                             extendedDynamicStateFeatures,
                             /*dbFeatures*/};

  return std::make_unique<vk::raii::Device>(physicalDevice->createDevice(c.get(), nullptr));
}
auto PhysicalDevice::getVkPhysicalDevice() const -> vk::raii::PhysicalDevice& {
  return *physicalDevice;
}

auto PhysicalDevice::getQueueFamilyIndices() const -> QueueFamilyIndices {
  return deviceQueueFamilyIndices;
}

auto PhysicalDevice::querySwapchainSupport() const -> SwapchainSupportDetails {
  const auto details = SwapchainSupportDetails{
      .capabilities = physicalDevice->getSurfaceCapabilitiesKHR(*surface->getVkSurface()),
      .formats = physicalDevice->getSurfaceFormatsKHR(*surface->getVkSurface()),
      .presentModes = physicalDevice->getSurfacePresentModesKHR(*surface->getVkSurface())};

  return details;
}

auto PhysicalDevice::getSurfaceSize() const -> std::pair<uint32_t, uint32_t> {
  const auto surfaceCaps = physicalDevice->getSurfaceCapabilitiesKHR(*surface->getVkSurface());
  return std::make_pair(surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height);
}

auto isDeviceSuitable(const vk::raii::PhysicalDevice& possibleDevice,
                      const vk::raii::SurfaceKHR& surface,
                      const std::vector<char const*>& desiredDeviceExtensions) -> bool {

  const QueueFamilyIndices queueFamilyIndices = findQueueFamilies(possibleDevice, surface);

  const bool extensionsSupported =
      checkDeviceExtensionSupport(possibleDevice, desiredDeviceExtensions);

  bool swapchainAdequate = false;
  if (extensionsSupported) {
    auto [capabilities, formats, presentModes] = querySwapchainSupport(possibleDevice, surface);
    swapchainAdequate = !formats.empty() && !presentModes.empty();
  }

  const auto features = possibleDevice.getFeatures();
  const auto props = possibleDevice.getProperties();
  const auto isDiscrete = props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;

  return queueFamilyIndices.isComplete() && extensionsSupported && swapchainAdequate &&
         isDiscrete && (features.samplerAnisotropy != 0u) && (features.tessellationShader != 0u);
}

auto findQueueFamilies(const vk::raii::PhysicalDevice& possibleDevice,
                       const vk::raii::SurfaceKHR& surface) -> QueueFamilyIndices {
  QueueFamilyIndices queueFamilyIndices;

  const auto queueFamilies = possibleDevice.getQueueFamilyProperties();

  std::optional<uint32_t> graphicsFamilyIndex; // Store for fallback
  std::optional<uint32_t> computeFamilyIndex;
  std::optional<uint32_t> transferFamilyIndex;

  for (uint32_t i = 0; i < queueFamilies.size(); i++) {
    const auto& queueFamily = queueFamilies[i];

    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      if (!queueFamilyIndices.graphicsFamily.has_value()) {
        queueFamilyIndices.graphicsFamily = i;
        queueFamilyIndices.graphicsFamilyCount = queueFamily.queueCount;
        queueFamilyIndices.graphicsFamilyPriorities.assign(queueFamily.queueCount, 0.f);
        queueFamilyIndices.graphicsFamilyPriorities[0] = 1.f;
        graphicsFamilyIndex = i; // Store for fallback
      }
    }

    if (possibleDevice.getSurfaceSupportKHR(i, *surface) != 0u) {
      if (!queueFamilyIndices.presentFamily.has_value()) {
        queueFamilyIndices.presentFamily = i;
        queueFamilyIndices.presentFamilyCount = queueFamily.queueCount;
        queueFamilyIndices.presentFamilyPriorities.assign(queueFamily.queueCount, 0.f);
        queueFamilyIndices.presentFamilyPriorities[0] = 1.f;
      }
    }

    if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
        !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
      if (!queueFamilyIndices.computeFamily.has_value()) {
        queueFamilyIndices.computeFamily = i;
        queueFamilyIndices.computeFamilyCount = queueFamily.queueCount;
        queueFamilyIndices.computeFamilyPriorities.assign(queueFamily.queueCount, 0.f);
        queueFamilyIndices.computeFamilyPriorities[0] = 1.f;
        computeFamilyIndex = i;
      }
    }

    if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) &&
        !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) &&
        !(queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
      if (!queueFamilyIndices.transferFamily.has_value()) {
        queueFamilyIndices.transferFamily = i;
        queueFamilyIndices.transferFamilyCount = queueFamily.queueCount;
        queueFamilyIndices.transferFamilyPriorities.assign(queueFamily.queueCount, 0.f);
        queueFamilyIndices.transferFamilyPriorities[0] = 1.f;
        transferFamilyIndex = i;
      }
    }
  }

  // **Fallbacks if needed**
  if (!queueFamilyIndices.computeFamily.has_value() && graphicsFamilyIndex.has_value()) {
    queueFamilyIndices.computeFamily = graphicsFamilyIndex;
    queueFamilyIndices.computeFamilyCount = queueFamilyIndices.graphicsFamilyCount;
    queueFamilyIndices.computeFamilyPriorities = queueFamilyIndices.graphicsFamilyPriorities;
  }

  if (!queueFamilyIndices.transferFamily.has_value() && graphicsFamilyIndex.has_value()) {
    queueFamilyIndices.transferFamily = graphicsFamilyIndex;
    queueFamilyIndices.transferFamilyCount = queueFamilyIndices.graphicsFamilyCount;
    queueFamilyIndices.transferFamilyPriorities = queueFamilyIndices.graphicsFamilyPriorities;
  }

  return queueFamilyIndices;
}

auto checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& possibleDevice,
                                 const std::vector<char const*>& desiredDeviceExtensions) -> bool {
  const auto availableExtensions = possibleDevice.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(desiredDeviceExtensions.begin(),
                                           desiredDeviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}
auto querySwapchainSupport(const vk::raii::PhysicalDevice& possibleDevice,
                           const vk::raii::SurfaceKHR& surface) -> SwapchainSupportDetails {
  SwapchainSupportDetails details;
  details.capabilities = possibleDevice.getSurfaceCapabilitiesKHR(*surface);
  details.formats = possibleDevice.getSurfaceFormatsKHR(*surface);
  details.presentModes = possibleDevice.getSurfacePresentModesKHR(*surface);
  return details;
}

}
