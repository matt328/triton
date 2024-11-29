#include "Instance.hpp"

namespace tr::gfx {
   Instance::Instance(std::shared_ptr<Context> newContext,
                      std::shared_ptr<IDebugManager> newDebugManager)
       : context{std::move(newContext)}, debugManager{std::move(newDebugManager)} {

      auto extensions = getInstanceExtensions();
      const auto debugExtensions = debugManager->getAdditionalInstanceExtensions();
      extensions.insert(extensions.end(), debugExtensions.begin(), debugExtensions.end());

      constexpr vk::ApplicationInfo appInfo{.pApplicationName = "Triton",
                                            .applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
                                            .pEngineName = "Triton Engine",
                                            .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
                                            .apiVersion = VK_API_VERSION_1_3};

      auto instanceCreateInfo = vk::InstanceCreateInfo{
          .pApplicationInfo = &appInfo,
          .enabledLayerCount = 0,
          .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
          .ppEnabledExtensionNames = extensions.data(),
      };

      if (isPortabilityRequired(extensions)) {
         instanceCreateInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
      }

      debugManager->addDebugCreateInfo(instanceCreateInfo);

      instance = context->createInstance(instanceCreateInfo);

      debugManager->initializeInstance(*instance);
   }

   Instance::~Instance() {
      Log.trace("Destroying Instance");
   }

   auto Instance::createSurface(IWindow& window) const -> std::unique_ptr<vk::raii::SurfaceKHR> {
      VkSurfaceKHR tempSurface = nullptr;

      auto* glfwWindow = static_cast<GLFWwindow*>(window.getNativeWindow());
      glfwCreateWindowSurface(**instance, glfwWindow, nullptr, &tempSurface);

      Log.trace("Created Surface");
      return std::make_unique<vk::raii::SurfaceKHR>(*instance, tempSurface);
   }

   auto Instance::getPhysicalDevices() const -> std::vector<vk::raii::PhysicalDevice> {
      return instance->enumeratePhysicalDevices();
   }

   auto Instance::getVkInstance() -> vk::Instance {
      return **instance;
   }

   auto Instance::getInstanceExtensions() -> std::vector<char const*> {
      uint32_t glfwExtensionCount = 0;
      auto* const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

      // NOLINTNEXTLINE This is ok because glfw's C api sucks.
      std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

      extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

      const auto extensionProperties = context->getExtensionProperties();

      std::vector<const char*> extNames = {};

      extNames.reserve(extensionProperties.size());
      for (const auto& ext : extensionProperties) {
         extNames.push_back(ext.extensionName);
      }

      extNames.emplace_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
      if (isPortabilityRequired(extNames)) {
         extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      }

      return extensions;
   }

   auto Instance::isPortabilityRequired([[maybe_unused]] std::span<const char*> extensionNames)
       -> bool {
      auto portabilityPresent = false;

#ifdef __APPLE__
      portabilityPresent = std::ranges::find_if(extensionNames, [](const std::string& name) {
                              return name == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
                           }) != extensionNames.end();
#endif
      return portabilityPresent;
   }
}