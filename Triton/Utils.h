#pragma once

class VulkanInstance final {
public:
   VulkanInstance();
  ~VulkanInstance();
 private:
   std::shared_ptr<vk::raii::Instance> instance;
   std::shared_ptr<vk::raii::SurfaceKHR> surface;
   std::shared_ptr<vk::DebugUtilsMessengerEXT> messenger;
   std::shared_ptr<vk::DebugReportCallbackEXT> reportCallback;
};