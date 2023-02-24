#pragma once

class Instance {
 public:
   explicit Instance(bool validationEnabled = false);
   ~Instance();

 private:
   bool validationEnabled;
   std::vector<const char*> desiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
   std::unique_ptr<vk::raii::Context> context;
   std::unique_ptr<vk::raii::Instance> instance;
   std::unique_ptr<vk::raii::SurfaceKHR> surface;
   std::unique_ptr<vk::DebugUtilsMessengerEXT> messenger;
   std::unique_ptr<vk::DebugReportCallbackEXT> reportCallback;

   bool checkValidationLayerSupport() const;

   std::pair<std::vector<const char*>, bool> getRequiredExtensions() const;

   vk::DebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerCreateInfoExt();
};
